#pragma once

// Generic serialization for checkpoints.
//
// Structs are written as tagged fields (name hash, length, payload) and every struct carries its own length, so a
// reader skips fields it doesn't know and leaves fields that are missing from the data untouched.  Which fields a
// struct has comes from field lists generated from its definition, which is marked "// @checkpoint fields" (see
// tools/fieldscan); how a field is written is decided by its type alone.
//
// This header is what engine files see: it handles scalars, key types (checkpoint_key / from_checkpoint_key), arrays,
// the SCP containers, strings, pair, tuple, unique_ptr, optional, and raw pointers to containers or strings, which are
// owned by the struct that holds them and saved by value.  Every other type (scanned structs, enums, flag sets, raw
// types, pool pointers and the special cases in checkpoint/rules.h) goes through save_external / load_external, which
// are compiled once, in checkpoint/checkpoint_fields.cpp, with the generated field lists and the rules (see
// checkpoint/codec_impl.h).  A type that has no rule there shows up as an undefined reference to
// checkpoint::save_external<T> when linking; inside a scanned struct, it is a compile error naming the type.

#include "globalincs/pstypes.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace checkpoint {

using Buffer = SCP_vector<ubyte>;

constexpr uint32_t name_hash(const char* name)
{
	uint32_t hash = 2166136261u;
	while (*name != '\0') {
		hash ^= static_cast<ubyte>(*name++);
		hash *= 16777619u;
	}
	return hash;
}

/**
 * @brief Collects things that can only be resolved once everything has been loaded, such as pointers into
 * structures that are rebuilt during a restore.
 */
class LoadContext {
  public:
	void defer(std::function<void()> fixup) { m_fixups.push_back(std::move(fixup)); }

	void run_fixups()
	{
		for (auto& fixup : m_fixups)
			fixup();
		m_fixups.clear();
	}

	// fields present in the data that the current build doesn't know
	int unknown_fields = 0;
	// fields whose saved size doesn't fit their current type; they keep their current value
	int mismatched_fields = 0;
	// enumerator names in the data that the current build doesn't know
	int unknown_enumerators = 0;
	// members of the current build that the data doesn't have; they keep their current value
	SCP_set<SCP_string> missing_fields;

	// For each restored ship (by object number), where each saved subsystem ended up in its current subsystem list, or
	// -1.  Subsystems are matched by name, so they survive changes to the order of a ship class's subsystems.
	SCP_unordered_map<int, SCP_vector<int>> subsystem_positions;

  private:
	SCP_vector<std::function<void()>> m_fixups;
};

class Writer {
  public:
	explicit Writer(Buffer& buffer) : m_buffer(buffer) {}

	size_t size() const { return m_buffer.size(); }

	void bytes(const void* data, size_t size)
	{
		auto bytes = static_cast<const ubyte*>(data);
		m_buffer.insert(m_buffer.end(), bytes, bytes + size);
	}

	template <typename T>
	void value(const T& v)
	{
		static_assert(std::is_trivially_copyable_v<T>, "Only trivially copyable values can be written as bytes");
		bytes(&v, sizeof(T));
	}

	void u32(uint32_t v) { value(v); }

	// Reserves room for the length of a block; end_block() fills it in.
	size_t begin_block()
	{
		auto at = m_buffer.size();
		u32(0);
		return at;
	}

	void end_block(size_t at)
	{
		auto length = static_cast<uint32_t>(m_buffer.size() - at - sizeof(uint32_t));
		memcpy(&m_buffer[at], &length, sizeof(length));
	}

  private:
	Buffer& m_buffer;
};

class Reader {
  public:
	Reader(const ubyte* begin, const ubyte* end, LoadContext* context = nullptr)
		: m_pos(begin), m_end(end), m_context(context)
	{
	}

	explicit Reader(const Buffer& buffer, LoadContext* context = nullptr)
		: Reader(buffer.data(), buffer.data() + buffer.size(), context)
	{
	}

	bool bytes(void* data, size_t size)
	{
		if (m_failed || static_cast<size_t>(m_end - m_pos) < size) {
			m_failed = true;
			return false;
		}
		memcpy(data, m_pos, size);
		m_pos += size;
		return true;
	}

	template <typename T>
	bool value(T& v)
	{
		static_assert(std::is_trivially_copyable_v<T>, "Only trivially copyable values can be read as bytes");
		return bytes(&v, sizeof(T));
	}

	uint32_t u32()
	{
		uint32_t v = 0;
		value(v);
		return v;
	}

	// Reads a length-prefixed block and returns a reader limited to it.
	Reader block()
	{
		auto length = u32();
		if (m_failed || static_cast<size_t>(m_end - m_pos) < length) {
			m_failed = true;
			return {m_end, m_end, m_context};
		}
		Reader sub(m_pos, m_pos + length, m_context);
		m_pos += length;
		return sub;
	}

	bool at_end() const { return m_pos >= m_end; }
	ptrdiff_t remaining() const { return m_end - m_pos; }
	const ubyte* position() const { return m_pos; }
	bool failed() const { return m_failed; }
	void fail() { m_failed = true; }
	LoadContext* context() const { return m_context; }

  private:
	const ubyte* m_pos;
	const ubyte* m_end;
	LoadContext* m_context;
	bool m_failed = false;
};

// ---------------------------------------------------------------------------------------------------------------------
// Types handled by checkpoint/checkpoint_fields.cpp

// Saves and loads a type that needs the generated field lists or the rules; defined in checkpoint/codec_impl.h and
// instantiated in checkpoint/checkpoint_fields.cpp for every scanned struct and enum and every type with a rule.
template <typename T>
void save_external(Writer& w, const T& v);
template <typename T>
void load_external(Reader& r, T& v);
// Creates an element of a type without a default constructor when loading grows a container (see Factory).
template <typename T>
T make_external();

// The part of the schema fingerprint that covers the layout of the scanned structs.
uint32_t scanned_struct_fingerprint();

// ---------------------------------------------------------------------------------------------------------------------
// Codecs

/**
 * @brief The codec for one type.  The primary template handles scalars and key types and hands everything else to
 * save_external / load_external; specializations handle containers.
 */
template <typename T, typename Enable = void>
struct Codec;

template <typename T>
struct dependent_false : std::false_type {};

// Types identified by a stable key save the key instead of their value (see globalincs/session_index.h).
template <typename T, typename = void>
struct has_checkpoint_key : std::false_type {};
template <typename T>
struct has_checkpoint_key<T, std::void_t<decltype(T::from_checkpoint_key(std::declval<const T&>().checkpoint_key()))>>
	: std::true_type {};

// Containers and strings.  Nothing points at a container it doesn't own, so a raw pointer to one is saved by value.
template <typename T>
struct is_container : std::false_type {};
template <typename T>
struct is_container<SCP_vector<T>> : std::true_type {};
template <typename T, typename A>
struct is_container<std::vector<T, A>> : std::true_type {};
template <typename T>
struct is_container<SCP_list<T>> : std::true_type {};
template <typename T, typename A>
struct is_container<std::deque<T, A>> : std::true_type {};
template <typename T, typename L>
struct is_container<SCP_set<T, L>> : std::true_type {};
template <typename K, typename V, typename L>
struct is_container<SCP_map<K, V, L>> : std::true_type {};
template <typename K, typename V, typename H, typename E>
struct is_container<SCP_unordered_map<K, V, H, E>> : std::true_type {};
template <typename C, typename Tr, typename A>
struct is_container<std::basic_string<C, Tr, A>> : std::true_type {};

template <typename T>
struct is_owned_pointer : std::false_type {};
template <typename T>
struct is_owned_pointer<T*> : is_container<std::remove_cv_t<T>> {};

template <typename T>
void save(Writer& w, const T& v);
template <typename T>
void load(Reader& r, T& v);

// Raw bytes, for scalars and trivially copyable types without pointers or handles
template <typename T>
struct RawCodec {
	static_assert(std::is_trivially_copyable_v<T>, "checkpoint: only trivially copyable types can be saved as raw bytes");
	static void save(Writer& w, const T& v) { w.value(v); }
	static void load(Reader& r, T& v) { r.value(v); }
};

// The stable key of a key type
template <typename T>
struct KeyCodec {
	static void save(Writer& w, const T& v) { checkpoint::save(w, v.checkpoint_key()); }
	static void load(Reader& r, T& v)
	{
		std::decay_t<decltype(v.checkpoint_key())> key{};
		checkpoint::load(r, key);
		if (!r.failed())
			v = T::from_checkpoint_key(key);
	}
};

// An owned container behind a raw pointer, saved by value; a null pointer is saved as absent
template <typename T>
struct OwnedPointerCodec {
	static void save(Writer& w, const T& v)
	{
		w.value(static_cast<ubyte>(v != nullptr));
		if (v != nullptr)
			checkpoint::save(w, *v);
	}
	static void load(Reader& r, T& v)
	{
		ubyte present = 0;
		r.value(present);
		if (present == 0) {
			delete v;
			v = nullptr;
			return;
		}
		if (v == nullptr)
			v = new std::remove_pointer_t<T>();
		checkpoint::load(r, *v);
	}
};

// Everything else, compiled in checkpoint/checkpoint_fields.cpp
template <typename T>
struct ExternalCodec {
	static void save(Writer& w, const T& v) { save_external<T>(w, v); }
	static void load(Reader& r, T& v) { load_external<T>(r, v); }
};

template <typename T, typename Enable>
struct Codec
	: std::conditional_t<has_checkpoint_key<T>::value, KeyCodec<T>,
		  std::conditional_t<std::is_arithmetic_v<T>, RawCodec<T>,
			  std::conditional_t<is_owned_pointer<T>::value, OwnedPointerCodec<T>, ExternalCodec<T>>>> {};

template <typename T>
void save(Writer& w, const T& v)
{
	Codec<T>::save(w, v);
}

template <typename T>
void load(Reader& r, T& v)
{
	Codec<T>::load(r, v);
}

// Saves a trivially copyable type that is defined in a source file as raw bytes.  Only that file can save the type, so
// its rule lives there instead of in checkpoint/rules.h.  Use at global scope, after the type's definition.
#define CHECKPOINT_LOCAL_RAW(T)                                                                                        \
	template <>                                                                                                        \
	struct checkpoint::Codec<T> : ::checkpoint::RawCodec<T> {}

// An element for a container that loading grows
template <typename T>
T make_element()
{
	if constexpr (std::is_default_constructible_v<T>)
		return T();
	else
		return make_external<T>();
}

// Sequences are written as a count followed by the elements.  Arrays with a fixed size load as many elements as fit.

template <typename T, size_t N>
struct Codec<T[N]> {
	static void save(Writer& w, const T (&v)[N])
	{
		w.u32(static_cast<uint32_t>(N));
		for (size_t i = 0; i < N; ++i)
			checkpoint::save(w, v[i]);
	}
	static void load(Reader& r, T (&v)[N])
	{
		auto count = r.u32();
		for (uint32_t i = 0; i < count && !r.failed(); ++i) {
			if (i < N) {
				checkpoint::load(r, v[i]);
			} else {
				// static, since fix-ups deferred while loading it may still write to it
				static T discard{};
				checkpoint::load(r, discard);
			}
		}
	}
};

template <typename T, size_t N>
struct Codec<std::array<T, N>> {
	static void save(Writer& w, const std::array<T, N>& v)
	{
		w.u32(static_cast<uint32_t>(N));
		for (const auto& element : v)
			checkpoint::save(w, element);
	}
	static void load(Reader& r, std::array<T, N>& v)
	{
		auto count = r.u32();
		for (uint32_t i = 0; i < count && !r.failed(); ++i) {
			if (i < N) {
				checkpoint::load(r, v[i]);
			} else {
				// static, since fix-ups deferred while loading it may still write to it
				static T discard{};
				checkpoint::load(r, discard);
			}
		}
	}
};

// Containers that can be refilled with push_back / emplace_back
template <typename C>
struct SequenceCodec {
	using T = typename C::value_type;
	static void save(Writer& w, const C& v)
	{
		w.u32(static_cast<uint32_t>(v.size()));
		for (const auto& element : v)
			checkpoint::save(w, element);
	}
	static void load(Reader& r, C& v)
	{
		auto count = r.u32();
		// every element takes at least one byte, which guards against counts from damaged data
		if (r.failed() || static_cast<size_t>(r.remaining()) < count) {
			r.fail();
			return;
		}

		// existing elements are loaded in place, so their skipped fields keep their values
		while (v.size() > count)
			v.pop_back();
		while (v.size() < count)
			v.push_back(make_element<T>());
		for (auto& element : v) {
			if (r.failed())
				break;
			checkpoint::load(r, element);
		}
	}
};

template <typename T>
struct Codec<SCP_vector<T>> : SequenceCodec<SCP_vector<T>> {};
template <typename T, typename A>
struct Codec<std::vector<T, A>> : SequenceCodec<std::vector<T, A>> {};
template <typename T>
struct Codec<SCP_list<T>> : SequenceCodec<SCP_list<T>> {};
template <typename T, typename A>
struct Codec<std::deque<T, A>> : SequenceCodec<std::deque<T, A>> {};

// Sequences of bool are written element by element, since vector<bool> has no references to its elements
template <typename C>
struct BoolSequenceCodec {
	static void save(Writer& w, const C& v)
	{
		w.u32(static_cast<uint32_t>(v.size()));
		for (bool b : v)
			w.value(static_cast<ubyte>(b));
	}
	static void load(Reader& r, C& v)
	{
		auto count = r.u32();
		v.clear();
		for (uint32_t i = 0; i < count && !r.failed(); ++i) {
			ubyte b = 0;
			r.value(b);
			v.push_back(b != 0);
		}
	}
};

template <>
struct Codec<SCP_vector<bool>> : BoolSequenceCodec<SCP_vector<bool>> {};
template <typename A>
struct Codec<std::vector<bool, A>> : BoolSequenceCodec<std::vector<bool, A>> {};
template <typename A>
struct Codec<std::deque<bool, A>> : BoolSequenceCodec<std::deque<bool, A>> {};

// Associative containers: elements are written in iteration order and inserted again on load
template <typename C>
struct AssociativeCodec {
	using K = std::remove_cv_t<typename C::key_type>;
	static void save(Writer& w, const C& v)
	{
		w.u32(static_cast<uint32_t>(v.size()));
		for (const auto& element : v)
			save_element(w, element);
	}
	static void load(Reader& r, C& v)
	{
		auto count = r.u32();
		v.clear();
		for (uint32_t i = 0; i < count && !r.failed(); ++i)
			load_element(r, v);
	}

  private:
	template <typename E>
	static void save_element(Writer& w, const E& element)
	{
		if constexpr (std::is_same_v<E, K>) {
			checkpoint::save(w, element);
		} else {
			checkpoint::save(w, element.first);
			checkpoint::save(w, element.second);
		}
	}
	static void load_element(Reader& r, C& v)
	{
		K key{};
		checkpoint::load(r, key);
		if constexpr (std::is_same_v<typename C::value_type, K>) {
			v.insert(std::move(key));
		} else {
			typename C::mapped_type mapped{};
			checkpoint::load(r, mapped);
			v.emplace(std::move(key), std::move(mapped));
		}
	}
};

template <typename T, typename L>
struct Codec<SCP_set<T, L>> : AssociativeCodec<SCP_set<T, L>> {};
template <typename K, typename V, typename L>
struct Codec<SCP_map<K, V, L>> : AssociativeCodec<SCP_map<K, V, L>> {};
template <typename K, typename V, typename H, typename E>
struct Codec<SCP_unordered_map<K, V, H, E>> : AssociativeCodec<SCP_unordered_map<K, V, H, E>> {};

template <typename C, typename Tr, typename A>
struct Codec<std::basic_string<C, Tr, A>> {
	static void save(Writer& w, const std::basic_string<C, Tr, A>& v)
	{
		w.u32(static_cast<uint32_t>(v.size()));
		w.bytes(v.data(), v.size() * sizeof(C));
	}
	static void load(Reader& r, std::basic_string<C, Tr, A>& v)
	{
		auto count = r.u32();
		// a length beyond the data means the data doesn't hold a string here
		if (r.failed() || static_cast<size_t>(r.remaining()) < count * sizeof(C)) {
			r.fail();
			return;
		}
		v.resize(count);
		if (count > 0)
			r.bytes(v.data(), count * sizeof(C));
	}
};

template <typename A, typename B>
struct Codec<std::pair<A, B>> {
	static void save(Writer& w, const std::pair<A, B>& v)
	{
		checkpoint::save(w, v.first);
		checkpoint::save(w, v.second);
	}
	static void load(Reader& r, std::pair<A, B>& v)
	{
		checkpoint::load(r, v.first);
		checkpoint::load(r, v.second);
	}
};

template <typename... Ts>
struct Codec<std::tuple<Ts...>> {
	static void save(Writer& w, const std::tuple<Ts...>& v)
	{
		std::apply([&w](const auto&... elements) { (checkpoint::save(w, elements), ...); }, v);
	}
	static void load(Reader& r, std::tuple<Ts...>& v)
	{
		std::apply([&r](auto&... elements) { (checkpoint::load(r, elements), ...); }, v);
	}
};

// Owned objects are saved by value; a null pointer is saved as absent.
template <typename T, typename D>
struct Codec<std::unique_ptr<T, D>> {
	static void save(Writer& w, const std::unique_ptr<T, D>& v)
	{
		w.value(static_cast<ubyte>(v != nullptr));
		if (v != nullptr)
			checkpoint::save(w, *v);
	}
	static void load(Reader& r, std::unique_ptr<T, D>& v)
	{
		ubyte present = 0;
		r.value(present);
		if (present == 0) {
			v.reset();
			return;
		}
		if (v == nullptr)
			v.reset(new T());
		checkpoint::load(r, *v);
	}
};

template <typename T>
struct Codec<std::optional<T>> {
	static void save(Writer& w, const std::optional<T>& v)
	{
		w.value(static_cast<ubyte>(v.has_value()));
		if (v.has_value())
			checkpoint::save(w, *v);
	}
	static void load(Reader& r, std::optional<T>& v)
	{
		ubyte present = 0;
		r.value(present);
		if (present == 0) {
			v.reset();
			return;
		}
		if (!v.has_value())
			v.emplace();
		checkpoint::load(r, *v);
	}
};

} // namespace checkpoint

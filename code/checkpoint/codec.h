#pragma once

// Generic serialization for checkpoints.
//
// Structs are written as tagged fields (name hash, length, payload) and every struct carries its own length, so a
// reader skips fields it doesn't know and leaves fields that are missing from the data untouched.  Which fields a
// struct has comes from generated field lists (see tools/fieldscan); how a field is written is decided by its type
// alone.  Supported out of the box: scalars, enums, scanned structs, arrays, the SCP containers, strings, pair, tuple,
// unique_ptr and optional.  Everything else needs one of the extension points below, which are filled in by
// checkpoint/rules.h; an unsupported type is a compile error.

#include "globalincs/pstypes.h"
#include "globalincs/flagset.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <deque>
#include <functional>
#include <memory>
#include <new>
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

/**
 * @brief Optionally records where each saved field ended up, by path (e.g. "objects[12].ship.weapons.next_primary_fire_stamp"),
 * so that two saves can be compared field by field.
 */
struct Trace {
	struct Entry {
		SCP_string path;
		size_t begin;
		size_t end;
	};

	SCP_vector<Entry> entries;
	SCP_vector<SCP_string> path;

	SCP_string current_path() const
	{
		SCP_string result;
		for (const auto& part : path) {
			if (!result.empty() && part[0] != '[')
				result += '.';
			result += part;
		}
		return result;
	}
};

class Writer {
  public:
	explicit Writer(Buffer& buffer, Trace* trace = nullptr) : m_buffer(buffer), m_trace(trace) {}

	size_t size() const { return m_buffer.size(); }
	Trace* trace() const { return m_trace; }

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
	Trace* m_trace;
};

// Adds a part to the trace path for the lifetime of the scope.
class TraceScope {
  public:
	TraceScope(Writer& w, SCP_string part) : m_trace(w.trace())
	{
		if (m_trace != nullptr)
			m_trace->path.push_back(std::move(part));
	}
	~TraceScope()
	{
		if (m_trace != nullptr)
			m_trace->path.pop_back();
	}
	TraceScope(const TraceScope&) = delete;
	TraceScope& operator=(const TraceScope&) = delete;

  private:
	Trace* m_trace;
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
// Extension points

/**
 * @brief The fields of a struct.  Specialized by the generated field lists via the macros below; each specialization
 * calls v.template field<&S::member>("member") for every member.
 */
template <typename T>
struct Fields {
	static constexpr bool scanned = false;
};

#define CHECKPOINT_FIELDS_BEGIN(S)                                                                                     \
	template <>                                                                                                        \
	struct checkpoint::Fields<S> {                                                                                     \
		static constexpr bool scanned = true;                                                                          \
		template <typename V>                                                                                          \
		static void each(V& v)                                                                                         \
		{
#define CHECKPOINT_FIELD(S, member) v.template field<&S::member>(#member);
#define CHECKPOINT_FIELDS_END(S)                                                                                       \
		}                                                                                                              \
	};

/**
 * @brief The enumerators of an enum, by name.  Specialized by the generated lists for the enums in the struct list;
 * enums and flagsets are saved by enumerator name, so reordering or inserting enumerators keeps saved data valid.
 */
template <typename E>
struct EnumNames {
	static constexpr bool listed = false;
};

#define CHECKPOINT_ENUM_BEGIN(E)                                                                                       \
	template <>                                                                                                        \
	struct checkpoint::EnumNames<E> {                                                                                  \
		static constexpr bool listed = true;                                                                           \
		static const SCP_vector<std::pair<const char*, E>>& values()                                                   \
		{                                                                                                              \
			static const SCP_vector<std::pair<const char*, E>> list = {
#define CHECKPOINT_ENUM_VALUE(E, enumerator) {#enumerator, E::enumerator},
#define CHECKPOINT_ENUM_END(E)                                                                                         \
			};                                                                                                         \
			return list;                                                                                               \
		}                                                                                                              \
	};

// A trivially copyable type that may be written as raw bytes (it holds no pointers, handles or padding-sensitive data).
template <typename T>
struct is_raw : std::false_type {};

// A type that is never saved: fields of this type keep whatever value they have when loading.
template <typename T>
struct is_transient : std::false_type {};

// A single field that is never saved, e.g. &ship::model_instance_num.
template <auto Member>
struct is_skipped : std::false_type {};

/**
 * @brief How to save a pointer into a pool of objects: as an index.  Specializations provide
 *   static int index_of(const T* p);
 *   static T* at(int index);
 * A null pointer is saved as -1; other negative indexes are available for sentinel objects outside the pool.
 */
template <typename T>
struct PoolTraits;

/**
 * @brief Creates elements when loading grows a container.  Types without a default constructor need a
 * specialization providing static T make(); raw types are created from zero bytes and then overwritten.
 */
template <typename T, typename Enable = void>
struct Factory {
	static T make()
	{
		if constexpr (std::is_default_constructible_v<T>) {
			return T();
		} else {
			static_assert(is_raw<T>::value, "checkpoint: this type has no default constructor; give it a Factory specialization in checkpoint/rules.h");
			alignas(T) ubyte storage[sizeof(T)] = {};
			return *std::launder(reinterpret_cast<T*>(storage));
		}
	}
};

// Additional data written after the fields of a struct, for members that need context from other members.
template <typename T>
struct Extra {
	static constexpr bool present = false;
};

/**
 * @brief The codec for one type.  The primary template handles scalars, enums, raw and scanned types and pool
 * pointers; specializations handle containers and the special cases in checkpoint/rules.h.
 */
template <typename T, typename Enable = void>
struct Codec;

// ---------------------------------------------------------------------------------------------------------------------
// Implementation

template <typename T>
struct dependent_false : std::false_type {};

// Types identified by a stable key save the key instead of their value (see globalincs/session_index.h).
template <typename T, typename = void>
struct has_checkpoint_key : std::false_type {};
template <typename T>
struct has_checkpoint_key<T, std::void_t<decltype(T::from_checkpoint_key(std::declval<const T&>().checkpoint_key()))>>
	: std::true_type {};

template <typename T, typename = void>
struct has_pool_traits : std::false_type {};
template <typename T>
struct has_pool_traits<T, std::void_t<decltype(PoolTraits<T>::index_of(nullptr))>> : std::true_type {};

// A field whose type is transient, or an array or sequence of transient elements, is not saved.
template <typename T>
struct is_transient_field : is_transient<T> {};
template <typename T, size_t N>
struct is_transient_field<T[N]> : is_transient_field<T> {};
template <typename T, size_t N>
struct is_transient_field<std::array<T, N>> : is_transient_field<T> {};
template <typename T>
struct is_transient_field<SCP_vector<T>> : is_transient_field<T> {};
template <typename T>
struct is_transient_field<SCP_list<T>> : is_transient_field<T> {};

template <typename T>
void save(Writer& w, const T& v);
template <typename T>
void load(Reader& r, T& v);

template <typename T>
struct FieldSaver {
	Writer& w;
	const T& object;

	template <auto Member>
	void field(const char* name)
	{
		using F = std::remove_cv_t<std::remove_reference_t<decltype(std::declval<const T&>().*Member)>>;
		if constexpr (is_skipped<Member>::value || is_transient_field<F>::value) {
			return;
		} else {
			w.u32(name_hash(name));
			auto at = w.begin_block();
			if (w.trace() == nullptr) {
				save(w, object.*Member);
			} else {
				TraceScope scope(w, name);
				save(w, object.*Member);
				// structs record their own fields; everything else is recorded as one value
				if constexpr (!Fields<F>::scanned)
					w.trace()->entries.push_back({w.trace()->current_path(), at + sizeof(uint32_t), w.size()});
			}
			w.end_block(at);
		}
	}
};

struct TaggedField {
	uint32_t hash;
	Reader data;
};

// Values written as raw bytes have a fixed size; everything else is self-delimiting.
template <typename T>
constexpr bool is_fixed_size_v = !has_checkpoint_key<T>::value && !std::is_enum_v<T> && (std::is_arithmetic_v<T> || is_raw<T>::value);

// Arrays and vectors whose elements are written as raw bytes
template <typename T>
struct fixed_size_elements {
	static constexpr bool value = false;
	using type = void;
};
template <typename T, size_t N>
struct fixed_size_elements<T[N]> {
	static constexpr bool value = is_fixed_size_v<T>;
	using type = T;
};
template <typename T, size_t N>
struct fixed_size_elements<std::array<T, N>> {
	static constexpr bool value = is_fixed_size_v<T>;
	using type = T;
};
template <typename T>
struct fixed_size_elements<SCP_vector<T>> {
	static constexpr bool value = is_fixed_size_v<T>;
	using type = T;
};

/**
 * @brief Whether the saved data of a field fits the field's current type.  A raw field, or an array or vector of raw
 * elements, whose type changed size since the data was saved would be misread; such a field is left as it is.
 */
template <typename F>
bool payload_fits(Reader data)
{
	if constexpr (is_fixed_size_v<F>) {
		return data.remaining() == static_cast<ptrdiff_t>(sizeof(F));
	} else if constexpr (fixed_size_elements<F>::value) {
		auto count = data.u32();
		return !data.failed() && data.remaining() == static_cast<ptrdiff_t>(count * sizeof(typename fixed_size_elements<F>::type));
	} else {
		return true;
	}
}

template <typename T>
struct FieldLoader {
	SCP_vector<TaggedField>& fields;
	T& object;
	LoadContext* context;
	int found = 0;

	template <auto Member>
	void field(const char* name)
	{
		using F = std::remove_cv_t<std::remove_reference_t<decltype(std::declval<T&>().*Member)>>;
		if constexpr (is_skipped<Member>::value || is_transient_field<F>::value) {
			return;
		} else {
			auto hash = name_hash(name);
			for (auto& tagged : fields) {
				if (tagged.hash == hash) {
					if (!payload_fits<F>(tagged.data)) {
						if (tagged.data.context() != nullptr)
							++tagged.data.context()->mismatched_fields;
						++found;
						return;
					}
					load(tagged.data, object.*Member);
					++found;
					return;
				}
			}
			// not in the data: keep the current value
			if (!fields.empty() && context != nullptr)
				context->missing_fields.insert(name);
		}
	}
};

constexpr uint32_t EXTRA_HASH = name_hash("#extra");

template <typename T>
void save_struct(Writer& w, const T& v)
{
	auto at = w.begin_block();
	FieldSaver<T> saver{w, v};
	Fields<T>::each(saver);
	if constexpr (Extra<T>::present) {
		w.u32(EXTRA_HASH);
		auto extra = w.begin_block();
		if (w.trace() == nullptr) {
			Extra<T>::save(w, v);
		} else {
			// recorded as one value, so verification compares it like a field
			TraceScope scope(w, "#extra");
			Extra<T>::save(w, v);
			w.trace()->entries.push_back({w.trace()->current_path(), extra + sizeof(uint32_t), w.size()});
		}
		w.end_block(extra);
	}
	w.end_block(at);
}

template <typename T>
void load_struct(Reader& r, T& v)
{
	auto block = r.block();
	SCP_vector<TaggedField> fields;
	while (!block.at_end() && !block.failed()) {
		auto hash = block.u32();
		fields.push_back({hash, block.block()});
	}
	if (block.failed()) {
		r.fail();
		return;
	}

	FieldLoader<T> loader{fields, v, r.context()};
	Fields<T>::each(loader);

	int known = loader.found;
	if constexpr (Extra<T>::present) {
		for (auto& tagged : fields) {
			if (tagged.hash == EXTRA_HASH) {
				Extra<T>::load(tagged.data, v);
				++known;
			}
		}
	}

	if (r.context() != nullptr)
		r.context()->unknown_fields += static_cast<int>(fields.size()) - known;
}

template <typename E>
void save_enumerator(Writer& w, E v);
template <typename E>
void load_enumerator(Reader& r, E& v);

template <typename T, typename Enable>
struct Codec {
	static void save(Writer& w, const T& v)
	{
		if constexpr (has_checkpoint_key<T>::value) {
			checkpoint::save(w, v.checkpoint_key());
		} else if constexpr (std::is_enum_v<T>) {
			static_assert(EnumNames<T>::listed,
				"checkpoint: enums are saved by name. Add \"enum <name> <header>\" to checkpoint/checkpoint_structs.txt.");
			save_enumerator(w, v);
		} else if constexpr (std::is_arithmetic_v<T> || is_raw<T>::value) {
			w.value(v);
		} else if constexpr (std::is_pointer_v<T> && has_pool_traits<std::remove_cv_t<std::remove_pointer_t<T>>>::value) {
			using P = PoolTraits<std::remove_cv_t<std::remove_pointer_t<T>>>;
			int32_t index = (v == nullptr) ? -1 : static_cast<int32_t>(P::index_of(v));
			w.value(index);
		} else if constexpr (Fields<T>::scanned) {
			save_struct(w, v);
		} else {
			static_assert(dependent_false<T>::value,
				"checkpoint: no rule for this type. Add it to the struct list, or give it a rule in checkpoint/rules.h "
				"(is_raw, is_transient, PoolTraits or a Codec specialization), or skip the field with is_skipped.");
		}
	}

	static void load(Reader& r, T& v)
	{
		if constexpr (has_checkpoint_key<T>::value) {
			std::decay_t<decltype(v.checkpoint_key())> key{};
			checkpoint::load(r, key);
			if (!r.failed())
				v = T::from_checkpoint_key(key);
		} else if constexpr (std::is_enum_v<T>) {
			load_enumerator(r, v);
		} else if constexpr (std::is_arithmetic_v<T> || is_raw<T>::value) {
			r.value(v);
		} else if constexpr (std::is_pointer_v<T> && has_pool_traits<std::remove_cv_t<std::remove_pointer_t<T>>>::value) {
			using P = PoolTraits<std::remove_cv_t<std::remove_pointer_t<T>>>;
			int32_t index = -1;
			if (r.value(index))
				v = (index == -1) ? nullptr : P::at(index);
		} else if constexpr (Fields<T>::scanned) {
			load_struct(r, v);
		} else {
			static_assert(dependent_false<T>::value, "checkpoint: no rule for this type (see Codec::save)");
		}
	}
};

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

// An enumerator is saved by name; values without a name (combinations, casts) are saved as "#<number>".
template <typename E>
void save_enumerator(Writer& w, E v)
{
	for (const auto& entry : EnumNames<E>::values()) {
		if (entry.second == v) {
			checkpoint::save(w, SCP_string(entry.first));
			return;
		}
	}
	checkpoint::save(w, "#" + std::to_string(static_cast<long long>(v)));
}

template <typename E>
void load_enumerator(Reader& r, E& v)
{
	SCP_string name;
	checkpoint::load(r, name);
	if (r.failed())
		return;
	if (!name.empty() && name[0] == '#') {
		v = static_cast<E>(strtoll(name.c_str() + 1, nullptr, 10));
		return;
	}
	for (const auto& entry : EnumNames<E>::values()) {
		if (name == entry.first) {
			v = entry.second;
			return;
		}
	}
	if (r.context() != nullptr)
		++r.context()->unknown_enumerators;
}

// A flagset is saved as the names of its set flags.
template <typename E>
struct Codec<flagset<E>> {
	static_assert(EnumNames<E>::listed,
		"checkpoint: flags are saved by name. Add \"enum <name> <header>\" to checkpoint/checkpoint_structs.txt.");

	static void save(Writer& w, const flagset<E>& v)
	{
		SCP_vector<SCP_string> names;
		for (const auto& entry : EnumNames<E>::values()) {
			if (v[entry.second])
				names.emplace_back(entry.first);
		}
		checkpoint::save(w, names);
	}
	static void load(Reader& r, flagset<E>& v)
	{
		SCP_vector<SCP_string> names;
		checkpoint::load(r, names);
		if (r.failed())
			return;
		v.reset();
		for (const auto& name : names) {
			bool known = false;
			for (const auto& entry : EnumNames<E>::values()) {
				if (name == entry.first) {
					v.set(entry.second);
					known = true;
					break;
				}
			}
			if (!known && r.context() != nullptr)
				++r.context()->unknown_enumerators;
		}
	}
};

// Sequences are written as a count followed by the elements.  Arrays with a fixed size load as many elements as fit.

template <typename T, size_t N>
struct Codec<T[N]> {
	static void save(Writer& w, const T (&v)[N])
	{
		w.u32(static_cast<uint32_t>(N));
		for (size_t i = 0; i < N; ++i) {
			TraceScope scope(w, w.trace() != nullptr ? "[" + std::to_string(i) + "]" : SCP_string());
			checkpoint::save(w, v[i]);
		}
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
		size_t index = 0;
		for (const auto& element : v) {
			TraceScope scope(w, w.trace() != nullptr ? "[" + std::to_string(index++) + "]" : SCP_string());
			checkpoint::save(w, element);
		}
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
			v.push_back(Factory<T>::make());
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

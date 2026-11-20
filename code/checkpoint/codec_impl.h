#pragma once

// The part of the checkpoint codec that needs the generated field lists and the rules: scanned structs, enums, flag
// sets, raw types, pool pointers and custom rules.  Only checkpoint/checkpoint_fields.cpp (and tests) include this;
// engine files include checkpoint/codec.h, which hands these types to save_external / load_external.

#include "checkpoint/codec.h"

#include "globalincs/flagset.h"

#include <new>

namespace checkpoint {

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
			static_assert(is_raw<T>::value, "checkpoint: this type has no default constructor; give it a Factory specialization (CHECKPOINT_FACTORY) in checkpoint/rules.h");
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
 * @brief A custom codec for a type that needs more than its fields, e.g. a pointer into a heap pool or a resource
 * saved by name.  Specializations provide
 *   static void save(Writer& w, const T& v);
 *   static void load(Reader& r, T& v);
 */
template <typename T, typename Enable = void>
struct Rule {
	using none = void;
};

template <typename T, typename = void>
struct has_rule : std::true_type {};
template <typename T>
struct has_rule<T, typename Rule<T>::none> : std::false_type {};

template <typename T>
struct is_flagset : std::false_type {};
template <typename E>
struct is_flagset<flagset<E>> : std::true_type {};

// ---------------------------------------------------------------------------------------------------------------------
// Implementation

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
struct FieldSaver {
	Writer& w;
	const T& object;

	template <auto Member>
	void field(const char* name)
	{
		using F = std::remove_cv_t<std::remove_reference_t<decltype(std::declval<const T&>().*Member)>>;
		if constexpr (is_transient_field<F>::value) {
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
		if constexpr (is_transient_field<F>::value) {
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
template <typename T>
struct FlagsCodec;
template <typename E>
struct FlagsCodec<flagset<E>> {
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

template <typename T>
struct EnumCodec {
	static_assert(EnumNames<T>::listed,
		"checkpoint: enums are saved by name. Add \"enum <name> <header>\" to checkpoint/checkpoint_structs.txt.");
	static void save(Writer& w, const T& v) { save_enumerator(w, v); }
	static void load(Reader& r, T& v) { load_enumerator(r, v); }
};

template <typename T>
struct PoolCodec {
	using P = PoolTraits<std::remove_cv_t<std::remove_pointer_t<T>>>;
	static void save(Writer& w, const T& v)
	{
		int32_t index = (v == nullptr) ? -1 : static_cast<int32_t>(P::index_of(v));
		w.value(index);
	}
	static void load(Reader& r, T& v)
	{
		int32_t index = -1;
		if (r.value(index))
			v = (index == -1) ? nullptr : P::at(index);
	}
};

template <typename T>
struct StructCodec {
	static void save(Writer& w, const T& v) { save_struct(w, v); }
	static void load(Reader& r, T& v) { load_struct(r, v); }
};

template <typename T>
struct NoRule {
	static_assert(dependent_false<T>::value,
		"checkpoint: no rule for this type. Add it to the struct list, or give it a rule in checkpoint/rules.h "
		"(CHECKPOINT_RAW, is_transient, CHECKPOINT_POOL or CHECKPOINT_RULE), or skip the field with a "
		"\"// @checkpoint skip: <reason>\" comment at the end of its line.");
	static void save(Writer&, const T&) {}
	static void load(Reader&, T&) {}
};

template <typename T>
struct is_pool_pointer : std::false_type {};
template <typename T>
struct is_pool_pointer<T*> : has_pool_traits<std::remove_cv_t<T>> {};

// The codec of a type handled here; the first that applies wins.
template <typename T>
using RuleCodec =
	std::conditional_t<has_rule<T>::value, Rule<T>,
	std::conditional_t<is_flagset<T>::value, FlagsCodec<T>,
	std::conditional_t<std::is_enum_v<T>, EnumCodec<T>,
	std::conditional_t<is_raw<T>::value, RawCodec<T>,
	std::conditional_t<is_pool_pointer<T>::value, PoolCodec<T>,
	std::conditional_t<Fields<T>::scanned, StructCodec<T>, NoRule<T>>>>>>>;

template <typename T>
void save_external(Writer& w, const T& v)
{
	RuleCodec<T>::save(w, v);
}

template <typename T>
void load_external(Reader& r, T& v)
{
	RuleCodec<T>::load(r, v);
}

template <typename T>
T make_external()
{
	return Factory<T>::make();
}

// ---------------------------------------------------------------------------------------------------------------------
// Instantiation
//
// Engine files only see the declarations of save_external and load_external, so every type they save directly needs
// an explicit instantiation in checkpoint/checkpoint_fields.cpp.  Scanned structs and enums get one from the generated
// lists; the macros below give one to each type with a rule, together with the rule.  Types that are only reached
// through the fields of scanned structs are instantiated there implicitly.

template <typename T>
using same_t = T;

#define CHECKPOINT_INSTANTIATE(T)                                                                                      \
	template void save_external<T>(Writer&, const ::checkpoint::same_t<T>&);                                          \
	template void load_external<T>(Reader&, ::checkpoint::same_t<T>&)

// A trivially copyable type without pointers or handles, written as raw bytes.
#define CHECKPOINT_RAW(T)                                                                                              \
	template <>                                                                                                        \
	struct is_raw<T> : std::true_type {};                                                                              \
	CHECKPOINT_INSTANTIATE(T)

// Pointers into the pool of T, after its PoolTraits specialization.
#define CHECKPOINT_POOL(T) CHECKPOINT_INSTANTIATE(T*)

// A type with a Rule, after its Rule specialization.
#define CHECKPOINT_RULE(T) CHECKPOINT_INSTANTIATE(T)

// A type without a default constructor, after its Factory specialization.
#define CHECKPOINT_FACTORY(T) template T make_external<T>()

} // namespace checkpoint

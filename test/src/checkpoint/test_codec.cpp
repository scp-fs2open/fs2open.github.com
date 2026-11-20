#include <gtest/gtest.h>

#include "checkpoint/codec_impl.h"

namespace {

enum class Mode { First, Second };

struct Inner {
	int a = 0;
	float b = 0.0f;
};

struct Outer {
	int number = 0;
	Mode mode = Mode::First;
	char name[8] = {};
	Inner inner;
	Inner inners[3];
	SCP_vector<Inner> list;
	SCP_string text;
	SCP_map<SCP_string, int> lookup;
	SCP_unordered_map<int, SCP_vector<int>> nested;
	std::unique_ptr<Inner> owned;
	std::optional<int> maybe;
	SCP_vector<bool> flags;
	std::deque<bool> more_flags;
	std::pair<int, SCP_string> pair;
	std::tuple<int, float, SCP_string> tuple;
	Inner* pooled = nullptr;
	int skipped = 0;
	SCP_vector<int> transient_type;
};

// raw pointers to containers and strings are owned by the struct and saved by value
struct OwningPointers {
	SCP_vector<int>* list = nullptr;
	SCP_string* text = nullptr;

	OwningPointers() = default;
	OwningPointers(const OwningPointers&) = delete;
	OwningPointers& operator=(const OwningPointers&) = delete;
	~OwningPointers()
	{
		delete list;
		delete text;
	}
};

// two versions of the same struct, for checking that changed structs still load
struct Old {
	int kept = 0;
	int removed = 0;
	float renamed_type = 0.0f;
};

struct New {
	int added = 0;
	int kept = 0;
};

Inner Pool[4];

// two versions of an enum and a flag list: reordered, one enumerator inserted and one removed
enum class OldOrder { Alpha, Beta, Gamma, Removed };
enum class NewOrder { Inserted, Gamma, Alpha, Beta };
enum class OldFlag : size_t { A, B, C, NUM_VALUES };
enum class NewFlag : size_t { C, X, A, NUM_VALUES };

struct EnumsOld {
	OldOrder value = OldOrder::Alpha;
	OldOrder gone = OldOrder::Alpha;
	flagset<OldFlag> flags;
	short size_changed = 0;
};

struct EnumsNew {
	NewOrder value = NewOrder::Inserted;
	NewOrder gone = NewOrder::Inserted;
	flagset<NewFlag> flags;
	int size_changed = 5;
};

// an index into a list that differs between sessions, saved by the name of its entry
SCP_vector<SCP_string> Registry;

struct Keyed {
	int index = -1;

	SCP_string checkpoint_key() const { return Registry[index]; }
	static Keyed from_checkpoint_key(const SCP_string& name)
	{
		for (size_t i = 0; i < Registry.size(); ++i) {
			if (Registry[i] == name)
				return {static_cast<int>(i)};
		}
		Registry.push_back(name);
		return {static_cast<int>(Registry.size() - 1)};
	}
};

} // namespace

namespace checkpoint {
template <>
struct PoolTraits<Inner> {
	static int index_of(const Inner* p) { return static_cast<int>(p - Pool); }
	static Inner* at(int index) { return &Pool[index]; }
};
} // namespace checkpoint

CHECKPOINT_ENUM_BEGIN(Mode)
CHECKPOINT_ENUM_VALUE(Mode, First)
CHECKPOINT_ENUM_VALUE(Mode, Second)
CHECKPOINT_ENUM_END(Mode)

CHECKPOINT_FIELDS_BEGIN(Inner)
CHECKPOINT_FIELD(Inner, a)
CHECKPOINT_FIELD(Inner, b)
CHECKPOINT_FIELDS_END(Inner)

CHECKPOINT_FIELDS_BEGIN(Outer)
CHECKPOINT_FIELD(Outer, number)
CHECKPOINT_FIELD(Outer, mode)
CHECKPOINT_FIELD(Outer, name)
CHECKPOINT_FIELD(Outer, inner)
CHECKPOINT_FIELD(Outer, inners)
CHECKPOINT_FIELD(Outer, list)
CHECKPOINT_FIELD(Outer, text)
CHECKPOINT_FIELD(Outer, lookup)
CHECKPOINT_FIELD(Outer, nested)
CHECKPOINT_FIELD(Outer, owned)
CHECKPOINT_FIELD(Outer, maybe)
CHECKPOINT_FIELD(Outer, flags)
CHECKPOINT_FIELD(Outer, more_flags)
CHECKPOINT_FIELD(Outer, pair)
CHECKPOINT_FIELD(Outer, tuple)
CHECKPOINT_FIELD(Outer, pooled)
// skipped: skipped
CHECKPOINT_FIELDS_END(Outer)

CHECKPOINT_FIELDS_BEGIN(OwningPointers)
CHECKPOINT_FIELD(OwningPointers, list)
CHECKPOINT_FIELD(OwningPointers, text)
CHECKPOINT_FIELDS_END(OwningPointers)

CHECKPOINT_FIELDS_BEGIN(Old)
CHECKPOINT_FIELD(Old, kept)
CHECKPOINT_FIELD(Old, removed)
CHECKPOINT_FIELD(Old, renamed_type)
CHECKPOINT_FIELDS_END(Old)

CHECKPOINT_FIELDS_BEGIN(New)
CHECKPOINT_FIELD(New, added)
CHECKPOINT_FIELD(New, kept)
CHECKPOINT_FIELDS_END(New)

CHECKPOINT_ENUM_BEGIN(OldOrder)
CHECKPOINT_ENUM_VALUE(OldOrder, Alpha)
CHECKPOINT_ENUM_VALUE(OldOrder, Beta)
CHECKPOINT_ENUM_VALUE(OldOrder, Gamma)
CHECKPOINT_ENUM_VALUE(OldOrder, Removed)
CHECKPOINT_ENUM_END(OldOrder)

CHECKPOINT_ENUM_BEGIN(NewOrder)
CHECKPOINT_ENUM_VALUE(NewOrder, Inserted)
CHECKPOINT_ENUM_VALUE(NewOrder, Gamma)
CHECKPOINT_ENUM_VALUE(NewOrder, Alpha)
CHECKPOINT_ENUM_VALUE(NewOrder, Beta)
CHECKPOINT_ENUM_END(NewOrder)

CHECKPOINT_ENUM_BEGIN(OldFlag)
CHECKPOINT_ENUM_VALUE(OldFlag, A)
CHECKPOINT_ENUM_VALUE(OldFlag, B)
CHECKPOINT_ENUM_VALUE(OldFlag, C)
CHECKPOINT_ENUM_END(OldFlag)

CHECKPOINT_ENUM_BEGIN(NewFlag)
CHECKPOINT_ENUM_VALUE(NewFlag, C)
CHECKPOINT_ENUM_VALUE(NewFlag, X)
CHECKPOINT_ENUM_VALUE(NewFlag, A)
CHECKPOINT_ENUM_END(NewFlag)

CHECKPOINT_FIELDS_BEGIN(EnumsOld)
CHECKPOINT_FIELD(EnumsOld, value)
CHECKPOINT_FIELD(EnumsOld, gone)
CHECKPOINT_FIELD(EnumsOld, flags)
CHECKPOINT_FIELD(EnumsOld, size_changed)
CHECKPOINT_FIELDS_END(EnumsOld)

CHECKPOINT_FIELDS_BEGIN(EnumsNew)
CHECKPOINT_FIELD(EnumsNew, value)
CHECKPOINT_FIELD(EnumsNew, gone)
CHECKPOINT_FIELD(EnumsNew, flags)
CHECKPOINT_FIELD(EnumsNew, size_changed)
CHECKPOINT_FIELDS_END(EnumsNew)

namespace {

Outer make_filled()
{
	Outer o;
	o.number = 42;
	o.mode = Mode::Second;
	strcpy_s(o.name, "alpha");
	o.inner = {1, 1.5f};
	o.inners[2] = {3, 3.5f};
	o.list = {{4, 4.5f}, {5, 5.5f}};
	o.text = "some text";
	o.lookup = {{"one", 1}, {"two", 2}};
	o.nested[7] = {7, 77, 777};
	o.owned = std::make_unique<Inner>(Inner{8, 8.5f});
	o.maybe = 9;
	o.flags = {true, false, true};
	o.more_flags = {false, true};
	o.pair = {10, "ten"};
	o.tuple = std::make_tuple(11, 11.5f, "eleven");
	o.pooled = &Pool[2];
	o.skipped = 12;
	return o;
}

checkpoint::Buffer save_buffer(const Outer& o)
{
	checkpoint::Buffer buffer;
	checkpoint::Writer w(buffer);
	checkpoint::save(w, o);
	return buffer;
}

} // namespace

TEST(CheckpointCodec, round_trip)
{
	auto original = make_filled();
	auto buffer = save_buffer(original);

	Outer loaded;
	checkpoint::LoadContext context;
	checkpoint::Reader r(buffer, &context);
	checkpoint::load(r, loaded);

	ASSERT_FALSE(r.failed());
	EXPECT_TRUE(r.at_end());
	EXPECT_EQ(context.unknown_fields, 0);

	EXPECT_EQ(loaded.number, 42);
	EXPECT_EQ(loaded.mode, Mode::Second);
	EXPECT_STREQ(loaded.name, "alpha");
	EXPECT_EQ(loaded.inner.a, 1);
	EXPECT_EQ(loaded.inner.b, 1.5f);
	EXPECT_EQ(loaded.inners[2].a, 3);
	ASSERT_EQ(loaded.list.size(), 2u);
	EXPECT_EQ(loaded.list[1].a, 5);
	EXPECT_EQ(loaded.text, "some text");
	EXPECT_EQ(loaded.lookup, original.lookup);
	EXPECT_EQ(loaded.nested, original.nested);
	ASSERT_NE(loaded.owned, nullptr);
	EXPECT_EQ(loaded.owned->a, 8);
	EXPECT_EQ(loaded.maybe, 9);
	EXPECT_EQ(loaded.flags, original.flags);
	EXPECT_EQ(loaded.more_flags, original.more_flags);
	EXPECT_EQ(loaded.pair, original.pair);
	EXPECT_EQ(loaded.tuple, original.tuple);
	EXPECT_EQ(loaded.pooled, &Pool[2]);

	// saving what was loaded gives the same bytes
	EXPECT_EQ(save_buffer(loaded), buffer);
}

TEST(CheckpointCodec, skipped_and_unlisted_fields_keep_their_value)
{
	auto original = make_filled();
	original.transient_type = {1, 2, 3};
	auto buffer = save_buffer(original);

	Outer loaded;
	loaded.skipped = -5;
	loaded.transient_type = {9};
	checkpoint::Reader r(buffer);
	checkpoint::load(r, loaded);

	EXPECT_EQ(loaded.skipped, -5);
	EXPECT_EQ(loaded.transient_type, SCP_vector<int>{9});
}

TEST(CheckpointCodec, empty_owned_and_optional_values_are_cleared)
{
	Outer original;
	auto buffer = save_buffer(original);

	Outer loaded;
	loaded.owned = std::make_unique<Inner>();
	loaded.maybe = 3;
	loaded.pooled = &Pool[1];
	checkpoint::Reader r(buffer);
	checkpoint::load(r, loaded);

	EXPECT_EQ(loaded.owned, nullptr);
	EXPECT_FALSE(loaded.maybe.has_value());
	EXPECT_EQ(loaded.pooled, nullptr);
}

TEST(CheckpointCodec, owned_container_pointers_are_saved_by_value)
{
	OwningPointers original;
	original.list = new SCP_vector<int>{1, 2};
	original.text = new SCP_string("text");

	checkpoint::Buffer buffer;
	checkpoint::Writer w(buffer);
	checkpoint::save(w, original);

	// a present value replaces the current one; an absent one clears it
	OwningPointers loaded;
	loaded.list = new SCP_vector<int>{9};
	checkpoint::Reader r(buffer);
	checkpoint::load(r, loaded);
	ASSERT_FALSE(r.failed());
	ASSERT_NE(loaded.list, nullptr);
	EXPECT_EQ(*loaded.list, (SCP_vector<int>{1, 2}));
	ASSERT_NE(loaded.text, nullptr);
	EXPECT_EQ(*loaded.text, "text");

	OwningPointers empty;
	checkpoint::Buffer empty_buffer;
	checkpoint::Writer empty_writer(empty_buffer);
	checkpoint::save(empty_writer, empty);
	checkpoint::Reader empty_reader(empty_buffer);
	checkpoint::load(empty_reader, loaded);
	EXPECT_EQ(loaded.list, nullptr);
	EXPECT_EQ(loaded.text, nullptr);
}

TEST(CheckpointCodec, changed_struct_loads_known_fields)
{
	Old old_version;
	old_version.kept = 7;
	old_version.removed = 8;
	old_version.renamed_type = 9.0f;

	checkpoint::Buffer buffer;
	checkpoint::Writer w(buffer);
	checkpoint::save(w, old_version);

	New new_version;
	new_version.added = 99;
	checkpoint::LoadContext context;
	checkpoint::Reader r(buffer, &context);
	checkpoint::load(r, new_version);

	ASSERT_FALSE(r.failed());
	EXPECT_EQ(new_version.kept, 7);
	EXPECT_EQ(new_version.added, 99);		// not in the data: unchanged
	EXPECT_EQ(context.unknown_fields, 2);	// removed, renamed_type
	EXPECT_EQ(context.missing_fields, SCP_set<SCP_string>{"added"});
}

TEST(CheckpointCodec, enums_and_flags_load_by_name)
{
	EnumsOld old_version;
	old_version.value = OldOrder::Gamma;
	old_version.gone = OldOrder::Removed;
	old_version.flags.set(OldFlag::A).set(OldFlag::B).set(OldFlag::C);
	old_version.size_changed = 3;

	checkpoint::Buffer buffer;
	checkpoint::Writer w(buffer);
	checkpoint::save(w, old_version);

	EnumsNew new_version;
	checkpoint::LoadContext context;
	checkpoint::Reader r(buffer, &context);
	checkpoint::load(r, new_version);

	ASSERT_FALSE(r.failed());
	EXPECT_EQ(new_version.value, NewOrder::Gamma);
	EXPECT_EQ(new_version.gone, NewOrder::Inserted);	// unknown name: unchanged
	EXPECT_TRUE(new_version.flags[NewFlag::A]);
	EXPECT_TRUE(new_version.flags[NewFlag::C]);
	EXPECT_FALSE(new_version.flags[NewFlag::X]);
	EXPECT_EQ(context.unknown_enumerators, 2);			// Removed, B
	EXPECT_EQ(new_version.size_changed, 5);				// saved as a short: kept
	EXPECT_EQ(context.mismatched_fields, 1);
}

TEST(CheckpointCodec, keyed_values_load_by_key)
{
	Registry = {"first", "second"};
	SCP_vector<Keyed> saved = {{1}, {0}};

	checkpoint::Buffer buffer;
	checkpoint::Writer w(buffer);
	checkpoint::save(w, saved);

	// another session built the list in another order, and lacks an entry until it is needed
	Registry = {"second"};
	SCP_vector<Keyed> loaded;
	checkpoint::Reader r(buffer);
	checkpoint::load(r, loaded);

	ASSERT_FALSE(r.failed());
	ASSERT_EQ(loaded.size(), 2u);
	EXPECT_EQ(Registry[loaded[0].index], "second");
	EXPECT_EQ(Registry[loaded[1].index], "first");
	EXPECT_EQ(loaded[0].index, 0);
	EXPECT_EQ(loaded[1].index, 1);
}

TEST(CheckpointCodec, fixed_arrays_load_as_many_elements_as_fit)
{
	int big[4] = {1, 2, 3, 4};
	checkpoint::Buffer buffer;
	checkpoint::Writer w(buffer);
	checkpoint::save(w, big);

	int small[2] = {};
	checkpoint::Reader r(buffer);
	checkpoint::load(r, small);

	ASSERT_FALSE(r.failed());
	EXPECT_TRUE(r.at_end());
	EXPECT_EQ(small[0], 1);
	EXPECT_EQ(small[1], 2);
}

TEST(CheckpointCodec, truncated_data_fails_without_crashing)
{
	auto buffer = save_buffer(make_filled());
	buffer.resize(buffer.size() / 2);

	Outer loaded;
	checkpoint::Reader r(buffer);
	checkpoint::load(r, loaded);

	EXPECT_TRUE(r.failed());
}

TEST(CheckpointCodec, deferred_fixups_run_on_request)
{
	checkpoint::LoadContext context;
	int value = 0;
	context.defer([&value]() { value = 5; });

	EXPECT_EQ(value, 0);
	context.run_fixups();
	EXPECT_EQ(value, 5);

	// fixups only run once
	value = 0;
	context.run_fixups();
	EXPECT_EQ(value, 0);
}

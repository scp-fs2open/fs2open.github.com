#include "pilotfile/BinaryFileHandler.h"

#include "cfile/cfile.h"
#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

class BinaryFileHandlerTest : public test::FSTestFixture {
  public:
	BinaryFileHandlerTest() : test::FSTestFixture(INIT_CFILE)
	{
		pushModDir("pilotfile");
	}
};

namespace {
// Builds the bytes of a binary pilot file (little-endian, as the cfwrite functions store them) as an in-memory file.
// The file reads from this object's buffer, so the object has to outlive the handler, which closes the file.
class PilotFileBytes {
  public:
	PilotFileBytes& add_section(Section id, int size)
	{
		add(static_cast<uint>(id), 2);
		return add_int(size);
	}

	PilotFileBytes& add_int(int value)
	{
		add(static_cast<uint>(value), 4);
		return *this;
	}

	CFILE* open() const
	{
		CFileLocation res(true);
		res.name_ext = "pilot.bin";
		res.full_name = "pilot.bin";
		res.data_ptr = _bytes.data();
		res.size = _bytes.size();
		return cfopen_special(res, "rb");
	}

  private:
	void add(uint value, int count)
	{
		for (int i = 0; i < count; ++i) {
			_bytes.push_back(static_cast<char>((value >> (8 * i)) & 0xFF));
		}
	}

	SCP_vector<char> _bytes;
};
} // namespace

TEST_F(BinaryFileHandlerTest, next_section_zero_size_is_invalid)
{
	PilotFileBytes file;
	file.add_section(Section::Flags, 0);
	pilot::BinaryFileHandler handler(file.open());

	EXPECT_EQ(handler.nextSection(), Section::Invalid);
}

TEST_F(BinaryFileHandlerTest, next_section_past_end_is_invalid)
{
	PilotFileBytes file;
	file.add_section(Section::Flags, 5).add_int(7);
	pilot::BinaryFileHandler handler(file.open());

	EXPECT_EQ(handler.nextSection(), Section::Invalid);
}

TEST_F(BinaryFileHandlerTest, next_section_exact_fit_reads_then_zero)
{
	PilotFileBytes file;
	file.add_section(Section::Flags, 4).add_int(7);
	pilot::BinaryFileHandler handler(file.open());

	EXPECT_EQ(handler.nextSection(), Section::Flags);
	EXPECT_FALSE(handler.atEndOfFile());
	EXPECT_EQ(handler.readInt("value"), 7);
	EXPECT_TRUE(handler.atEndOfFile());

	int past_the_end = -1;
	EXPECT_NO_THROW(past_the_end = handler.readInt("value"));
	EXPECT_EQ(past_the_end, 0);
	EXPECT_FALSE(handler.hasMoreSections());
}

TEST_F(BinaryFileHandlerTest, array_count_without_data_is_cut_to_the_bytes_left)
{
	PilotFileBytes file;
	file.add_section(Section::Variables, 8).add_int(2000000000).add_int(7);
	pilot::BinaryFileHandler handler(file.open());

	ASSERT_EQ(handler.nextSection(), Section::Variables);
	EXPECT_EQ(handler.startArrayRead("variables", false), static_cast<size_t>(4));
}

TEST_F(BinaryFileHandlerTest, negative_array_count_is_cut_to_the_bytes_left)
{
	PilotFileBytes file;
	file.add_section(Section::Variables, 8).add_int(-1).add_int(7);
	pilot::BinaryFileHandler handler(file.open());

	ASSERT_EQ(handler.nextSection(), Section::Variables);
	EXPECT_EQ(handler.startArrayRead("variables", false), static_cast<size_t>(4));
}

TEST_F(BinaryFileHandlerTest, array_count_up_to_the_bytes_left_is_kept)
{
	PilotFileBytes file;
	file.add_section(Section::Variables, 8).add_int(4).add_int(7);
	pilot::BinaryFileHandler handler(file.open());

	ASSERT_EQ(handler.nextSection(), Section::Variables);
	EXPECT_EQ(handler.startArrayRead("variables", false), static_cast<size_t>(4));
}

#include "globalincs/pstypes.h"

#include "missionui/chatbox.h"
#include "network/multi.h"

#include <gtest/gtest.h>

extern int Chatbox_created;
extern int Chatbox_disp_w;

class ChatboxAddLineTest : public ::testing::Test {
  protected:
	void SetUp() override
	{
		_saved_created = Chatbox_created;
		_saved_disp_w = Chatbox_disp_w;
		_saved_net_player = Net_player;

		Chatbox_created = 1;
		Chatbox_disp_w = 400;
		Net_player = &Net_players[0];
		Brief_chat.clear();
	}

	void TearDown() override
	{
		chatbox_clear();
		Net_player = _saved_net_player;
		Chatbox_disp_w = _saved_disp_w;
		Chatbox_created = _saved_created;
	}

	// the scripting API rebuilds a message by appending the text of its indented lines
	static SCP_string joined()
	{
		SCP_string text;
		for (const auto& chat : Brief_chat) {
			text += chat.text;
		}
		return text;
	}

	static void expect_wrapped(const SCP_string& msg)
	{
		ASSERT_GE(Brief_chat.size(), static_cast<size_t>(2));

		bool first = true;
		for (const auto& chat : Brief_chat) {
			EXPECT_LT(strlen(chat.text), msg.size());
			EXPECT_LT(strlen(chat.text), static_cast<size_t>(CHATBOX_MAX_LEN));
			EXPECT_EQ(chat.indent, !first);
			EXPECT_EQ(chat.callsign[0], '\0');
			first = false;
		}
		EXPECT_EQ(joined(), msg);
	}

  private:
	int _saved_created = 0;
	int _saved_disp_w = 0;
	net_player* _saved_net_player = nullptr;
};

// no font is loaded here, so every width measures the same and lines wrap at the length limit alone

TEST_F(ChatboxAddLineTest, long_message_wraps_at_spaces_and_joins_back)
{
	SCP_string msg;
	for (int i = 0; i < 60; ++i) {
		msg += "word";
		msg += std::to_string(i);
		msg += ' ';
	}
	msg.pop_back();

	chatbox_add_line(msg.c_str(), 0, 0);

	expect_wrapped(msg);
}

TEST_F(ChatboxAddLineTest, long_word_is_split_without_adding_spaces)
{
	SCP_string msg(400, 'x');

	chatbox_add_line(msg.c_str(), 0, 0);

	expect_wrapped(msg);
}

TEST_F(ChatboxAddLineTest, short_message_is_one_line)
{
	chatbox_add_line("short line", 0, 0);

	ASSERT_EQ(Brief_chat.size(), static_cast<size_t>(1));
	EXPECT_STREQ(Brief_chat.front().text, "short line");
	EXPECT_FALSE(Brief_chat.front().indent);
}

TEST_F(ChatboxAddLineTest, message_needing_many_lines_keeps_all_of_them)
{
	// longer than four lines at the length limit
	SCP_string msg;
	for (int i = 0; i < 150; ++i) {
		msg += "word";
		msg += std::to_string(i);
		msg += ' ';
	}
	msg.pop_back();

	chatbox_add_line(msg.c_str(), 0, 0);

	EXPECT_GT(Brief_chat.size(), static_cast<size_t>(4));
	EXPECT_EQ(joined(), msg);
}

TEST_F(ChatboxAddLineTest, line_breaks_cannot_flood_the_scrollback)
{
	SCP_string msg;
	for (int i = 0; i < 100; ++i) {
		msg += "a\n";
	}

	chatbox_add_line(msg.c_str(), 0, 0);

	EXPECT_LE(Brief_chat.size(), static_cast<size_t>(9));
}

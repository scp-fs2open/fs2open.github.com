#include "network/psnet2.h"

#include <gtest/gtest.h>

namespace {

const char* const Expected = "2001:db8:1111:2222:3333:4444:5555:6666";

net_addr make_address()
{
	const uint8_t bytes[] =
		{0x20, 0x01, 0x0d, 0xb8, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33, 0x44, 0x44, 0x55, 0x55, 0x66, 0x66};

	net_addr addr;
	memset(&addr, 0, sizeof(addr));
	memcpy(addr.addr, bytes, sizeof(bytes));

	return addr;
}

} // namespace

TEST(PsnetAddrToString, writes_the_whole_address_when_it_fits)
{
	const net_addr addr = make_address();
	char text[INET6_ADDRSTRLEN];

	EXPECT_EQ(psnet_addr_to_string(&addr, text, sizeof(text)), text);
	EXPECT_STREQ(text, Expected);
}

TEST(PsnetAddrToString, truncates_instead_of_emptying_a_short_buffer)
{
	const net_addr addr = make_address();
	char text[10];

	psnet_addr_to_string(&addr, text, sizeof(text));

	EXPECT_STREQ(text, "2001:db8:");
}

TEST(PsnetAddrToString, string_version_returns_its_own_buffer)
{
	const net_addr addr = make_address();
	SCP_string text;

	const char* result = psnet_addr_to_string(&addr, text);

	EXPECT_EQ(text, Expected);
	EXPECT_EQ(result, text.c_str());
}

#include "network/psnet2.h"

#include <gtest/gtest.h>

namespace {

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

TEST(PsnetAddrToString, truncates_instead_of_emptying_a_short_buffer)
{
	const net_addr addr = make_address();
	char text[16];
	memset(text, 'x', sizeof(text));
	text[sizeof(text) - 1] = '\0';

	psnet_addr_to_string(&addr, text, 10);

	EXPECT_STREQ(text, "2001:db8:");
	EXPECT_EQ(text[10], 'x');
}

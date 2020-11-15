
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include "globalincs/pstypes.h"
#include "network/psnet2.h"
#include "network/multimsgs.h"
#include "network/multi_log.h"
#include "network/multi_mdns.h"

PUSH_SUPPRESS_WARNINGS
#include "mdns.h"
POP_SUPPRESS_WARNINGS


static const SCP_string SERVICE_NAME = "_fso._hard-light._udp.local.";
static SCP_string HOST_NAME = "fs2open";

static SCP_vector<int> mSockets;

static in_addr IPv4_addr;
static bool has_ipv4 = false;
static uint8_t IPv6_addr[sizeof(in6_addr)];
static bool has_ipv6 = false;


static int query_callback(int sock __UNUSED, const struct sockaddr *from __UNUSED, size_t addrlen __UNUSED, mdns_entry_type_t entry __UNUSED,
							   uint16_t query_id __UNUSED, uint16_t rtype, uint16_t rclass __UNUSED, uint32_t ttl __UNUSED, const void *data,
							   size_t size, size_t name_offset __UNUSED, size_t name_length __UNUSED, size_t record_offset,
							   size_t record_length, void *user_data)
{
	SOCKADDR_IN in4;
	SOCKADDR_IN6 sockaddr;
	auto *addr = reinterpret_cast<net_addr *>(user_data);

	if (rtype == MDNS_RECORDTYPE_SRV) {
		std::array<char, 256> NAME_BUFFER{};
		mdns_record_srv_t svr = mdns_record_parse_srv(data, size, record_offset, record_length, NAME_BUFFER.data(), NAME_BUFFER.size());

		addr->port = svr.port;
	} else if (rtype == MDNS_RECORDTYPE_A) {
		mdns_record_parse_a(data, size, record_offset, record_length, &in4);

		psnet_map4to6(&in4.sin_addr, &sockaddr.sin6_addr);
		memcpy(&addr->addr, &sockaddr.sin6_addr, sizeof(addr->addr));
	}
	// if we have both A and AAAA records, prefer A
	else if ( (rtype == MDNS_RECORDTYPE_AAAA) && !IN6_IS_ADDR_UNSPECIFIED(reinterpret_cast<in6_addr *>(&addr->addr)) ) {
		mdns_record_parse_aaaa(data, size, record_offset, record_length, &sockaddr);

		memcpy(&addr->addr, &sockaddr.sin6_addr, sizeof(addr->addr));
	}

	return 0;
}

static int service_callback(int sock, const struct sockaddr* from, size_t addrlen, mdns_entry_type_t entry,
							uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl __UNUSED, const void* data,
							size_t size, size_t name_offset __UNUSED, size_t name_length __UNUSED, size_t record_offset,
							size_t record_length, void* user_data __UNUSED)
{
	if (entry != MDNS_ENTRYTYPE_QUESTION) {
		return 0;
	}

	std::array<char, 256> BUFFER{};

	if (rtype == MDNS_RECORDTYPE_PTR) {
		mdns_string_t service = mdns_record_parse_ptr(data, size, record_offset, record_length, BUFFER.data(), BUFFER.size());

		// check for special discovery record and reply accordingly
		const SCP_string dns_sd = "_services._dns-sd._udp.local.";

		if ( (service.length == dns_sd.size()) && (dns_sd == service.str) ) {
			mdns_discovery_answer(sock, from, addrlen, BUFFER.data(), BUFFER.size(), SERVICE_NAME.c_str(), SERVICE_NAME.size());
			return 0;
		}

		// ignore anything not meant for us
		if ( (service.length != SERVICE_NAME.size()) || !(SERVICE_NAME == service.str) ) {
			return 0;
		}

		uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);

		// this one call will send all required records
		mdns_query_answer(sock, from, (unicast) ? addrlen : 0, BUFFER.data(), BUFFER.size(), query_id,
						  SERVICE_NAME.c_str(), SERVICE_NAME.size(), HOST_NAME.c_str(), HOST_NAME.size(),
						  has_ipv4 ? IPv4_addr.s_addr : 0, has_ipv6 ? IPv6_addr : nullptr,
						  Psnet_default_port, nullptr, 0);
	}

	return 0;
}

static void close_mdns_sockets()
{
	for (auto &sock : mSockets) {
		mdns_socket_close(sock);
	}

	mSockets.clear();
}

static bool open_mdns_sockets(bool add_addr = false)
{
	const int ip_mode = psnet_get_ip_mode();
	int sock;

	if ( !mSockets.empty() ) {
		close_mdns_sockets();
	}

	if (ip_mode & PSNET_IP_MODE_V4) {
		SOCKADDR_IN saddr;

		if (add_addr) {
			memset(&saddr, 0, sizeof(saddr));

			saddr.sin_family = AF_INET;
			saddr.sin_addr.s_addr = INADDR_ANY;
			saddr.sin_port = htons(MDNS_PORT);
		}

		sock = mdns_socket_open_ipv4(add_addr ? &saddr : nullptr);

		if (sock >= 0) {
			mSockets.push_back(sock);
		}
	}

	if (ip_mode & PSNET_IP_MODE_V6) {
		SOCKADDR_IN6 saddr;

		if (add_addr) {
			memset(&saddr, 0, sizeof(saddr));

			saddr.sin6_family = AF_INET6;
			saddr.sin6_addr = in6addr_any;
			saddr.sin6_port = htons(MDNS_PORT);
		}

		sock = mdns_socket_open_ipv6(add_addr ? &saddr : nullptr);

		if (sock >= 0) {
			mSockets.push_back(sock);
		}
	}

	return !mSockets.empty();
}


bool multi_mdns_query()
{
	size_t failed = 0;

	if ( !open_mdns_sockets() ) {
		return false;
	}

	std::array<uint8_t, 2048> buffer{};

	for (auto &sock : mSockets) {
		int rval = mdns_query_send(sock, MDNS_RECORDTYPE_PTR, SERVICE_NAME.c_str(),
								   SERVICE_NAME.size(), buffer.data(), buffer.size(), 0);

		if (rval < 0) {
			ml_printf("Failed to send mDNS query: %s\n", strerror(errno));
			++failed;
		}
	}

	if (failed == mSockets.size()) {
		multi_mdns_query_close();
		return false;
	}

	return true;
}

void multi_mdns_query_do()
{
	if (mSockets.empty()) {
		return;
	}

	struct timeval timeout;
	fd_set read_fds;
	std::array<uint8_t, 2048> buffer{};
	net_addr addr;

	for (auto &sock : mSockets) {
		do {
			timeout.tv_sec = 0;
			timeout.tv_usec = 0;

			FD_ZERO(&read_fds);
			FD_SET(static_cast<SOCKET>(sock), &read_fds);

			if ( select(sock+1, &read_fds, nullptr, nullptr, &timeout) == SOCKET_ERROR ) {
				break;
			}

			if ( !FD_ISSET(sock, &read_fds) ) {
				break;
			}

			memset(&addr, 0, sizeof(net_addr));

			mdns_query_recv(sock, buffer.data(), buffer.size(), query_callback, &addr, 0);

			if ( (addr.port == 0) || IN6_IS_ADDR_UNSPECIFIED(reinterpret_cast<in6_addr *>(&addr.addr)) ) {
				continue;
			}

			send_server_query(&addr);
		} while (true);
	}
}

void multi_mdns_query_close()
{
	close_mdns_sockets();
}

bool multi_mdns_service_init()
{
	if ( !open_mdns_sockets(true) ) {
		return false;
	}

	// setup local ip info
	IPv4_addr.s_addr = INADDR_ANY;
	memset(&IPv6_addr, 0, sizeof(IPv6_addr));

	has_ipv4 = false;
	has_ipv6 = false;

	int ip_mode = psnet_get_ip_mode();

	if (ip_mode & PSNET_IP_MODE_V4) {
		auto *in6 = psnet_get_local_ip(AF_INET);

		if (in6 && psnet_map6to4(in6, &IPv4_addr)) {
			has_ipv4 = true;
		}
	}

	if (ip_mode & PSNET_IP_MODE_V6) {
		auto in6 = psnet_get_local_ip(AF_INET6);

		if (in6) {
			memcpy(&IPv6_addr, in6, sizeof(in6_addr));
			has_ipv6 = true;
		}
	}

	// setup hostname
	std::array<char, 256> hostname{};

	if ( !gethostname(hostname.data(), static_cast<int>(hostname.size())) ) {
		HOST_NAME = hostname.data();
	}

	return true;
}

void multi_mdns_service_do()
{
	if (mSockets.empty()) {
		return;
	}

	struct timeval timeout;
	fd_set read_fds;
	std::array<uint8_t, 2048> buffer{};

	for (auto &sock : mSockets) {
		do {
			timeout.tv_sec = 0;
			timeout.tv_usec = 0;

			FD_ZERO(&read_fds);
			FD_SET(static_cast<SOCKET>(sock), &read_fds);

			if ( select(sock+1, &read_fds, nullptr, nullptr, &timeout) == SOCKET_ERROR ) {
				break;
			}

			if ( !FD_ISSET(sock, &read_fds) ) {
				break;
			}

			mdns_socket_listen(sock, buffer.data(), buffer.size(), service_callback, nullptr);
		} while (true);
	}
}

void multi_mdns_service_close()
{
	close_mdns_sockets();
}


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

static SOCKADDR_IN IPv4_addr;
static bool has_ipv4 = false;
static SOCKADDR_IN6 IPv6_addr;
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
							size_t size, size_t name_offset, size_t name_length __UNUSED, size_t record_offset __UNUSED,
							size_t record_length __UNUSED, void* user_data __UNUSED)
{
	if (entry != MDNS_ENTRYTYPE_QUESTION) {
		return 0;
	}

	const SCP_string dns_sd = "_services._dns-sd._udp.local.";
	const SCP_string SERVICE_INSTANCE = HOST_NAME + "." + SERVICE_NAME;
	const SCP_string HOSTNAME_QUALIFIED = HOST_NAME + ".local.";

	std::array<char, 256> NAME{};
	mdns_record_t answer;
	SCP_vector<mdns_record_t> records;
	mdns_record_t n_record;

	answer.type = MDNS_RECORDTYPE_IGNORE;

	size_t offset = name_offset;
	const mdns_string_t name = mdns_string_extract(data, size, &offset, NAME.data(), NAME.size());

	// check for special discovery record and reply accordingly
	if ( (dns_sd.length() == name.length) && (dns_sd == name.str) ) {
		if ( (rtype == MDNS_RECORDTYPE_PTR) || (rtype == MDNS_RECORDTYPE_ANY) ) {
			answer.name = name;
			answer.type = MDNS_RECORDTYPE_PTR;
			answer.data.ptr.name = { SERVICE_NAME.c_str(), SERVICE_NAME.length() };
		}
	}
	// look for our service
	else if ( (SERVICE_NAME.length() == name.length) && (SERVICE_NAME == name.str) ) {
		if ( (rtype == MDNS_RECORDTYPE_PTR) || (rtype == MDNS_RECORDTYPE_ANY) ) {
			// base answer is PTR reverse mapping (service)
			answer.name = { SERVICE_NAME.c_str(), SERVICE_NAME.length() };
			answer.type = MDNS_RECORDTYPE_PTR;
			answer.data.ptr.name = { SERVICE_INSTANCE.c_str(), SERVICE_INSTANCE.length() };

			// additional records...
			records.reserve(3);

			// SRV record mapping (service instance)
			n_record.name = { SERVICE_INSTANCE.c_str(), SERVICE_INSTANCE.length() };
			n_record.type = MDNS_RECORDTYPE_SRV;
			n_record.data.srv.name = { HOSTNAME_QUALIFIED.c_str(), HOSTNAME_QUALIFIED.length() };
			n_record.data.srv.port = Psnet_default_port;
			n_record.data.srv.priority = 0;
			n_record.data.srv.weight = 0;

			records.push_back(n_record);

			// add A record
			if (has_ipv4) {
				n_record.name = { HOSTNAME_QUALIFIED.c_str(), HOSTNAME_QUALIFIED.length() };
				n_record.type = MDNS_RECORDTYPE_A;
				n_record.data.a.addr = IPv4_addr;

				records.push_back(n_record);
			}

			// add AAAA record
			if (has_ipv6) {
				n_record.name = { HOSTNAME_QUALIFIED.c_str(), HOSTNAME_QUALIFIED.length() };
				n_record.type = MDNS_RECORDTYPE_AAAA;
				n_record.data.aaaa.addr = IPv6_addr;

				records.push_back(n_record);
			}
		}
	}
	// look for direct service instance
	else if ( (SERVICE_INSTANCE.length() == name.length) && (SERVICE_INSTANCE == name.str) ) {
		if ( (rtype == MDNS_RECORDTYPE_SRV) || (rtype == MDNS_RECORDTYPE_ANY) ) {
			// base answer is SRV record mapping (service instance)
			answer.name = { SERVICE_INSTANCE.c_str(), SERVICE_INSTANCE.length() };
			answer.type = MDNS_RECORDTYPE_SRV;
			answer.data.srv.name = { HOSTNAME_QUALIFIED.c_str(), HOSTNAME_QUALIFIED.length() };
			answer.data.srv.port = Psnet_default_port;
			answer.data.srv.priority = 0;
			answer.data.srv.weight = 0;

			// additional records ...
			records.reserve(2);

			// add A record
			if (has_ipv4) {
				n_record.name = { HOSTNAME_QUALIFIED.c_str(), HOSTNAME_QUALIFIED.length() };
				n_record.type = MDNS_RECORDTYPE_A;
				n_record.data.a.addr = IPv4_addr;

				records.push_back(n_record);
			}

			// add AAAA record
			if (has_ipv6) {
				n_record.name = { HOSTNAME_QUALIFIED.c_str(), HOSTNAME_QUALIFIED.length() };
				n_record.type = MDNS_RECORDTYPE_AAAA;
				n_record.data.aaaa.addr = IPv6_addr;

				records.push_back(n_record);
			}
		}
	}
	// hostname A/AAAA records
	else if ( (HOSTNAME_QUALIFIED.length() == name.length) && (HOSTNAME_QUALIFIED == name.str) ) {
		if ( has_ipv4 && ((rtype == MDNS_RECORDTYPE_A) || (rtype == MDNS_RECORDTYPE_ANY)) ) {
			// base answer is A record
			answer.name = { HOSTNAME_QUALIFIED.c_str(), HOSTNAME_QUALIFIED.length() };
			answer.type = MDNS_RECORDTYPE_A;
			answer.data.a.addr = IPv4_addr;

			// additional records ...

			// add AAAA record
			if (has_ipv6) {
				n_record.name = { HOSTNAME_QUALIFIED.c_str(), HOSTNAME_QUALIFIED.length() };
				n_record.type = MDNS_RECORDTYPE_AAAA;
				n_record.data.aaaa.addr = IPv6_addr;

				records.push_back(n_record);
			}
		} else if ( has_ipv6 && ((rtype == MDNS_RECORDTYPE_AAAA) || (rtype == MDNS_RECORDTYPE_ANY)) ) {
			// base answer is AAAA record
			answer.name = { HOSTNAME_QUALIFIED.c_str(), HOSTNAME_QUALIFIED.length() };
			answer.type = MDNS_RECORDTYPE_AAAA;
			answer.data.aaaa.addr = IPv6_addr;

			// additional records ...

			// add A record
			if (has_ipv4) {
				n_record.name = { HOSTNAME_QUALIFIED.c_str(), HOSTNAME_QUALIFIED.length() };
				n_record.type = MDNS_RECORDTYPE_A;
				n_record.data.a.addr = IPv4_addr;

				records.push_back(n_record);
			}
		}
	}

	if (answer.type != MDNS_RECORDTYPE_IGNORE) {
		const bool unicast = (rclass & MDNS_UNICAST_RESPONSE) == MDNS_UNICAST_RESPONSE;
		mdns_record_t *records_ptr = records.empty() ? nullptr : &records.front();
		std::array<uint8_t, 1024> buffer{};

		if (unicast) {
			mdns_query_answer_unicast(sock, from, addrlen, buffer.data(), buffer.size(), query_id, static_cast<mdns_record_type_t>(rtype),
											name.str, name.length, answer, nullptr, 0, records_ptr, records.size());
		} else {
			mdns_query_answer_multicast(sock, buffer.data(), buffer.size(), answer, nullptr, 0, records_ptr, records.size());
		}
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
	memset(&IPv4_addr, 0, sizeof(IPv4_addr));
	memset(&IPv6_addr, 0, sizeof(IPv6_addr));

	has_ipv4 = false;
	has_ipv6 = false;

	int ip_mode = psnet_get_ip_mode();

	if (ip_mode & PSNET_IP_MODE_V4) {
		auto *in6 = psnet_get_local_ip(AF_INET);

		if (in6 && psnet_map6to4(in6, &IPv4_addr.sin_addr)) {
			has_ipv4 = true;
		}
	}

	if (ip_mode & PSNET_IP_MODE_V6) {
		auto *in6 = psnet_get_local_ip(AF_INET6);

		if (in6) {
			memcpy(&IPv6_addr.sin6_addr, in6, sizeof(in6_addr));
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

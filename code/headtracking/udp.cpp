#ifndef _WIN32

#include "headtracking/udp.h"

#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace
{
	const unsigned short DEFAULT_PORT = 4242;
	const char* const PORT_ENV_VAR = "FS2_HEADTRACKING_UDP_PORT";

	// opentrack sends exactly six doubles per frame
	const size_t PACKET_SIZE = 6 * sizeof(double);

	// Without a packet for this long the tracker is considered stopped
	const std::chrono::milliseconds ACTIVE_TIMEOUT(1000);

	enum PoseIndex : std::uint8_t
	{
		TX = 0, // centimeters
		TY,     // centimeters
		TZ,     // centimeters
		Yaw,    // degrees
		Pitch,  // degrees
		Roll,   // degrees
	};

	unsigned short getPort()
	{
		const char* env = std::getenv(PORT_ENV_VAR);
		if (env != nullptr && *env != '\0')
		{
			char* end = nullptr;
			long value = std::strtol(env, &end, 10);
			if (end != env && *end == '\0' && value > 0 && value <= 65535)
			{
				return static_cast<unsigned short>(value);
			}
			mprintf(("    Ignoring invalid %s value '%s'\n", PORT_ENV_VAR, env));
		}
		return DEFAULT_PORT;
	}

	// The wire format is little-endian. All platforms FSO currently targets are little-endian,
	// but swap if we ever end up on a big-endian host.
	double decodeLittleEndianDouble(const unsigned char* bytes)
	{
		uint64_t bits = 0;
		for (int i = 7; i >= 0; --i)
		{
			bits = (bits << 8) | bytes[i];
		}
		double value;
		static_assert(sizeof(value) == sizeof(bits), "double must be 64 bits");
		std::memcpy(&value, &bits, sizeof(value));
		return value;
	}
}

namespace headtracking::udp
{
	UDPProvider::UDPProvider()
	{
		const unsigned short port = getPort();

		mSocket = socket(AF_INET, SOCK_DGRAM, 0);
		if (mSocket < 0)
		{
			throw internal::HeadTrackingException(SCP_string("socket() failed: ") + std::strerror(errno));
		}

		const int flags = fcntl(mSocket, F_GETFL, 0);
		if (flags < 0 || fcntl(mSocket, F_SETFL, flags | O_NONBLOCK) < 0)
		{
			SCP_string err = std::strerror(errno);
			close(mSocket);
			mSocket = -1;
			throw internal::HeadTrackingException("Failed to make socket non-blocking: " + err);
		}

		sockaddr_in addr;
		std::memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(port);

		if (bind(mSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
		{
			SCP_string err = std::strerror(errno);
			close(mSocket);
			mSocket = -1;
			throw internal::HeadTrackingException("Failed to bind UDP port " + std::to_string(port) + ": " + err);
		}

		mprintf(("    Listening for opentrack UDP head tracking data on port %u\n", static_cast<unsigned>(port)));
	}

	UDPProvider::~UDPProvider()
	{
		if (mSocket >= 0)
		{
			close(mSocket);
			mSocket = -1;
		}
	}

	void UDPProvider::drainSocket()
	{
		// Drain every queued datagram so only the newest pose is used; otherwise packets that
		// arrive faster than the frame rate would pile up in the socket buffer and add latency.
		unsigned char buffer[64];
		for (;;)
		{
			const ssize_t received = recv(mSocket, buffer, sizeof(buffer), 0);
			if (received < 0)
			{
				// EAGAIN/EWOULDBLOCK: queue is empty. Any other error: keep the last pose.
				break;
			}
			if (static_cast<size_t>(received) != PACKET_SIZE)
			{
				// Not an opentrack packet; skip it
				continue;
			}

			for (int i = 0; i < 6; ++i)
			{
				mPose[i] = decodeLittleEndianDouble(buffer + (i * sizeof(double)));
			}
			mHaveData = true;
			mLastPacket = std::chrono::steady_clock::now();
		}
	}

	bool UDPProvider::isActive()
	{
		drainSocket();
		return mHaveData && (std::chrono::steady_clock::now() - mLastPacket) < ACTIVE_TIMEOUT;
	}

	bool UDPProvider::query(HeadTrackingStatus* statusOut)
	{
		drainSocket();

		if (!mHaveData)
		{
			// Nothing received yet; report a neutral pose so the view stays centered
			statusOut->x = statusOut->y = statusOut->z = 0.0f;
			statusOut->pitch = statusOut->yaw = statusOut->roll = 0.0f;
			return true;
		}

		// Conversion matches what opentrack's FreeTrack protocol writes (proto-ft: yaw and pitch
		// negated, roll unchanged, cm -> mm) combined with FSO's FreeTrack provider
		// (radians / PI, mm / 1000). The result is that a given opentrack output moves the
		// FSO camera identically whether it arrives via FreeTrack on Windows or UDP here.
		//
		//  Rotations: HeadTrackingStatus wants -1..1 for -180..180 degrees.
		//    yaw   = -opentrack_yaw   / 180
		//    pitch = -opentrack_pitch / 180
		//    roll  =  opentrack_roll  / 180
		//  Translation: centimeters -> meters (/ 100), no sign changes.
		//    The camera code in playercontrol.cpp applies its own scale/sign (-0.4x, 0.4y, -z, z >= 0).
		statusOut->yaw   = static_cast<float>(-mPose[Yaw] / 180.0);
		statusOut->pitch = static_cast<float>(-mPose[Pitch] / 180.0);
		statusOut->roll  = static_cast<float>(mPose[Roll] / 180.0);

		statusOut->x = static_cast<float>(mPose[TX] / 100.0);
		statusOut->y = static_cast<float>(mPose[TY] / 100.0);
		statusOut->z = static_cast<float>(mPose[TZ] / 100.0);

		return true;
	}

	SCP_string UDPProvider::getName()
	{
		return "opentrack UDP";
	}
}

#endif // !_WIN32

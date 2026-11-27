#ifndef HEADTRACKING_UDP_H
#define HEADTRACKING_UDP_H

#ifndef _WIN32

#include "headtracking/headtracking.h"
#include "headtracking/headtracking_internal.h"

#include <chrono>

namespace headtracking::udp
{
	/**
	 * @brief Receives head pose from opentrack's "UDP over network" output
	 * (also used by Eyeware Beam and other opentrack-compatible senders).
	 *
	 * Each datagram is 48 bytes: six little-endian IEEE 754 doubles
	 * { X, Y, Z (centimeters), Yaw, Pitch, Roll (degrees) }.
	 *
	 * The default port is 4242. It can be overridden with the
	 * FS2_HEADTRACKING_UDP_PORT environment variable.
	 *
	 * The provider only reports itself active while packets are arriving, so having the
	 * listener open with no tracker running does not disable the regular view controls.
	 */
	class UDPProvider : public internal::HeadTrackingProvider
	{
	private:
		int mSocket = -1;
		bool mHaveData = false;
		double mPose[6] = {};
		std::chrono::steady_clock::time_point mLastPacket;

		// Reads all queued datagrams, keeping only the newest pose
		void drainSocket();

	public:
		UDPProvider();

		~UDPProvider() override;

		UDPProvider(const UDPProvider&) = delete;
		UDPProvider& operator=(const UDPProvider&) = delete;

		bool query(HeadTrackingStatus* statusOut) override;

		// True only while opentrack is sending; lets normal view slewing work otherwise
		bool isActive() override;

		static SCP_string getName();
	};
}

#endif // !_WIN32

#endif // HEADTRACKING_UDP_H

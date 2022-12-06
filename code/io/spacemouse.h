#pragma once

#include "globalincs/vmallocator.h"
#include "math/vecmat.h"
#include "io/timer.h"

#include <tl/optional.hpp>

struct hid_device_;
typedef hid_device_ hid_device;

namespace io
{
	namespace spacemouse {
		struct SpaceMouseMovement {
			vec3d translation;
			angles rotation;
		};

		struct SpaceMouseDefinition {
			unsigned int vendorID, productID;
			size_t buttons;
			size_t packetSize;
		};

		class SpaceMouse {
			const SpaceMouseDefinition& m_definition;
			const int m_pollingFrequency;

			hid_device* m_deviceHandle;
			SpaceMouseMovement m_current;
			SCP_vector<bool> m_keypresses;
			UI_TIMESTAMP m_lastPolled;
			
			void poll();
			void pollMaybe();
			SpaceMouse(const SpaceMouseDefinition& definition, hid_device* deviceHandle, int pollingFrequency = 10);
		public:
			~SpaceMouse();

			const SpaceMouseMovement& getMovement();
			bool isButtonPressed(size_t number);

			static tl::optional<SpaceMouse> searchSpaceMouses(int pollingFrequency = 10);
		};
	}
}
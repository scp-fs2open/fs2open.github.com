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
			enum class Protocol { CONNEXION_3D_OLD, CONNEXION_3D_NEW } protocol;
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

			/*
			@brief Get the current requested movement from the space mouse. Automatically polls for an HID update if required.
			@returns The current movement of the space mouse
			*/
			const SpaceMouseMovement& getMovement();

			/*
			@brief Test if the requested button is currently held down. Automatically polls for an HID update if required.
			@param number The number of the button to test.
			@returns Whether or not the button is pressed. Always false for buttons not present on this space mouse.
			*/
			bool isButtonPressed(size_t number);

			/*
			@brief Polls connected HID devices and tests for known and supported space mice
			@param pollingFrequency The frequency any found space mice should be polled for HID updates, in ms.
			@returns An optional SpaceMouse object, if found
			*/
			static tl::optional<SpaceMouse> searchSpaceMouses(int pollingFrequency = 10);
		};
	}
}
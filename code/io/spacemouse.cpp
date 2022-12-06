#include "spacemouse.h"

#include <hidapi.h>

using namespace io::spacemouse;

void SpaceMouse::poll() {

}

void SpaceMouse::pollMaybe() {
	UI_TIMESTAMP now = ui_timestamp();
	if (ui_timestamp_get_delta(m_lastPolled, now) > m_pollingFrequency) {
		m_lastPolled = std::move(now);
		poll();
	}
}

SpaceMouse::SpaceMouse(const SpaceMouseDefinition& definition, hid_device* deviceHandle, int pollingFrequency)
	: m_current(), m_keypresses(), m_lastPolled(UI_TIMESTAMP::never()), m_pollingFrequency(pollingFrequency), m_deviceHandle(deviceHandle) {

}

SpaceMouse::~SpaceMouse() {

}

const SpaceMouseMovement& SpaceMouse::getMovement() {
	pollMaybe();
	return m_current;
}

bool SpaceMouse::isButtonPressed(size_t number) {
	pollMaybe();
	return true;
}

tl::optional<SpaceMouse> SpaceMouse::searchSpaceMouses() {
	hid_device_info* devices = hid_enumerate(0, 0);


	hid_free_enumeration(devices);

	return tl::nullopt;
}
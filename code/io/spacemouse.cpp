#include "spacemouse.h"

#include <hidapi.h>

using namespace io::spacemouse;

void SpaceMouse::poll() {
	auto buffer = make_unique<unsigned char[]>(m_definition.packetSize);

	int bytes_read = hid_read(m_deviceHandle, buffer.get(), m_definition.packetSize);
	if (bytes_read <= 0)
		return;


}

void SpaceMouse::pollMaybe() {
	UI_TIMESTAMP now = ui_timestamp();
	if (ui_timestamp_get_delta(m_lastPolled, now) > m_pollingFrequency) {
		m_lastPolled = std::move(now);
		poll();
	}
}

SpaceMouse::SpaceMouse(const SpaceMouseDefinition& definition, hid_device* deviceHandle, int pollingFrequency)
	: m_definition(definition), m_pollingFrequency(pollingFrequency), m_deviceHandle(deviceHandle),
	  m_current({ ZERO_VECTOR, ZERO_ANGLES }), m_keypresses(definition.buttons, false), m_lastPolled(UI_TIMESTAMP::never()) {
	hid_set_nonblocking(m_deviceHandle, 1);
	
}

SpaceMouse::~SpaceMouse() {
	hid_close(m_deviceHandle);
}

const SpaceMouseMovement& SpaceMouse::getMovement() {
	pollMaybe();
	return m_current;
}

bool SpaceMouse::isButtonPressed(size_t number) {
	if (number >= m_definition.buttons)
		return false;

	pollMaybe();
	return m_keypresses[number];
}

const static SCP_vector<SpaceMouseDefinition> knownSpaceMice{

};

tl::optional<SpaceMouse> SpaceMouse::searchSpaceMouses(int pollingFrequency) {
	tl::optional<SpaceMouse> mouse = tl::nullopt;
	
	hid_device_info* devices = hid_enumerate(0, 0);

	for (const hid_device_info* head = devices; head != nullptr && !mouse.has_value(); head = head->next) {
		for (const SpaceMouseDefinition& mouseDef : knownSpaceMice) {
			if (mouseDef.vendorID != head->vendor_id || mouseDef.productID != head->product_id)
				continue;

			hid_device* device = hid_open(head->vendor_id, head->product_id, head->serial_number);

			if (device != nullptr) {
				mouse.emplace(SpaceMouse(mouseDef, device, pollingFrequency));
				break;
			}
		}
	}

	hid_free_enumeration(devices);

	return mouse;
}
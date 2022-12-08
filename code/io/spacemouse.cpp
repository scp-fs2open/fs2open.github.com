#include "spacemouse.h"

#include <hidapi.h>

using namespace io::spacemouse;

constexpr static size_t SPACEMOUSE_HID_BUFFER_SIZE = 0xff;
static unsigned char spacemouse_hid_buffer[SPACEMOUSE_HID_BUFFER_SIZE];

#pragma pack(push,1)
struct connexion_3d_old_protocol {
	uint8_t type;
	union {
		struct {
			int16_t trans_x, trans_y, trans_z;
		} type1;
		struct {
			int16_t rot_x, rot_y, rot_z;
		} type2;
		struct {
			uint8_t buttons[4];
		} type3;
	} data;
};

struct connexion_3d_new_protocol {
	uint8_t type;
	union {
		struct {
			int16_t trans_x, trans_y, trans_z;
			int16_t rot_x, rot_y, rot_z;
		} type1;
		struct {
			uint8_t buttons[4];
		} type3;
	} data;
};
#pragma pack(pop)

static size_t requiredPollCount(SpaceMouseDefinition::Protocol protocol) {
	switch (protocol) {
	case SpaceMouseDefinition::Protocol::CONNEXION_3D_OLD:
		return 4;
	case SpaceMouseDefinition::Protocol::CONNEXION_3D_NEW:
		return 2;
	default:
		UNREACHABLE("Bad SpaceMouse protocol specified!");
		return 0;
	}
}

void SpaceMouse::poll() {
	for (size_t report = 0; report < requiredPollCount(m_definition.protocol); report++) {
		int bytes_read = hid_read(m_deviceHandle, spacemouse_hid_buffer, SPACEMOUSE_HID_BUFFER_SIZE);
		if (bytes_read <= 0)
			continue;

		switch (m_definition.protocol) {
		case SpaceMouseDefinition::Protocol::CONNEXION_3D_OLD: {
			const connexion_3d_old_protocol& data = *reinterpret_cast<connexion_3d_old_protocol*>(spacemouse_hid_buffer);
			switch (data.type) {
			case 1:
				m_current.translation.xyz.x = static_cast<float>(data.data.type1.trans_x) / 150.0f;
				m_current.translation.xyz.y = static_cast<float>(data.data.type1.trans_y) / -150.0f;
				m_current.translation.xyz.z = static_cast<float>(data.data.type1.trans_z) / -150.0f;
				break;
			case 2:
				m_current.rotation.p = static_cast<float>(data.data.type2.rot_x) / -350.0f;
				m_current.rotation.b = static_cast<float>(data.data.type2.rot_y) / 350.0f;
				m_current.rotation.h = static_cast<float>(data.data.type2.rot_z) / 350.0f;
				break;
			case 3:
				//Buttons are not yet handled
				break;
			default:
				UNREACHABLE("Bad data in old 3Dconnexion protocol!");
			}
			break;
		}
		case SpaceMouseDefinition::Protocol::CONNEXION_3D_NEW: {
			const connexion_3d_new_protocol& data = *reinterpret_cast<connexion_3d_new_protocol*>(spacemouse_hid_buffer);
			switch (data.type) {
			case 1:
				m_current.translation.xyz.x = static_cast<float>(data.data.type1.trans_x) / 150.0f;
				m_current.translation.xyz.y = static_cast<float>(data.data.type1.trans_y) / -150.0f;
				m_current.translation.xyz.z = static_cast<float>(data.data.type1.trans_z) / -150.0f;
				m_current.rotation.p = static_cast<float>(data.data.type1.rot_x) / -350.0f;
				m_current.rotation.b = static_cast<float>(data.data.type1.rot_y) / 350.0f;
				m_current.rotation.h = static_cast<float>(data.data.type1.rot_z) / 350.0f;
				break;
			case 3:
				//Buttons are not yet handled
				break;
			default:
				UNREACHABLE("Bad data in new 3Dconnexion protocol!");
			}
			break;
		}
		default:
			UNREACHABLE("Bad SpaceMouse protocol specified!");
		}
	}
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

const static SCP_vector<SpaceMouseDefinition> knownSpaceMice {
	SpaceMouseDefinition { 0x046D, 0xC626,  2, SpaceMouseDefinition::Protocol::CONNEXION_3D_OLD }, //SpaceNavigator
	SpaceMouseDefinition { 0x256F, 0xC635,  2, SpaceMouseDefinition::Protocol::CONNEXION_3D_OLD }, //SpaceMouse Compact
	SpaceMouseDefinition { 0x256F, 0xC62E,  2, SpaceMouseDefinition::Protocol::CONNEXION_3D_NEW }, //SpaceMouse Wireless
	SpaceMouseDefinition { 0x046D, 0xC62B, 15, SpaceMouseDefinition::Protocol::CONNEXION_3D_OLD }, //SpaceMouse Pro
	SpaceMouseDefinition { 0x256F, 0xC631, 15, SpaceMouseDefinition::Protocol::CONNEXION_3D_NEW }, //SpaceMouse Pro Wireless
	SpaceMouseDefinition { 0x256F, 0xC632, 15, SpaceMouseDefinition::Protocol::CONNEXION_3D_NEW }, //SpaceMouse Pro Wireless
	SpaceMouseDefinition { 0x046D, 0xC629, 21, SpaceMouseDefinition::Protocol::CONNEXION_3D_OLD }, //SpacePilot Pro
	SpaceMouseDefinition { 0x256F, 0xC652, 15, SpaceMouseDefinition::Protocol::CONNEXION_3D_NEW }, //3Dconnexion Universal Receiver
};

std::unique_ptr<SpaceMouse> SpaceMouse::searchSpaceMouses(int pollingFrequency) {
	std::unique_ptr<SpaceMouse> mouse = nullptr;
	
	hid_device_info* devices = hid_enumerate(0, 0);

	for (const hid_device_info* head = devices; head != nullptr && mouse == nullptr; head = head->next) {
		for (const SpaceMouseDefinition& mouseDef : knownSpaceMice) {
			if (mouseDef.vendorID != head->vendor_id || mouseDef.productID != head->product_id)
				continue;

			hid_device* device = hid_open_path(head->path);

			if (device != nullptr) {
				mouse = std::unique_ptr<SpaceMouse>(new SpaceMouse(mouseDef, device, pollingFrequency));
				break;
			}
		}
	}

	hid_free_enumeration(devices);

	return mouse;
}
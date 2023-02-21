#include "controlconfig/controlsconfig.h"
#include "io/joy.h"
#include "io/joy_ff.h"
#include "io/timer.h"
#include "mod_table/mod_table.h"
#include "options/Option.h"
#include "osapi/osapi.h"

#include <algorithm>
#include <array>
#include <bitset>
#include <memory>
#include <utility>

#include "libs/jansson.h"

using namespace io::joystick;
using namespace os::events;

namespace {

typedef std::unique_ptr<Joystick> JoystickPtr;

SCP_vector<JoystickPtr> joysticks;	//!< All joysticks detected by SDL
std::array<Joystick*, CID_JOY_MAX> pJoystick = {nullptr};	//!< The 4 joysticks FSO are using. pJoystick[0] replaces CurrentJoystick

bool initialized = false;

/**
* @brief Compatibility conversion from Button index to HatPosition
*/
inline
HatPosition hatBtnToEnum(int in) {
	Assertion(in >= JOY_NUM_BUTTONS, "Invalid button value passed to hatBtnToEnum: %i", in);

	in -= JOY_NUM_BUTTONS;

	switch (in) {
	case 0:
		return HAT_DOWN;

	case 1:
		return HAT_UP;

	case 2:
		return HAT_LEFT;

	case 3:
		return HAT_RIGHT;

	case 4:
		return HAT_LEFTDOWN;

	case 5:
		return HAT_LEFTUP;

	case 6:
		return HAT_RIGHTDOWN;

	case 7:
		return HAT_RIGHTUP;

	default:
		// Invalid index
		return HAT_CENTERED;
	}
};

SCP_string getJoystickGUID(SDL_Joystick* stick)
{
	auto guid = SDL_JoystickGetGUID(stick);

	const size_t GUID_STR_SIZE = 33;
	SCP_string joystickGUID;
	joystickGUID.resize(GUID_STR_SIZE);

	SDL_JoystickGetGUIDString(guid, &joystickGUID[0], static_cast<int>(joystickGUID.size()));

	// Remove trailing \0
	joystickGUID.resize(GUID_STR_SIZE - 1);

	// Make sure the GUID is upper case
	SCP_toupper(joystickGUID);

	return joystickGUID;
}

bool joystickMatchesGuid(Joystick* testStick, const SCP_string& guid, int id)
{
	if (guid.empty()) {
		// Only use the id
		return id == testStick->getDeviceId();
	}

	SCP_string guidStr(guid);
	SCP_toupper(guidStr);

	if (testStick->getGUID() != guidStr) {
		return false; // GUID doesn't match
	}

	// Build a list of all joysticks with the right guid
	size_t num_sticks = 0;
	for (auto& stick : joysticks) {
		if (stick->getGUID() == guidStr) {
			++num_sticks;
		}
	}

	if (num_sticks == 0) {
		// Not the right GUID
		return false;
	}
	if (num_sticks == 1) {
		// Only one option -> this is the right stick
		return true;
	}

	// Multiple sticks -> check if the device id is the same
	return testStick->getDeviceId() == id;
}

/**
 * Returns True if the given stick should be assigned the given cid
 *
 * @param[in] testStick	The joystick ptr to test
 * @param[in] cid       The cid to test this against
 *
 * @details
 *   Should cid be CID_JOY0, it also checks for legacy .ini keys.
 *   Should Joy0ID key be null, legacy keys will be tried
 */
bool isPlayerJoystick(Joystick* testStick, short cid) {
	SCP_string GUID;
	int Id;

	SCP_string key_guid;
	SCP_string key_id;

	// Select key strings according to cid
	switch (cid) {
		case CID_JOY0:
			key_guid = "Joy0GUID";
			key_id = "Joy0ID";
			break;
		case CID_JOY1:
			key_guid = "Joy1GUID";
			key_id = "Joy1ID";
			break;
		case CID_JOY2:
			key_guid = "Joy2GUID";
			key_id = "Joy2ID";
			break;
		case CID_JOY3:
			key_guid = "Joy3GUID";
			key_id = "Joy3ID";
			break;
		case CID_NONE:
		case CID_KEYBOARD:
		case CID_MOUSE:
			return false;
		default:
			mprintf(("Unknown CID %i\n", cid));
			return false;
	}

	GUID = os_config_read_string(nullptr, key_guid.c_str(), "");
	Id = static_cast<int>(os_config_read_uint(nullptr, key_id.c_str(), 0));

	if ((cid == CID_JOY0) && GUID.empty()) {
		// Check legacy keys
		GUID = os_config_read_string(nullptr, "CurrentJoystickGUID", "");
		Id = static_cast<int>(os_config_read_uint(nullptr, "CurrentJoystick", 0));
	}

	return joystickMatchesGuid(testStick, GUID, Id);
}

/**
 * Reads/deserializes the given json line and retrieves the joystick that it references
 */
Joystick* joystick_deserialize(const json_t* value)
{
	const char* guid;
	int id;

	json_error_t err;
	if (json_unpack_ex((json_t*)value, &err, 0, "{s:s, s:i}", "guid", &guid, "id", &id) != 0) {
		throw json_exception(err);
	}

	for (auto& test_stick : joysticks) {
		if (joystickMatchesGuid(test_stick.get(), guid, id)) {
			return test_stick.get();
		}
	}

	return nullptr;
}

/**
 * Converts/serializes joystick reference data into a json line
 */
json_t* joystick_serializer(Joystick* joystick)
{
	if (joystick == nullptr) {
		return json_null();
	}

	return json_pack("{sssi}", "guid", joystick->getGUID().c_str(), "id", joystick->getDeviceId());
}

/**
 * Scripting callback for displaying the given joystick's name
 */
SCP_string joystick_display(Joystick* stick)
{
	if (stick == nullptr) {
		return "None";
	}
	return stick->getName();
}

/**
 * Scripting callback for enumerating a SCP_vector with all available joysticks
 */
SCP_vector<Joystick*> joystick_enumerator()
{
	SCP_vector<Joystick*> out;
	out.push_back(nullptr);
	for (auto& stick : joysticks) {
		out.push_back(stick.get());
	}
	return out;
}

/**
 * Joystick options for the new menu system
 * These should be displayed as a dropdown box type widget
 */
auto JoystickOption = options::OptionBuilder<Joystick*>("Input.Joystick", "Joystick", "The current joystick 0.")
                          .category("Input")                    // Category this option shows up in the scripting heirachy
                          .deserializer(joystick_deserialize)   // callback for json to C++
                          .serializer(joystick_serializer)      // callback for C++ to json
                          .display(joystick_display)            // callback for constructing the display label
                          .enumerator(joystick_enumerator)      // callback for enumerating/constructing the values this option may take
                          .level(options::ExpertLevel::Beginner)
                          .default_val(nullptr)                 // initial/default value for this option
                          .flags({options::OptionFlags::ForceMultiValueSelection})
                          .importance(3)
                          .finish();

auto JoystickOption1 = options::OptionBuilder<Joystick*>("Input.Joystick1", "Joystick1", "The current joystick 1.")
                          .category("Input")
                          .deserializer(joystick_deserialize)
                          .serializer(joystick_serializer)
                          .display(joystick_display)
                          .enumerator(joystick_enumerator)
                          .level(options::ExpertLevel::Beginner)
                          .default_val(nullptr)
                          .flags({ options::OptionFlags::ForceMultiValueSelection })
                          .importance(3)
                          .finish();

auto JoystickOption2 = options::OptionBuilder<Joystick*>("Input.Joystick2", "Joystick2", "The current joystick 2.")
                          .category("Input")
                          .deserializer(joystick_deserialize)
                          .serializer(joystick_serializer)
                          .display(joystick_display)
                          .enumerator(joystick_enumerator)
                          .level(options::ExpertLevel::Beginner)
                          .default_val(nullptr)
                          .flags({ options::OptionFlags::ForceMultiValueSelection })
                          .importance(3)
                          .finish();

auto JoystickOption3 = options::OptionBuilder<Joystick*>("Input.Joystick3", "Joystick3", "The current joystick 3.")
                           .category("Input")
                           .deserializer(joystick_deserialize)
                           .serializer(joystick_serializer)
                           .display(joystick_display)
                           .enumerator(joystick_enumerator)
                           .level(options::ExpertLevel::Beginner)
                           .default_val(nullptr)
                           .flags({ options::OptionFlags::ForceMultiValueSelection })
                           .importance(3)
                           .finish();

HatPosition convertSDLHat(int val)
{
	switch (val)
	{
		case SDL_HAT_CENTERED:
			return HAT_CENTERED;
		case SDL_HAT_UP:
			return HAT_UP;
		case SDL_HAT_RIGHT:
			return HAT_RIGHT;
		case SDL_HAT_DOWN:
			return HAT_DOWN;
		case SDL_HAT_LEFT:
			return HAT_LEFT;
		case SDL_HAT_RIGHTUP:
			return HAT_RIGHTUP;
		case SDL_HAT_RIGHTDOWN:
			return HAT_RIGHTDOWN;
		case SDL_HAT_LEFTUP:
			return HAT_LEFTUP;
		case SDL_HAT_LEFTDOWN:
			return HAT_LEFTDOWN;
		default:
			return HAT_CENTERED;
	}
}

void enumerateJoysticks(SCP_vector<JoystickPtr>& outVec)
{
	auto num = SDL_NumJoysticks();
	outVec.clear();
	outVec.reserve(static_cast<size_t>(num));

	mprintf(("Printing joystick info:\n"));

	for (auto i = 0; i < num; ++i) {
		auto ptr = JoystickPtr(new Joystick(i));
		ptr->printInfo();

		outVec.push_back(std::move(ptr));
	}
}

/**
 * Assigns the given cid to the given stick
 *
 * @param[in] stick  The stick that is being assigned
 * @param[in] cid    The cid being assigned, must be a valid short between CID_JOY0 and CID_JOY_MAX
 */
void setPlayerJoystick(Joystick *stick, short cid)
{
	Assert((cid >= CID_JOY0) && (cid < CID_JOY_MAX));
	pJoystick[cid] = stick;

	if (pJoystick[cid] != nullptr)
	{
		mprintf(("  Using '%s' as Joy-%i\n", pJoystick[cid]->getName().c_str(), cid));
		mprintf(("\n"));
		mprintf(("  Number of axes: %d\n", pJoystick[cid]->numAxes()));
		mprintf(("  Number of buttons: %d\n", pJoystick[cid]->numButtons()));
		mprintf(("  Number of hats: %d\n", pJoystick[cid]->numHats()));
		mprintf(("  Number of trackballs: %d\n", pJoystick[cid]->numBalls()));
		mprintf(("\n"));
	}
	else
	{
		mprintf((" Joystick %i removed\n", cid));
	}
}

Joystick *getJoystickForID(SDL_JoystickID id)
{
	for (auto iter = joysticks.begin(); iter != joysticks.end(); ++iter)
	{
		if ((*iter)->getID() == id)
		{
			return (*iter).get();
		}
	}

	return nullptr;
}

bool axis_event_handler(const SDL_Event &evt)
{
	auto stick = getJoystickForID(evt.jaxis.which);

	if (stick != nullptr)
	{
		stick->handleJoyEvent(evt);
	}

	return true;
}

bool ball_event_handler(const SDL_Event &evt)
{
	auto stick = getJoystickForID(evt.jball.which);

	if (stick != nullptr)
	{
		stick->handleJoyEvent(evt);
	}

	return true;
}

bool button_event_handler(const SDL_Event &evt)
{
	auto stick = getJoystickForID(evt.jbutton.which);

	if (stick != nullptr)
	{
		stick->handleJoyEvent(evt);
	}

	return true;
}

bool hat_event_handler(const SDL_Event &evt)
{
	auto stick = getJoystickForID(evt.jhat.which);

	if (stick != nullptr)
	{
		stick->handleJoyEvent(evt);
	}

	return true;
}

bool device_event_handler(const SDL_Event &evt)
{
	auto &joyDeviceEvent = evt.jdevice;

	auto evtType = evt.type;

	if (evtType == SDL_JOYDEVICEADDED)
	{
		// A new joystick has been added, add it to our list and check if it's the configured joystick
		Joystick *addedStick;
		{
			auto device = SDL_JoystickOpen(joyDeviceEvent.which);

			if (device == nullptr)
			{
				mprintf(("Failed to open connecting joystick: %s\n", SDL_GetError()));
				return true;
			}

			// Add the new device to our list
			auto added = JoystickPtr(new Joystick(joyDeviceEvent.which));

			for (auto iter = joysticks.begin(); iter != joysticks.end(); ++iter)
			{
				if ((*iter)->getID() == added->getID())
				{
					// Already have this stick
					*(*iter) = std::move(*added);
					return true;
				}
			}

			addedStick = added.get();
			// This is a new stick so we can output it's information
			mprintf(("A new joystick has been connected:\n"));
			addedStick->printInfo();

			joysticks.push_back(std::move(added));
		}

		// Is true if all of the pJoysticks are bound
		bool all_bound = std::all_of(pJoystick.begin(), pJoystick.end(), [](Joystick* pJoy){ return pJoy != nullptr; });

		if (all_bound)
		{
			// We already have all valid joystick devices, bail early
			return true;
		}	// Else, at least one player stick is unbound

		// Bind the added stick, if we want it
		if (Using_in_game_options)
		{
			// New menu system lets players select joysticks in-game
			Joystick* value[CID_JOY_MAX];
			value[0] = JoystickOption->getValue();
			value[1] = JoystickOption1->getValue();
			value[2] = JoystickOption2->getValue();
			value[3] = JoystickOption3->getValue();

			
			for (short i = CID_JOY0; i < CID_JOY_MAX; ++i)
			{
				if (value[i] == addedStick)
				{
					mprintf(("Joystick %i connected\n", i));
					setPlayerJoystick(addedStick, i);
					break;
				}
			}
		}
		else
		{
			// Legacy system requires setting joysticks in the launcher
			for (short i = CID_JOY0; i < CID_JOY_MAX; ++i)
			{
				if (isPlayerJoystick(addedStick, i))
				{
					mprintf(("Joystick %i connected\n", i));
					setPlayerJoystick(addedStick, i);
					break;
				}
			}
		}
	}
	else if (evtType == SDL_JOYDEVICEREMOVED)
	{
		bool any_bound = std::any_of(pJoystick.begin(), pJoystick.end(), [](Joystick* pJoy){ return pJoy != nullptr; });

		if (!any_bound)
		{
			// Nothing bound, ignore
			return true;
		}

		// Else, Find if any of our joysticks were lost
		for (short i = CID_JOY0; i < CID_JOY_MAX; ++i)
		{
			if (joyDeviceEvent.which == pJoystick[i]->getID())
			{
				// We just lost our joystick, reset the data
				mprintf(("Joystick %i disconnected\n", i));
				setPlayerJoystick(nullptr, i);
				break;
			}
		}
	}

	return true;
}
} // namespace

namespace io
{
namespace joystick
{
	Joystick::Joystick(int device_id) :
		_device_id(device_id)
	{
		_joystick = SDL_JoystickOpen(device_id);

		Assertion(_joystick != nullptr, "Failed to open a joystick, get a coder!");

		fillValues();
	}

	Joystick::Joystick(Joystick &&other) noexcept :
			_joystick(nullptr)
	{
		*this = std::move(other);
	}

	Joystick::~Joystick()
	{
		if (_joystick != nullptr)
		{
			SDL_JoystickClose(_joystick);
			_joystick = nullptr;
		}
	}

	Joystick &Joystick::operator=(Joystick &&other) noexcept
	{
		std::swap(_device_id, other._device_id);
		std::swap(_joystick, other._joystick);

		fillValues();

		return *this;
	}

	bool Joystick::isAttached() const
	{
		return SDL_JoystickGetAttached(_joystick) == SDL_TRUE;
	}

	Sint16 Joystick::getAxis(int index) const
	{
		Assertion(index >= 0 && index < numAxes(), "Invalid index %d!", index);

		return _axisValues[index];
	}

	bool Joystick::isButtonDown(int index) const
	{
		Assertion(index >= 0 && index < numButtons(), "Invalid index %d!", index);

		return _button[index].DownTimestamp.isValid();
	}

	float Joystick::getButtonDownTime(int index) const
	{
		Assertion(index >= 0 && index < numButtons(), "Invalid index %d!", index);

		if (_button[index].DownTimestamp.isValid())
		{
			auto diff = ui_timestamp_since(_button[index].DownTimestamp);

			return static_cast<float>(diff) / MILLISECONDS_PER_SECOND;
		}

		return 0.0f;
	}

	int Joystick::getButtonDownCount(int index, bool reset)
	{
		Assertion(index >= 0 && index < numButtons(), "Invalid index %d!", index);

		auto val = _button[index].DownCount;

		if (reset)
		{
			_button[index].DownCount = 0;
		}

		return val;
	}

	coord2d Joystick::getBall(int index) const
	{
		Assertion(index >= 0 && index < numBalls(), "Invalid index %d!", index);

		return _ballValues[index];
	}

	HatPosition Joystick::getHatPosition(int index) const
	{
		Assertion(index >= 0 && index < numHats(), "Invalid index %d!", index);

		return _hat[index].Value;
	}

	int Joystick::getHatDownCount(int index, HatPosition pos, bool ext, bool reset) {
		Assertion(index >= 0 && index < numHats(), "Invalid index %d!", index);

		int val;

		if (pos == HAT_CENTERED) {
			return 0;
		}

		if (ext) {
			// Use 8-position
			val = _hat[index].DownCount8[pos];

			if (reset) {
				_hat[index].DownCount8[pos] = 0;
			}
		} else {
			// Use 4-position
			val = _hat[index].DownCount4[pos];

			if (reset) {
				_hat[index].DownCount4[pos] = 0;
			}
		}

		return val;
	}

	float Joystick::getHatDownTime(int index, HatPosition pos, bool ext) const
	{
		Assertion(index >= 0 && index < numHats(), "Invalid index %d!", index);

		if (pos == HAT_CENTERED) {
			return 0.0f;
		}

		UI_TIMESTAMP val;

		if (ext) {
			// Use 8-position
			val = _hat[index].DownTimestamp8[pos];

		} else {
			// Use 4-position
			val = _hat[index].DownTimestamp4[pos];
		}

		if (!val.isValid()) {
			return 0.0f;
		}

		auto diff = ui_timestamp_since(val);

		return static_cast<float>(diff) / MILLISECONDS_PER_SECOND;
	}

	int Joystick::numAxes() const
	{
		return static_cast<int>(_axisValues.size());
	}

	int Joystick::numBalls() const
	{
		return static_cast<int>(_ballValues.size());
	}

	int Joystick::numButtons() const
	{
		return static_cast<int>(_button.size());
	}

	int Joystick::numHats() const
	{
		return static_cast<int>(_hat.size());
	}

	SCP_string Joystick::getGUID() const
	{
		return _guidStr;
	}

	SCP_string Joystick::getName() const
	{
		return _name;
	}

	SDL_JoystickID Joystick::getID() const
	{
		return _id;
	}

	void Joystick::fillValues()
	{
		_name.assign(SDL_JoystickName(_joystick));
		_guidStr = getJoystickGUID(_joystick);
		_id = SDL_JoystickInstanceID(_joystick);
		_isHaptic = SDL_JoystickIsHaptic(_joystick);

		// Initialize values of the axes
		auto numSticks = SDL_JoystickNumAxes(_joystick);
		_axisValues.resize(static_cast<size_t>(numSticks));
		for (auto i = 0; i < numSticks; ++i)
		{
			_axisValues[i] = SDL_JoystickGetAxis(_joystick, i);
		}

		// Initialize ball values
		auto ballNum = SDL_JoystickNumBalls(_joystick);
		_ballValues.resize(static_cast<size_t>(ballNum));
		for (auto i = 0; i < ballNum; ++i)
		{
			coord2d coord;
			if (SDL_JoystickGetBall(_joystick, i, &coord.x, &coord.y))
			{
				mprintf(("Failed to get ball %d value for joystick %s: %s\n", i, _name.c_str(), SDL_GetError()));
			}
		}

		// Initialize buttons
		auto buttonNum = SDL_JoystickNumButtons(_joystick);
		_button.resize(static_cast<size_t>(buttonNum));
		for (auto i = 0; i < buttonNum; ++i)
		{
			if (SDL_JoystickGetButton(_joystick, i) == 1)
			{
				_button[i].DownTimestamp = ui_timestamp();
			}
			else
			{
				_button[i].DownTimestamp = UI_TIMESTAMP::invalid();
			}
		}

		// Initialize hats
		auto hatNum = SDL_JoystickNumHats(_joystick);
		_hat.resize(static_cast<size_t>(hatNum));
		for (auto i = 0; i < hatNum; ++i)
		{
			std::bitset<4> hatset = SDL_JoystickGetHat(_joystick, i);
			auto hatval = convertSDLHat(SDL_JoystickGetHat(_joystick, i));
			_hat[i].Value = hatval;

			// Reset timestampts
			for (auto j = 0; j < 4; ++j) {
				_hat[i].DownTimestamp4[j] = UI_TIMESTAMP::invalid();
			}
			for (auto j = 0; j < 8; ++j) {
				_hat[i].DownTimestamp8[j] = UI_TIMESTAMP::invalid();
			}

			if (_hat[i].Value != HAT_CENTERED)
			{
				// Set the 4-pos timestamp(s)
				if ((hatset[HAT_DOWN])) {
					_hat[i].DownTimestamp4[HAT_DOWN] = ui_timestamp();
				}
				if ((hatset[HAT_UP])) {
					_hat[i].DownTimestamp4[HAT_UP] = ui_timestamp();
				}
				if ((hatset[HAT_LEFT])) {
					_hat[i].DownTimestamp4[HAT_LEFT] = ui_timestamp();
				}
				if ((hatset[HAT_RIGHT])) {
					_hat[i].DownTimestamp4[HAT_RIGHT] = ui_timestamp();
				}

				// Set the 8-pos timestamp
				_hat[i].DownTimestamp8[hatval] = ui_timestamp();
			}
		}
	}

	SDL_Joystick *Joystick::getDevice()
	{
		return _joystick;
	}

	void Joystick::handleJoyEvent(const SDL_Event &evt)
	{
		switch (evt.type)
		{
			case SDL_JOYAXISMOTION:
				handleAxisEvent(evt.jaxis);
				break;
			case SDL_JOYBALLMOTION:
				handleBallEvent(evt.jball);
				break;
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				handleButtonEvent(evt.jbutton);
				break;
			case SDL_JOYHATMOTION:
				handleHatEvent(evt.jhat);
				break;
			default:
				break;
		}
	}

	void Joystick::handleAxisEvent(const SDL_JoyAxisEvent &evt)
	{
		auto axis = evt.axis;

		Assertion(axis < numAxes(), "SDL event contained invalid axis index!");

		_axisValues[axis] = evt.value;
	}

	void Joystick::handleButtonEvent(const SDL_JoyButtonEvent &evt)
	{
		auto button = evt.button;

		Assertion(button < numButtons(), "SDL event contained invalid button index!");

		auto down = evt.state == SDL_PRESSED;

		_button[button].DownTimestamp = down ? ui_timestamp() : UI_TIMESTAMP::invalid();

		if (down)
		{
			++_button[button].DownCount;
		}
	}

	void Joystick::handleHatEvent(const SDL_JoyHatEvent &evt)
	{
		auto hat = evt.hat;

		Assertion(hat < numHats(), "SDL event contained invalid hat index!");

		std::bitset<4> hatset = evt.value;
		auto hatpos = convertSDLHat(evt.value);

		// Set current values
		_hat[hat].Value = hatpos;

		// Reset inactive hat positions
		for (auto i = 0; i < 4; ++i) {
			if (!hatset[i]) {
				_hat[hat].DownTimestamp4[i] = UI_TIMESTAMP::invalid();
			}
		}
		for (auto i = 0; i < 8; ++i) {
			if (i != hatpos) {
				_hat[hat].DownTimestamp8[i] = UI_TIMESTAMP::invalid();
			}
		}

		if (hatpos != HAT_CENTERED) {
			// Set the 4-pos timestamps and down counts if the timestamp is not set
			if ((hatset[HAT_DOWN]) && (!_hat[hat].DownTimestamp4[HAT_DOWN].isValid())) {
				_hat[hat].DownTimestamp4[HAT_DOWN] = ui_timestamp();
				++_hat[hat].DownCount4[HAT_DOWN];
			}
			if ((hatset[HAT_UP]) && (!_hat[hat].DownTimestamp4[HAT_UP].isValid())) {
				_hat[hat].DownTimestamp4[HAT_UP] = ui_timestamp();
				++_hat[hat].DownCount4[HAT_UP];
			}
			if ((hatset[HAT_LEFT]) && (!_hat[hat].DownTimestamp4[HAT_LEFT].isValid())) {
				_hat[hat].DownTimestamp4[HAT_LEFT] = ui_timestamp();
				++_hat[hat].DownCount4[HAT_LEFT];
			}
			if ((hatset[HAT_RIGHT]) && (!_hat[hat].DownTimestamp4[HAT_RIGHT].isValid())) {
				_hat[hat].DownTimestamp4[HAT_RIGHT] = ui_timestamp();
				++_hat[hat].DownCount4[HAT_RIGHT];
			}

			// Set the 8-pos timestamp and down count
			_hat[hat].DownTimestamp8[hatpos] = ui_timestamp();
			++_hat[hat].DownCount8[hatpos];
		}
	}

	void Joystick::handleBallEvent(const SDL_JoyBallEvent &evt)
	{
		auto ball = evt.ball;

		Assertion(ball < numBalls(), "SDL event contained invalid ball index!");

		coord2d newVal;
		newVal.x = evt.xrel;
		newVal.y = evt.yrel;

		_ballValues[ball] = newVal;
	}

	void Joystick::printInfo() {
		mprintf(("  Joystick name: %s\n", getName().c_str()));
		mprintf(("  Joystick GUID: %s\n", getGUID().c_str()));
		mprintf(("  Joystick ID: %d\n", getID()));
		mprintf(("  Joystick device ID: %d\n", _device_id));
	}

	json_t* Joystick::getJSON() {
		json_t* object;

		object = json_pack("{s:s, s:s, s:i, s:i, s:i, s:i, s:i, s:i, s:b}",
			"name", getName().c_str(),          // s:s
			"guid", getGUID().c_str(),          // s:s
			"id", static_cast<int>( getID() ),  // s:i
			"device", _device_id,               // s:i
			"num_axes", numAxes(),              // s:i
			"num_balls", numBalls(),            // s:i
			"num_buttons", numButtons(),        // s:i
			"num_hats", numHats(),              // s:i
			"is_haptic", _isHaptic              // s:b
		);

		return object;
	}

	int Joystick::getDeviceId() const {
		return _device_id;
	}

	bool init(bool no_register)
	{
		using namespace os::events;

		if (initialized)
		{
			return true;
		}

		mprintf(("Initializing Joystick...\n"));

		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
		{
			mprintf(("  Could not initialize joystick: %s\n", SDL_GetError()));
			return false;
		}

		// enable event processing of the joystick
		if ((SDL_JoystickEventState(SDL_ENABLE)) != SDL_ENABLE)
		{
			mprintf(("  ERROR: Unable to initialize joystick event processing!\n"));
			SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
			return false;
		}

		if (SDL_NumJoysticks() < 1)
		{
			mprintf(("  No joysticks found\n"));
		}

		// Get the initial list of connected joysticks
		enumerateJoysticks(joysticks);

		if (no_register) {
			// bail before registering anything
			return false;
		}

		// Register all the event handlers
		addEventListener(SDL_JOYAXISMOTION, DEFAULT_LISTENER_WEIGHT, axis_event_handler);
		addEventListener(SDL_JOYBALLMOTION, DEFAULT_LISTENER_WEIGHT, ball_event_handler);

		addEventListener(SDL_JOYBUTTONDOWN, DEFAULT_LISTENER_WEIGHT, button_event_handler);
		addEventListener(SDL_JOYBUTTONUP, DEFAULT_LISTENER_WEIGHT, button_event_handler);

		addEventListener(SDL_JOYHATMOTION, DEFAULT_LISTENER_WEIGHT, hat_event_handler);

		addEventListener(SDL_JOYDEVICEADDED, DEFAULT_LISTENER_WEIGHT, device_event_handler);
		addEventListener(SDL_JOYDEVICEREMOVED, DEFAULT_LISTENER_WEIGHT, device_event_handler);

		// Search for the correct stick
		if (Using_in_game_options)
		{
			// The new options system is in use
			setPlayerJoystick(JoystickOption->getValue(), 0);
			setPlayerJoystick(JoystickOption1->getValue(), 1);
			setPlayerJoystick(JoystickOption2->getValue(), 2);
			setPlayerJoystick(JoystickOption3->getValue(), 3);
		}
		else
		{
			// Fall back to the legacy config values
			for (auto& stick : joysticks)
			{
				for (short i = CID_JOY0; i < CID_JOY_MAX; ++i)
				{
					if (isPlayerJoystick(stick.get(), i))
					{
						// Joystick found
						setPlayerJoystick(stick.get(), i);
						break;
					}
				}
			}
		}

		bool used = false;
		for (const auto pJoy : pJoystick) {
			if (pJoy != nullptr) {
				used = true;
				break;
			}
		}
		if (!used) {
			mprintf(("  No joystick is being used.\n"));
		}

		initialized = true;

		joy_ff_init();

		return true;
	}

	size_t getJoystickCount()
	{
		return joysticks.size();
	}

	Joystick *getJoystick(size_t index)
	{
		Assertion(index < getJoystickCount(), "Invalid joystick index " SIZE_T_ARG "!", index);

		return joysticks[index].get();
	}

	Joystick *getPlayerJoystick(short cid)
	{
		if ((cid >= CID_JOY0) && (cid < CID_JOY_MAX))
		{
			return pJoystick[cid];
		}
		else
		{
			return nullptr;
		}
	}

	int getPlayerJoystickCount() {
		int count = 0;

		for (auto pJoy : pJoystick) {
			if (pJoy != nullptr) {
				count++;
			}
		}

		return count;
	}

	void shutdown()
	{
		joy_ff_shutdown();

		initialized = false;
		std::for_each(pJoystick.begin(), pJoystick.end(), [](Joystick*& pJoy){ pJoy = nullptr; });

		// Automatically frees joystick resources
		joysticks.clear();

		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	}

	json_t* getJsonArray() {
		// Do a full init of the SDL_JOYSTICK systems
		init();

		json_t* array = json_array();
		size_t len = 0;

		// Get the JSON info of each detected joystick and attach it to array
		for (auto& joystick : joysticks) {
			json_t* object = joystick->getJSON();

			if (object != nullptr) {
				json_array_append_new(array, object);
				len++;
			}
		}

		// Do a full de-init of the SDL_JOYSTICK systems
		shutdown();

		return array;
	}

	void printJoyJSON() {
		// Create root
		json_t* root = json_object();

		// Get Joystick data as JSON array
		json_t* array = getJsonArray();

		// Attach array to root
		json_object_set_new(root, "joysticks", array);

		// Dump to STDOUT
		json_dumpf(root, stdout, JSON_INDENT(4));

		shutdown();
	}
}
}

// Compatibility for the rest of the engine follows

int joystick_read_raw_axis(short cid, int num_axes, int *axis)
{
	int i;

	auto current = io::joystick::getPlayerJoystick(cid);
	if (!initialized || current == nullptr)
	{
		// fake a return value so that controlconfig doesn't get freaky with no joystick
		for (i = 0; i < num_axes; i++)
		{
			axis[i] = 32768;
		}
		return 0;
	}

	Assert(num_axes <= JOY_NUM_AXES);

	for (i = 0; i < num_axes; i++)
	{
		if (i < current->numAxes())
		{
			axis[i] = current->getAxis(i) + 32768;
		} else
		{
			axis[i] = 32768;
		}
	}

	return 1;
}

float joy_down_time(const CC_bind &bind)
{
	int btn = bind.get_btn();
	if (btn < 0) return 0.f;

	auto current = io::joystick::getPlayerJoystick(bind.get_cid());

	if (current == nullptr)
	{
		return 0.0f;
	}

	if (btn >= JOY_TOTAL_BUTTONS || (btn >= current->numButtons() && btn < JOY_NUM_BUTTONS))
	{
		// Not a valid button
		return 0.f;

	}
	else if (btn >= JOY_NUM_BUTTONS)
	{
		// Is hat
		if (current->numHats() > 0) {
			return current->getHatDownTime(0, hatBtnToEnum(btn), false);
		} else {
			// Don't have any hats to query
			return 0.0f;
		}

	} // Else, Is a button

	return current->getButtonDownTime(btn);
}

int joy_down_count(const CC_bind &bind, int reset_count)
{
	int btn = bind.get_btn();
	if (btn < 0) return 0;

	auto current = io::joystick::getPlayerJoystick(bind.get_cid());

	if (current == nullptr)
	{
		return 0;
	}

	if (btn >= JOY_TOTAL_BUTTONS || (btn >= current->numButtons() && btn < JOY_NUM_BUTTONS))
	{
		// Not a valid button
		return 0;

	}
	else if (btn >= JOY_NUM_BUTTONS)
	{
		// Is hat
		if (current->numHats() > 0) {
			return current->getHatDownCount(0, hatBtnToEnum(btn), false, reset_count != 0);
		} else {
			// Don't have any hats to query
			return 0;
		}

	} // Else, is a button

	return current->getButtonDownCount(btn, reset_count != 0);
}

int joy_down(const CC_bind &bind)
{
	return joy_down_time(bind) > 0.0f;
}

bool joy_present(short cid) {
	if ((cid < CID_JOY0) || (cid >= CID_JOY_MAX)) {
		return false;
	}

	return pJoystick[cid] != nullptr;
}

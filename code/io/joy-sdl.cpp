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
	auto guid = SDL_GetJoystickGUID(stick);

	const size_t GUID_STR_SIZE = 33;
	SCP_string joystickGUID;
	joystickGUID.resize(GUID_STR_SIZE);

	SDL_GUIDToString(guid, joystickGUID.data(), static_cast<int>(joystickGUID.size()));

	// Remove trailing \0
	joystickGUID.resize(GUID_STR_SIZE - 1);

	// Make sure the GUID is upper case
	SCP_toupper(joystickGUID);

	return joystickGUID;
}

bool joystickMatchesGuid(Joystick* testStick, const SCP_string& guid)
{
	if (guid.empty()) {
		return false;
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
	return false;
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

	SCP_string key_guid;

	// Select key strings according to cid
	switch (cid) {
		case CID_JOY0:
			key_guid = "Joy0GUID";
			break;
		case CID_JOY1:
			key_guid = "Joy1GUID";
			break;
		case CID_JOY2:
			key_guid = "Joy2GUID";
			break;
		case CID_JOY3:
			key_guid = "Joy3GUID";
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

	if ((cid == CID_JOY0) && GUID.empty()) {
		// Check legacy keys
		GUID = os_config_read_string(nullptr, "CurrentJoystickGUID", "");
	}

	return joystickMatchesGuid(testStick, GUID);
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
		// throw json_exception(err);
		// Changed by wookieejedi.
		// If errors detected then return nullptr (ie, no joystick),
		// because if we throw errors instead of returning nullptr
		// then the listening functions short circuits and does not run.
		return nullptr;
	}

	for (auto& test_stick : joysticks) {
		if (joystickMatchesGuid(test_stick.get(), guid)) {
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

	return json_pack("{sssi}", "guid", joystick->getGUID().c_str(), "id", -1);
}

/**
 * Assigns the given cid to the given stick
 *
 * @param[in] stick  The stick that is being assigned
 * @param[in] cid    The cid being assigned, must be a valid short between CID_JOY0 and CID_JOY_MAX
 */
void setPlayerJoystick(Joystick* stick, short cid)
{
	Assert((cid >= CID_JOY0) && (cid < CID_JOY_MAX));
	pJoystick[cid] = stick;

	if (cid == CID_JOY0) {
		joy_ff_shutdown();
	}

	if (pJoystick[cid] != nullptr) {
		mprintf(("  Using '%s' as Joy-%i\n", pJoystick[cid]->getName().c_str(), cid));
		mprintf(("\n"));
		mprintf(("  Is gamepad: %s\n", pJoystick[cid]->isGamepad() ? "YES" : "NO"));
		if (pJoystick[cid]->isGamepad()) {
			mprintf(("  Gamepad type: %d\n", SDL_GetGamepadType(pJoystick[cid]->getGamepad())));
		}
		mprintf(("  Number of axes: %d\n", pJoystick[cid]->numAxes()));
		mprintf(("  Number of buttons: %d\n", pJoystick[cid]->numButtons()));
		mprintf(("  Number of hats: %d\n", pJoystick[cid]->numHats()));
		mprintf(("  Number of trackballs: %d\n", pJoystick[cid]->numBalls()));
		mprintf(("\n"));

		if (cid == CID_JOY0) {
			joy_ff_init();
		}
	} else {
		mprintf((" Joystick %i removed\n", cid));
	}
}

/**
 * Scripting callback for displaying the given joystick's name
 */
SCP_string joystick_display(Joystick* stick)
{
	if (stick == nullptr) {
		return XSTR("None", 1673);
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
auto JoystickOption = options::OptionBuilder<Joystick*>("Input.Joystick",
                     std::pair<const char*, int>{"Joystick 0", 1705},
                     std::pair<const char*, int>{"The current joystick 0", 1706})
                     .category(std::make_pair("Input", 1827))                    // Category this option shows up in the scripting heirachy
                     .deserializer(joystick_deserialize)   // callback for json to C++
                     .serializer(joystick_serializer)      // callback for C++ to json
                     .display(joystick_display)            // callback for constructing the display label
                     .enumerator(joystick_enumerator)      // callback for enumerating/constructing the values this option may take
                     .level(options::ExpertLevel::Beginner)
                     .default_val(nullptr)                 // initial/default value for this option
                     .flags({options::OptionFlags::ForceMultiValueSelection})
                     .importance(100)
                     .change_listener([](Joystick* joy, bool) {
                         setPlayerJoystick(joy, CID_JOY0);
                         return true;
                     })
                     .finish();

auto JoystickOption1 = options::OptionBuilder<Joystick*>("Input.Joystick1",
                     std::pair<const char*, int>{"Joystick 1", 1707},
                     std::pair<const char*, int>{"The current joystick 1", 1708})
                     .category(std::make_pair("Input", 1827))
                     .deserializer(joystick_deserialize)
                     .serializer(joystick_serializer)
                     .display(joystick_display)
                     .enumerator(joystick_enumerator)
                     .level(options::ExpertLevel::Beginner)
                     .default_val(nullptr)
                     .flags({ options::OptionFlags::ForceMultiValueSelection })
                     .importance(90)
                     .change_listener([](Joystick* joy, bool) {
                         setPlayerJoystick(joy, CID_JOY1);
                         return true;
                     })
                     .finish();

auto JoystickOption2 = options::OptionBuilder<Joystick*>("Input.Joystick2",
                     std::pair<const char*, int>{"Joystick 2", 1709},
                     std::pair<const char*, int>{"The current joystick 2", 1710})
                     .category(std::make_pair("Input", 1827))
                     .deserializer(joystick_deserialize)
                     .serializer(joystick_serializer)
                     .display(joystick_display)
                     .enumerator(joystick_enumerator)
                     .level(options::ExpertLevel::Beginner)
                     .default_val(nullptr)
                     .flags({ options::OptionFlags::ForceMultiValueSelection })
                     .importance(80)
                     .change_listener([](Joystick* joy, bool) {
                         setPlayerJoystick(joy, CID_JOY2);
                         return true;
                     })
                     .finish();

auto JoystickOption3 = options::OptionBuilder<Joystick*>("Input.Joystick3",
                     std::pair<const char*, int>{"Joystick 3", 1711},
                     std::pair<const char*, int>{"The current joystick 3", 1712})
                     .category(std::make_pair("Input", 1827))
                     .deserializer(joystick_deserialize)
                     .serializer(joystick_serializer)
                     .display(joystick_display)
                     .enumerator(joystick_enumerator)
                     .level(options::ExpertLevel::Beginner)
                     .default_val(nullptr)
                     .flags({ options::OptionFlags::ForceMultiValueSelection })
                     .importance(70)
                     .change_listener([](Joystick* joy, bool) {
                         setPlayerJoystick(joy, CID_JOY3);
                         return true;
                     })
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
	int num;

	auto sticks = SDL_GetJoysticks(&num);

	outVec.clear();
	outVec.reserve(static_cast<size_t>(num));

	mprintf(("Printing joystick info:\n"));

	for (auto i = 0; i < num; ++i) {
			try
			{
				auto ptr = JoystickPtr(new Joystick(sticks[i]));
				ptr->printInfo();

				outVec.push_back(std::move(ptr));
			}
			catch (const std::exception &e)
			{
				mprintf(("  An error occured while attempting to enumerate joystick %i.\n", i));
				mprintf(("    %s\n", e.what()));
				mprintf(("    %s\n", SDL_GetError()));
				SDL_ClearError();
			}
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

	if (evtType == SDL_EVENT_JOYSTICK_ADDED)
	{
		// A new joystick has been added, add it to our list and check if it's the configured joystick
		Joystick *addedStick;
		{
			auto device = SDL_OpenJoystick(joyDeviceEvent.which);

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
	else if (evtType == SDL_EVENT_JOYSTICK_REMOVED)
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
	Joystick::Joystick(int id) :
		_id(id)
	{
		if (SDL_IsGamepad(id)) {
			_gamepad = SDL_OpenGamepad(id);
			_joystick = SDL_GetGamepadJoystick(_gamepad);
		} else {
			_joystick = SDL_OpenJoystick(id);
		}

		if (_joystick == nullptr) {
			SCP_stringstream msg;
			msg << "Failed to open joystick with id " << id << ", get a coder!";
			throw std::runtime_error(msg.str());
		}

		fillValues();
	}

	Joystick::Joystick(Joystick &&other) noexcept :
			_joystick(nullptr), _gamepad(nullptr)
	{
		*this = std::move(other);
	}

	Joystick::~Joystick()
	{
		if (_gamepad != nullptr)
		{
			SDL_CloseGamepad(_gamepad);	// also closes joystick
			_gamepad = nullptr;
			_joystick = nullptr;
		}
		else if (_joystick != nullptr)
		{
			SDL_CloseJoystick(_joystick);
			_joystick = nullptr;
		}
	}

	Joystick &Joystick::operator=(Joystick &&other) noexcept
	{
		if (this == &other)
			return *this;

		std::swap(_id, other._id);
		std::swap(_joystick, other._joystick);
		std::swap(_gamepad, other._gamepad);

		fillValues();

		return *this;
	}

	bool Joystick::isAttached() const
	{
		return isGamepad() ? SDL_GamepadConnected(_gamepad) : SDL_JoystickConnected(_joystick);
	}

	bool Joystick::isHaptic() const
	{
		return _isHaptic;
	}

	bool Joystick::isGamepad() const
	{
		return _isGamepad;
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
		// To avoid some weirdness and build compatiblity issues we always use
		// _joystick here rather than comparable _gamepad functions

		_name.assign(SDL_GetJoystickName(_joystick));
		_guidStr = getJoystickGUID(_joystick);
		_isHaptic = SDL_IsJoystickHaptic(_joystick);
		_isGamepad = SDL_IsGamepad(_id);

		// Initialize values of the axes
		if (_isGamepad) {
			// gamepads may not have all axes, but they don't necessarily match
			// the number or index of what's reported by the joystick api either
			_axisValues.resize(static_cast<size_t>(SDL_GAMEPAD_AXIS_COUNT));
			for (size_t i = 0; i < _axisValues.size(); ++i) {
				// will return 0 (center) if axis not supported
				_axisValues[i] = SDL_GetGamepadAxis(_gamepad, static_cast<SDL_GamepadAxis>(i));
			}
		} else {
			auto numSticks = SDL_GetNumJoystickAxes(_joystick);
			if (numSticks >= 0) {
				_axisValues.resize(static_cast<size_t>(numSticks));
				for (auto i = 0; i < numSticks; ++i) {
					_axisValues[i] = SDL_GetJoystickAxis(_joystick, i);
				}

			} else {
				_axisValues.resize(0);
				mprintf(("Failed to get number of axes for joystick %s: %s\n", _name.c_str(), SDL_GetError()));
			}
		}

		// Initialize ball values
		auto ballNum = SDL_GetNumJoystickBalls(_joystick);
		if (ballNum >= 0) {
			_ballValues.resize(static_cast<size_t>(ballNum));

			for (auto i = 0; i < ballNum; ++i) {
				coord2d coord;
				if ( !SDL_GetJoystickBall(_joystick, i, &coord.x, &coord.y) ) {
					mprintf(("Failed to get ball %d value for joystick %s: %s\n", i, _name.c_str(), SDL_GetError()));
				}
			}

		} else {
			mprintf(("Failed to get number of ball axes for joystick %s: %s\n", _name.c_str(), SDL_GetError()));
			_ballValues.resize(0);
		}

		// Initialize buttons
		if (_isGamepad) {
			// gamepads may not support all buttons, but they don't necessarily match
			// the number or index of what's reported by the joystick api either
			// NOTE: +2 added so we can map left & right triggers to a button
			_button.resize(static_cast<size_t>(SDL_GAMEPAD_BUTTON_COUNT + 2));
			for (size_t i = 0; i < _button.size(); ++i) {
				if (SDL_GetGamepadButton(_gamepad, static_cast<SDL_GamepadButton>(i))) {
					_button[i].DownTimestamp = ui_timestamp();
				} else {
					_button[i].DownTimestamp = UI_TIMESTAMP::invalid();
				}
			}
		} else {
			auto buttonNum = SDL_GetNumJoystickButtons(_joystick);
			if (buttonNum >= 0) {
				_button.resize(static_cast<size_t>(buttonNum));
				for (auto i = 0; i < buttonNum; ++i) {
					if (SDL_GetJoystickButton(_joystick, i)) {
						_button[i].DownTimestamp = ui_timestamp();
					} else {
						_button[i].DownTimestamp = UI_TIMESTAMP::invalid();
					}
				}

			} else {
				_button.resize(0);
				mprintf(("Failed to get number of buttons for joystick %s: %s\n", _name.c_str(), SDL_GetError()));
			}
		}

		// Initialize hats (consider gamepads to always have one hat)
		auto hatNum = _isGamepad ? 1 : SDL_GetNumJoystickHats(_joystick);
		if (hatNum >= 0) {
			_hat.resize(static_cast<size_t>(hatNum));
			for (auto i = 0; i < hatNum; ++i) {
				auto hatval = _isGamepad ? HAT_CENTERED : convertSDLHat(SDL_GetJoystickHat(_joystick, i));
				_hat[i].Value = hatval;

				// Reset timestampts
				for (auto j = 0; j < 4; ++j) {
					_hat[i].DownTimestamp4[j] = UI_TIMESTAMP::invalid();
				}
				for (auto j = 0; j < 8; ++j) {
					_hat[i].DownTimestamp8[j] = UI_TIMESTAMP::invalid();
				}

				if (_hat[i].Value != HAT_CENTERED) {
					std::bitset<4> hatset = SDL_GetJoystickHat(_joystick, i);

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
		} else {
			_hat.resize(0);
			mprintf(("Failed to get number of hats for joystick %s: %s", _name.c_str(), SDL_GetError()));
		}
	}

	SDL_Joystick *Joystick::getJoystick()
	{
		return _joystick;
	}

	SDL_Gamepad *Joystick::getGamepad()
	{
		return _gamepad;
	}

	void Joystick::handleJoyEvent(const SDL_Event &evt)
	{
		// gamepads also get joy events, so make sure we ignore those
		if (isGamepad()) {
			switch (evt.type) {
				case SDL_EVENT_GAMEPAD_AXIS_MOTION:
					handleAxisEvent(evt.gaxis);
					break;
				case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
				case SDL_EVENT_GAMEPAD_BUTTON_UP:
					handleButtonEvent(evt.gbutton);
					break;
				default:
					break;
			}
		} else {
			switch (evt.type)
			{
				case SDL_EVENT_JOYSTICK_AXIS_MOTION:
					handleAxisEvent(evt.jaxis);
					break;
				case SDL_EVENT_JOYSTICK_BALL_MOTION:
					handleBallEvent(evt.jball);
					break;
				case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
				case SDL_EVENT_JOYSTICK_BUTTON_UP:
					handleButtonEvent(evt.jbutton);
					break;
				case SDL_EVENT_JOYSTICK_HAT_MOTION:
					handleHatEvent(evt.jhat);
					break;
				default:
					break;
			}
		}
	}

	void Joystick::handleAxisEvent(const SDL_JoyAxisEvent &evt)
	{
		auto axis = evt.axis;

		Assertion(axis < numAxes(), "SDL event contained invalid axis index!");

		_axisValues[axis] = evt.value;
	}

	// gamepad version of event
	void Joystick::handleAxisEvent(const SDL_GamepadAxisEvent &evt)
	{
		auto axis = evt.axis;
		auto value = evt.value;

		Assertion(axis < numAxes(), "SDL event contained invalid axis index!");

		if ((axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER) || (axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)) {
			// Triggers range from 0..32767 so we need to scale the value for FSO
			// since it expects a full -32768..32767 range. Note that precision is
			// lost in the scaling so only scale if it's not a min/max trigger value
			if (value == 0) {
				value = -32768;
			} else if (value < 32767) {
				value = static_cast<decltype(value)>((value * 2) - 32768);
			}

			// We can also take this opportunity to map triggers to a button
			int button = SDL_GAMEPAD_BUTTON_COUNT + (axis - SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
			bool down = (value >= 0);

			Assertion(button < numButtons(), "Gamepad trigger button is out of bounds!");

			// We also need to avoid bumping DownCount for every tiny axis movement.
			if ( !down || (_button[button].DownCount == 0) ) {
				_button[button].DownTimestamp = down ? ui_timestamp() : UI_TIMESTAMP::invalid();

				if (down) {
					++_button[button].DownCount;
				}
			}
		}

		_axisValues[axis] = value;
	}

	void Joystick::handleButtonEvent(const SDL_JoyButtonEvent &evt)
	{
		auto button = evt.button;
		auto down = evt.down;

		Assertion(button < numButtons(), "SDL event contained invalid button index!");

		_button[button].DownTimestamp = down ? ui_timestamp() : UI_TIMESTAMP::invalid();

		if (down)
		{
			++_button[button].DownCount;
		}
	}

	// gamepad version of event (we deal with dpad->hat translation here too)
	void Joystick::handleButtonEvent(const SDL_GamepadButtonEvent &evt)
	{
		auto button = evt.button;
		auto down = evt.down;

		Assertion(button < numButtons(), "SDL event contained invalid button index!");

		// treat dpad as hat
		if (button >= SDL_GAMEPAD_BUTTON_DPAD_UP && button <= SDL_GAMEPAD_BUTTON_DPAD_RIGHT) {
			HatPosition hatpos;

			if (numHats() != 1) {
				return;
			}

			switch (button) {
				case SDL_GAMEPAD_BUTTON_DPAD_UP:
					hatpos = HAT_UP;
					break;
				case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
					hatpos = HAT_DOWN;
					break;
				case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
					hatpos = HAT_LEFT;
					break;
				case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
					hatpos = HAT_RIGHT;
					break;
				default:
					return;
			}

			// Set current values
			_hat[0].Value = hatpos;

			_hat[0].DownTimestamp4[hatpos] = down ? ui_timestamp() : UI_TIMESTAMP::invalid();
			_hat[0].DownTimestamp8[hatpos] = down ? ui_timestamp() : UI_TIMESTAMP::invalid();

			if (down) {
				++_hat[0].DownCount4[hatpos];
				++_hat[0].DownCount8[hatpos];
			}
		} else {
			_button[button].DownTimestamp = down ? ui_timestamp() : UI_TIMESTAMP::invalid();

			if (down) {
				++_button[button].DownCount;
			}
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
	}

	json_t* Joystick::getJSON() {
		json_t* object;

		object = json_pack("{s:s, s:s, s:i, s:i, s:i, s:i, s:i, s:i, s:b}",
			"name", getName().c_str(),          // s:s
			"guid", getGUID().c_str(),          // s:s
			"id", static_cast<int>( getID() ),  // s:i
			"device", -1,                       // s:i
			"num_axes", numAxes(),              // s:i
			"num_balls", numBalls(),            // s:i
			"num_buttons", numButtons(),        // s:i
			"num_hats", numHats(),              // s:i
			"is_haptic", _isHaptic              // s:b
		);

		return object;
	}

	bool init(bool no_register)
	{
		using namespace os::events;

		if (initialized)
		{
			return true;
		}

		mprintf(("Initializing Joystick...\n"));

		// NOTE: gamepad depends on joystick, so this handles both
		if ( !SDL_InitSubSystem(SDL_INIT_GAMEPAD) )
		{
			mprintf(("  Could not initialize joystick: %s\n", SDL_GetError()));
			return false;
		}

		if ( !SDL_HasJoystick() )
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
		addEventListener(SDL_EVENT_JOYSTICK_AXIS_MOTION, DEFAULT_LISTENER_WEIGHT, axis_event_handler);
		addEventListener(SDL_EVENT_JOYSTICK_BALL_MOTION, DEFAULT_LISTENER_WEIGHT, ball_event_handler);

		addEventListener(SDL_EVENT_JOYSTICK_BUTTON_DOWN, DEFAULT_LISTENER_WEIGHT, button_event_handler);
		addEventListener(SDL_EVENT_JOYSTICK_BUTTON_UP, DEFAULT_LISTENER_WEIGHT, button_event_handler);

		addEventListener(SDL_EVENT_JOYSTICK_HAT_MOTION, DEFAULT_LISTENER_WEIGHT, hat_event_handler);

		addEventListener(SDL_EVENT_JOYSTICK_ADDED, DEFAULT_LISTENER_WEIGHT, device_event_handler);
		addEventListener(SDL_EVENT_JOYSTICK_REMOVED, DEFAULT_LISTENER_WEIGHT, device_event_handler);

		// Gamepad events. NOTE: This is on top of joystick events, so both will be fired for gamepads!
		// (we can ignore add/remove events here since the normal joystick ones will do it)
		addEventListener(SDL_EVENT_GAMEPAD_AXIS_MOTION, DEFAULT_LISTENER_WEIGHT, axis_event_handler);
		addEventListener(SDL_EVENT_GAMEPAD_BUTTON_DOWN, DEFAULT_LISTENER_WEIGHT, button_event_handler);
		addEventListener(SDL_EVENT_GAMEPAD_BUTTON_UP, DEFAULT_LISTENER_WEIGHT, button_event_handler);

		// Search for the correct stick
		if (Using_in_game_options)
		{
			// The new options system is in use
			setPlayerJoystick(JoystickOption->getValue(), CID_JOY0);
			setPlayerJoystick(JoystickOption1->getValue(), CID_JOY1);
			setPlayerJoystick(JoystickOption2->getValue(), CID_JOY2);
			setPlayerJoystick(JoystickOption3->getValue(), CID_JOY3);
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

		SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
	}

	json_t* getJsonArray() {
		// Do a full init of the SDL_JOYSTICK systems
		init();

		json_t* array = json_array();

		// Get the JSON info of each detected joystick and attach it to array
		for (auto& joystick : joysticks) {
			json_t* object = joystick->getJSON();

			if (object != nullptr) {
				json_array_append_new(array, object);
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

// Check for special trigger buttons and return axis for button
// This is primarily for easier UI/UX in controlconfig (like conflict detection)
// returns axis or -1 if not an axis button
short joy_get_button_axis(short cid, short btn)
{
	auto current = io::joystick::getPlayerJoystick(cid);

	// gamepads only (for now?)
	if ( !current || !current->isGamepad() ) {
		return -1;
	}

	// the special trigger buttons are extended on normal buttons so we need to
	// remove those and sort out whether this is the left (0) or right (1) trigger
	auto axis_button = btn - SDL_GAMEPAD_BUTTON_COUNT;

	if (axis_button < 0 || axis_button > 1) {
		return -1;
	}

	return static_cast<short>(SDL_GAMEPAD_AXIS_LEFT_TRIGGER + axis_button);
}

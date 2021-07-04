#include "io/joy.h"
#include "io/joy_ff.h"
#include "io/timer.h"
#include "mod_table/mod_table.h"
#include "options/Option.h"
#include "osapi/osapi.h"

#include <algorithm>
#include <bitset>
#include <memory>
#include <utility>

#include "libs/jansson.h"

using namespace io::joystick;
using namespace os::events;

namespace {

typedef std::unique_ptr<Joystick> JoystickPtr;

SCP_vector<JoystickPtr> joysticks;
Joystick *currentJoystick = nullptr;

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

bool isCurrentJoystick(Joystick* testStick)
{
	const auto currentGUID = os_config_read_string(nullptr, "CurrentJoystickGUID", "");
	const auto currentId   = static_cast<int>(os_config_read_uint(nullptr, "CurrentJoystick", 0));

	return joystickMatchesGuid(testStick, currentGUID, currentId);
}

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

json_t* joystick_serializer(Joystick* joystick)
{
	if (joystick == nullptr) {
		return json_null();
	}

	return json_pack("{sssi}", "guid", joystick->getGUID().c_str(), "id", joystick->getID());
}

SCP_string joystick_display(Joystick* stick)
{
	if (stick == nullptr) {
		return "None";
	}
	return stick->getName();
}

SCP_vector<Joystick*> joystick_enumerator()
{
	SCP_vector<Joystick*> out;
	out.push_back(nullptr);
	for (auto& stick : joysticks) {
		out.push_back(stick.get());
	}
	return out;
}

auto JoystickOption = options::OptionBuilder<Joystick*>("Input.Joystick", "Joystick", "The current joystick.")
                          .category("Input")
                          .deserializer(joystick_deserialize)
                          .serializer(joystick_serializer)
                          .display(joystick_display)
                          .enumerator(joystick_enumerator)
                          .level(options::ExpertLevel::Beginner)
                          .default_val(nullptr)
                          .flags({options::OptionFlags::ForceMultiValueSelection})
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

void setCurrentJoystick(Joystick *stick)
{
	currentJoystick = stick;

	if (currentJoystick != nullptr)
	{
		mprintf(("  Using '%s' as the primary joystick\n", currentJoystick->getName().c_str()));
		mprintf(("\n"));
		mprintf(("  Number of axes: %d\n", currentJoystick->numAxes()));
		mprintf(("  Number of buttons: %d\n", currentJoystick->numButtons()));
		mprintf(("  Number of hats: %d\n", currentJoystick->numHats()));
		mprintf(("  Number of trackballs: %d\n", currentJoystick->numBalls()));
	}
	else
	{
		mprintf((" Using no joystick.\n"));
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

		if (currentJoystick != nullptr)
		{
			// We already have a valid joystick device, ignore any further events
			return true;
		}

		if (Using_in_game_options) {
			auto value = JoystickOption->getValue();

			// Since the new stick has been added to the joystick list, the option will return the same pointer here
			if (value == addedStick) {
				setCurrentJoystick(addedStick);
			}
		} else {
			if (isCurrentJoystick(addedStick)) {
				// found our wanted stick!
				setCurrentJoystick(addedStick);
			}
		}
	}
	else if (evtType == SDL_JOYDEVICEREMOVED)
	{
		if (currentJoystick != nullptr && joyDeviceEvent.which == currentJoystick->getID())
		{
			// We just lost our joystick, reset the data
			setCurrentJoystick(nullptr);
		}
	}

	return true;
}
}

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

		return _button[index].DownTimestamp >= 0;
	}

	float Joystick::getButtonDownTime(int index) const
	{
		Assertion(index >= 0 && index < numButtons(), "Invalid index %d!", index);

		if (_button[index].DownTimestamp >= 0)
		{
			auto diff = timer_get_milliseconds() - _button[index].DownTimestamp;

			return static_cast<float>(diff) / 1000.f;
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

		int val;

		if (ext) {
			// Use 8-position
			val = _hat[index].DownTimestamp8[pos];

		} else {
			// Use 4-position
			val = _hat[index].DownTimestamp4[pos];
		}

		if (val < 0) {
			return 0.0f;
		}

		auto diff = timer_get_milliseconds() - val;

		return static_cast<float>(diff) / 1000.f;
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
				_button[i].DownTimestamp = timer_get_milliseconds();
			}
			else
			{
				_button[i].DownTimestamp = -1;
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
				_hat[i].DownTimestamp4[j] = -1;
			}
			for (auto j = 0; j < 8; ++j) {
				_hat[i].DownTimestamp8[j] = -1;
			}

			if (_hat[i].Value != HAT_CENTERED)
			{
				// Set the 4-pos timestamp(s)
				if ((hatset[HAT_DOWN])) {
					_hat[i].DownTimestamp4[HAT_DOWN] = timer_get_milliseconds();
				}
				if ((hatset[HAT_UP])) {
					_hat[i].DownTimestamp4[HAT_UP] = timer_get_milliseconds();
				}
				if ((hatset[HAT_LEFT])) {
					_hat[i].DownTimestamp4[HAT_LEFT] = timer_get_milliseconds();
				}
				if ((hatset[HAT_RIGHT])) {
					_hat[i].DownTimestamp4[HAT_RIGHT] = timer_get_milliseconds();
				}

				// Set the 8-pos timestamp
				_hat[i].DownTimestamp8[hatval] = timer_get_milliseconds();
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

		_button[button].DownTimestamp = down ? timer_get_milliseconds() : -1;

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
				_hat[hat].DownTimestamp4[i] = -1;
			}
		}
		for (auto i = 0; i < 8; ++i) {
			if (i != hatpos) {
				_hat[hat].DownTimestamp8[i] = -1;
			}
		}

		if (hatpos != HAT_CENTERED) {
			// Set the 4-pos timestamps and down counts if the timestamp is not set
			if ((hatset[HAT_DOWN]) && (_hat[hat].DownTimestamp4[HAT_DOWN] == -1)) {
				_hat[hat].DownTimestamp4[HAT_DOWN] = timer_get_milliseconds();
				++_hat[hat].DownCount4[HAT_DOWN];
			}
			if ((hatset[HAT_UP]) && (_hat[hat].DownTimestamp4[HAT_UP] == -1)) {
				_hat[hat].DownTimestamp4[HAT_UP] = timer_get_milliseconds();
				++_hat[hat].DownCount4[HAT_UP];
			}
			if ((hatset[HAT_LEFT]) && (_hat[hat].DownTimestamp4[HAT_LEFT] == -1)) {
				_hat[hat].DownTimestamp4[HAT_LEFT] = timer_get_milliseconds();
				++_hat[hat].DownCount4[HAT_LEFT];
			}
			if ((hatset[HAT_RIGHT]) && (_hat[hat].DownTimestamp4[HAT_RIGHT] == -1)) {
				_hat[hat].DownTimestamp4[HAT_RIGHT] = timer_get_milliseconds();
				++_hat[hat].DownCount4[HAT_RIGHT];
			}

			// Set the 8-pos timestamp and down count
			_hat[hat].DownTimestamp8[hatpos] = timer_get_milliseconds();
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
	int Joystick::getDeviceId() const {
		return _device_id;
	}

	bool init()
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

		// Register all the event handlers
		addEventListener(SDL_JOYAXISMOTION, DEFAULT_LISTENER_WEIGHT, axis_event_handler);
		addEventListener(SDL_JOYBALLMOTION, DEFAULT_LISTENER_WEIGHT, ball_event_handler);

		addEventListener(SDL_JOYBUTTONDOWN, DEFAULT_LISTENER_WEIGHT, button_event_handler);
		addEventListener(SDL_JOYBUTTONUP, DEFAULT_LISTENER_WEIGHT, button_event_handler);

		addEventListener(SDL_JOYHATMOTION, DEFAULT_LISTENER_WEIGHT, hat_event_handler);

		addEventListener(SDL_JOYDEVICEADDED, DEFAULT_LISTENER_WEIGHT, device_event_handler);
		addEventListener(SDL_JOYDEVICEREMOVED, DEFAULT_LISTENER_WEIGHT, device_event_handler);

		// Search for the correct stick
	    if (Using_in_game_options) {
		    // The new options system is in use
		    setCurrentJoystick(JoystickOption->getValue());
	    } else {
		    // Fall back to the legacy config values
		    for (auto& stick : joysticks) {
			    if (isCurrentJoystick(stick.get())) {
				    // Joystick found
				    setCurrentJoystick(stick.get());
				    break;
			    }
		    }

		    if (currentJoystick == nullptr) {
			    mprintf(("  No joystick is being used.\n"));
		    }
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

	Joystick *getCurrentJoystick()
	{
		return currentJoystick;
	}

	void shutdown()
	{
		joy_ff_shutdown();

		initialized = false;
		currentJoystick = nullptr;
		// Automatically frees joystick resources
		joysticks.clear();

		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	}

	SCP_vector<JoystickInformation> getJoystickInformations() {
		SCP_vector<JoystickInformation> joystickInfo;

		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0) {
			return SCP_vector<JoystickInformation>();
		}

		auto num_joysticks = SDL_NumJoysticks();
		for (auto i = 0; i < num_joysticks; ++i) {
			auto joystick = SDL_JoystickOpen(i);

			JoystickInformation info{};

			auto name = SDL_JoystickName(joystick);
			if (name != nullptr) {
				info.name = name;
			}
			info.guid = getJoystickGUID(joystick);

			info.num_axes = static_cast<uint32_t>(SDL_JoystickNumAxes(joystick));
			info.num_balls = static_cast<uint32_t>(SDL_JoystickNumBalls(joystick));
			info.num_buttons = static_cast<uint32_t>(SDL_JoystickNumButtons(joystick));
			info.num_hats = static_cast<uint32_t>(SDL_JoystickNumHats(joystick));

			info.is_haptic = SDL_JoystickIsHaptic(joystick) == SDL_TRUE;

			joystickInfo.push_back(info);

			SDL_JoystickClose(joystick);
		}

		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);

		return joystickInfo;
	}
}
}

// Compatibility for the rest of the engine follows

int joystick_read_raw_axis(int num_axes, int *axis)
{
	int i;

	auto current = io::joystick::getCurrentJoystick();
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

float joy_down_time(int btn)
{
	if (btn < 0) return 0.f;

	auto current = io::joystick::getCurrentJoystick();

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

int joy_down_count(int btn, int reset_count)
{
	if (btn < 0) return 0;

	auto current = io::joystick::getCurrentJoystick();

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

int joy_down(int btn)
{
	return joy_down_time(btn) > 0.0f;
}

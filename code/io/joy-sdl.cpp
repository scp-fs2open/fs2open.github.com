#include "io/joy.h"
#include "io/timer.h"
#include "osapi/osapi.h"

#include <memory>
#include <utility>
#include <algorithm>

using namespace io::joystick;
using namespace os::events;

namespace {
typedef std::unique_ptr<Joystick> JoystickPtr;

SCP_vector<JoystickPtr> joysticks;
Joystick *currentJoystick = nullptr;

bool initialized = false;

/**
 * @brief Compatibility conversion from HatPosition to array index
 */
inline
int hatEnumToIdx(HatPosition in) {
	return static_cast<int>(in - JOY_NUM_BUTTONS);
}

/**
* @brief Compatibility conversion from array index to HatPosition
*/
inline
HatPosition hatIdxToEnum(int in) {
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

SCP_string getJoystickGUID(SDL_Joystick *stick)
{
	auto guid = SDL_JoystickGetGUID(stick);

	const size_t GUID_STR_SIZE = 33;
	SCP_string joystickGUID;
	joystickGUID.resize(GUID_STR_SIZE);

	SDL_JoystickGetGUIDString(guid, &joystickGUID[0], static_cast<int>(joystickGUID.size()));

	// Remove trailing \0
	joystickGUID.resize(GUID_STR_SIZE - 1);

	// Make sure the GUID is upper case
	transform(begin(joystickGUID), end(joystickGUID), begin(joystickGUID), toupper);

	return joystickGUID;
}

SCP_string getCurrentJoystickGUID()
{
	SCP_string guidStr(os_config_read_string(nullptr, "CurrentJoystickGUID", ""));

	// Make sure we get upper case strings
	transform(begin(guidStr), end(guidStr), begin(guidStr), toupper);

	return guidStr;
}

void enumerateJoysticks(SCP_vector<JoystickPtr>& outVec)
{
	auto num = SDL_NumJoysticks();
	outVec.clear();
	outVec.reserve(static_cast<size_t>(num));

	for (auto i = 0; i < num; ++i)
	{
		auto ptr = JoystickPtr(new Joystick(SDL_JoystickOpen(i)));

		outVec.push_back(std::move(ptr));
	}
}

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
			auto added = JoystickPtr(new Joystick(device));

			for (auto iter = joysticks.begin(); iter != joysticks.end(); ++iter)
			{
				if ((*iter)->getGUID() == added->getGUID())
				{
					// Already have this stick
					*(*iter) = std::move(*added);
					return true;
				}
			}

			addedStick = added.get();

			joysticks.push_back(std::move(added));
		}

		if (currentJoystick != nullptr)
		{
			// We already have a valid joystick device, ignore any further events
			return true;
		}

		auto guid = addedStick->getGUID();

		if (guid == getCurrentJoystickGUID())
		{
			// found our wanted stick!
			setCurrentJoystick(addedStick);
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
	Joystick::Joystick(SDL_Joystick *joystick) :
			_joystick(joystick)
	{
		Assertion(joystick != nullptr, "Invalid joystick pointer passed!");

		fillValues();
	}

	Joystick::Joystick(Joystick &&other) :
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

	Joystick &Joystick::operator=(Joystick &&other)
	{
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

	int Joystick::getHatDownCount(int index, int pos, bool reset) {
		Assertion(index >= 0 && index < numHats(), "Invalid index %d!", index);
		Assertion(pos >= 0 && pos < HAT_NUM_POS, "Invalid hat position %d!", pos);

		auto val = _hat[index].DownCount[pos];

		if (reset) {
			_hat[index].DownCount[pos] = 0;
		}

		return val;
	}

	float Joystick::getHatDownTime(int index, int pos) const
	{
		Assertion(index >= 0 && index < numHats(), "Invalid index %d!", index);
		Assertion(pos >= 0 && pos < HAT_NUM_POS, "Invalid hat position %d!", pos);

		if ((_hat[index].Value == HAT_CENTERED) || (_hat[index].DownTimestamp < 0)) {
			// Hat is inactive
			return 0.0f;

		} else if (_hat[index].Value != hatIdxToEnum(pos)) {
			// Hat is active, but not in this position
			return 0.0f;

		} // Else Hat is active in this position

		auto diff = timer_get_milliseconds() - _hat[index].DownTimestamp;

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
				mprintf(("Failed to get ball %d value for joystick %s: %s", i, _name.c_str(), SDL_GetError()));
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
			_hat[i].Value = convertSDLHat(SDL_JoystickGetHat(_joystick, i));

			if (_hat[i].Value != HAT_CENTERED)
			{
				_hat[i].DownTimestamp = timer_get_milliseconds();
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

		auto hatpos = convertSDLHat(evt.value);
		_hat[hat].DownTimestamp = (hatpos != HAT_CENTERED) ? timer_get_milliseconds() : -1;
		_hat[hat].Value = hatpos;

		if (hatpos != HAT_CENTERED)
		{
			++_hat[hat].DownCount[hatEnumToIdx(hatpos)];
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

		auto configGUID = getCurrentJoystickGUID();

		// If there is a GUID in the settings then that joystick should be used
		if (!configGUID.empty())
		{
			for (auto iter = joysticks.begin(); iter != joysticks.end(); ++iter)
			{
				if ((*iter)->getGUID() == configGUID)
				{
					setCurrentJoystick((*iter).get());
					break;
				}
			}

			if (currentJoystick == nullptr)
			{
				mprintf(("  Couldn't find requested joystick GUID %s!\n", configGUID.c_str()));
			}
		}
		else
		{
			// Old joystick configuration, this will likely not match the list of joysticks
			// in the launcher but it's better than nothing...
			auto joystickID = os_config_read_uint(NULL, "CurrentJoystick", 0);

			if (joystickID >= static_cast<uint>(joysticks.size()))
			{
				mprintf(("Found invalid joystick index %u!", joystickID));
			}
			else
			{
				setCurrentJoystick(getJoystick(joystickID));
			}
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

	Joystick *getCurrentJoystick()
	{
		return currentJoystick;
	}

	void shutdown()
	{
		initialized = false;
		currentJoystick = nullptr;
		// Automatically frees joystick resources
		joysticks.clear();

		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
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

	if (btn >= JOY_TOTAL_BUTTONS)
	{
		// Not a valid button
		return 0.f;

	}
	else if (btn >= JOY_NUM_BUTTONS)
	{
		// Is hat
		btn -= JOY_NUM_BUTTONS;

		return current->getHatDownTime(0, btn);

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

	if (btn >= JOY_TOTAL_BUTTONS)
	{
		// Not a valid button
		return 0;

	}
	else if (btn >= JOY_NUM_BUTTONS)
	{
		// Is hat
		btn -= JOY_NUM_BUTTONS;

		return current->getHatDownCount(0, btn, reset_count != 0);

	} // Else, is a button

	return current->getButtonDownCount(btn, reset_count != 0);
}

int joy_down(int btn)
{
	if (btn < 0) return 0;

	auto current = io::joystick::getCurrentJoystick();

	if (current == nullptr)
	{
		return 0;
	}

	if (btn >= JOY_TOTAL_BUTTONS)
	{
		// Not a valid button
		return 0;

	}
	else if (btn >= JOY_NUM_BUTTONS)
	{
		// Is hat
		btn -= JOY_NUM_BUTTONS;

		return (current->getHatPosition(0) == btn) ? 1 : 0;
	} // Else, is a button


	return current->isButtonDown(btn) ? 1 : 0;
}

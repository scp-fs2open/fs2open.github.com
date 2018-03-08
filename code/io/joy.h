/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
 */

#ifndef __JOY_H__
#define __JOY_H__

#include "globalincs/pstypes.h"

#include "SDL_joystick.h"

// z64: Moved up here for compatibility. Bye bye, organization!
const int JOY_NUM_BUTTONS = 32;
const int JOY_NUM_HAT_POS = 4;
const int JOY_TOTAL_BUTTONS = (JOY_NUM_BUTTONS + JOY_NUM_HAT_POS);

namespace io
{
	/**
	 * @brief Namespace containing all joystick API functions
	 */
	namespace joystick
	{
		/**
		 * @brief A hat position
		 * @note Order here is based on SDL bit positions
		 */
		enum HatPosition
		{
			HAT_CENTERED = -1,
			HAT_UP = 0,
			HAT_RIGHT,
			HAT_DOWN,
			HAT_LEFT,
			HAT_RIGHTUP,
			HAT_RIGHTDOWN,
			HAT_LEFTUP,
			HAT_LEFTDOWN
		};

		/**
		 * @brief Represents a SDL joystick
		 *
		 * @details The implementation is event based so repeatedly polling the values will
		 * not cause a performance penalty as the joystick values will be cached.
		 */
		class Joystick
		{
		public:
			/**
			 * @brief Constructs a new joystick instance from a SDL handle
			 *
			 * This object will take ownership of the passed SDL handle and it will be freed when this
			 * object is deleted.
			 *
			 * @param device_id The SDL device index
			 */
			explicit Joystick(int device_id);

			/**
			 * @brief Moves the resources of the other object into @c this
			 *
			 * @param other A rvalue reference
			 */
			Joystick(Joystick &&other);

			/**
			 * @brief Frees the owned SDL handle
			 */
			~Joystick();

			/**
			 * @brief Moves the resources of the other object into @c this
			 *
			 * @param other A rvalue reference to the other object
			 */
			Joystick &operator=(Joystick &&other);

			/**
			 * @brief Determines if this joystick is still connected to the computer
			 *
			 * @return @c true if the joystick is attached, @c false otherwise
			 */
			bool isAttached() const;

			/**
			 * @brief Gets the value of the specified axis
			 *
			 * @param index The index of the axis, must be in [0, numAxes())
			 * @return The axis value, in range [-2^16, 2^16-1]
			 */
			Sint16 getAxis(int index) const;

			/**
			 * @brief Determines if this button is currently pressed
			 *
			 * @param index The index of the button, must be in [0, numButtons())
			 *
			 * @return @c true if the button is currently pressed
			 */
			bool isButtonDown(int index) const;

			/**
			 * @brief Gets the time the button has been pressed
			 *
			 * @param index The index of the button, must be in [0, numButtons())
			 *
			 * @return How long (in seconds) the button has been pressed.
			 */
			float getButtonDownTime(int index) const;

			/**
			 * @brief Times the specified button has been pressed since the last reset
			 * @param index The index of the button, must be in [0, numButtons())
			 * @param reset @c true to reset the count
			 * @return The number of times the button has been pressed
			 *
			 * @warning This function may be removed in the future, it's only here for compatibility with the
			 * current code base.
			 */
			int getButtonDownCount(int index, bool reset);

			/**
			 * @brief Gets the relative ball movement
			 * @param index The index of the ball, must be in [0, numBalls())
			 * @return The @c x and @c y movement of the ball
			 */
			coord2d getBall(int index) const;

			/**
			 * @brief Gets the current position of the hat.
			 * @param index The index of the hat to query, must be in [0, numHats())
			 * @return The hat position
			 */
			HatPosition getHatPosition(int index) const;

			/**
			 * @brief Gets the down time of the given hat and position
			 * @param[in] index The index of the hat to query, must be in [0, numHats())
			 * @param[in] pos The position to query, must be in [0, JOY_NUM_HAT_POS)
			 * @param[in] ext If @c false, uses the four-position hat values. If @c ture, uses the eight-position hat values
			 *
			 * @returns 0.0f If the queried hat positions is not active, or
			 * @returns The time, in seconds, that the hat has been active in that position
			 */
			float getHatDownTime(int index, HatPosition pos, bool ext) const;

			/**
			* @brief Times the specified button has been pressed since the last reset
			* @param[in] index The index of the button, must be in [0, numHats())
			* @param[in] pos The position to query, must be in [0, JOY_NUM_HAT_POS)
			* @param[in] ext If @c false, uses the four-position hat values. If @c true, uses the eight-position hat values
			* @param reset @c true to reset the count
			* @return The number of times the button has been pressed
			*
			* @warning This function may be removed in the future, it's only here for compatibility with the
			* current code base.
			*/
			int getHatDownCount(int index, HatPosition pos, bool ext, bool reset);

			/**
			 * @brief Gets the number of axes on this joystick
			 * @return The number of axes
			 */
			int numAxes() const;

			/**
			 * @brief Gets the number of balls on this joystick
			 * @return The number of balls
			 */
			int numBalls() const;

			/**
			 * @brief Gets the number of buttons on this joystick
			 * @return The number of buttons
			 */
			int numButtons() const;

			/**
			 * @brief Gets the number of hats on this joystick
			 * @return The number of hats
			 */
			int numHats() const;

			/**
			 * @brief Gets a globally unique identifier of this joystick
			 * This is useful for identifying a joystick if it has been disconnected at some point
			 * @return The string representation of the GUID
			 */
			SCP_string getGUID() const;

			/**
			 * @brief Gets the name of the joystick
			 * @return The name
			 */
			SCP_string getName() const;

			/**
			 * @brief Gets the unique instance ID of the joystick
			 * @return The instance ID
			 */
			SDL_JoystickID getID() const;

			/**
			 * @brief Gets the device index of this joystick
			 * @return The device index
			 */
			int getDeviceId() const;

			/**
			 * @brief The SDL joystick handle
			 * @return The handle
			 */
			SDL_Joystick* getDevice();

			/**
			 * @brief Handles a SDL joystick event
			 * @param evt The event to handle
			 *
			 * @note Only for internal use, don't call externally
			 */
			void handleJoyEvent(const SDL_Event &evt);

			void printInfo();
		private:
			Joystick(const Joystick &);
			Joystick &operator=(const Joystick &);

			/**
			 * @brief Fills internal array
			 */
			void fillValues();

			void handleAxisEvent(const SDL_JoyAxisEvent& evt);
			void handleBallEvent(const SDL_JoyBallEvent& evt);
			void handleButtonEvent(const SDL_JoyButtonEvent& evt);
			void handleHatEvent(const SDL_JoyHatEvent& evt);

			int _device_id; //!< The SDL device index
			SDL_Joystick *_joystick; //!< The SDL joystick handle

			SCP_string _guidStr; //!< The GUID string
			SCP_string _name; //!< The joystick name

			SDL_JoystickID _id; //!< The instance ID

			SCP_vector<Sint16> _axisValues; //!< The current axes values
			SCP_vector<coord2d> _ballValues; //!< The ball values
			

			struct button_info
			{
				int DownTimestamp;  //!< The timestamp since when the button is pressed, -1 if not pressed
				int DownCount;      //!< The number of times the button was pressed
			};

			SCP_vector<button_info> _button;

			struct hat_info
			{
				HatPosition Value;      //!< The current hat position

				int DownTimestamp4[4];  //!< The timestamp when each 4-hat position was last hit, -1 if inactive.
				int DownTimestamp8[8];  //!< The timestamp when each 8-hat posision was last hit, -1 if inactive.

				int DownCount4[4];      //!< The number of times each 4-hat position has been hit since we last checked.
				int DownCount8[8];      //!< The number of times each 8-hat position has been hit since we last checked.
			};

			SCP_vector<hat_info> _hat;
		};

		/**
		 * @brief Initializes the joystick subsystem
		 * @return @c true when successfully initialized
		 */
		bool init();

		/**
		 * @brief Gets number of connected joysticks
		 * This value can change if new devices are connected to the system
		 * @return The number of joysticks
		 */
		size_t getJoystickCount();

		/**
		 * @brief Gets the joystick with the specified index
		 * The returned pointer stays valid even if a device is disconnected in case it is reconnected at a later time.
		 * @param index The index, must be in range [0, getJoystickCount())
		 * @return The Joystick pointer, the pointer is owned by the joystick subsystem
		 */
		Joystick *getJoystick(size_t index);

		/**
		 * @brief Gets the primary joystick
		 * @return The joystick pointer, may be @c nullptr of no primary joystick
		 *
		 * @warning This function may be removed in the future when multi-joystick support is implemented
		 */
		Joystick *getCurrentJoystick();

		/**
		 * @brief Frees resources of the joystick subsystem
		 */
		void shutdown();

		struct JoystickInformation {
			SCP_string name;
			SCP_string guid;

			uint32_t num_axes;
			uint32_t num_balls;
			uint32_t num_buttons;
			uint32_t num_hats;

			bool is_haptic;
		};

		SCP_vector<JoystickInformation> getJoystickInformations();
	}
}

// For now this is a constant for the rest of the engine
const int JOY_NUM_AXES = 6;

const int JOY_AXIS_MIN = 0;
const int JOY_AXIS_CENTER = 32768;
const int JOY_AXIS_MAX = 65536;

int joystick_read_raw_axis(int num_axes, int *axis);

float joy_down_time(int btn);

int joy_down_count(int btn, int reset_count);

int joy_down(int btn);


#endif	/* __JOY_H__ */

#include "globalincs/pstypes.h"

class camera
{
private:
	vector	position;
	vector	desired_position;
	vector	translation_velocity;
	vector	translation_velocity_delta;
	float	translation_time_left;
	float	translation_time;

	matrix	orientation;
	matrix	desired_orientation;
	angles	rotation_rate;
	angles	rotation_rate_delta;
	float	rotation_time_left;
	float	rotation_time;
public:
	camera(){reset();}
	void reset();

	//Set
	void set_position(vector *in_position);
	void set_translation_velocity(vector *in_velocity, float in_translation_time = 0.0f);
	void set_rotation(matrix *in_orientation, float in_rotation_time = 0.0f, float in_rotation_acceleration_time = 0.0f);
	void set_rotation(angles *in_angles, float in_rotation_time = 0.0f, float in_rotation_acceleration_time = 0.0f);
	void set_rotation_facing(vector *in_target, float in_rotation_time = 0.0f, float in_rotation_acceleration_time = 0.0f);
	void set_rotation_rate(angles *in_rotation_rate, float in_rotation_time = 0.0f);

	//Get
	vector *get_position() {return &position;}
	matrix *get_orientation() {return &orientation;}

	//Do
	void do_movement(float in_frametime);
};

//Some global cameras
extern camera free_camera;
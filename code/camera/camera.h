#include "globalincs/pstypes.h"

class camera
{
private:
	vector	position;
	vector	*desired_position;		//NULL unless we're trying to move somewhere
	vector	translation_velocity;
	vector	translation_vel_limit;
	vector	translation_acc_limit;

	matrix	orientation;
	matrix	*desired_orientation;	//NULL unless we're trying to point somewhere
	vector	rotation_rate;
	vector	rotation_vel_limit;
	vector	rotation_acc_limit;
public:
	camera();
	~camera();
	void reset();

	//Set
	void set_position(vector *in_position, float in_translation_time = 0.0f, float in_translation_acceleration_time = 0.0f);
	void set_translation_velocity(vector *in_velocity);
	void set_rotation(matrix *in_orientation, float in_rotation_time = 0.0f, float in_rotation_acceleration_time = 0.0f);
	void set_rotation(angles *in_angles, float in_rotation_time = 0.0f, float in_rotation_acceleration_time = 0.0f);
	void set_rotation_facing(vector *in_target, float in_rotation_time = 0.0f, float in_rotation_acceleration_time = 0.0f);
	void set_rotation_velocity(vector *in_rotation_rate);

	//Get
	//These return values should never ever be changed, I do this in the interest of speed.
	vector *get_position() {return &position;}
	matrix *get_orientation() {return &orientation;}

	//Do
	void do_movement(float in_frametime);
};

//Some global cameras
extern camera free_camera;
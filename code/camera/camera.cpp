#include "camera/camera.h"
#include "math/vecmat.h"

//Some global cameras
camera free_camera;

//This is where the camera class beings! :D
void camera::reset()
{
	memset(&position, 0, sizeof(position));
	desired_position = position;
	memset(&translation_velocity, 0, sizeof(translation_velocity));
	memset(&translation_velocity_delta, 0, sizeof(translation_velocity_delta));
	translation_time_left = 0.0f;

	vm_set_identity(&orientation);
	desired_orientation = orientation;
	memset(&rotation_rate, 0, sizeof(rotation_rate));
	memset(&rotation_rate_delta, 0, sizeof(rotation_rate_delta));
	rotation_time_left = 0.0f;
}

void camera::set_position(vector *in_position)
{
	position = *in_position;

}

void camera::set_translation_velocity(vector *in_velocity, float in_translation_time)
{
	if ( in_translation_time < 0.0001f )	{
		translation_velocity = *in_velocity;
		return;
	}
	translation_time_left = in_translation_time;

	vm_vec_copy_scale(&translation_velocity_delta, in_velocity, (1.0f/in_translation_time));
}

#define square(a) (a*a)
void camera::set_rotation(matrix *in_orientation, float in_rotation_time, float in_rotation_acceleration_time)
{
	orientation = *in_orientation;
	
	if(in_rotation_time > 0.0f || in_rotation_acceleration_time > 0.0f)
	{
		if(in_rotation_acceleration_time <= 0.0f)
			in_rotation_acceleration_time = 1.0f;
		if(in_rotation_time < 0.0f)
			in_rotation_time = 3.0f;

		angles curr_orientation, new_orientation, new_delta;
		vm_extract_angles_matrix(&curr_orientation, &orientation);
		vm_extract_angles_matrix(&new_orientation, in_orientation);

		new_delta.p = (-(in_rotation_time - 2.0f*(in_rotation_acceleration_time)) + sqrt(square(in_rotation_time-2.0f*(in_rotation_acceleration_time))) + 4.0f*(curr_orientation.p - new_orientation.p))/2;
		new_delta.b = (-(in_rotation_time - 2.0f*(in_rotation_acceleration_time)) + sqrt(square(in_rotation_time-2.0f*(in_rotation_acceleration_time))) + 4.0f*(curr_orientation.b - new_orientation.b))/2;
		new_delta.h = (-(in_rotation_time - 2.0f*(in_rotation_acceleration_time)) + sqrt(square(in_rotation_time-2.0f*(in_rotation_acceleration_time))) + 4.0f*(curr_orientation.h - new_orientation.h))/2;

		set_rotation_rate(&new_delta, in_rotation_acceleration_time);
	}
}

void camera::set_rotation(angles *in_angles, float in_rotation_time, float in_rotation_acceleration_time)
{
	matrix temp_matrix;
	vm_angles_2_matrix(&temp_matrix, in_angles);
	set_rotation(&temp_matrix, in_rotation_time, in_rotation_acceleration_time);
}

void camera::set_rotation_facing(vector *in_target, float in_rotation_time, float in_rotation_acceleration_time)
{
	matrix temp_matrix;
	vm_vector_2_matrix(&temp_matrix, in_target, NULL, NULL);
	set_rotation(&temp_matrix, in_rotation_time, in_rotation_acceleration_time);
}

void camera::set_rotation_rate(angles *in_rotation_rate, float in_rotation_acceleration_time)
{
	if(in_rotation_acceleration_time < 0.0001f)
	{
		rotation_rate = *in_rotation_rate;
		return;
	}

	rotation_time_left = in_rotation_acceleration_time;

	in_rotation_acceleration_time = (1.0f/in_rotation_acceleration_time);

	rotation_rate_delta.p = in_rotation_rate->p * in_rotation_acceleration_time;
	rotation_rate_delta.b = in_rotation_rate->b * in_rotation_acceleration_time;
	rotation_rate_delta.h = in_rotation_rate->h * in_rotation_acceleration_time;
}

void camera::do_movement(float in_frametime)
{
	float frametime;

	if(rotation_time_left)
	{
		if(in_frametime > rotation_time_left)
			frametime = rotation_time_left;
		else
			frametime = in_frametime;

		rotation_rate.p += (rotation_rate_delta.p) * frametime;
		rotation_rate.b += (rotation_rate_delta.b) * frametime;
		rotation_rate.h += (rotation_rate_delta.h) * frametime;

		rotation_time_left -= frametime;
	}
	if(translation_time_left)
	{
		if(in_frametime > translation_time_left)
			frametime = translation_time_left;
		else
			frametime = in_frametime;

		vm_vec_scale_add2(&translation_velocity, &translation_velocity_delta, frametime);

		translation_time_left -= frametime;
	}

	//TODO: Add code for *stopping* here

	vm_vec_scale_add2(&position, &translation_velocity, in_frametime);
	vm_rotate_matrix_by_angles(&orientation, &rotation_rate);
}
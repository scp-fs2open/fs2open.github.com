#include "camera/camera.h"
#include "math/vecmat.h"
#include "graphics/2d.h"
#include "globalincs/alphacolors.h"

//Some global cameras
camera free_camera;

//This is where the camera class beings! :D
camera::camera()
{
	//These are our two pointers, new'd and delete'd only by this class
	desired_position = NULL;
	desired_orientation = NULL;

	reset();
}

camera::~camera()
{
	//Cleanup
	if(desired_position != NULL)
		delete desired_position;

	if(desired_orientation != NULL)
		delete desired_orientation;
}

void camera::reset()
{
	memset(&position, 0, sizeof(position));
	if(desired_position != NULL)
	{
		delete desired_position;
		desired_position = NULL;
	}
	translation_velocity = translation_vel_limit = translation_acc_limit = vmd_zero_vector;

	vm_set_identity(&orientation);
	if(desired_orientation != NULL)
	{
		delete desired_orientation;
		desired_orientation = NULL;
	}
	rotation_rate = rotation_vel_limit = rotation_acc_limit = vmd_zero_vector;
}

void camera::set_position(vector *in_position, float in_translation_time, float in_translation_acceleration_time)
{
	position = *in_position;

}

void camera::set_translation_velocity(vector *in_velocity)
{
	if(in_velocity == NULL)
		translation_velocity = *in_velocity;
	else
		vm_vec_zero(&translation_velocity);
}

#define square(a) (a*a)
void camera::set_rotation(matrix *in_orientation, float in_rotation_time, float in_rotation_acceleration_time)
{
	if(in_rotation_time > 0.0f || in_rotation_acceleration_time > 0.0f)
	{
		//Safeties
		if(in_rotation_acceleration_time <= 0.0f)
			in_rotation_acceleration_time = 1.0f;
		if(in_rotation_time <= 0.0f)
			in_rotation_time = 3.0f;
		if(in_rotation_acceleration_time*2.0f > in_rotation_time)
			in_rotation_acceleration_time = in_rotation_time/2.0f;

		//Get the angles in radians
		angles curr_orientation, new_orientation;
		vm_extract_angles_matrix(&curr_orientation, &orientation);
		vm_extract_angles_matrix(&new_orientation, in_orientation);

		//Find the distance
		angles d;
		d.p = new_orientation.p - curr_orientation.p;
		d.b = new_orientation.b - curr_orientation.b;
		d.h = new_orientation.h - curr_orientation.h;
		/*d.p = curr_orientation.p - new_orientation.p;
		d.b = curr_orientation.b - new_orientation.b;
		d.h = curr_orientation.h - new_orientation.h;*/

		/*ad_dist = -(V/At)square((At));

		((Xf - Xo) - ad_dist)/V*/

		/*v.p = (-(in_rotation_time - 2.0f*(in_rotation_acceleration_time)) + sqrt(square(in_rotation_time-2.0f*(in_rotation_acceleration_time))) + 4.0f*(curr_orientation.p - new_orientation.p))/2;
		v.b = (-(in_rotation_time - 2.0f*(in_rotation_acceleration_time)) + sqrt(square(in_rotation_time-2.0f*(in_rotation_acceleration_time))) + 4.0f*(curr_orientation.b - new_orientation.b))/2;
		v.h = (-(in_rotation_time - 2.0f*(in_rotation_acceleration_time)) + sqrt(square(in_rotation_time-2.0f*(in_rotation_acceleration_time))) + 4.0f*(curr_orientation.h - new_orientation.h))/2;*/
		/*v.p = (new_orientation.p - curr_orientation.p)/(in_rotation_time - in_rotation_acceleration_time);
		v.b = (new_orientation.p - curr_orientation.p)/(in_rotation_time - in_rotation_acceleration_time);
		v.h = (new_orientation.p - curr_orientation.p)/(in_rotation_time - in_rotation_acceleration_time);*/

		/*
		angles *U = &rotation_rate;

		float At = in_rotation_acceleration_time;
		float t = in_rotation_time;	//- 2.0f*At*/
		/*v.p = ((d.p/At - U->p)/t);
		v.b = ((d.b/At - U->b)/t);
		v.h = ((d.h/At - U->h)/t);*/
		/*v.p = ((d.p - U->p*At)/t);
		v.b = ((d.b - U->b*At)/t);
		v.h = ((d.h - U->h*At)/t);*/

		//Set the desired orientation
		if(desired_orientation == NULL)
			desired_orientation = new matrix;

		if(in_orientation != NULL)
			*desired_orientation = *in_orientation;
		else
			vm_set_identity(desired_orientation);
		
		//Calculate the maximum acceleration speed
		rotation_acc_limit.xyz.z = fabsf(d.p / ((in_rotation_time * in_rotation_acceleration_time) - square(in_rotation_acceleration_time)));
		rotation_acc_limit.xyz.x = fabsf(d.b / ((in_rotation_time * in_rotation_acceleration_time) - square(in_rotation_acceleration_time)));
		rotation_acc_limit.xyz.y = fabsf(d.h / ((in_rotation_time * in_rotation_acceleration_time) - square(in_rotation_acceleration_time)));

		//Calculate the maximum velocity
		rotation_vel_limit.xyz.x = fabsf(rotation_acc_limit.xyz.x * in_rotation_acceleration_time);
		rotation_vel_limit.xyz.y = fabsf(rotation_acc_limit.xyz.y * in_rotation_acceleration_time);
		rotation_vel_limit.xyz.z = fabsf(rotation_acc_limit.xyz.z * in_rotation_acceleration_time);

		//rotation_rate_delta_time = rotation_rate_delta_time_left = in_rotation_acceleration_time;
		//rotation_rate_decel_time_til = in_rotation_time - in_rotation_acceleration_time;

		//set_rotation_rate(&v, in_rotation_acceleration_time);

		//rotation_rate_decel_time_til = in_rotation_time - in_rotation_acceleration_time;
	}
	else
	{
		if(in_orientation != NULL)
			orientation = *in_orientation;
		else
			vm_set_identity(&orientation);

		set_rotation_velocity(NULL);
	}
}

void camera::set_rotation(angles *in_angles, float in_rotation_time, float in_rotation_acceleration_time)
{
	matrix temp_matrix = IDENTITY_MATRIX;

	if(in_angles != NULL)
		vm_angles_2_matrix(&temp_matrix, in_angles);

	set_rotation(&temp_matrix, in_rotation_time, in_rotation_acceleration_time);
}

void camera::set_rotation_facing(vector *in_target, float in_rotation_time, float in_rotation_acceleration_time)
{
	matrix temp_matrix = IDENTITY_MATRIX;

	if(in_target != NULL)
	{
		vector targetvec;
		vm_vec_normalized_dir(&targetvec, in_target, &position);
		vm_vector_2_matrix(&temp_matrix, &targetvec, NULL, NULL);
	}

	set_rotation(&temp_matrix, in_rotation_time, in_rotation_acceleration_time);
}

void camera::set_rotation_velocity(vector *in_rotation_rate)
{
	if(in_rotation_rate == NULL)
		rotation_rate = *in_rotation_rate;
	else
		vm_vec_zero(&rotation_rate);

	/*if(in_rotation_rate == NULL)
	{
		rotation_rate.p = rotation_rate.b = rotation_rate.h = 0.0f;

		rotation_rate_delta_time = rotation_rate_delta_time_left = in_rotation_acceleration_time = 0.0f;
		rotation_rate_decel_time_til = -1.0f;
		return;
	}

	rotation_rate_delta_time = rotation_rate_delta_time_left = in_rotation_acceleration_time;
	rotation_rate_decel_time_til = -1.0f;

	rotation_rate_delta.p = in_rotation_rate->p / in_rotation_acceleration_time;
	rotation_rate_delta.b = in_rotation_rate->b / in_rotation_acceleration_time;
	rotation_rate_delta.h = in_rotation_rate->h / in_rotation_acceleration_time;*/
}

void camera::do_movement(float in_frametime)
{
	//float frametime, rot_frametime, trans_frametime;
	//DO ROTATION STUFF
	if(desired_orientation != NULL)
	{
		matrix ori_out;
		vector vel_out;

		vm_matrix_interpolate(desired_orientation, &orientation, &rotation_rate, in_frametime, &ori_out, &vel_out, &rotation_vel_limit, &rotation_acc_limit, 1);
		orientation = ori_out;
		rotation_rate = vel_out;

		//TODO: Make this "if(*desired_orienation == orientation)"
		if(vel_out.xyz.x == 0.0f && vel_out.xyz.y == 0.0f && vel_out.xyz.z == 0.0f)
		{
			delete desired_orientation;
			desired_orientation = NULL;
		}
	}
	else
	{
		//TODO: Make this right.
		angles new_orientation;
		vm_angles_2_matrix(&orientation, &new_orientation);
		new_orientation.p = rotation_rate.xyz.z;
		new_orientation.b = rotation_rate.xyz.x;
		new_orientation.h = rotation_rate.xyz.y;
		vm_angles_2_matrix(&orientation, &new_orientation);
	}

/*	rot_frametime = trans_frametime = in_frametime;

	do
	{
		frametime = rot_frametime;
		if(rotation_rate_delta_time_left || rotation_rate_decel_time_til > 0.0f)
		{
			if(frametime > rotation_rate_delta_time_left && rotation_rate_delta_time_left)
				frametime = rotation_rate_delta_time_left;
			if(frametime > rotation_rate_decel_time_til && rotation_rate_decel_time_til > 0.0f)
				frametime = rotation_rate_decel_time_til;

			if(rotation_rate_delta_time_left)
			{
				rotation_rate.p += (rotation_rate_delta.p) * frametime;
				rotation_rate.b += (rotation_rate_delta.b) * frametime;
				rotation_rate.h += (rotation_rate_delta.h) * frametime;
			}

			//mprintf(("DX:%f DY:%f DZ:%f",rotation_rate_delta.p,rotation_rate_delta.b,rotation_rate_delta.h));
			//mprintf(( " X:%f", rotation_rate.p));
			//mprintf(( " Y:%f", rotation_rate.b));
			//mprintf(( " Z:%f\n", rotation_rate.h));

			if(rotation_rate_delta_time_left)
			{
				rotation_rate_delta_time_left -= frametime;
			}
			if(rotation_rate_decel_time_til > 0.0f)
			{
				rotation_rate_decel_time_til -= frametime;
				if(rotation_rate_decel_time_til == 0.0f)
				{
					rotation_rate_delta.p *= -1.0f;
					rotation_rate_delta.b *= -1.0f;
					rotation_rate_delta.h *= -1.0f;

					rotation_rate_delta_time_left = rotation_rate_delta_time;
					rotation_rate_decel_time_til = -1.0f;
				}
			}
		}
		rot_frametime -= frametime;
	} while(rot_frametime);*/

	//DO TRANSLATION STUFF
//	float frametime = in_frametime;
	/*if(translation_time_left)
	{
		if(in_frametime > translation_time_left)
			frametime = translation_time_left;
		else
			frametime = in_frametime;

		vm_vec_scale_add2(&translation_velocity, &translation_velocity_delta, frametime);

		translation_time_left -= frametime;
	}*/

	vm_vec_scale_add2(&position, &translation_velocity, in_frametime);
}

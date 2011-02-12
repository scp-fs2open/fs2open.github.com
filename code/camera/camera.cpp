#include "camera/camera.h"
#include "math/vecmat.h"
#include "graphics/2d.h"
#include "globalincs/alphacolors.h"
#include "parse/parselo.h"
#include "physics/physics.h" //apply_physics
#include "globalincs/systemvars.h" //VM_FREECAMERA etc
#include "hud/hud.h" //hud_get_draw
#include "model/model.h" //polymodel, model_get
#include "playerman/player.h" //player_get_padlock_orient
#include "ship/ship.h" //compute_slew_matrix
#include "graphics/font.h"

//*************************IMPORTANT GLOBALS*************************
float VIEWER_ZOOM_DEFAULT = 0.75f;			//	Default viewer zoom, 0.625 as per multi-lateral agreement on 3/24/97
float Sexp_fov = 0.0f;
warp_camera Warp_camera;

//*************************OTHER STUFF*************************
//Some global vars
SCP_vector<subtitle> Subtitles;
SCP_vector<camera*> Cameras;
//Preset cameras
camid Current_camera;
camid Main_camera;

//*************************INTERNAL FUNCS*************************
//static avd_camera *cam_upgrade_to_avd(camera *cam);

//*************************CLASS: camera*************************
//This is where the camera class beings! :D
camera::camera(char *in_name, int in_signature)
{
	set_name(in_name);
	sig = in_signature;
//	flags = CAM_DEFAULT_FLAGS;

	reset();
}

camera::~camera()
{
	//Check if this is in use
	if(Current_camera.getSignature() == this->sig)
	{
		Current_camera = camid();
	}
}

void camera::clear()
{
	strcpy_s(name, "");
	sig = -1;
	reset();
}

void camera::reset()
{
	flags = CAM_DEFAULT_FLAGS;

	object_host = object_target = object_h();
	object_host_submodel = object_target_submodel = -1;

	c_fov = VIEWER_ZOOM_DEFAULT;
	c_pos = vmd_zero_vector;
	c_ori = vmd_identity_matrix;

	func_custom_position = NULL;
	func_custom_orientation = NULL;

	fov.clear();
	fov.set(VIEWER_ZOOM_DEFAULT);

	pos_x.clear();
	pos_y.clear();
	pos_z.clear();

	/*
	ori_p.clear();
	ori_b.clear();
	ori_h.clear();
	*/
	/*
	ori_progress.clear();
	ori_initial = ori_final = vmd_identity_matrix;
	*/
	for(int i = 0; i < 9; i++)
	{
		ori[i].clear();
		ori[i].set(vmd_identity_matrix.a1d[i]);
	}
	/*
	flags = CAM_DEFAULT_FLAGS;

	desired_position = position = vmd_zero_vector;
	
	translation_velocity = translation_vel_limit = translation_acc_limit = vmd_zero_vector;

	vm_set_identity(&orientation);
	vm_set_identity(&desired_orientation);
	rotation_rate = rotation_vel_limit = rotation_acc_limit = vmd_zero_vector;
	*/
}

void camera::set_name(char *in_name)
{
	if(name != NULL)
		strncpy(name, in_name, NAME_LENGTH-1);
}

void camera::set_fov(float in_fov, float in_fov_time, float in_fov_acceleration_time, float in_fov_deceleration_time)
{
	if(in_fov_time == 0.0f && in_fov_acceleration_time == 0.0f && in_fov_deceleration_time == 0.0f)
	{
		flags |= CAM_STATIONARY_FOV;
		c_fov = in_fov;
		return;
	}

	flags &= ~CAM_STATIONARY_FOV;
	if(in_fov <= 0.0f)
	{
		fov.setVD(in_fov_time, in_fov_acceleration_time, 0.0f);
	}

	fov.setAVD(in_fov, in_fov_time, in_fov_acceleration_time, in_fov_deceleration_time, 0.0f);
}

void camera::set_object_host(object *objp, int n_object_host_submodel)
{
	if(objp == NULL)
		object_host = object_h();

	object_host = object_h(objp);
	object_host_submodel = n_object_host_submodel;
}

void camera::set_object_target(object *objp, int n_object_target_submodel)
{
	if(objp == NULL)
		object_target = object_h();

	object_target = object_h(objp);
	object_target_submodel = n_object_target_submodel;
}

//Custom function receives the already-modified current position value.
//It should be replaced or added to as the custom function modifier sees fit.
void camera::set_custom_position_function(void (*n_func_custom_position)(camera *cam, vec3d *camera_pos))
{
	func_custom_position = n_func_custom_position;
}

//Custom function receives the already-modified current orientation value.
//It should be replaced or added to as the custom function modifier sees fit.
void camera::set_custom_orientation_function(void (*n_func_custom_orientation)(camera *cam, matrix *camera_ori))
{
	func_custom_orientation = n_func_custom_orientation;
}

void camera::set_position(vec3d *in_position, float in_translation_time, float in_translation_acceleration_time, float in_translation_deceleration_time, float in_end_velocity)
{
	if(in_translation_time == 0.0f && in_translation_acceleration_time == 0.0f && in_translation_deceleration_time == 0.0f)
	{
		c_pos = *in_position;
		pos_x.set(c_pos.xyz.x);
		pos_y.set(c_pos.xyz.y);
		pos_z.set(c_pos.xyz.z);
		flags |= CAM_STATIONARY_POS;
		return;
	}

	flags &= ~CAM_STATIONARY_POS;
	if(in_position == NULL)
	{
		pos_x.setVD(in_translation_time, in_translation_acceleration_time, in_end_velocity);
		pos_y.setVD(in_translation_time, in_translation_acceleration_time, in_end_velocity);
		pos_z.setVD(in_translation_time, in_translation_acceleration_time, in_end_velocity);
		return;
	}

	pos_x.setAVD(in_position->xyz.x, in_translation_time, in_translation_acceleration_time, in_translation_deceleration_time, in_end_velocity);
	pos_y.setAVD(in_position->xyz.y, in_translation_time, in_translation_acceleration_time, in_translation_deceleration_time, in_end_velocity);
	pos_z.setAVD(in_position->xyz.z, in_translation_time, in_translation_acceleration_time, in_translation_deceleration_time, in_end_velocity);
}

void camera::set_translation_velocity(vec3d *in_velocity, float in_acceleration_time)
{
	pos_x.setVD(in_acceleration_time, in_acceleration_time, in_velocity->xyz.x);
	pos_y.setVD(in_acceleration_time, in_acceleration_time, in_velocity->xyz.y);
	pos_z.setVD(in_acceleration_time, in_acceleration_time, in_velocity->xyz.z);
}

void camera::set_rotation(matrix *in_orientation, float in_rotation_time, float in_rotation_acceleration_time, float in_rotation_deceleration_time)
{
	if(in_rotation_time == 0.0f && in_rotation_acceleration_time == 0.0f && in_rotation_deceleration_time == 0.0f)
	{
		c_ori = *in_orientation;
		for(int i = 0; i < 9; i++)
			ori[i].set(in_orientation->a1d[i]);
		flags |= CAM_STATIONARY_ORI;
		return;
	}

	flags &= ~CAM_STATIONARY_ORI;
	if(in_orientation == NULL)
	{
		for(int i = 0; i < 9; i++)
			ori[i].setVD(in_rotation_time, in_rotation_acceleration_time, 0.0f);
		return;
	}

	for(int i = 0; i < 9; i++)
		ori[i].setAVD(in_orientation->a1d[i], in_rotation_time, in_rotation_acceleration_time, in_rotation_deceleration_time, 0.0f);
	//WMC - This stationary check is here because it's a lot more
	//accurate to directly save the matrix, if we've been provided one.
	//IMPORTANT: Without this chunk, the HUD and view waves around with
	//				the main camera.
	/*
	angles a;
	vm_extract_angles_matrix(&a, in_orientation);
	if(in_rotation_time == 0.0f && in_rotation_acceleration_time == 0.0f)
	{
		c_ori = *in_orientation;
		ori_p.set(a.p);
		ori_b.set(a.b);
		ori_h.set(a.h);
		flags |= CAM_STATIONARY_ORI;
		return;
	}

	//Continue on
	this->set_rotation(&a, in_rotation_time, in_rotation_acceleration_time);
	*/
}

void camera::set_rotation(angles *in_angles, float in_rotation_time, float in_rotation_acceleration_time, float in_rotation_deceleration_time)
{
	matrix mtx = IDENTITY_MATRIX;
	vm_rotate_matrix_by_angles(&mtx, in_angles);
	this->set_rotation(&mtx, in_rotation_time, in_rotation_acceleration_time, in_rotation_deceleration_time);
	/*
	if(in_rotation_time == 0.0f && in_rotation_acceleration_time == 0.0f)
	{
		ori_p.set(in_angles->p);
		ori_b.set(in_angles->b);
		ori_h.set(in_angles->h);

		c_ori = vmd_identity_matrix;
		vm_rotate_matrix_by_angles(&c_ori, in_angles);

		flags |= CAM_STATIONARY_ORI;
		return;
	}

	flags &= ~CAM_STATIONARY_ORI;
	if(in_angles == NULL)
	{
		ori_p.setVD(in_rotation_time, in_rotation_acceleration_time, 0.0f);
		ori_b.setVD(in_rotation_time, in_rotation_acceleration_time, 0.0f);
		ori_h.setVD(in_rotation_time, in_rotation_acceleration_time, 0.0f);
		return;
	}

	ori_p.setAVD(in_angles->p, in_rotation_time, in_rotation_acceleration_time, in_rotation_acceleration_time, 0.0f);
	ori_b.setAVD(in_angles->b, in_rotation_time, in_rotation_acceleration_time, in_rotation_acceleration_time, 0.0f);
	ori_h.setAVD(in_angles->h, in_rotation_time, in_rotation_acceleration_time, in_rotation_acceleration_time, 0.0f);
	*/
}

void camera::set_rotation_facing(vec3d *in_target, float in_rotation_time, float in_rotation_acceleration_time, float in_rotation_deceleration_time)
{
	matrix temp_matrix = IDENTITY_MATRIX;

	if(in_target != NULL)
	{
		vec3d position = vmd_zero_vector;
		this->get_info(&position, NULL);

		if(in_target->xyz.x == position.xyz.x && in_target->xyz.y == position.xyz.y && in_target->xyz.z == position.xyz.z)
		{
			Warning(LOCATION, "Camera tried to point to self");
			return;
		}

		vec3d targetvec;
		vm_vec_normalized_dir(&targetvec, in_target, &position);
		vm_vector_2_matrix(&temp_matrix, &targetvec, NULL, NULL);
	}

	set_rotation(&temp_matrix, in_rotation_time, in_rotation_acceleration_time, in_rotation_deceleration_time);
}

void camera::set_rotation_velocity(angles *in_rotation_rate, float in_acceleration_time)
{
	Error(LOCATION, "This function is disabled until further notice.");
	/*
	ori_p.setVD(in_acceleration_time, in_acceleration_time, in_rotation_rate->p);
	ori_b.setVD(in_acceleration_time, in_acceleration_time, in_rotation_rate->b);
	ori_h.setVD(in_acceleration_time, in_acceleration_time, in_rotation_rate->h);
	*/

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

void camera::do_frame(float in_frametime)
{

}

object *camera::get_object_host()
{
	if(object_host.IsValid())
		return object_host.objp;
	else
		return NULL;
}

int camera::get_object_host_submodel()
{
	return object_host_submodel;
}

object *camera::get_object_target()
{
	if(object_target.IsValid())
		return object_target.objp;
	else
		return NULL;
}

int camera::get_object_target_submodel()
{
	return object_target_submodel;
}

float camera::get_fov()
{
	if(!(flags & CAM_STATIONARY_FOV))
	{
		fov.get(&c_fov, NULL);
	}

	return c_fov;
}

void camera::get_info(vec3d *position, matrix *orientation)
{
	//WTF?
	if(position == NULL && orientation == NULL)
		return;

	//POSITION
	if(!(flags & CAM_STATIONARY_POS) || object_host.IsValid())
	{
		c_pos = vmd_zero_vector;

		vec3d pt;
		pos_x.get(&pt.xyz.x, NULL);
		pos_y.get(&pt.xyz.y, NULL);
		pos_z.get(&pt.xyz.z, NULL);

		if(object_host.IsValid())
		{
			object *objp = object_host.objp;
			int model_num = object_get_model(objp);
			polymodel *pm = NULL;
			
			if(model_num > -1)
			{
				pm = model_get(model_num);
			}

			if(object_host_submodel < 0 || pm == NULL)
			{
				vm_vec_unrotate(&c_pos, &pt, &object_host.objp->orient);
				vm_vec_add2(&c_pos, &object_host.objp->pos);
			}
			else
			{
				model_find_world_point( &c_pos, &pt, pm->id, object_host_submodel, &objp->orient, &objp->pos );
			}
		}
		else
		{
			c_pos = pt;
		}

		//Do custom position stuff, if needed
		if(func_custom_position != NULL)
		{
			func_custom_position(this, &c_pos);
		}
	}

	if(position != NULL)
		*position = c_pos;

	//ORIENTATION
	if(orientation != NULL)
	{
		if(!(flags & CAM_STATIONARY_ORI) || object_target.IsValid() || object_host.IsValid())
		{
			if(object_target.IsValid())
			{
				object *objp = object_target.objp;
				int model_num = object_get_model(objp);
				polymodel *pm = NULL;
				vec3d target_pos = vmd_zero_vector;
				
				//See if we can get the model
				if(model_num > -1)
				{
					pm = model_get(model_num);
				}

				//If we don't have a submodel or don't have the model use object pos
				//Otherwise, find the submodel pos as it is rotated
				if(object_target_submodel < 0 || pm == NULL)
				{
					target_pos = objp->pos;
				}
				else
				{
					model_find_world_point( &target_pos, &vmd_zero_vector, pm->id, object_target_submodel, &objp->orient, &objp->pos );
				}

				vec3d targetvec;
				vm_vec_normalized_dir(&targetvec, &target_pos, &c_pos);
				vm_vector_2_matrix(&c_ori, &targetvec, NULL, NULL);
			}
			else if(object_host.IsValid())
			{
				c_ori = object_host.objp->orient;
			}
			else
			{
				c_ori = vmd_identity_matrix;
			}

			//Do human interaction
			//WMC - Nevermind for now, maybe toggleable later.
			/*
			if ( Viewer_obj == object_host.objp || Viewer_mode & VM_CHASE ) {
				if ( Viewer_mode & VM_PADLOCK_ANY ) {
					player_get_padlock_orient(&c_ori);
				} else {
					compute_slew_matrix(&c_ori, &Viewer_slew_angles);
				}
			}*/

			matrix mtxA = c_ori;
			matrix mtxB = IDENTITY_MATRIX;
			float pos = 0.0f;
			for(int i = 0; i < 9; i++)
			{
				ori[i].get(&pos, NULL);
				mtxB.a1d[i] = pos;
			}
			vm_matrix_x_matrix(&c_ori, &mtxA, &mtxB);

			vm_orthogonalize_matrix(&c_ori);
			/*
			angles a;
			ori_p.get(&a.p, NULL);
			ori_b.get(&a.b, NULL);
			ori_h.get(&a.h, NULL);

			vm_rotate_matrix_by_angles(&c_ori, &a);
			*/
			//vm_angles_2_matrix(&c_ori, &a);
		}
		//Do custom orientation stuff, if needed
		if(func_custom_orientation != NULL)
		{
			func_custom_orientation(this, &c_ori);
		}
		*orientation = c_ori;
	}
}
//*************************warp_camera*************************
warp_camera::warp_camera()
{
	this->reset();
}
warp_camera::warp_camera(object *objp)
{
	this->reset();

	vec3d object_pos = objp->pos;
	matrix tmp;
	ship_get_eye(&object_pos, &tmp, objp);
	/*
	//matrix tmp_m = vmd_identity_matrix;
	camid cid = ship_get_eye( objp );
	if(cid.isValid())
	{
		camera *cam = cid.getCamera();
		cam->get_info(&player_pos, NULL);
		//tmp_m = *cam->get_orientation();
	}
	*/
	vm_vec_scale_add2( &object_pos, &Player_obj->orient.vec.rvec, 0.0f );
	vm_vec_scale_add2( &object_pos, &Player_obj->orient.vec.uvec, 0.952f );
	vm_vec_scale_add2( &object_pos, &Player_obj->orient.vec.fvec, -1.782f );

	vec3d tmp_vel = { { { 0.0f, 5.1919f, 14.7f } } };

	this->set_position( &object_pos );
	this->set_rotation( &objp->orient );
	this->set_velocity( &tmp_vel, true);
}

void warp_camera::reset()
{
	c_pos = c_vel = c_desired_vel = vmd_zero_vector;
	c_ori = vmd_identity_matrix;
	c_damping = 1.0f;
	c_time = 0.0f;
}

void warp_camera::set_position( vec3d *in_pos )
{
	c_pos = *in_pos;
}

void warp_camera::set_rotation( matrix *in_ori )
{
	c_ori = *in_ori;
}

void warp_camera::set_velocity( vec3d *in_vel, bool instantaneous )
{
	c_desired_vel.xyz.x = 0.0f;
	c_desired_vel.xyz.y = 0.0f;
	c_desired_vel.xyz.z = 0.0f;

	vm_vec_scale_add2( &c_desired_vel, &c_ori.vec.rvec, in_vel->xyz.x );
	vm_vec_scale_add2( &c_desired_vel, &c_ori.vec.uvec, in_vel->xyz.y );
	vm_vec_scale_add2( &c_desired_vel, &c_ori.vec.fvec, in_vel->xyz.z );

	if ( instantaneous )	{
		c_vel = c_desired_vel;
	}

}

//
void warp_camera::do_frame(float in_frametime)
{
	vec3d new_vel, delta_pos;

	apply_physics( c_damping, c_desired_vel.xyz.x, c_vel.xyz.x, in_frametime, &new_vel.xyz.x, &delta_pos.xyz.x );
	apply_physics( c_damping, c_desired_vel.xyz.y, c_vel.xyz.y, in_frametime, &new_vel.xyz.y, &delta_pos.xyz.y );
	apply_physics( c_damping, c_desired_vel.xyz.z, c_vel.xyz.z, in_frametime, &new_vel.xyz.z, &delta_pos.xyz.z );

	c_vel = new_vel;

//	mprintf(( "Camera velocity = %.1f,%.1f, %.1f\n", Camera_velocity.xyz.x, Camera_velocity.xyz.y, Camera_velocity.xyz.z ));

	vm_vec_add2( &c_pos, &delta_pos );

	float ot = c_time+0.0f;

	c_time += in_frametime;

	if ( (ot < 0.667f) && ( c_time >= 0.667f ) )	{
		vec3d tmp;
		
		tmp.xyz.z = 4.739f;		// always go this fast forward.

		// pick x and y velocities so they are always on a 
		// circle with a 25 m radius.

		float tmp_angle = frand()*PI2;
	
		tmp.xyz.x = 22.0f * (float)sin(tmp_angle);
		tmp.xyz.y = -22.0f * (float)cos(tmp_angle);

		//mprintf(( "Angle = %.1f, vx=%.1f, vy=%.1f\n", tmp_angle, tmp.xyz.x, tmp.xyz.y ));

		//mprintf(( "Changing velocity!\n" ));
		this->set_velocity( &tmp, 0 );
	}

	if ( (ot < 3.0f ) && ( c_time >= 3.0f ) )	{
		vec3d tmp = ZERO_VECTOR;
		this->set_velocity( &tmp, 0 );
	}
	
}

void warp_camera::get_info(vec3d *position, matrix *orientation)
{
	if(position != NULL)
		*position = c_pos;

	if(orientation != NULL)
		*orientation = c_ori;
}

//*************************subtitle*************************

#define MAX_SUBTITLE_LINES		64
subtitle::subtitle(int in_x_pos, int in_y_pos, char* in_text, char* in_imageanim, float in_display_time, float in_fade_time, color *in_text_color, int in_text_fontnum, bool center_x, bool center_y, int in_width, int in_height, bool in_post_shaded)
{
	// basic init, this always has to be done
	memset( imageanim, 0, sizeof(imageanim) );
	memset( &text_pos, 0, 2*sizeof(int) );
	memset( &image_pos, 0, 4*sizeof(int) );
	image_id = -1;

	if ( ((in_text != NULL) && (strlen(in_text) <= 0)) && ((in_imageanim != NULL) && (strlen(in_imageanim) <= 0)) )
		return;

	char text_buf[256];
	if (in_text != NULL && strlen(in_text) > 0)
	{
		strcpy_s(text_buf, in_text);
		sexp_replace_variable_names_with_values(text_buf, 256);
		in_text = text_buf;
	}


	int num_text_lines = 0;
	char *text_line_ptrs[MAX_SUBTITLE_LINES];
	int text_line_lens[MAX_SUBTITLE_LINES];

	//Setup text
	if ( (in_text != NULL) && (strlen(in_text) > 0) ) {
		int split_width = (in_width > 0) ? in_width : 200;

		num_text_lines = split_str(in_text, split_width, text_line_lens, text_line_ptrs, MAX_SUBTITLE_LINES);
		std::string temp_str;
		for(int i = 0; i < num_text_lines; i++)
		{
			temp_str.assign(text_line_ptrs[i], text_line_lens[i]);
			text_lines.push_back(temp_str);
		}
	}

	//Setup text color
	if(in_text_color != NULL)
		text_color = *in_text_color;
	else
		gr_init_alphacolor(&text_color, 255, 255, 255, 255);
	text_fontnum = in_text_fontnum;

	//Setup display and fade time
	display_time = fl_abs(in_display_time);
	fade_time = fl_abs(in_fade_time);

	//Setup image
	if ( (in_imageanim != NULL) && (strlen(in_imageanim) > 0) )
	{
		image_id = bm_load(in_imageanim);

		if (image_id >= 0)
			strncpy(imageanim, in_imageanim, sizeof(imageanim) - 1);
	}

	//Setup pos
	int w=0, h=0, tw=0, th=0;
	if(center_x || center_y)
	{
		// switch font because we need to measure it
		int old_fontnum;
		if (text_fontnum >= 0)
		{
			old_fontnum = gr_get_current_fontnum();
			gr_set_font(text_fontnum);
		}
		else
		{
			old_fontnum = -1;
		}

		//Get text size
		for(int i = 0; i < num_text_lines; i++)
		{
			gr_get_string_size(&w, &h, text_line_ptrs[i], text_line_lens[i]);

			if(w > tw)
				tw = w;

			th += h;
		}

		// restore old font
		if (old_fontnum >= 0)
		{
			gr_set_font(old_fontnum);
		}

		//Get image size
		if(image_id != -1)
		{
			bm_get_info(image_id, &w, &h);
			if (in_width > 0)
				w = in_width;
			if (in_height > 0)
				h = in_height;

			tw += w;
			if(h > th)
				th = h;
		}

		//Center it?
		if(center_x)
			image_pos.x = (gr_screen.max_w - tw)/2;
		if(center_y)
			image_pos.y = (gr_screen.max_h - th)/2;
	}

	if(in_x_pos < 0 && !center_x)
		image_pos.x += gr_screen.max_w + in_x_pos;
	else if(!center_x)
		image_pos.x += in_x_pos;

	if(in_y_pos < 0 && !center_y)
		image_pos.y += gr_screen.max_h + in_y_pos;
	else if(!center_y)
		image_pos.y += in_y_pos;

	image_pos.w = in_width;
	image_pos.h = in_height;

	if(image_id != -1)
		text_pos.x = image_pos.x + w;	//Still set from bm_get_info call
	else
		text_pos.x = image_pos.x;
	text_pos.y = image_pos.y;

	time_displayed = 0.0f;
	time_displayed_end = 2.0f*fade_time + display_time;

	post_shaded = in_post_shaded;
}

void subtitle::do_frame(float frametime)
{
	//Figure out how much alpha
	if(time_displayed < fade_time)
	{
		text_color.alpha = (ubyte)fl2i(255.0f*(time_displayed/fade_time));
	}
	else if(time_displayed > time_displayed_end)
	{
		//We're finished
		return;
	}
	else if((time_displayed - fade_time) > display_time)
	{
		text_color.alpha = (ubyte)fl2i(255.0f*(1-(time_displayed - fade_time - display_time)/fade_time));
	}
	else
	{
		text_color.alpha = 255;
	}

	gr_set_color_fast(&text_color);

	// save old font and set new font
	int old_fontnum;
	if (text_fontnum >= 0)
	{
		old_fontnum = gr_get_current_fontnum();
		gr_set_font(text_fontnum);
	}
	else
	{
		old_fontnum = -1;
	}

	int font_height = gr_get_font_height();
	int x = text_pos.x;
	int y = text_pos.y;

	for(unsigned int i = 0; i < text_lines.size(); i++)
	{
		gr_string(x, y, (char*)text_lines[i].c_str(), false);
		y += font_height;
	}

	// restore old font
	if (old_fontnum >= 0)
	{
		gr_set_font(old_fontnum);
	}

	if(image_id >= 0)
	{
		gr_set_bitmap(image_id, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, text_color.alpha/255.0f);

		// scaling?
		if (image_pos.w > 0 || image_pos.h > 0)
		{
			int orig_w, orig_h;
			vec3d scale;

			bm_get_info(image_id, &orig_w, &orig_h);
			scale.xyz.x = image_pos.w / (float) orig_w;
			scale.xyz.y = image_pos.h / (float) orig_h;
			scale.xyz.z = 1.0f;

			gr_push_scale_matrix(&scale);
			gr_bitmap(image_pos.x, image_pos.y, false);
			gr_pop_scale_matrix();
		}
		// no scaling
		else
		{
			gr_bitmap(image_pos.x, image_pos.y, false);
		}
	}

	time_displayed += frametime;
}

subtitle::~subtitle()
{
	if (image_id != -1) {
		if ( bm_release(image_id) ) {
			image_id = -1;
		}
	}
}

void subtitle::clone(const subtitle &sub)
{
	uint i = 0;

	for (i = 0; i < sub.text_lines.size(); i++) {
		text_lines.push_back(sub.text_lines[i]);
	}
	text_fontnum = sub.text_fontnum;

	// copy the structs
	text_pos = sub.text_pos;
	image_pos = sub.image_pos;

	display_time = sub.display_time;
	fade_time = sub.fade_time;
	memcpy( &text_color, &sub.text_color, sizeof(color) );

	if ( strlen(sub.imageanim) ) {
		memcpy( imageanim, sub.imageanim, MAX_FILENAME_LEN );
		// we bm_load() again here to kick the refcount up
		image_id = bm_load(imageanim);
	} else {
		memset( imageanim, 0, MAX_FILENAME_LEN );
		image_id = -1;
	}

	time_displayed = sub.time_displayed;
	time_displayed_end = sub.time_displayed_end;

	post_shaded = sub.post_shaded;
}

const subtitle &subtitle::operator=(const subtitle &sub)
{
	if (this != &sub) {
		if (image_id != -1) {
			if ( bm_release(image_id) ) {
				image_id = -1;
			}
		}

		clone(sub);
	}

	return *this;
}

//*************************camid*************************
camid::camid()
{
	idx = 0;
	sig = -1;
}

camid::camid(int n_idx, int n_sig)
{
	idx = n_idx;
	sig = n_sig;
}

camera* camid::getCamera()
{
	if(!isValid())
		return NULL;

	return Cameras[idx];
}

uint camid::getIndex()
{
	return idx;
}

int camid::getSignature()
{
	return sig;
}

bool camid::isValid()
{
	if(idx < 0 || idx >= Cameras.size())
		return false;

	if(Cameras[idx] == NULL)
		return false;
	
	if(Cameras[idx]->get_signature() != this->sig)
		return false;

	return true;
}

//*************************camera functions*************************
void cam_init()
{
	//Rest FOV override
	Sexp_fov = 0.0;
}

void cam_close()
{
	//Set Current_camera to nothing
	Current_camera = camid();
	for(unsigned int i = 0; i < Cameras.size(); i++)
	{
		if(Cameras[i] != NULL)
		{
			delete Cameras[i];
			Cameras[i] = NULL;
		}
	}
	Cameras.clear();
}

int cam_get_next_sig()
{
	static int next_sig = 0;
	return next_sig++;
}

camid cam_create(char *n_name, vec3d *n_pos, vec3d *n_norm, object *n_object, int n_object_host_submodel)
{
	matrix ori;
	vm_vector_2_matrix_norm(&ori, n_norm);
	return cam_create(n_name, n_pos, &ori, n_object, n_object_host_submodel);
}

camid cam_create(char *n_name, vec3d *n_pos, matrix *n_ori, object *n_object, int n_object_host_submodel)
{
	camera *cam = NULL;
	camid cid;

	//Get signature
	int sig = cam_get_next_sig();

	//Get name
	char buf[NAME_LENGTH] = {'\0'};
	if(n_name == NULL)
		sprintf(buf, "Camera %d", cid.getSignature());
	else
		strncpy(buf, n_name, NAME_LENGTH-1);

	//Find a free slot
	for(uint i = 0; i < Cameras.size(); i++)
	{
		if(Cameras[i] == NULL)
		{
			cam = new camera(buf, sig);
			cid = camid(i, sig);
			Cameras[i] = cam;
			break;
		}
		else if(Cameras[i]->is_empty())
		{
			delete Cameras[i];
			cam = new camera(buf, sig);
			cid = camid(i, sig);
			Cameras[i] = cam;
		}
	}
	if(cam == NULL)
	{
		cam = new camera(buf, sig);
		cid = camid(Cameras.size(), sig);
		Cameras.push_back(cam);
	}

	//Set attributes
	if(n_pos != NULL)
		cam->set_position(n_pos);
	if(n_ori != NULL)
		cam->set_rotation(n_ori);
	if(n_object != NULL)
		cam->set_object_host(n_object, n_object_host_submodel);

	return cid;
}

void cam_delete(camid cid)
{
	if(cid.isValid())
	{
		cid.getCamera()->clear();
	}
}

void cam_do_frame(float frametime)
{
	unsigned int i,size=Cameras.size();
	for(i = 0; i < size; i++)
	{
		if(Cameras[i] != NULL)
			Cameras[i]->do_frame(frametime);
	}
}

camid cam_get_camera(uint idx)
{
	if(idx < 0 || idx >= Cameras.size())
		return camid();

	return camid(idx, Cameras[idx]->get_signature());
}

camid cam_get_current()
{
	return Current_camera;
}

uint cam_get_num()
{
	return Cameras.size();
}

//Looks up camera by name, returns -1 on failure
camid cam_lookup(char *name)
{
	unsigned int i,size=Cameras.size();
	for(i = 0; i < size; i++)
	{
		if(Cameras[i] != NULL && !stricmp(Cameras[i]->get_name(), name))
			return camid(i, Cameras[i]->get_signature());
	}

	return camid();
}

static bool Camera_hud_draw_saved = false;
static int Camera_hud_draw_value = 0;

bool cam_set_camera(camid cid)
{
	if(!cid.isValid())
	{
		return false;
	}

	Viewer_mode |= VM_FREECAMERA;
	Current_camera = cid;

	if(!Camera_hud_draw_saved)
	{
		Camera_hud_draw_value = hud_get_draw();
		Camera_hud_draw_saved = true;
	}
	hud_set_draw(0);
	return true;
}

void cam_reset_camera()
{
	Viewer_mode &= ~VM_FREECAMERA;

	hud_set_draw(Camera_hud_draw_value);
	Camera_hud_draw_saved = false;
}

//Functions must delete the original camera after calling this function.
//Note that due to the esoteric nature of this function, it should not
//be used outside camera.cpp
/*
static avd_camera *cam_upgrade_to_avd(camera *cam)
{
	for(uint i = 0; i < Cameras.size(); i++)
	{
		if(Cameras[i] == cam)
			break;
	}

	if(i == Cameras.size())
		return NULL;

	avd_camera *avdcam = new avd_camera(cam);

	//Make the switch
	Cameras[i] = avdcam;

	return avdcam;
}
*/

void subtitles_init()
{
	//Do nothing!!
}

void subtitles_close()
{
	Subtitles.clear();
}

void subtitles_do_frame(float frametime)
{
	unsigned int i,size=Subtitles.size();
	for(i = 0; i < size; i++)
	{
		if ( !Subtitles[i].is_post_shaded( ) )
			Subtitles[i].do_frame(frametime);
	}
}

void subtitles_do_frame_post_shaded(float frametime)
{
	unsigned int i,size=Subtitles.size();
	for(i = 0; i < size; i++)
	{
		if ( Subtitles[i].is_post_shaded( ) )
			Subtitles[i].do_frame(frametime);
	}
}
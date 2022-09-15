#include "camera/camera.h"
#include "globalincs/alphacolors.h"
#include "globalincs/linklist.h"
#include "globalincs/systemvars.h" //VM_FREECAMERA etc
#include "graphics/matrix.h"
#include "hud/hud.h" //hud_get_draw
#include "math/vecmat.h"
#include "mod_table/mod_table.h"
#include "model/model.h" //polymodel, model_get
#include "options/Option.h"
#include "parse/parselo.h"
#include "parse/sexp_container.h"
#include "playerman/player.h" //player_get_padlock_orient
#include "ship/ship.h"        //compute_slew_matrix

//*************************IMPORTANT GLOBALS*************************
float VIEWER_ZOOM_DEFAULT = 0.75f;			//	Default viewer zoom, 0.625 as per multi-lateral agreement on 3/24/97
float COCKPIT_ZOOM_DEFAULT = 0.75f;
float Sexp_fov = 0.0f;
warp_camera Warp_camera;

//*************************OTHER STUFF*************************
//Some global vars
SCP_vector<subtitle> Subtitles;
SCP_vector<camera*> Cameras;
//Preset cameras
camid Current_camera;
camid Main_camera;

static SCP_string fov_display(float val)
{
	auto degrees = fl_degrees(val);
	SCP_string out;
	sprintf(out, u8"%.1f\u00B0", degrees);
	return out;
}
auto FovOption = options::OptionBuilder<float>("Graphics.FOV", "Field Of View", "The vertical field of view.")
                     .category("Graphics")
                     .range(0.436332f, 1.5708f)
                     .bind_to(&VIEWER_ZOOM_DEFAULT)
                     .display(fov_display)
                     .default_val(0.75f)
                     .level(options::ExpertLevel::Advanced)
                     .importance(60)
                     .finish();

//*************************CLASS: camera*************************
//This is where the camera class begins! :D
camera::camera(const char *in_name, int in_signature)
{
	set_name(in_name);
	sig = in_signature;

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

	for(int i = 0; i < 9; i++)
	{
		ori[i].clear();
		ori[i].set(vmd_identity_matrix.a1d[i]);
	}
}

void camera::set_name(const char *in_name)
{
	if(in_name != NULL)
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
	set_custom_position_function(NULL);
	set_custom_orientation_function(NULL);
	if(n_object_host_submodel > 0)
	{
		if(objp->type == OBJ_SHIP)
		{
			ship_subsys* ssp = GET_FIRST(&Ships[objp->instance].subsys_list);
			while ( ssp != END_OF_LIST( &Ships[objp->instance].subsys_list ) )
			{
				if(ssp->system_info->subobj_num == n_object_host_submodel)
				{
					if(ssp->system_info->type == SUBSYSTEM_TURRET)
					{
						set_custom_position_function(get_turret_cam_pos);
						set_custom_orientation_function(get_turret_cam_orient);
					}
				}
				ssp = GET_NEXT( ssp );
			}
		}
	}
}

void camera::set_object_target(object *objp, int n_object_target_submodel)
{
	if(objp == NULL)
		object_target = object_h();

	object_target = object_h(objp);
	object_target_submodel = n_object_target_submodel;
}

/**
 * Custom function receives the already-modified current position value.
 * It should be replaced or added to as the custom function modifier sees fit.
 */
void camera::set_custom_position_function(void (*n_func_custom_position)(camera* /*cam*/, vec3d* /*camera_pos*/))
{
	func_custom_position = n_func_custom_position;
}

/**
 * Custom function receives the already-modified current orientation value.
 * It should be replaced or added to as the custom function modifier sees fit.
 */
void camera::set_custom_orientation_function(void (*n_func_custom_orientation)(camera* /*cam*/, matrix* /*camera_ori*/))
{
	func_custom_orientation = n_func_custom_orientation;
}

void camera::set_position(vec3d *in_position, float in_translation_time, float in_translation_acceleration_time, float in_translation_deceleration_time, float in_end_velocity)
{
	if(in_position != NULL && in_translation_time == 0.0f && in_translation_acceleration_time == 0.0f && in_translation_deceleration_time == 0.0f)
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

void camera::set_rotation(matrix *in_orientation, float in_rotation_time, float in_rotation_acceleration_time, float in_rotation_deceleration_time)
{
	if(in_orientation != NULL && in_rotation_time == 0.0f && in_rotation_acceleration_time == 0.0f && in_rotation_deceleration_time == 0.0f)
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
}

void camera::set_rotation(angles *in_angles, float in_rotation_time, float in_rotation_acceleration_time, float in_rotation_deceleration_time)
{
	matrix mtx = IDENTITY_MATRIX;
	vm_rotate_matrix_by_angles(&mtx, in_angles);
	this->set_rotation(&mtx, in_rotation_time, in_rotation_acceleration_time, in_rotation_deceleration_time);
}

void camera::set_rotation_facing(vec3d *in_target, float in_rotation_time, float in_rotation_acceleration_time, float in_rotation_deceleration_time)
{
	matrix temp_matrix = IDENTITY_MATRIX;

	if (in_target != nullptr)
	{
		vec3d position;
		matrix orient_buf;
		matrix *orient = nullptr;

		if (Use_host_orientation_for_set_camera_facing)
		{
			// we want to retrieve the camera's orientation, so provide a buffer for it
			orient = &orient_buf;

			// reset the camera's orientation state because if we have no host object, that is what the camera will default to
			c_ori = vmd_identity_matrix;
		}

		this->get_info(&position, orient, false);

		if (vm_vec_same(in_target, &position))
		{
			Warning(LOCATION, "Camera tried to point to self");
			return;
		}

		vec3d targetvec;
		vm_vec_normalized_dir(&targetvec, in_target, &position);

		if (Use_host_orientation_for_set_camera_facing)
		{
			// point along the target vector, but using the host orient's roll
			vm_vector_2_matrix(&temp_matrix, &targetvec, &orient->vec.uvec, nullptr);

			// if we have a host, we need the difference between the camera's current orient and the orient we want
			// if not, we will later set the absolute orientation, rather than the orientation relative to the host
			if (object_host.IsValid())
			{
				vm_transpose(orient);
				temp_matrix = temp_matrix * *orient;
			}
		}
		else
		{
			// point directly along the target vector
			vm_vector_2_matrix(&temp_matrix, &targetvec, nullptr, nullptr);
		}
	}

	set_rotation(&temp_matrix, in_rotation_time, in_rotation_acceleration_time, in_rotation_deceleration_time);
}

void camera::do_frame(float  /*in_frametime*/)
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

eye* get_submodel_eye(polymodel *pm, int submodel_num);
void camera::get_info(vec3d *position, matrix *orientation, bool apply_camera_orientation)
{
	if(position == NULL && orientation == NULL)
		return;
	
	eye* eyep = NULL;
	vec3d host_normal;
	matrix host_orient = vmd_identity_matrix;
	bool use_host_orient = false;

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
			polymodel *pm = nullptr;
			polymodel_instance *pmi = nullptr;
			
			if(model_num >= 0)
			{
				pm = model_get(model_num);
				if (objp->type == OBJ_SHIP)
					pmi = model_get_instance(Ships[objp->instance].model_instance_num);
			}

			if(object_host_submodel < 0 || pm == NULL)
			{
				vm_vec_unrotate(&c_pos, &pt, &object_host.objp->orient);
				vm_vec_add2(&c_pos, &object_host.objp->pos);
			}
			else
			{
				eyep = get_submodel_eye(pm, object_host_submodel);
				if(eyep)
				{
					Assertion(objp->type == OBJ_SHIP, "This part of the code expects the object to be a ship");

					vec3d c_pos_in;
					model_instance_local_to_global_point_dir(&c_pos_in, &host_normal, &eyep->pnt, &eyep->norm, pm, pmi, eyep->parent);
					vm_vec_unrotate(&c_pos, &c_pos_in, &objp->orient);
					vm_vec_add2(&c_pos, &objp->pos);
				}
				else
				{
					if (pmi != nullptr)
					{
						model_instance_local_to_global_point_orient(&c_pos, &host_orient, &pt, &vmd_identity_matrix, pm, pmi, object_host_submodel, &objp->orient, &objp->pos);
						use_host_orient = true;
					}
					else
						model_local_to_global_point( &c_pos, &pt, pm, object_host_submodel, &objp->orient, &objp->pos );
				}
			}
		}
		else
		{
			c_pos = pt;
		}

		//Do custom position stuff, if needed
		if(func_custom_position != NULL && !eyep)
		{
			func_custom_position(this, &c_pos);
		}
	}

	if (position != nullptr)
		*position = c_pos;

	//ORIENTATION
	if(orientation != NULL)
	{
		bool target_set = false;
		if(!(flags & CAM_STATIONARY_ORI) || object_target.IsValid() || object_host.IsValid())
		{
			if(object_target.IsValid())
			{
				object *target_objp = object_target.objp;
				int model_num = object_get_model(target_objp);
				polymodel *target_pm = nullptr;
				polymodel_instance *target_pmi = nullptr;
				vec3d target_pos = vmd_zero_vector;
				
				//See if we can get the model
				if(model_num >= 0)
				{
					target_pm = model_get(model_num);
					if (target_objp->type == OBJ_SHIP)
						target_pmi = model_get_instance(Ships[target_objp->instance].model_instance_num);
				}

				//If we don't have a submodel or don't have the model use object pos
				//Otherwise, find the submodel pos as it is rotated
				if (object_target_submodel < 0 || target_pm == nullptr)
				{
					target_pos = target_objp->pos;
				}
				else
				{
					if (target_pmi != nullptr)
						model_instance_local_to_global_point(&target_pos, &vmd_zero_vector, target_pm, target_pmi, object_target_submodel, &target_objp->orient, &target_objp->pos);
					else
						model_local_to_global_point( &target_pos, &vmd_zero_vector, target_pm, object_target_submodel, &target_objp->orient, &target_objp->pos );
				}

				vec3d targetvec;
				vm_vec_normalized_dir(&targetvec, &target_pos, &c_pos);
				vm_vector_2_matrix(&c_ori, &targetvec, NULL, NULL);
				target_set = true;
			}
			else if(object_host.IsValid())
			{
				if(eyep)
				{
					vm_vector_2_matrix(&c_ori, &host_normal, vm_vec_same(&host_normal, &object_host.objp->orient.vec.uvec)?NULL:&object_host.objp->orient.vec.uvec, NULL);
					target_set = true;
				}
				else if (use_host_orient)
				{
					c_ori = host_orient;
				}
				else
				{
					c_ori = object_host.objp->orient;
				}
			}
			else
			{
				c_ori = vmd_identity_matrix;
			}

			//Do custom orientation stuff, if needed
			if (func_custom_orientation != nullptr && !target_set)
			{
				func_custom_orientation(this, &c_ori);
			}

			// only do this if we want to find where the camera is actually pointing;
			// skip this if we just want the orientation of the host
			if (apply_camera_orientation)
			{
				matrix mtxA = c_ori;
				matrix mtxB = IDENTITY_MATRIX;
				float pos = 0.0f;
				for (int i = 0; i < 9; i++)
				{
					ori[i].get(&pos, nullptr);
					mtxB.a1d[i] = pos;
				}
				vm_matrix_x_matrix(&c_ori, &mtxA, &mtxB);

				vm_orthogonalize_matrix(&c_ori);
			}
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

void warp_camera::do_frame(float in_frametime)
{
	vec3d new_vel, delta_pos;

	apply_physics( c_damping, c_desired_vel.xyz.x, c_vel.xyz.x, in_frametime, &new_vel.xyz.x, &delta_pos.xyz.x );
	apply_physics( c_damping, c_desired_vel.xyz.y, c_vel.xyz.y, in_frametime, &new_vel.xyz.y, &delta_pos.xyz.y );
	apply_physics( c_damping, c_desired_vel.xyz.z, c_vel.xyz.z, in_frametime, &new_vel.xyz.z, &delta_pos.xyz.z );

	c_vel = new_vel;

	vm_vec_add2( &c_pos, &delta_pos );

	float ot = c_time+0.0f;

	c_time += in_frametime;

	if ( (ot < 0.667f) && ( c_time >= 0.667f ) )	{
		vec3d tmp;
		
		tmp.xyz.z = 4.739f;		// always go this fast forward.

		// pick x and y velocities so they are always on a 
		// circle with a 25 m radius.

		float tmp_angle = frand()*PI2;
	
		tmp.xyz.x = 22.0f * sinf(tmp_angle);
		tmp.xyz.y = -22.0f * cosf(tmp_angle);

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
subtitle::subtitle(int in_x_pos, int in_y_pos, const char* in_text, const char* in_imageanim, float in_display_time,
	float in_fade_time, const color* in_text_color, int in_text_fontnum, bool center_x, bool center_y, int in_width,
	int in_height, bool in_post_shaded, int in_line_height_modifier, bool in_adjust_wh)
	: display_time(-1.0f), fade_time(-1.0f), text_fontnum(-1), line_height_modifier(0),
	image_id(-1), time_displayed(-1.0f), time_displayed_end(-1.0f), post_shaded(false), do_screen_scaling(false)
{
	if ((in_text == nullptr || *in_text == '\0') && (in_imageanim == nullptr || *in_imageanim == '\0'))
		return;

	auto screen_w = gr_screen.center_w;
	auto screen_h = gr_screen.center_h;

	// we might be doing screen scaling
	if (Show_subtitle_screen_base_res[0] > 0 && Show_subtitle_screen_base_res[1] > 0)
	{
		// The way this works is we specify our subtitle coordinates relative to a base resolution, e.g. 1024x768.
		// The base resolution is adjusted behind the scenes to match the aspect ratio of the current resolution
		// (and the subtitle coordinates are adjusted as well).  Then the adjusted subtitles are resized by the
		// graphics code when they are drawn to the screen.
		do_screen_scaling = true;

		// we might need to adjust the coordinates for aspect ratio
		if (in_adjust_wh)
		{
			if (Show_subtitle_screen_base_res[0] != Show_subtitle_screen_adjusted_res[0])
			{
				in_x_pos = in_x_pos * Show_subtitle_screen_adjusted_res[0] / Show_subtitle_screen_base_res[0];
				in_width = in_width * Show_subtitle_screen_adjusted_res[0] / Show_subtitle_screen_base_res[0];
			}
			if (Show_subtitle_screen_base_res[1] != Show_subtitle_screen_adjusted_res[1])
			{
				in_y_pos = in_y_pos * Show_subtitle_screen_adjusted_res[1] / Show_subtitle_screen_base_res[1];
				in_height = in_height * Show_subtitle_screen_adjusted_res[1] / Show_subtitle_screen_base_res[1];
			}
		}

		// use these because we will be scaling the subtitle when it is displayed
		screen_w = Show_subtitle_screen_adjusted_res[0];
		screen_h = Show_subtitle_screen_adjusted_res[1];
	}

	// Initialize color
	gr_init_color(&text_color, 0, 0, 0);
	
	SCP_string text_buf;

	// basic init, this always has to be done
	memset( imageanim, 0, sizeof(imageanim) );
	memset( &text_pos, 0, 2*sizeof(int) );
	memset( &image_pos, 0, 4*sizeof(int) );

	if (in_text != NULL && in_text[0] != '\0')
	{
		text_buf = in_text;
		sexp_replace_variable_names_with_values(text_buf);
		sexp_container_replace_refs_with_values(text_buf);
		in_text = text_buf.c_str();
	}

	int num_text_lines = 0;
	const char *text_line_ptrs[MAX_SUBTITLE_LINES];
	int text_line_lens[MAX_SUBTITLE_LINES];

	//Setup text
	if ( (in_text != NULL) && (in_text[0] != '\0') ) {
		int split_width = (in_width > 0) ? in_width : 200;

		num_text_lines = split_str(in_text, split_width, text_line_lens, text_line_ptrs, MAX_SUBTITLE_LINES);
		for(int i = 0; i < num_text_lines; i++)
		{
			text_buf.assign(text_line_ptrs[i], text_line_lens[i]);
			text_lines.push_back(text_buf);
		}
	}

	//Setup text color
	if(in_text_color != NULL)
		text_color = *in_text_color;
	else
		gr_init_alphacolor(&text_color, 255, 255, 255, 255);
	text_fontnum = in_text_fontnum;
	line_height_modifier = in_line_height_modifier;

	//Setup display and fade time
	display_time = fl_abs(in_display_time);
	fade_time = fl_abs(in_fade_time);

	//Setup image
	if ( (in_imageanim != NULL) && (in_imageanim[0] != '\0') )
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
			old_fontnum = font::get_current_fontnum();
			font::set_font(text_fontnum);
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

			if (line_height_modifier != 0.0f)
			{
				if (Show_subtitle_uses_pixels)
					h = line_height_modifier;
				else
					h = fl2i(h * line_height_modifier / 100.0f);
			}

			th += h;
		}

		// restore old font
		if (old_fontnum >= 0)
		{
			font::set_font(old_fontnum);
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
			image_pos.x = (screen_w - tw)/2;
		if(center_y)
			image_pos.y = (screen_h - th)/2;
	}

	if(in_x_pos < 0 && !center_x)
		image_pos.x += screen_w + in_x_pos;
	else if(!center_x)
		image_pos.x += in_x_pos;

	if(in_y_pos < 0 && !center_y)
		image_pos.y += screen_h + in_y_pos;
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
		old_fontnum = font::get_current_fontnum();
		font::set_font(text_fontnum);
	}
	else
	{
		old_fontnum = -1;
	}

	int font_height = gr_get_font_height();
	int x = text_pos.x;
	int y = text_pos.y;

	auto sizing_mode = do_screen_scaling ? GR_RESIZE_FULL : GR_RESIZE_NONE;
	if (do_screen_scaling)
	{
		gr_set_screen_scale(Show_subtitle_screen_adjusted_res[0], Show_subtitle_screen_adjusted_res[1]);
		gr_set_clip(0, 0, Show_subtitle_screen_adjusted_res[0], Show_subtitle_screen_adjusted_res[1]);
	}

	// do the actual drawing ---------------------

	for(SCP_vector<SCP_string>::iterator line = text_lines.begin(); line != text_lines.end(); ++line)
	{
		gr_string(x, y, (char*)line->c_str(), sizing_mode);

		if (line_height_modifier != 0.0f)
		{
			if (Show_subtitle_uses_pixels)
				font_height = line_height_modifier;
			else
				font_height = fl2i(font_height * line_height_modifier / 100.0f);
		}

		y += font_height;
	}

	if(image_id >= 0)
	{
		gr_set_bitmap(image_id, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, text_color.alpha/255.0f);

		// image scaling?
		if (image_pos.w > 0 || image_pos.h > 0)
		{
			int orig_w, orig_h;
			vec3d scale;

			bm_get_info(image_id, &orig_w, &orig_h);
			scale.xyz.x = (image_pos.w > 0) ? (image_pos.w / (float) orig_w) : 1.0f;
			scale.xyz.y = (image_pos.h > 0) ? (image_pos.h / (float) orig_h) : 1.0f;
			scale.xyz.z = 1.0f;

			gr_push_scale_matrix(&scale);
			gr_bitmap(fl2i(image_pos.x / scale.xyz.x), fl2i(image_pos.y / scale.xyz.y), sizing_mode);
			gr_pop_scale_matrix();
		}
		// no scaling
		else
		{
			gr_bitmap(image_pos.x, image_pos.y, sizing_mode);
		}
	}

	// finished the actual drawing ---------------

	if (do_screen_scaling)
	{
		gr_reset_screen_scale();
		gr_reset_clip();
	}

	// restore old font
	if (old_fontnum >= 0)
		font::set_font(old_fontnum);

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
	text_lines = sub.text_lines;
	text_fontnum = sub.text_fontnum;
	line_height_modifier = sub.line_height_modifier;

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
	do_screen_scaling = sub.do_screen_scaling;
}

subtitle& subtitle::operator=(const subtitle &sub)
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

camid::camid(size_t n_idx, int n_sig)
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

size_t camid::getIndex()
{
	return idx;
}

int camid::getSignature()
{
	return sig;
}

bool camid::isValid()
{
	if(idx >= Cameras.size())
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
	for (auto ii = Cameras.begin(); ii != Cameras.end(); ++ii) {
		delete * ii;
	}
	Cameras.clear();
}

int cam_get_next_sig()
{
	static int next_sig = 0;
	return next_sig++;
}

camid cam_create(const char *n_name, vec3d *n_pos, vec3d *n_norm, object *n_object, int n_object_host_submodel)
{
	matrix ori;
	vm_vector_2_matrix_norm(&ori, n_norm);
	return cam_create(n_name, n_pos, &ori, n_object, n_object_host_submodel);
}

camid cam_create(const char *n_name, vec3d *n_pos, matrix *n_ori, object *n_object, int n_object_host_submodel)
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
	cam = new camera(buf, sig);
	cid = camid(Cameras.size(), sig);
	Cameras.push_back(cam);

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
	size_t i,size=Cameras.size();
	for(i = 0; i < size; i++)
	{
		if(Cameras[i] != NULL)
			Cameras[i]->do_frame(frametime);
	}
}

camid cam_get_camera(uint idx)
{
	if(idx >= Cameras.size())
		return camid();

	return camid(idx, Cameras[idx]->get_signature());
}

camid cam_get_current()
{
	return Current_camera;
}

size_t cam_get_num()
{
	return Cameras.size();
}

/**
 * Looks up camera by name, returns -1 on failure
 */
camid cam_lookup(const char* name)
{
	size_t i, size=Cameras.size();
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

	if (!Cutscene_camera_displays_hud) 
	{
		if(!Camera_hud_draw_saved)
		{
			Camera_hud_draw_value = hud_get_draw();
			Camera_hud_draw_saved = true;
		}
		hud_set_draw(0);
	}
	return true;
}

void cam_reset_camera()
{
	Viewer_mode &= ~VM_FREECAMERA;

	if (!Cutscene_camera_displays_hud) 
	{
		hud_set_draw(Camera_hud_draw_value);
		Camera_hud_draw_saved = false;
	}
}

void subtitles_close()
{
	Subtitles.clear();
}

void subtitles_do_frame(float frametime)
{
	SCP_vector<subtitle>::iterator sub;
	for(sub = Subtitles.begin(); sub != Subtitles.end(); ++sub)
	{
		if ( !sub->is_post_shaded( ) )
			sub->do_frame(frametime);
	}
}

void subtitles_do_frame_post_shaded(float frametime)
{
	SCP_vector<subtitle>::iterator sub;
	for(sub = Subtitles.begin(); sub != Subtitles.end(); ++sub)
	{
		if ( sub->is_post_shaded( ) )
			sub->do_frame(frametime);
	}
}

vec3d normal_cache;

void get_turret_cam_pos(camera *cam, vec3d *pos)
{
	object_h obj(cam->get_object_host());
	if(!obj.IsValid())
		return;
	ship* shipp = &Ships[cam->get_object_host()->instance];
	ship_subsys* ssp = GET_FIRST(&shipp->subsys_list);
	while ( ssp != END_OF_LIST( &shipp->subsys_list ) )
	{
		if(ssp->system_info->subobj_num == cam->get_object_host_submodel())
		{
			ship_get_global_turret_gun_info(cam->get_object_host(), ssp, pos, &normal_cache, 1, NULL);
			vec3d offset = vmd_zero_vector;
			offset.xyz.x = 0.0001f;
			vm_vec_add2(pos, &offset); // prevent beam turrets from crashing with a nullvec
			break;
		}
		ssp = GET_NEXT( ssp );
	}
}

void get_turret_cam_orient(camera *cam, matrix *ori)
{
	object_h obj(cam->get_object_host());
	if(!obj.IsValid())
		return;
	vm_vector_2_matrix(ori, &normal_cache, vm_vec_same(&normal_cache, &cam->get_object_host()->orient.vec.uvec)?NULL:&cam->get_object_host()->orient.vec.uvec, NULL);
}

eye* get_submodel_eye(polymodel *pm, int submodel_num)
{
	if(pm->n_view_positions > 0)
	{
		for(int i = 0; i < pm->n_view_positions; ++i)
		{
			if(pm->view_positions[i].parent == submodel_num)
			{
				return &pm->view_positions[i];
			}
			int sm = pm->submodel[submodel_num].first_child;
			while(sm > 0) //look for eyepoints attached to children, important for turret arms
			{
				if(pm->view_positions[i].parent == sm)
				{
					return &pm->view_positions[i];
				}
				sm = pm->submodel[sm].next_sibling;
			}
		}
	}
	return NULL;
}

// maybe push the camera away from preferred dist if it would intersect the ship's bounding box
float cam_get_bbox_dist(const object* viewer_obj, float preferred_distance, const matrix* cam_orient) {
	if (viewer_obj == nullptr)
		return preferred_distance;

	// radius is the maximal extent of the ship's bbox from the center, so if its at least that plus the padding its guaranteed to be good
	// plus a little fudge factor at the end because shields count against the bounding box, but not the radius
	if (preferred_distance > (viewer_obj->radius * EXTERN_CAM_BBOX_MULTIPLIER_PADDING + EXTERN_CAM_BBOX_CONSTANT_PADDING) * 1.5f) 
		return preferred_distance;
	
	int modelnum = object_get_model(viewer_obj);
	polymodel* pm = model_get(modelnum);
	vec3d adjusted_bbox;
	const vec3d* cam_vec = &cam_orient->vec.fvec;

	// the bounding box is expanded by the padding and maybe use negative min instead of max, because they could differ significantly
	// and we only care about the one thats on the side of the camera
	adjusted_bbox.xyz.x = (cam_vec->xyz.x < 0 ? -pm->mins.xyz.x : pm->maxs.xyz.x) * EXTERN_CAM_BBOX_MULTIPLIER_PADDING + EXTERN_CAM_BBOX_CONSTANT_PADDING;
	adjusted_bbox.xyz.y = (cam_vec->xyz.y < 0 ? -pm->mins.xyz.y : pm->maxs.xyz.y) * EXTERN_CAM_BBOX_MULTIPLIER_PADDING + EXTERN_CAM_BBOX_CONSTANT_PADDING;
	adjusted_bbox.xyz.z = (cam_vec->xyz.z < 0 ? -pm->mins.xyz.z : pm->maxs.xyz.z) * EXTERN_CAM_BBOX_MULTIPLIER_PADDING + EXTERN_CAM_BBOX_CONSTANT_PADDING;

	vec3d adjusted_dist;
	// adjusted_dist is the vector (with an inverted magnitude) that contacts the surface of an 
	// ellipsoid which contacts the adjusted_bbox at each of its faces 
	adjusted_dist.xyz.x = cam_vec->xyz.x / adjusted_bbox.xyz.x;
	adjusted_dist.xyz.y = cam_vec->xyz.y / adjusted_bbox.xyz.y;
	adjusted_dist.xyz.z = cam_vec->xyz.z / adjusted_bbox.xyz.z;
	float compensation =  vm_vec_mag(&adjusted_dist);

	return fmaxf(1 / compensation, preferred_distance);
}

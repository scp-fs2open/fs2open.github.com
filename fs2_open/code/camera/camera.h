
#ifndef _CAMERA_H
#define _CAMERA_H

#pragma warning(push, 2)	// ignore all those warnings for Microsoft stuff
#include <string>
#include <vector>
#pragma warning(pop)

#include "globalincs/pstypes.h"
#include "graphics/2d.h"

class camera
{
private:
	vec3d	position;
	vec3d	*desired_position;		//NULL unless we're trying to move somewhere
	vec3d	translation_velocity;
	vec3d	translation_vel_limit;
	vec3d	translation_acc_limit;

	matrix	orientation;
	matrix	*desired_orientation;	//NULL unless we're trying to point somewhere
	vec3d	rotation_rate;
	vec3d	rotation_vel_limit;
	vec3d	rotation_acc_limit;
public:
	camera();
	~camera();
	void reset();

	//Set
	void set_position(vec3d *in_position, float in_translation_time = 0.0f, float in_translation_acceleration_time = 0.0f);
	void set_translation_velocity(vec3d *in_velocity);
	void set_rotation(matrix *in_orientation, float in_rotation_time = 0.0f, float in_rotation_acceleration_time = 0.0f);
	void set_rotation(angles *in_angles, float in_rotation_time = 0.0f, float in_rotation_acceleration_time = 0.0f);
	void set_rotation_facing(vec3d *in_target, float in_rotation_time = 0.0f, float in_rotation_acceleration_time = 0.0f);
	void set_rotation_velocity(vec3d *in_rotation_rate);

	//Get
	//These return values should never ever be changed, I do this in the interest of speed.
	vec3d *get_position() {return &position;}
	matrix *get_orientation() {return &orientation;}

	//Do
	void do_frame(float in_frametime);
};

class subtitle
{
private:
	std::vector<std::string> text_lines;
	int text_pos[2];

	float display_time;
	float fade_time;
	color text_color;

	//Done with set
	char imageanim[MAX_FILENAME_LEN];
	int image_id;
	int image_pos[2];
	
	//Time this has been displayed
	float time_displayed;

	//When to end it
	float time_displayed_end;
public:
	subtitle(int in_x_pos, int in_y_pos, char* in_text, float in_display_time, char* in_imageanim = NULL, float in_fade_time = 0.0f, color *in_text_color = NULL, bool center_x = false, bool center_y = false, int in_width = 200);
	~subtitle();

	void do_frame(float frametime);
};

//Some global stuff
extern std::vector<subtitle> Subtitles;
extern std::vector<camera> Cameras;
//Preset cameras
extern camera* Free_camera;

//Helpful functions
void cameras_init();
void cameras_close();
void cameras_do_frame(float frametime);
void subtitles_init();
void subtitles_close();
void subtitles_do_frame(float frametime);

#endif // _CAMERA_H

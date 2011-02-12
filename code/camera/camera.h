
#ifndef _CAMERA_H
#define _CAMERA_H

#include "graphics/2d.h"
#include "globalincs/pstypes.h"
#include "globalincs/globals.h"
#include "physics/physics.h"	//for avd
#include "object/object.h"

#include <string>

#define CAM_STATIONARY_FOV			(1<<0)
#define CAM_STATIONARY_ORI			(1<<1)
#define CAM_STATIONARY_POS			(1<<2)
#define CAM_DEFAULT_FLAGS			0

class camera
{
	friend class avd_camera;
protected:
	char name[NAME_LENGTH];
	int sig;
	int flags;

	object_h object_host;
	int object_host_submodel;

	object_h object_target;
	int object_target_submodel;

	void (*func_custom_position)(camera *cam, vec3d *pos);
	void (*func_custom_orientation)(camera *cam, matrix *ori);

	avd_movement fov;

	avd_movement pos_x;
	avd_movement pos_y;
	avd_movement pos_z;

	//avd_movement ori_p;
	//avd_movement ori_b;
	//avd_movement ori_h;
	//avd_movement ori_progress;
	//matrix ori_initial;
	//matrix ori_final;
	avd_movement ori[9];

	//Cache stuff
	float c_fov;
	vec3d c_pos;
	matrix c_ori;
public:
	camera(char *in_name=NULL, int in_signature=-1);
	~camera();
	void clear();
	void reset();

	//Set
	void set_name(char *in_name);

	void set_object_host(object *objp, int n_object_host_submodel = -1);
	void set_object_target(object *objp, int n_object_target_submodel = -1);

	void set_custom_position_function(void (*n_func_custom_position)(camera *cam, vec3d *pos));
	void set_custom_orientation_function(void (*n_func_custom_orientation)(camera *cam, matrix *ori));

	void set_fov(float in_fov, float in_fov_time = 0.0f, float in_fov_acceleration_time = 0.0f, float in_deceleration_time = 0.0f);

	void set_position(vec3d *in_position = NULL, float in_translation_time = 0.0f, float in_translation_acceleration_time = 0.0f, float in_translation_deceleration_time = 0.0f, float in_end_velocity = 0.0f);
	void set_translation_velocity(vec3d *in_velocity, float in_acceleration_time = 0.0f);

	void set_rotation(matrix *in_orientation = NULL, float in_rotation_time = 0.0f, float in_rotation_acceleration_time = 0.0f, float in_rotation_deceleration_time = 0.0f);
	void set_rotation(angles *in_angles, float in_rotation_time = 0.0f, float in_rotation_acceleration_time = 0.0f, float in_rotation_deceleration_time = 0.0f);
	void set_rotation_facing(vec3d *in_target, float in_rotation_time = 0.0f, float in_rotation_acceleration_time = 0.0f, float in_rotation_deceleration_time = 0.0f);
	void set_rotation_velocity(angles *in_rotation_rate, float in_acceleration_time = 0.0f);

	//Get
	char *get_name() {return name;}
	int get_signature() {return sig;}
	object *get_object_host();
	int get_object_host_submodel();
	object *get_object_target();
	int get_object_target_submodel();
	float get_fov();
	void get_info(vec3d *position, matrix *orientation);

	//Is
	bool is_empty(){return sig < 0;}

	//Do
	void do_frame(float in_frametime);
};

class warp_camera
{
private:
	float c_time;
	float c_damping;

	vec3d c_vel;
	vec3d c_desired_vel;

	vec3d c_pos;
	matrix c_ori;
public:
	warp_camera();
	warp_camera(object *objp);
	void reset();

	void set_position(vec3d *in_pos);
	void set_rotation(matrix *in_ori);
	void set_velocity(vec3d *in_vel, bool instantaneous);

	void do_frame(float in_frametime);

	void get_info(vec3d *position, matrix *orientation);
};

extern warp_camera Warp_camera;

class subtitle
{
private:
	void clone(const subtitle &sub);

	SCP_vector<std::string> text_lines;
	struct { int x; int y; } text_pos;

	float display_time;
	float fade_time;
	color text_color;
	int text_fontnum;

	//Done with set
	char imageanim[MAX_FILENAME_LEN];
	int image_id;
	struct { int x; int y; int w; int h; } image_pos;
	
	//Time this has been displayed
	float time_displayed;

	//When to end it
	float time_displayed_end;

	bool post_shaded;
public:
	subtitle(int in_x_pos, int in_y_pos, char* in_text = NULL, char* in_imageanim = NULL,
			 float in_display_time = 0, float in_fade_time = 0.0f, color *in_text_color = NULL, int in_text_fontnum = -1,
			 bool center_x = false, bool center_y = false, int in_width = 0, int in_height = 0, bool post_shaded=false);
	~subtitle();

    subtitle(const subtitle &sub) { clone(sub); }
    const subtitle &operator=(const subtitle &sub);

	void do_frame(float frametime);
	bool is_post_shaded( ) { return post_shaded; }
};

//Some global stuff
extern SCP_vector<subtitle> Subtitles;
extern float VIEWER_ZOOM_DEFAULT;
extern float Sexp_fov;

//Helpful functions
void cam_init();
void cam_close();
void cam_do_frame(float frametime);
camid cam_create(char *n_name=NULL, vec3d *n_pos=NULL, matrix *n_ori=NULL, object *n_object=NULL, int n_submodel_parent=-1);
camid cam_create(char *n_name, vec3d *n_pos, vec3d *n_norm, object *n_object=NULL, int n_submodel_parent=-1);
void cam_delete(camid cid);
bool cam_set_camera(camid cid);
void cam_reset_camera();
camid cam_lookup(char *name);
camid cam_get_camera(uint index);
camid cam_get_current();
uint cam_get_num();

void subtitles_init();
void subtitles_close();
void subtitles_do_frame(float frametime);
void subtitles_do_frame_post_shaded(float frametime);

#endif // _CAMERA_H

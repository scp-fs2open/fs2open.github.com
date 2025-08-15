/*
 * Created by Olivier "LuaPineapple" Hamel for the Freespace 2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */


#ifndef _RADAR_DRADIS_H
#define _RADAR_DRADIS_H

#include "globalincs/pstypes.h"
#include "radar/radarsetup.h"
#include "gamesnd/gamesnd.h"

class object;
struct blip;
struct color;

class HudGaugeRadarDradis: public HudGaugeRadar
{
	// bitmap IDs of the textures used to render DRADIS
	int xy_plane;
	int xz_yz_plane;
	int sweep_plane;
	int target_brackets;
	int unknown_contact_icon;

	int sweep_duration; // in milliseconds
	float sweep_angle;

	matrix view_perturb;
	vec3d Orb_eye_position;

	vec3d fx_guides0_0;
	vec3d fx_guides0_1;

	vec3d fx_guides1_0;
	vec3d fx_guides1_1;

	vec3d fx_guides2_0;
	vec3d fx_guides2_1;

	vec3d sweep_normal_x;
	vec3d sweep_normal_y;
	vec3d sweep_normal_z;

	float scale;

	bool sub_y_clip;

	sound_handle loop_sound_handle;
	gamesnd_id m_loop_snd;
	float loop_sound_volume;

	gamesnd_id arrival_beep_snd;
	gamesnd_id departure_beep_snd;

	gamesnd_id stealth_arrival_snd;
	gamesnd_id stealth_departure_snd;

	int arrival_beep_delay;
	int departure_beep_delay;

	TIMESTAMP arrival_beep_next_check;
	TIMESTAMP departure_beep_next_check;
protected:
	bool shouldDoSounds();
public:
	HudGaugeRadarDradis();
	void initBitmaps(char* fname_xy, char* fname_xz_yz, char* fname_sweep, char* fname_target_brackets, char* fname_unknown);
	void initSound(gamesnd_id loop_snd, float _loop_sound_volume,  gamesnd_id arrival_snd, gamesnd_id departure_snd, gamesnd_id _stealth_arrival_snd, gamesnd_id _stealth_departure_snd, float arrival_delay, float departure_delay);

	void blipDrawDistorted(blip *b, vec3d *pos, float alpha);
	void blipDrawFlicker(blip *b, vec3d *pos, float alpha);
	void drawBlips(int blip_type, int bright, int distort);
	void drawBlipsSorted(int distort);
	void drawContact(vec3d *pnt, int idx, int clr_idx, float dist, float alpha, float scale_factor);
	void drawSweeps();
	void doneDrawingHtl();
	void drawOutlinesHtl();
	void setupViewHtl();
	void render(float frametime, bool config = false) override;
	void pageIn() override;
	void plotBlip(blip* b, vec3d *pos, float *alpha);

	void onFrame(float frametime) override;
	void initialize() override;

	// Sound specific functions
	void doLoopSnd();
	void doBeeps();
};

#endif


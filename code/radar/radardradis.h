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

extern int Radar_static_looping;

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

	float sweep_duration; // in seconds
	float sweep_percent;

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

	int loop_sound_handle;
	int m_loop_snd;
	float loop_sound_volume;

	int arrival_beep_snd;
	int departure_beep_snd;

	int m_stealth_arrival_snd;
	int stealth_departure_snd;

	int arrival_beep_delay;
	int departure_beep_delay;

	int arrival_beep_next_check;
	int departure_beep_next_check;
protected:
	bool shouldDoSounds();
public:
	HudGaugeRadarDradis();
	void initBitmaps(char* fname_xy, char* fname_xz_yz, char* fname_sweep, char* fname_target_brackets, char* fname_unknown);
	void initSound(int loop_snd, float loop_sound_volume,  int arrival_snd, int departue_snd, int stealth_arrival_snd, int stealth_departue_snd, float arrival_delay, float departure_delay);

	void blipDrawDistorted(blip *b, vec3d *pos, float alpha);
	void blipDrawFlicker(blip *b, vec3d *pos, float alpha);
	void drawBlips(int blip_type, int bright, int distort);
	void drawBlipsSorted(int distort);
	void drawContact(vec3d *pnt, int idx, int clr_idx, float dist, float alpha, float scale);
	void drawContactImage(vec3d *pnt, int rad, int idx, int clr_idx, float mult);
	void drawSweeps();
	void drawCrosshairs( vec3d pnt );
	void doneDrawing();
	void doneDrawingHtl();
	void drawOutlines();
	void drawOutlinesHtl();
	void setupViewHtl();
	int calcAlpha(vec3d* pt);
	void render(float frametime);
	void pageIn();
	void plotBlip(blip* b, vec3d *pos, float *alpha);

	virtual void onFrame(float frametime);
	virtual void initialize();

	// Sound specific functions
	void doLoopSnd();
	void doBeeps();
};

#endif


/*
 * Created by Olivier "LuaPineapple" Hamel for the Freespace 2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */

#include "radar/radarorb.h"
#include "graphics/font.h"
#include "bmpman/bmpman.h"
#include "object/object.h"
#include "ship/ship.h"
#include "playerman/player.h"
#include "weapon/weapon.h"
#include "io/timer.h"
#include "hud/hud.h"
#include "hud/hudconfig.h"
#include "ship/subsysdamage.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/linklist.h"
#include "network/multi.h"
#include "weapon/emp.h"
#include "freespace2/freespace.h"
#include "localization/localize.h"
#include "ship/awacs.h"
#include "render/3d.h"
#include "iff_defs/iff_defs.h"
#include "jumpnode/jumpnode.h"
#include "math/staticrand.h"

#include "radar/radarsetup.h"
#include "radar/radardradis.h"

#include "hud/hudwingmanstatus.h"
#include "globalincs/systemvars.h"

#define RADIANS_PER_DEGREE (PI / 180.0f)

HudGaugeRadarDradis::HudGaugeRadarDradis():
HudGaugeRadar(HUD_OBJECT_RADAR_BSG, 255, 255, 255), 
xy_plane(-1), xz_yz_plane(-1), sweep_plane(-1), target_brackets(-1), unknown_contact_icon(-1), sweep_duration(6.0), sweep_percent(0.0), scale(1.20f), sub_y_clip(false)
{
	vm_vec_copy_scale(&sweep_normal_x, &vmd_zero_vector, 1.0f);
	vm_vec_copy_scale(&sweep_normal_y, &vmd_zero_vector, 1.0f);
	vm_vec_copy_scale(&sweep_normal_z, &vmd_zero_vector, 1.0f);

	// init the view perturb matrix
	view_perturb.a2d[0][0] = 1.0f;
	view_perturb.a2d[0][1] = 0.0f;
	view_perturb.a2d[0][2] = 0.0f;
	view_perturb.a2d[1][0] = 0.0f;
	view_perturb.a2d[1][1] = -1.0f;
	view_perturb.a2d[1][2] = 0.0f;
	view_perturb.a2d[2][0] = 0.0f;
	view_perturb.a2d[2][1] = 0.0f;
	view_perturb.a2d[2][2] = 1.0f;

	// init the orb eye position
	Orb_eye_position.a1d[0] = 0.0f;
	Orb_eye_position.a1d[1] = 0.0f;
	Orb_eye_position.a1d[2] = -2.5f;

	fx_guides0_0.a1d[0] = -1.0f;
	fx_guides0_0.a1d[1] = 0.0f;
	fx_guides0_0.a1d[2] = 0.0f;
	
	fx_guides0_1.a1d[0] = 1.0f;
	fx_guides0_1.a1d[1] = 0.0f;
	fx_guides0_1.a1d[2] = 0.0f;

	fx_guides1_0.a1d[0] = 0.0f;
	fx_guides1_0.a1d[1] = -1.0f;
	fx_guides1_0.a1d[2] = 0.0f;
	
	fx_guides1_1.a1d[0] = 0.0f;
	fx_guides1_1.a1d[1] = 1.0f;
	fx_guides1_1.a1d[2] = 0.0f;

	fx_guides2_0.a1d[0] = 0.0f;
	fx_guides2_0.a1d[1] = 0.0f;
	fx_guides2_0.a1d[2] = -1.0f;
	
	fx_guides2_1.a1d[0] = 0.0f;
	fx_guides2_1.a1d[1] = 0.0f;
	fx_guides2_1.a1d[2] = 1.0f;
	
	// give it some color
	gr_init_alphacolor(&orb_color, 48, 96, 160, 1337);

	this->loop_sound_handle = -1;
}

void HudGaugeRadarDradis::initBitmaps(char* fname_xy, char* fname_xz_yz, char* fname_sweep, char* fname_target_brackets, char* fname_unknown)
{
	xy_plane = bm_load(fname_xy); // Base
	if ( xy_plane < 0 ) {
		Warning(LOCATION,"Cannot load hud bitmap: %s\n", fname_xy);
	}

	xz_yz_plane = bm_load(fname_xz_yz); // Two vertical cross rings
	if ( xz_yz_plane < 0 ) {
		Warning(LOCATION,"Cannot load hud bitmap: %s\n", fname_xz_yz);
	}

	sweep_plane = bm_load(fname_sweep); // Sweep lines
	if ( sweep_plane < 0 ) {
		Warning(LOCATION,"Cannot load hud bitmap: %s\n", fname_sweep);
	}

	target_brackets = bm_load(fname_target_brackets);
	if ( target_brackets < 0 ) {
		Warning(LOCATION,"Cannot load hud bitmap: %s\n", fname_target_brackets);
	}

	unknown_contact_icon = bm_load(fname_unknown);
	if ( unknown_contact_icon < 0 ) {
		Warning(LOCATION,"Cannot load hud bitmap: %s\n", fname_unknown);
	}
}

void HudGaugeRadarDradis::plotBlip(blip* b, vec3d *pos, float *alpha)
{
	*pos = b->position;
	vm_vec_normalize(pos);

	if (ship_is_tagged(b->objp)) {
		*alpha = 1.0f;
		return;
	}

	float fade_multi = 1.5f;
	
	if (b->objp->type == OBJ_SHIP) {
		if (Ships[b->objp->instance].flags2 & SF2_STEALTH) {
			fade_multi *= 2.0f;
		}
	}

	*alpha = 1.0f - (sweep_percent /(PI*2))*fade_multi/2.0f;
	
	if (*alpha < 0.0f) {
		*alpha = 0.0f;
	}
}

void HudGaugeRadarDradis::drawContact(vec3d *pnt, int idx, float dist, float alpha)
{
	vec3d  p;
	int h, w;
	vertex vert;
	float aspect_mp;
	float scale = 0.6f;

	if ((sub_y_clip && (pnt->xyz.y > 0)) || ((!sub_y_clip) && (pnt->xyz.y <= 0)))
		return;

	vm_vec_rotate(&p, pnt,  &vmd_identity_matrix); 
	g3_transfer_vertex(&vert, &p);

	float range = player_farthest_weapon_range();
	if(dist <= range) {
		scale = 0.6f + (range-dist)/range;
	}
	
	bm_get_info(idx, &w, &h);

	if (h == w) {
        aspect_mp = 1.0f;
    } else {
        aspect_mp = (((float) h) / ((float) w));
    }

	float sizef = fl_sqrt(vm_vec_dist(&Orb_eye_position, pnt) * 8.0f) * scale;

	matrix flip = vmd_identity_matrix;
	flip.vec.uvec.xyz.y = -1.0;

	gr_set_bitmap(idx, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, alpha);
	g3_draw_polygon(&p, &vmd_identity_matrix, sizef/35.0f, aspect_mp*sizef/35.0f, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);
}

// radar is damaged, so make blips dance around
void HudGaugeRadarDradis::blipDrawDistorted(blip *b, vec3d *pos, float alpha)
{
	float scale;
	float dist = vm_vec_normalize(pos);
	vec3d out;
	float distortion_angle=20;

	// maybe alter the effect if EMP is active
	if (emp_active_local())
	{
		scale = emp_current_intensity();
		dist *= frand_range(MAX(0.75f, 0.75f*scale), MIN(1.25f, 1.25f*scale));
		distortion_angle *= frand_range(-3.0f,3.0f)*frand_range(0.0f, scale);

		if (dist > 1.0f) dist = 1.0f;
		if (dist < 0.1f) dist = 0.1f;
	}

	vm_vec_random_cone(&out, pos, distortion_angle);
	vm_vec_scale(&out, dist);

	drawContact(&out, unknown_contact_icon, b->dist, alpha);
}

// blip is for a target immune to sensors, so cause to flicker in/out with mild distortion
void HudGaugeRadarDradis::blipDrawFlicker(blip *b, vec3d *pos, float alpha)
{
	int flicker_index;

	float dist=vm_vec_normalize(pos);
	vec3d out;
	float distortion_angle=10;

	if ((b-Blips) & 1)
		flicker_index=0;
	else
		flicker_index=1;
	

	if (timestamp_elapsed(Radar_flicker_timer[flicker_index])) {
		Radar_flicker_timer[flicker_index] = timestamp_rand(50,1000);
		Radar_flicker_on[flicker_index] ^= 1;
	}

	if (!Radar_flicker_on[flicker_index])
		return;

	if (rand() & 1)
	{
		distortion_angle *= frand_range(0.1f,2.0f);
		dist *= frand_range(0.75f, 1.25f);

		if (dist > 1.0f) dist = 1.0f;
		if (dist < 0.1f) dist = 0.1f;
	}
	
	vm_vec_random_cone(&out,pos,distortion_angle);
	vm_vec_scale(&out,dist);

	drawContact(&out, unknown_contact_icon, b->dist, alpha);
}

// Draw all the active radar blips
void HudGaugeRadarDradis::drawBlips(int blip_type, int bright, int distort)
{
	blip *b = NULL;
	blip *blip_head;
	vec3d pos;
	float alpha;
	
	Assert((blip_type >= 0) && (blip_type < MAX_BLIP_TYPES));
	
	//long frametime = timer_get_approx_seconds();
	// Need to set font.
	gr_set_font(FONT1);

	if(bright) {
		blip_head = &Blip_bright_list[blip_type];
	} else {
		blip_head = &Blip_dim_list[blip_type];
	}
	
	// draw all blips of this type
	for (b = GET_FIRST(blip_head); b != END_OF_LIST(blip_head); b = GET_NEXT(b))
	{
		plotBlip(b, &pos, &alpha);
		
		gr_set_color_fast(b->blip_color);

		// maybe draw cool blip to indicate current target
		if (b->flags & BLIP_CURRENT_TARGET)
		{
			alpha = 1.0;
			b->rad = Radar_blip_radius_target;
			drawContact(&pos, target_brackets, b->dist, alpha);
		}
		else {
			b->rad = Radar_blip_radius_normal;
		}

		// maybe distort blip
		if (distort) {
			blipDrawDistorted(b, &pos, alpha);
		} else {
			if (b->flags & BLIP_DRAW_DISTORTED) {
				blipDrawFlicker(b, &pos, alpha);
			} else if (b->radar_image_2d >= 0) {
				drawContact(&pos, b->radar_image_2d, b->dist, alpha);
			} else {
				drawContact(&pos, unknown_contact_icon, b->dist, alpha);
			}
		}
	}
}

void HudGaugeRadarDradis::setupViewHtl()
{
	setClip(position[0], position[1], Radar_radius[0], Radar_radius[1]);
	gr_set_proj_matrix(.625f * PI_2, i2fl(Radar_radius[0])/i2fl(Radar_radius[1]), 0.001f, 5.0f);
	gr_set_view_matrix(&Orb_eye_position, &vmd_identity_matrix);

	gr_zbuffer_set(0);
}

void HudGaugeRadarDradis::doneDrawingHtl()
{
	gr_end_view_matrix();
	gr_end_proj_matrix();
	
	//hud_save_restore_camera_data(0);

	gr_zbuffer_set(1);
}

void HudGaugeRadarDradis::drawOutlinesHtl()
{
	matrix base_tilt = vmd_identity_matrix;
	vec3d base_tilt_norm;

	if ((xy_plane == -1) || (xz_yz_plane == -1))
		return;
	
	g3_start_instance_matrix(&vmd_zero_vector, /*&Player_obj->orient*/&vmd_identity_matrix, true);
		gr_init_alphacolor(&orb_color, 100, 150, 210, 255);
		gr_set_color_fast(&orb_color);
		
		gr_set_line_width(1.5f);
			g3_draw_htl_line(&fx_guides0_0, &fx_guides0_1);
			g3_draw_htl_line(&fx_guides1_0, &fx_guides1_1);
			g3_draw_htl_line(&fx_guides2_0, &fx_guides2_1);
		gr_set_line_width(1.0f);
		
		// Tilt the base disc component of DRADIS-style radar 30 degrees down
		vm_angle_2_matrix(&base_tilt, PI/6, 0);
		vm_vec_rotate(&base_tilt_norm, &vmd_y_vector, &base_tilt);
		
		gr_set_bitmap(xy_plane, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f); // base
		g3_draw_polygon(&vmd_zero_vector, &base_tilt_norm, scale, scale, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);
		
		gr_set_bitmap(xz_yz_plane, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);
		
		g3_draw_polygon(&vmd_zero_vector, &vmd_x_vector, scale, scale, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT); // forward facing ring
		g3_draw_polygon(&vmd_zero_vector, &vmd_z_vector, scale, scale, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT); // side facing ring
	g3_done_instance(true);
}

void HudGaugeRadarDradis::drawSweeps()
{
	if (sweep_plane == -1)
		return;
	
	sweep_percent = (fmod(((float)game_get_overall_frametime() / (float)65536), sweep_duration) /  sweep_duration) * PI * 2; // convert to radians from 0 <-> 1
	float sweep_perc_z = sweep_percent * -0.5f;

	vec3d sweep_a;
	vec3d sweep_b;
	vec3d sweep_c;
	
	vm_rot_point_around_line(&sweep_a, &vmd_y_vector, sweep_percent, &vmd_zero_vector, &vmd_z_vector); // Sweep line: XZ
	vm_rot_point_around_line(&sweep_b, &vmd_y_vector, sweep_percent, &vmd_zero_vector, &vmd_x_vector); // Sweep line: YZ
	vm_rot_point_around_line(&sweep_c, &vmd_x_vector, sweep_perc_z, &vmd_zero_vector, &vmd_y_vector); // Sweep line: XY
	
	vm_vec_copy_scale(&sweep_normal_x, &sweep_a, 1.0f);
	vm_vec_copy_scale(&sweep_normal_y, &sweep_b, 1.0f);
	
	g3_start_instance_matrix(&vmd_zero_vector, /*&Player_obj->orient*/&vmd_identity_matrix, true);
		gr_set_bitmap(sweep_plane, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);
		
		g3_draw_polygon(&vmd_zero_vector, &sweep_a, scale, scale, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);
		g3_draw_polygon(&vmd_zero_vector, &sweep_b, scale, scale, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);
		g3_draw_polygon(&vmd_zero_vector, &sweep_c, scale, scale, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);

		float rotation = sweep_percent;

		vm_rot_point_around_line(&sweep_a, &vmd_y_vector, rotation, &vmd_zero_vector, &vmd_z_vector); // Sweep line: XZ
		vm_rot_point_around_line(&sweep_b, &vmd_y_vector, rotation, &vmd_zero_vector, &vmd_x_vector); // Sweep line: YZ
		vm_rot_point_around_line(&sweep_c, &vmd_x_vector,sweep_perc_z, &vmd_zero_vector, &vmd_y_vector); // Sweep line: YZ
		
		gr_set_bitmap(sweep_plane, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL);

		g3_draw_polygon(&vmd_zero_vector, &sweep_a, scale, scale, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT); // Sweep line: XZ
		g3_draw_polygon(&vmd_zero_vector, &sweep_b, scale, scale, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT); // Sweep line: YZ
		g3_draw_polygon(&vmd_zero_vector, &sweep_c, scale, scale, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);

		/*int dist = 90;

		for(int i = 1; i < dist; i++)
		{
			float rotation = sweep_percent - (i * RADIANS_PER_DEGREE);
			float alpha	= (1.0f - (float)((float)i / (float)dist)) * 0.25f;
			
			//if (i < 2)
				//alpha = 1.0f;

			gr_set_bitmap(sweep_plane, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, alpha);
			
			vm_rot_point_around_line(&sweep_a, &vmd_y_vector, rotation, &vmd_zero_vector, &vmd_z_vector); // Sweep line: XZ
			vm_rot_point_around_line(&sweep_b, &vmd_y_vector, rotation, &vmd_zero_vector, &vmd_x_vector); // Sweep line: YZ
			
			g3_draw_polygon(&vmd_zero_vector, &sweep_a, scale, scale, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT); // Sweep line: XZ
			g3_draw_polygon(&vmd_zero_vector, &sweep_b, scale, scale, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT); // Sweep line: YZ
			
			if (i < (dist * 0.5f))
			{
				vm_rot_point_around_line(&sweep_c, &vmd_x_vector,sweep_perc_z + (i * RADIANS_PER_DEGREE), &vmd_zero_vector, &vmd_y_vector); // Sweep line: YZ
				g3_draw_polygon(&vmd_zero_vector, &sweep_c, scale, scale, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);
			}
		}*/
		
	g3_done_instance(true);
}

void HudGaugeRadarDradis::drawBlipsSorted(int distort)
{
	matrix base_tilt = vmd_identity_matrix;
	
	vm_angle_2_matrix(&base_tilt, -PI/6, 0);
	
	for(int is_bright = 0; is_bright < 2; is_bright++) {
		sub_y_clip = true;
		g3_start_instance_matrix(&vmd_zero_vector, /*&Player_obj->orient*/&base_tilt, true);
			drawBlips(BLIP_TYPE_BOMB, is_bright, distort);
			drawBlips(BLIP_TYPE_JUMP_NODE, is_bright, distort);
			drawBlips(BLIP_TYPE_NORMAL_SHIP, is_bright, distort);
			drawBlips(BLIP_TYPE_TAGGED_SHIP, is_bright, distort);
			drawBlips(BLIP_TYPE_WARPING_SHIP, is_bright, distort);
			drawBlips(BLIP_TYPE_NAVBUOY_CARGO, is_bright, distort);
		g3_done_instance(true);
		
		drawOutlinesHtl();

		sub_y_clip = false;
		g3_start_instance_matrix(&vmd_zero_vector, /*&Player_obj->orient*/&base_tilt, true);
			drawBlips(BLIP_TYPE_BOMB, is_bright, distort);
			drawBlips(BLIP_TYPE_JUMP_NODE, is_bright, distort);
			drawBlips(BLIP_TYPE_NORMAL_SHIP, is_bright, distort);
			drawBlips(BLIP_TYPE_TAGGED_SHIP, is_bright, distort);
			drawBlips(BLIP_TYPE_WARPING_SHIP, is_bright, distort);
			drawBlips(BLIP_TYPE_NAVBUOY_CARGO, is_bright, distort);
		g3_done_instance(true);
	}
}


void HudGaugeRadarDradis::render(float frametime)
{
	float sensors_str;
	int   ok_to_blit_radar;
	
	ok_to_blit_radar = 1;

	sensors_str = ship_get_subsystem_strength(Player_ship, SUBSYSTEM_SENSORS);

	if (ship_subsys_disrupted(Player_ship, SUBSYSTEM_SENSORS))
		sensors_str = MIN_SENSOR_STR_TO_RADAR - 1;

	// note that on lowest skill level, there is no radar effects due to sensors damage
	if ((Game_skill_level == 0) || (sensors_str > SENSOR_STR_RADAR_NO_EFFECTS))
	{
		Radar_static_playing = 0;
		Radar_static_next = 0;
		Radar_death_timer = 0;
		Radar_avail_prev_frame = 1;
	}
	else
		if (sensors_str < MIN_SENSOR_STR_TO_RADAR)
		{
			if (Radar_avail_prev_frame)
			{
				Radar_death_timer = timestamp(2000);
				Radar_static_next = 1;
			}

			Radar_avail_prev_frame = 0;
		}
		else
		{
			Radar_death_timer = 0;

			if (Radar_static_next == 0)
				Radar_static_next = 1;
		}

	if (timestamp_elapsed(Radar_death_timer))
		ok_to_blit_radar = 0;

	setupViewHtl();

	//WMC - This strikes me as a bit hackish
	bool g3_yourself = !g3_in_frame();
	if(g3_yourself)
		g3_start_frame(1);

	drawSweeps();

	if (timestamp_elapsed(Radar_static_next))
	{
		Radar_static_playing ^= 1;
		Radar_static_next = timestamp_rand(50, 750);
	}

	// if the emp effect is active, always draw the radar wackily
	if (emp_active_local())
		Radar_static_playing = 1;

	if (ok_to_blit_radar)
	{
		if (Radar_static_playing)
		{
			drawBlipsSorted(1);	// passing 1 means to draw distorted

			if (Radar_static_looping == -1)
				Radar_static_looping = snd_play_looping(&Snds[SND_STATIC]);
		}
		else
		{
			drawBlipsSorted(0);

			if (Radar_static_looping != -1)
			{
				snd_stop(Radar_static_looping);
				Radar_static_looping = -1;
			}
		}
	}
	else
	{
		if (Radar_static_looping != -1)
		{
			snd_stop(Radar_static_looping);
			Radar_static_looping = -1;
		}
	}

	if(g3_yourself)
		g3_end_frame();

	doneDrawingHtl();
}

void HudGaugeRadarDradis::pageIn()
{
	bm_page_in_texture(xy_plane);
	bm_page_in_texture(xz_yz_plane);
	bm_page_in_texture(sweep_plane);
	bm_page_in_texture(target_brackets);
	bm_page_in_texture(unknown_contact_icon);
}

void HudGaugeRadarDradis::doLoopSnd()
{
	if (this->loop_snd < 0)
	{
		return;
	}

	if (!this->shouldDoSounds())
	{
		if (loop_sound_handle >= 0 && snd_is_playing(loop_sound_handle))
		{
			snd_stop(loop_sound_handle);
			loop_sound_handle = -1;
		}
	}
	else if (this->loop_sound_handle < 0 || !snd_is_playing(this->loop_sound_handle))
	{
		loop_sound_handle = snd_play(&Snds[loop_snd], 0.0f, loop_sound_volume);
	}
}

void HudGaugeRadarDradis::doBeeps()
{
	if (!this->shouldDoSounds())
	{
		return;
	}

	if (Missiontime == 0 || Missiontime == Frametime)
	{
		// don't play sounds in first frame
		return;
	}

	if (arrival_beep_snd < 0 &&
		departure_beep_snd < 0 &&
		stealth_arrival_snd < 0 &&
		stealth_departure_snd < 0)
	{
		return;
	}

	bool departure_happened = false;
	bool stealth_departure_happened = false;

	bool arrival_happened = false;
	bool stealth_arrival_happened = false;
	
	for (int i = 0; i < MAX_SHIPS; i++)
	{
		ship * shipp = &Ships[i];

		if (shipp->objnum >= 0)
		{
			if (shipp->radar_visible_since >= 0 || shipp->radar_last_contact >= 0)
			{
				if (shipp->radar_visible_since == Missiontime)
				{
					if (shipp->radar_current_status == DISTORTED)
					{
						stealth_arrival_happened = true;
					}
					else
					{
						arrival_happened = true;
					}
				}
				else if (shipp->radar_visible_since < 0 && shipp->radar_last_contact == Missiontime)
				{
					if (shipp->radar_last_status == DISTORTED)
					{
						stealth_departure_happened = true;
					}
					else
					{
						departure_happened = true;
					}
				}
			}
		}
	}
	
	if (timestamp_elapsed(arrival_beep_next_check))
	{
		if (arrival_beep_snd >= 0 && arrival_happened)
		{
			snd_play(&Snds[arrival_beep_snd]);

			arrival_beep_next_check = timestamp(arrival_beep_delay);
		}
		else if (stealth_arrival_snd >= 0 && stealth_arrival_happened)
		{
			snd_play(&Snds[stealth_arrival_snd]);

			arrival_beep_next_check = timestamp(arrival_beep_delay);
		}

	}

	if (timestamp_elapsed(departure_beep_next_check))
	{
		if (departure_beep_snd >= 0 && departure_happened)
		{
			snd_play(&Snds[departure_beep_snd]);

			departure_beep_next_check = timestamp(departure_beep_delay);
		}
		else if (stealth_departure_snd >= 0 && stealth_departure_happened)
		{
			snd_play(&Snds[stealth_departure_snd]);

			departure_beep_next_check = timestamp(departure_beep_delay);
		}
	}
}

void HudGaugeRadarDradis::initSound(int loop_snd, float loop_snd_volume, int arrival_snd, int departure_snd, int stealth_arrival_snd, int stealth_departue_snd, float arrival_delay, float departure_delay)
{
	this->loop_snd = loop_snd;
	this->loop_sound_handle = -1;
	this->loop_sound_volume = loop_snd_volume;

	this->arrival_beep_snd = arrival_snd;
	this->departure_beep_snd = departure_snd;

	this->stealth_arrival_snd = stealth_arrival_snd;
	this->stealth_departure_snd = stealth_departue_snd;

	this->arrival_beep_delay = fl2i(arrival_delay * 1000.0f);
	this->departure_beep_delay = fl2i(departure_delay * 1000.0f);
}

void HudGaugeRadarDradis::onFrame(float frametime)
{
	// Play the specified radar sound
	this->doLoopSnd();

	// Play beeps for ship arrival and departure
	this->doBeeps();
}

void HudGaugeRadarDradis::initialize()
{
	HudGaugeRadar::initialize();

	this->arrival_beep_next_check = timestamp();
	this->departure_beep_next_check = timestamp();
}

bool HudGaugeRadarDradis::shouldDoSounds()
{
	if (hud_disabled())
		return false;

	if (Viewer_mode & (VM_EXTERNAL | VM_CHASE | VM_DEAD_VIEW | VM_OTHER_SHIP))
		return false;

	return true;
}

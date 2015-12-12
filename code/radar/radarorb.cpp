/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "bmpman/bmpman.h"
#include "freespace2/freespace.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/linklist.h"
#include "graphics/font.h"
#include "iff_defs/iff_defs.h"
#include "io/timer.h"
#include "jumpnode/jumpnode.h"
#include "localization/localize.h"
#include "network/multi.h"
#include "object/object.h"
#include "playerman/player.h"
#include "radar/radarorb.h"
#include "render/3d.h"
#include "ship/awacs.h"
#include "ship/ship.h"
#include "ship/subsysdamage.h"
#include "weapon/emp.h"
#include "weapon/weapon.h"

extern rcol Radar_color_rgb[MAX_RADAR_COLORS][MAX_RADAR_LEVELS];

extern int radar_target_id_flags;

extern int Cmdline_nohtl;

vec3d orb_ring_yz[NUM_ORB_RING_SLICES];
vec3d orb_ring_xy[NUM_ORB_RING_SLICES];
vec3d orb_ring_xz[NUM_ORB_RING_SLICES];

//special view matrix to get the orb rotating the correct way
static matrix view_perturb = { { { { { { 1.0f, 0.0f, 0.0f } } },
                                   { { { 0.0f, -1.0f, 0.0f } } },
                                   { { { 0.0f, 0.0f, -1.0f } } } } } };

static vec3d Orb_eye_position = { { { 0.0f, 0.0f, -3.0f } } };

HudGaugeRadarOrb::HudGaugeRadarOrb():
HudGaugeRadar(HUD_OBJECT_RADAR_ORB, 255, 255, 255)
{
	int i;
	float s,c;

	memset(orb_ring_xy, 0, sizeof(orb_ring_xy));
	memset(orb_ring_xz, 0, sizeof(orb_ring_xz));
	memset(orb_ring_yz, 0, sizeof(orb_ring_yz));
	
    for (i=0; i < NUM_ORB_RING_SLICES; i++)
    {
        s=(float)sin(float(i*PI2)/NUM_ORB_RING_SLICES);
        c=(float)cos(float(i*PI2)/NUM_ORB_RING_SLICES);

        orb_ring_xy[i].xyz.x = c;
        orb_ring_xy[i].xyz.y = s;

        orb_ring_yz[i].xyz.y = c;
        orb_ring_yz[i].xyz.z = s;

        orb_ring_xz[i].xyz.x = c;
        orb_ring_xz[i].xyz.z = s;
	}

    gr_init_alphacolor(&Orb_color_orange, 192, 96, 32,  192);
    gr_init_alphacolor(&Orb_color_teal,   48, 160, 96,  192);
    gr_init_alphacolor(&Orb_color_purple, 112, 16, 192, 192);
	gr_init_alphacolor(&Orb_crosshairs,   255, 255,255, 192);
}

void HudGaugeRadarOrb::initCenterOffsets(float x, float y)
{
	Radar_center_offsets[0] = x;
	Radar_center_offsets[1] = y;
}

void HudGaugeRadarOrb::initBitmaps(char *fname)
{
	Radar_gauge.first_frame = bm_load_animation(fname, &Radar_gauge.num_frames);
	if ( Radar_gauge.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname);
	}
}

void HudGaugeRadarOrb::plotBlip(blip *b, vec3d *scaled_pos)
{
	*scaled_pos = b->position;
	
	if (IS_VEC_NULL_SQ_SAFE(scaled_pos)) {
		vm_vec_make(scaled_pos, 1.0f, 0.0f, 0.0f);
	} else {
		vm_vec_normalize(scaled_pos);
	}

	float scale = b->dist / Radar_bright_range;
	if (scale > 1.25f) scale = 1.25f;
	if (scale < .75f) scale = .75f;

	vm_vec_scale(scaled_pos, scale);
}

void HudGaugeRadarOrb::drawContact(vec3d *pnt, int rad)
{
	vertex verts[2];
	vec3d p;

	p=*pnt;
	vm_vec_normalize(&p);

	g3_rotate_vertex(&verts[0], &p);
	g3_project_vertex(&verts[0]);

	g3_rotate_vertex(&verts[1], pnt);
	g3_project_vertex(&verts[1]);

	float size = fl_sqrt(vm_vec_dist(&Orb_eye_position, pnt) * 8.0f);
	if (size < i2fl(rad))	size = i2fl(rad);
 
	if (rad == Radar_blip_radius_target)
	{
		g3_draw_sphere(&verts[1],size/100.0f);
	}
	else
	{
		g3_draw_sphere(&verts[1],size/300.0f);
	}

	g3_draw_line(&verts[0],&verts[1]);
}

void HudGaugeRadarOrb::drawContactHtl(vec3d *pnt, int rad)
{
	vec3d p;

	p=*pnt;

	vm_vec_normalize(&p);

    float size = fl_sqrt(vm_vec_dist(&Orb_eye_position, pnt) * 8.0f);
	if (size < i2fl(rad))	size = i2fl(rad);
 
	if (rad == Radar_blip_radius_target)
	{
		if (radar_target_id_flags & RTIF_PULSATE) {
			// use mask to make the darn thing work faster
			size *= 1.3f + (sinf(10 * f2fl(Missiontime)) * 0.3f);
		}
		if (radar_target_id_flags & RTIF_BLINK) {
			if (Missiontime & 8192)
				return;
		}
		g3_draw_htl_sphere(pnt,size/100.0f);
	}
	else
	{
		g3_draw_htl_sphere(pnt,size/300.0f);
	}

	g3_draw_htl_line(&p,pnt);
}

// radar is damaged, so make blips dance around
void HudGaugeRadarOrb::blipDrawDistorted(blip *b, vec3d *pos)
{
	float scale;
	float dist=vm_vec_normalize(pos);
	vec3d out;
	float distortion_angle=20;
	
	// maybe alter the effect if EMP is active
	if(emp_active_local()){
		scale = emp_current_intensity();
		distortion_angle *= frand_range(-3.0f,3.0f)*frand_range(0.0f, scale);
		dist *= frand_range(MAX(0.75f, 0.75f*scale), MIN(1.25f, 1.25f*scale));

		if (dist > 1.25f) dist = 1.25f;
		if (dist < 0.75f) dist = 0.75f;
	}

	vm_vec_random_cone(&out,pos,distortion_angle);
	vm_vec_scale(&out,dist);

    if (Cmdline_nohtl)
    {
	    drawContact(&out,b->rad);
    }
    else
    {
        drawContactHtl(&out,b->rad);
    }
}

// blip is for a target immune to sensors, so cause to flicker in/out with mild distortion
void HudGaugeRadarOrb::blipDrawFlicker(blip *b, vec3d *pos)
{
	int flicker_index;

	float dist=vm_vec_normalize(pos);
	vec3d out;
	float distortion_angle=10;

	if ( (b-Blips) & 1 ) {
		flicker_index=0;
	} else {
		flicker_index=1;
	}

	if ( timestamp_elapsed(Radar_flicker_timer[flicker_index]) ) {
		Radar_flicker_timer[flicker_index] = timestamp_rand(50,1000);
		Radar_flicker_on[flicker_index] ^= 1;
	}

	if ( !Radar_flicker_on[flicker_index] ) {
		return;
	}

	if ( rand() & 1 ) {

		distortion_angle *= frand_range(0.1f,2.0f);
		dist *= frand_range(0.75f, 1.25f);

		if (dist > 1.25f) dist = 1.25f;
		if (dist < 0.75f) dist = 0.75f;
	}
	
	vm_vec_random_cone(&out,pos,distortion_angle);
	vm_vec_scale(&out,dist);

	if (Cmdline_nohtl)
    {
	    drawContact(&out,b->rad);
    }
    else
    {
        drawContactHtl(&out,b->rad);
    }
}

// Draw all the active radar blips
void HudGaugeRadarOrb::drawBlips(int blip_type, int bright, int distort)
{
	blip *b = NULL;
	blip *blip_head = NULL;
	vec3d pos;

	Assert((blip_type >= 0) && (blip_type < MAX_BLIP_TYPES));


	// Need to set font.
	gr_set_font(FONT1);


	// get the appropriate blip list
	if (bright)
		blip_head = &Blip_bright_list[blip_type];
	else
		blip_head = &Blip_dim_list[blip_type];


	// draw all blips of this type
	for (b = GET_FIRST(blip_head); b != END_OF_LIST(blip_head); b = GET_NEXT(b))
	{
		gr_set_color_fast(b->blip_color);
		plotBlip(b, &pos);

		// maybe draw cool blip to indicate current target
		if (b->flags & BLIP_CURRENT_TARGET)
		{
			b->rad = Radar_blip_radius_target;				
			target_position = pos;
		}
		else
		{
			b->rad = Radar_blip_radius_normal;
		}

		// maybe distort blip
		if (distort)
		{
			blipDrawDistorted(b, &pos);
		}
		else if (b->flags & BLIP_DRAW_DISTORTED)
		{
			blipDrawFlicker(b, &pos);
		}
		else
		{
            if (Cmdline_nohtl)
            {
               drawContact(&pos,b->rad);
            }
            else if (b->radar_image_2d >= 0 || b->radar_color_image_2d >= 0)
			{
				drawContactImage(&pos, b->rad, b->radar_image_2d, b->radar_color_image_2d, b->radar_projection_size);
			}
            else
            {
                drawContactHtl(&pos,b->rad);
            }
        }
	}
}

void HudGaugeRadarOrb::drawBlipsSorted(int distort)
{
	g3_start_instance_matrix(&vmd_zero_vector, &view_perturb, false);

	vm_vec_zero(&target_position);
	// draw dim blips first, then bright blips
	for (int is_bright = 0; is_bright < 2; is_bright++)
	{
		drawBlips(BLIP_TYPE_JUMP_NODE, is_bright, distort);
		drawBlips(BLIP_TYPE_WARPING_SHIP, is_bright, distort);
		drawBlips(BLIP_TYPE_NAVBUOY_CARGO, is_bright, distort);
		drawBlips(BLIP_TYPE_NORMAL_SHIP, is_bright, distort);
		drawBlips(BLIP_TYPE_BOMB, is_bright, distort);
		drawBlips(BLIP_TYPE_TAGGED_SHIP, is_bright, distort);
	}

	if (radar_target_id_flags & RTIF_CROSSHAIRS) {
		drawCrosshairs(target_position);
	}

	g3_done_instance(false);
}

void HudGaugeRadarOrb::doneDrawing()
{
	g3_done_instance(false);
	g3_end_frame();
	g3_start_frame(0);
	hud_save_restore_camera_data(0);
	resetClip();
}

void HudGaugeRadarOrb::doneDrawingHtl()
{
	gr_end_view_matrix();
	gr_end_proj_matrix();
	resetClip();
    gr_zbuffer_set(GR_ZBUFF_FULL);
}

void HudGaugeRadarOrb::drawOutlines()
{
	int i;
	vertex center;
//	vertex extents[6];
	vertex proj_orb_lines_xy[NUM_ORB_RING_SLICES];
	vertex proj_orb_lines_xz[NUM_ORB_RING_SLICES];
	vertex proj_orb_lines_yz[NUM_ORB_RING_SLICES];

	g3_start_instance_matrix(&vmd_zero_vector, &view_perturb, false);
	g3_start_instance_matrix(&vmd_zero_vector, &Player_obj->orient, false);

	g3_rotate_vertex(&center, &vmd_zero_vector);
	g3_rotate_vertex(&proj_orb_lines_xy[0], &orb_ring_xy[0]);
	g3_rotate_vertex(&proj_orb_lines_yz[0], &orb_ring_yz[0]);
	g3_rotate_vertex(&proj_orb_lines_xz[0], &orb_ring_xz[0]);

	g3_project_vertex(&center);
	gr_set_color(255,255,255);
	g3_draw_sphere(&center, .05f);

	g3_project_vertex(&proj_orb_lines_xy[0]);
	g3_project_vertex(&proj_orb_lines_yz[0]);
	g3_project_vertex(&proj_orb_lines_xz[0]);

	for (i=1; i < NUM_ORB_RING_SLICES; i++)
	{
		g3_rotate_vertex(&proj_orb_lines_xy[i], &orb_ring_xy[i]);
		g3_rotate_vertex(&proj_orb_lines_yz[i], &orb_ring_yz[i]);
		g3_rotate_vertex(&proj_orb_lines_xz[i], &orb_ring_xz[i]);

		g3_project_vertex(&proj_orb_lines_xy[i]);
		g3_project_vertex(&proj_orb_lines_yz[i]);
		g3_project_vertex(&proj_orb_lines_xz[i]);
		
		gr_set_color(192,96,32);
		g3_draw_sphere(&proj_orb_lines_xy[i-1], .01f);
		g3_draw_sphere(&proj_orb_lines_xz[i-1], .01f);
		g3_draw_line(&proj_orb_lines_xy[i-1],&proj_orb_lines_xy[i]);
		g3_draw_line(&proj_orb_lines_xz[i-1],&proj_orb_lines_xz[i]);

		gr_set_color(112,16,192);
		g3_draw_sphere(&proj_orb_lines_yz[i-1], .01f);
		g3_draw_line(&proj_orb_lines_yz[i-1],&proj_orb_lines_yz[i]);
	}

	g3_done_instance(false);
}

int HudGaugeRadarOrb::calcAlpha(vec3d* pt)
{
    Assert(pt);
    Assert(Player_obj);

    vec3d new_pt;
    vec3d fvec = { { { 0.0f, 0.0f, 1.0f } } };

    vm_vec_unrotate(&new_pt, pt, &Player_obj->orient);
    vm_vec_normalize(&new_pt);

    float dot = vm_vec_dot(&fvec, &new_pt);
    float angle = fabs(acos(dot));
    int alpha = int(angle*192.0f/PI);
    
    return alpha;
}

void HudGaugeRadarOrb::drawOutlinesHtl()
{
	int i, last = NUM_ORB_RING_SLICES - 1;

	g3_start_instance_matrix(&vmd_zero_vector, &view_perturb, true);
	g3_start_instance_matrix(&vmd_zero_vector, &Player_obj->orient, true);

	gr_set_color(255, 255, 255);
	g3_draw_htl_sphere(&vmd_zero_vector, .05f);

    gr_set_line_width(2.0f);

	for (i = 0; i < NUM_ORB_RING_SLICES; i++)
	{
        gr_init_alphacolor(&Orb_color_orange, 192, 96, 32, calcAlpha(&orb_ring_xy[last]));
		gr_set_color_fast(&Orb_color_orange);
		g3_draw_htl_line(&orb_ring_xy[last],&orb_ring_xy[i]);

        gr_init_alphacolor(&Orb_color_teal, 48, 160, 96, calcAlpha(&orb_ring_xz[last]));
        gr_set_color_fast(&Orb_color_teal);
		g3_draw_htl_line(&orb_ring_xz[last],&orb_ring_xz[i]);

        gr_init_alphacolor(&Orb_color_purple, 112, 16, 192, calcAlpha(&orb_ring_yz[last]));
		gr_set_color_fast(&Orb_color_purple);
		g3_draw_htl_line(&orb_ring_yz[last],&orb_ring_yz[i]);

        last = i;
	}

    gr_set_line_width(1.0f);

	g3_done_instance(true);
    g3_done_instance(true);
}

void HudGaugeRadarOrb::render(float frametime)
{
	float	sensors_str;
	int ok_to_blit_radar;

	//WMC - This strikes me as a bit hackish
	bool g3_yourself = !g3_in_frame();
	if(g3_yourself)
		g3_start_frame(1);

	ok_to_blit_radar = 1;

	sensors_str = ship_get_subsystem_strength( Player_ship, SUBSYSTEM_SENSORS );

	if ( ship_subsys_disrupted(Player_ship, SUBSYSTEM_SENSORS) ) {
		sensors_str = MIN_SENSOR_STR_TO_RADAR-1;
	}

	// note that on lowest skill level, there is no radar effects due to sensors damage
	if ( (Game_skill_level == 0) || (sensors_str > SENSOR_STR_RADAR_NO_EFFECTS) ) {
		Radar_static_playing = 0;
		Radar_static_next = 0;
		Radar_death_timer = 0;
		Radar_avail_prev_frame = 1;
	} else if ( sensors_str < MIN_SENSOR_STR_TO_RADAR ) {
		if ( Radar_avail_prev_frame ) {
			Radar_death_timer = timestamp(2000);
			Radar_static_next = 1;
		}
		Radar_avail_prev_frame = 0;
	} else {
		Radar_death_timer = 0;
		if ( Radar_static_next == 0 )
			Radar_static_next = 1;
	}

	if ( timestamp_elapsed(Radar_death_timer) ) {
		ok_to_blit_radar = 0;
	}

	setGaugeColor();
	blitGauge();
	drawRange();

    if (Cmdline_nohtl)
    {
        setupView();
        drawOutlines();
    }
    else
    {
        setupViewHtl();
        drawOutlinesHtl();
    }
	

	if ( timestamp_elapsed(Radar_static_next) ) {
		Radar_static_playing ^= 1;
		Radar_static_next = timestamp_rand(50, 750);
	}

	// if the emp effect is active, always draw the radar wackily
	if(emp_active_local()){
		Radar_static_playing = 1;
	}

	if ( ok_to_blit_radar ) {
		if ( Radar_static_playing ) {
			drawBlipsSorted(1);	// passing 1 means to draw distorted
			if ( Radar_static_looping == -1 ) {
				Radar_static_looping = snd_play_looping(&Snds[SND_STATIC]);
			}
		} else {
			drawBlipsSorted(0);
			if ( Radar_static_looping != -1 ) {
				snd_stop(Radar_static_looping);
				Radar_static_looping = -1;
			}
		}
	} else {
		if ( Radar_static_looping != -1 ) {
			snd_stop(Radar_static_looping);
			Radar_static_looping = -1;
		}
	}
	
    if (Cmdline_nohtl)
    {
	    doneDrawing();
    }
    else
    {
        doneDrawingHtl();
    }

	if(g3_yourself)
		g3_end_frame();
}

void HudGaugeRadarOrb::blitGauge()
{
	SPECMAP = -1;
	GLOWMAP = -1;

	renderBitmap(Radar_gauge.first_frame+1, position[0], position[1] );
}

void HudGaugeRadarOrb::pageIn()
{
	bm_page_in_aabitmap( Radar_gauge.first_frame, Radar_gauge.num_frames );
}

void HudGaugeRadarOrb::drawContactImage(vec3d *pnt, int rad, int idx, int clr_idx, float mult)
{
    int tmap_flags = 0;
    int h, w;
    float aspect_mp;

    // need to get bitmap info
    bm_get_info(idx, &w, &h);

    Assert(w > 0);

    // get multiplier
    if (h == w) {
        aspect_mp = 1.0f;
    } else {
        aspect_mp = (((float) h) / ((float) w));
    }

    gr_set_bitmap(idx,GR_ALPHABLEND_NONE,GR_BITBLT_MODE_NORMAL,1.0f);

    float sizef = fl_sqrt(vm_vec_dist(&Orb_eye_position, pnt) * 8.0f);

    // might need checks unless the targeted blip is always wanted to be larger
    float radius = (float) Radar_blip_radius_normal;

    if (sizef < radius)
        sizef = radius;

    //Make so no evil things happen
    Assert(mult > 0.0f);

    //modify size according to value from tables
    sizef *= mult;

	// animate the targeted icon - option 1 of highlighting the targets
	if ( rad == Radar_blip_radius_target ) {
		if (radar_target_id_flags & RTIF_PULSATE) {
			// use mask to make the darn thing work faster
			sizef *= 1.3f + (sinf(10 * f2fl(Missiontime)) * 0.3f);
		}
		if (radar_target_id_flags & RTIF_BLINK) {
			if (Missiontime & 8192)
				return;
		}
		if (radar_target_id_flags & RTIF_ENLARGE) {
			sizef *= 1.3f;
		}
	}

    tmap_flags = TMAP_FLAG_TEXTURED | TMAP_FLAG_BW_TEXTURE | TMAP_HTL_3D_UNLIT;

	if ( idx >= 0 ) {
		g3_draw_polygon(pnt, &vmd_identity_matrix, sizef/35.0f, aspect_mp*sizef/35.0f, tmap_flags);
	}

	if ( clr_idx >= 0 ) {
		g3_draw_polygon(pnt, &vmd_identity_matrix, sizef/35.0f, aspect_mp*sizef/35.0f, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);
	}
}

void HudGaugeRadarOrb::drawCrosshairs( vec3d pnt )
{
	int i,j,m;
	vec3d pnt_end, pnt_start;

	gr_set_color_fast(&Orb_crosshairs);

	for(i = 0; i < 2; i++) {
		m = (i * 2) - 1;
		pnt_end = pnt_start = pnt;
		pnt_start.xyz.x += (float) m*0.05f;
		pnt_end.xyz.x += (float) m*0.15f;
		g3_draw_htl_line(&pnt_start, &pnt_end);
	}
	for(j = 0; j < 2; j++) {
		m = (j * 2) - 1;
		pnt_end = pnt_start = pnt;
		pnt_start.xyz.y += (float) m*0.05f;
		pnt_end.xyz.y += (float) m*0.15f;
		g3_draw_htl_line(&pnt_start, &pnt_end);
	}
}

extern void hud_save_restore_camera_data(int);
extern float View_zoom;

void HudGaugeRadarOrb::setupView()
{
	hud_save_restore_camera_data(1);

	g3_end_frame();

	int w,h;
	bm_get_info(Radar_gauge.first_frame,&w, &h, NULL, NULL, NULL);
	
	setClip(position[0], position[1],w, h);
	g3_start_frame(1);
	
	float old_zoom=View_zoom;
	View_zoom=.75;

	g3_set_view_matrix( &Orb_eye_position, &vmd_identity_matrix, View_zoom);

	View_zoom=old_zoom;
}

void HudGaugeRadarOrb::setupViewHtl()
{
    int w,h;
	bm_get_info(Radar_gauge.first_frame,&w, &h, NULL, NULL, NULL);
    
    setClip(position[0],
                 position[1],
                 w, h);

    gr_set_proj_matrix( .625f * PI_2, float(w)/float(h), 0.001f, 5.0f);
	gr_set_view_matrix( &Orb_eye_position, &vmd_identity_matrix );

    gr_zbuffer_set(GR_ZBUFF_NONE);
}

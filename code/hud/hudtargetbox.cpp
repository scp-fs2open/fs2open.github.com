/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "hud/hudtargetbox.h"
#include "object/object.h"
#include "hud/hud.h"
#include "hud/hudbrackets.h"
#include "model/model.h"
#include "mission/missionparse.h"
#include "debris/debris.h"
#include "playerman/player.h"
#include "gamesnd/gamesnd.h"
#include "freespace2/freespace.h"
#include "io/timer.h"
#include "ship/subsysdamage.h"
#include "graphics/font.h"
#include "asteroid/asteroid.h"
#include "jumpnode/jumpnode.h"
#include "weapon/emp.h"
#include "localization/localize.h"
#include "cmdline/cmdline.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "parse/parselo.h"
#include "object/objectdock.h"
#include "species_defs/species_defs.h"
#include "iff_defs/iff_defs.h"
#include "network/multi.h"
#include "graphics/gropenglshader.h"

#ifndef NDEBUG
#include "hud/hudets.h"
#endif


extern float View_zoom;

int Target_window_coords[GR_NUM_RESOLUTIONS][4] =
{
	{ // GR_640
		8, 362, 131, 112
	},
	{ // GR_1024
		8, 629, 131, 112
	}
};

object *Enemy_attacker = NULL;

static int Target_static_next;
static int Target_static_playing;
int Target_static_looping;

int Target_display_cargo;
char Cargo_string[256] = "";

#ifndef NDEBUG
extern int Show_target_debug_info;
extern int Show_target_weapons;
#endif

// used to print out + or - after target distance and speed
char* modifiers[] = {
//XSTR:OFF
"+",
"-",
""
//XSTR:ON
};

#define NUM_TBOX_COORDS			11	// keep up to date
#define TBOX_BACKGROUND			0
#define TBOX_NAME					1
#define TBOX_CLASS				2
#define TBOX_DIST					3
#define TBOX_SPEED				4
#define TBOX_CARGO				5
#define TBOX_HULL					6
#define TBOX_EXTRA				7
#define TBOX_EXTRA_ORDERS		8
#define TBOX_EXTRA_TIME			9
#define TBOX_EXTRA_DOCK			10

// cargo scanning extents
int Cargo_scan_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		7, 364, 130, 109
	},
	{ // GR_1024
		7, 635, 130, 109
	}
};

// first element is time flashing expires
int Targetbox_flash_timers[NUM_TBOX_FLASH_TIMERS];

int Targetbox_wire = 0;
int Targetbox_shader_effect = -1;
bool Lock_targetbox_mode = false;

// Different target states.  This drives the text display right below the hull integrity on the targetbox.
#define TS_DIS		0
#define TS_OK		1
#define TS_DMG		2
#define TS_CRT		3

static int Current_ts; // holds current target status
static int Last_ts;	// holds last target status.

/**
 * @note Cut down long subsystem names to a more manageable length
 */
void hud_targetbox_truncate_subsys_name(char *outstr)
{	
	if(Lcl_gr){
		if ( strstr(outstr, "communication") )	{
			strcpy(outstr, "Komm");
		} else if ( !stricmp(outstr, "weapons") ) {
			strcpy(outstr, "Waffen");
		} else if ( strstr(outstr, "engine") || strstr(outstr, "Engine")) {
			strcpy(outstr, "Antrieb");
		} else if ( !stricmp(outstr, "sensors") ) {
			strcpy(outstr, "Sensoren");
		} else if ( strstr(outstr, "navigat") ) {
			strcpy(outstr, "Nav");
		} else if ( strstr(outstr, "fighterbay") || strstr(outstr, "Fighterbay") ) {
			strcpy(outstr, "J\x84gerhangar");
		} else if ( strstr(outstr, "missile") ) {
			strcpy(outstr, "Raketenwerfer");
		} else if ( strstr(outstr, "laser") || strstr(outstr, "turret") ) {
			strcpy(outstr, "Gesch\x81tzturm");
		} else if ( strstr(outstr, "Command Tower") || strstr(outstr, "Bridge") ) {
			strcpy(outstr, "Br\x81""cke");
		} else if ( strstr(outstr, "Barracks") ) {
			strcpy(outstr, "Quartiere");
		} else if ( strstr(outstr, "Reactor") ) {
			strcpy(outstr, "Reaktor");
		} else if ( strstr(outstr, "RadarDish") ) {
			strcpy(outstr, "Radarantenne");
		} else if (!stricmp(outstr, "Gas Collector")) {
			strcpy(outstr, "Sammler");
		} 
	} else if(Lcl_fr){	
		if ( strstr(outstr, "communication") )	{
			strcpy(outstr, "comm");
		} else if ( !stricmp(outstr, "weapons") ) {
			strcpy(outstr, "armes");
		} else if ( strstr(outstr, "engine") ) {
			strcpy(outstr, "moteur");
		} else if ( !stricmp(outstr, "sensors") ) {
			strcpy(outstr, "detecteurs");
		} else if ( strstr(outstr, "navi") ) {
			strcpy(outstr, "nav");
		} else if ( strstr(outstr, "missile") ) {
			strcpy(outstr, "lanceur de missiles");
		} else if ( strstr(outstr, "fighter") ) {
			strcpy(outstr, "baie de chasse");
		} else if ( strstr(outstr, "laser") || strstr(outstr, "turret") || strstr(outstr, "missile") ) {
			strcpy(outstr, "tourelle");
		} 
	} else {
		if (strstr(outstr, XSTR("communication", 333)))	{
			strcpy(outstr, XSTR("comm", 334));
		} else if (strstr(outstr, XSTR("navigation", 335)))	{
			strcpy(outstr, XSTR("nav", 336));
		} else if (strstr(outstr, "gas collector")) {
			strcpy(outstr, "collector");
		}
	}
}

HudGaugeTargetBox::HudGaugeTargetBox():
HudGauge(HUD_OBJECT_TARGET_MONITOR, HUD_TARGET_MONITOR, true, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY), 255, 255, 255)
{
}

void HudGaugeTargetBox::initViewportOffsets(int x, int y)
{
	Viewport_offsets[0] = x;
	Viewport_offsets[1] = y;
}

void HudGaugeTargetBox::initViewportSize(int w, int h)
{
	Viewport_w = w;
	Viewport_h = h;
}

void HudGaugeTargetBox::initIntegrityOffsets(int x, int y)
{
	Integrity_bar_offsets[0] = x;
	Integrity_bar_offsets[1] = y;
}

void HudGaugeTargetBox::initIntegrityHeight(int h)
{
	integrity_bar_h = h;
}

void HudGaugeTargetBox::initStatusOffsets(int x, int y)
{
	Status_offsets[0] = x;
	Status_offsets[1] = y;
}

void HudGaugeTargetBox::initNameOffsets(int x, int y)
{
	Name_offsets[0] = x;
	Name_offsets[1] = y;
}

void HudGaugeTargetBox::initClassOffsets(int x, int y)
{
	Class_offsets[0] = x;
	Class_offsets[1] = y;
}

void HudGaugeTargetBox::initDistOffsets(int x, int y)
{
	Dist_offsets[0] = x;
	Dist_offsets[1] = y;
}

void HudGaugeTargetBox::initSpeedOffsets(int x, int y)
{
	Speed_offsets[0] = x;
	Speed_offsets[1] = y;
}

void HudGaugeTargetBox::initCargoStringOffsets(int x, int y)
{
	Cargo_string_offsets[0] = x;
	Cargo_string_offsets[1] = y;
}

void HudGaugeTargetBox::initHullOffsets(int x, int y)
{
	Hull_offsets[0] = x;
	Hull_offsets[1] = y;
}

void HudGaugeTargetBox::initCargoScanStartOffsets(int x, int y)
{
	Cargo_scan_start_offsets[0] = x;
	Cargo_scan_start_offsets[1] = y;
}

void HudGaugeTargetBox::initCargoScanSize(int w, int h)
{
	Cargo_scan_w = w;
	Cargo_scan_h = h;
}

void HudGaugeTargetBox::initBitmaps(char *fname_monitor, char *fname_integrity, char *fname_static)
{
	Monitor_frame.first_frame = bm_load_animation(fname_monitor, &Monitor_frame.num_frames);
	if ( Monitor_frame.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname_monitor);
	}

	Integrity_bar.first_frame = bm_load_animation(fname_integrity, &Integrity_bar.num_frames);
	if ( Integrity_bar.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname_integrity);
	}

	strcpy_s(static_fname, fname_static);
}

void HudGaugeTargetBox::initialize()
{
	hud_anim_init(&Monitor_static, position[0] + Viewport_offsets[0], position[1] + Viewport_offsets[1], NOX(static_fname));

	for(int i = 0; i < NUM_TBOX_FLASH_TIMERS; i++) {
		initFlashTimer(i);
	}
}

void HudGaugeTargetBox::initFlashTimer(int index)
{
	Next_flash_timers[index] = 1;
	flash_flags &= ~(1<<index);
}

void HudGaugeTargetBox::render(float frametime)
{
	object	*target_objp;

	if ( Player_ai->target_objnum == -1)
		return;
	
	if ( Target_static_playing ) 
		return;

	target_objp = &Objects[Player_ai->target_objnum];

	setGaugeColor();

	// blit the background frame
	renderBitmap(Monitor_frame.first_frame, position[0], position[1]);

	switch ( target_objp->type ) {
		case OBJ_SHIP:
			renderTargetShip(target_objp);
			break;
	
		case OBJ_DEBRIS:
			renderTargetDebris(target_objp);
			break;

		case OBJ_WEAPON:
			renderTargetWeapon(target_objp);
			break;

		case OBJ_ASTEROID:
			renderTargetAsteroid(target_objp);
			break;

		case OBJ_JUMP_NODE:
			renderTargetJumpNode(target_objp);
			break;

		default:
			hud_cease_targeting();
			break;
	} // end switch

	if ( Target_static_playing ) {
		setGaugeColor();
		gr_set_screen_scale(base_w, base_h);
		hud_anim_render(&Monitor_static, frametime, 1);
		gr_reset_screen_scale();
	} else {
		showTargetData(frametime);
	}

	if(Target_display_cargo) {
		// Print out what the cargo is
		if ( maybeFlashSexp() == 1 ) {
			setGaugeColor(HUD_C_BRIGHT);
		} else {
			maybeFlashElement(TBOX_FLASH_CARGO);
		}

		renderString(position[0] + Cargo_string_offsets[0], position[1] + Cargo_string_offsets[1], EG_TBOX_CARGO, Cargo_string);
	}
}

void HudGaugeTargetBox::renderTargetForeground()
{
	setGaugeColor();

	renderBitmap(Monitor_frame.first_frame+1, position[0], position[1]);	
}

/**
 * Draw the integrity bar that is on the right of the target monitor
 */
void HudGaugeTargetBox::renderTargetIntegrity(int disabled,int force_obj_num)
{
	object	*objp;
	int		clip_h,w,h;
	char		buf[16];

	if ( Integrity_bar.first_frame == -1 ) 
		return;

	if ( disabled ) {
		renderBitmap(Integrity_bar.first_frame, position[0] + Integrity_bar_offsets[0], position[1] + Integrity_bar_offsets[1]);
		return;
	}

	if(force_obj_num == -1){
		Assert(Player_ai->target_objnum >= 0 );
		objp = &Objects[Player_ai->target_objnum];
	} else {
		objp = &Objects[force_obj_num];
	}

	clip_h = fl2i( (1 - Pl_target_integrity) * integrity_bar_h );

	// print out status of ship
	switch(Current_ts) {
		case TS_DIS:
			sprintf(buf,XSTR( "dis", 344));
			break;
		case TS_OK:
			sprintf(buf,XSTR( "ok", 345));
			break;
		case TS_DMG:
			sprintf(buf,XSTR( "dmg", 346));
			break;
		case TS_CRT:
			sprintf(buf,XSTR( "crt", 347));
			break;
	}

	maybeFlashElement(TBOX_FLASH_STATUS);
	
	// finally print out the status of this ship
	renderString(position[0] + Status_offsets[0], position[1] + Status_offsets[1], EG_TBOX_INTEG, buf);	

	setGaugeColor();

	bm_get_info(Integrity_bar.first_frame,&w,&h);
	
	if ( clip_h > 0 ) {
		// draw the dark portion
		renderBitmapEx(Integrity_bar.first_frame, position[0] + Integrity_bar_offsets[0], position[1] + Integrity_bar_offsets[1], w, clip_h,0,0);		
	}

	if ( clip_h <= integrity_bar_h ) {
		// draw the bright portion
		renderBitmapEx(Integrity_bar.first_frame+1, position[0] + Integrity_bar_offsets[0], position[1] + Integrity_bar_offsets[1]+clip_h,w,h-clip_h,0,clip_h);		
	}
}

void HudGaugeTargetBox::renderTargetSetup(vec3d *camera_eye, matrix *camera_orient, float zoom)
{
	// JAS: g3_start_frame uses clip_width and clip_height to determine the
	// size to render to.  Normally, you would set this by using gr_set_clip,
	// but because of the hacked in hud jittering, I couldn't.  So come talk
	// to me before modifying or reusing the following code. Thanks.

	int clip_width = Viewport_w;
	int clip_height = Viewport_h;

	gr_screen.clip_width = clip_width;
	gr_screen.clip_height = clip_height;
	g3_start_frame(1);		// Turn on zbuffering
	hud_save_restore_camera_data(1);
	g3_set_view_matrix( camera_eye, camera_orient, zoom);	
	model_set_detail_level(1);		// use medium detail level

	setClip(position[0] + Viewport_offsets[0], position[1] + Viewport_offsets[1], Viewport_w, Viewport_h);

	if (!Cmdline_nohtl) {
		gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
	}

}

void HudGaugeTargetBox::renderTargetShip(object *target_objp)
{
	vec3d		obj_pos = ZERO_VECTOR;
	vec3d		camera_eye = ZERO_VECTOR;
	matrix		camera_orient = IDENTITY_MATRIX;
	ship		*target_shipp;
	ship_info	*target_sip;
	vec3d		orient_vec, up_vector;
	int			sx, sy;
	int			subsys_in_view;
	float		factor;
	
	target_shipp	= &Ships[target_objp->instance];
	target_sip		= &Ship_info[target_shipp->ship_info_index];

	int flags=0;
	if ( Detail.targetview_model )	{
		// take the forward orientation to be the vector from the player to the current target
		vm_vec_sub(&orient_vec, &target_objp->pos, &Player_obj->pos);
		vm_vec_normalize(&orient_vec);

		factor = -target_sip->closeup_pos.xyz.z;

		// use the player's up vector, and construct the viewers orientation matrix
		up_vector = Player_obj->orient.vec.uvec;
		vm_vector_2_matrix(&camera_orient,&orient_vec,&up_vector,NULL);

		// normalize the vector from the player to the current target, and scale by a factor to calculate
		// the objects position
		vm_vec_copy_scale(&obj_pos,&orient_vec,factor);

		// RT, changed scaling here
		renderTargetSetup(&camera_eye, &camera_orient, target_sip->closeup_zoom);
		ship_model_start( target_objp );

		switch (Targetbox_wire) {
			case 0:
				flags |= MR_NO_LIGHTING;

				break;
			case 1:
				if (ship_is_tagged(target_objp))
					model_set_outline_color_fast(iff_get_color(IFF_COLOR_TAGGED, true));
				else
					model_set_outline_color_fast(iff_get_color_by_team_and_object(target_shipp->team, Player_ship->team, true, target_objp));

				flags = (Cmdline_nohtl) ? MR_SHOW_OUTLINE : MR_SHOW_OUTLINE_HTL;
				flags |= MR_NO_POLYS | MR_NO_LIGHTING;

				break;
			case 2:
				break;
			case 3:
				if (ship_is_tagged(target_objp))
					model_set_outline_color_fast(iff_get_color(IFF_COLOR_TAGGED, true));
				else
					model_set_outline_color_fast(iff_get_color_by_team_and_object(target_shipp->team, Player_ship->team, true, target_objp));

				flags |= MR_NO_LIGHTING | MR_NO_TEXTURING;

				break;
		}

		if (target_sip->hud_target_lod >= 0) {
			model_set_detail_level(target_sip->hud_target_lod);
		}

		if(Targetbox_shader_effect > -1) {
			flags |= MR_ANIMATED_SHADER;

			opengl_shader_set_animated_effect(Targetbox_shader_effect);
		}

		// maybe render a special hud-target-only model
		if(target_sip->model_num_hud >= 0){
			model_render( target_sip->model_num_hud, &target_objp->orient, &obj_pos, flags | MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING);
		} else {
			model_render( target_sip->model_num, &target_objp->orient, &obj_pos, flags | MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING, -1, -1, target_shipp->ship_replacement_textures);
		}
		ship_model_stop( target_objp );

		sx = 0;
		sy = 0;
		// check if subsystem target has changed
		if ( Player_ai->targeted_subsys == Player_ai->last_subsys_target ) {
			vec3d save_pos;

			gr_set_screen_scale(base_w, base_h);
			save_pos = target_objp->pos;
			target_objp->pos = obj_pos;
			subsys_in_view = hud_targetbox_subsystem_in_view(target_objp, &sx, &sy);
			target_objp->pos = save_pos;

			if ( subsys_in_view != -1 ) {

				// AL 29-3-98: If subsystem is destroyed, draw gray brackets
				// Goober5000 - hm, caught a tricky bug for destroyable fighterbays
				if ( (Player_ai->targeted_subsys->current_hits <= 0) && ship_subsys_takes_damage(Player_ai->targeted_subsys) ) {
					gr_set_color_fast(iff_get_color(IFF_COLOR_MESSAGE, 1));
				} else {
					hud_set_iff_color( target_objp, 1 );
				}

				if ( subsys_in_view ) {
					draw_brackets_square_quick(sx - 10, sy - 10, sx + 10, sy + 10);
				} else {
					draw_brackets_diamond_quick(sx - 10, sy - 10, sx + 10, sy + 10);
				}
			}
		}
		renderTargetClose();
	}
	renderTargetForeground();
	renderTargetIntegrity(0,OBJ_INDEX(target_objp));

	setGaugeColor();

	renderTargetShipInfo(target_objp);
	maybeRenderCargoScan(target_sip);
}

/**
 * @note formerly hud_render_target_debris(object *target_objp) (Swifty)
 */
void HudGaugeTargetBox::renderTargetDebris(object *target_objp)
{
	vec3d	obj_pos = ZERO_VECTOR;
	vec3d	camera_eye = ZERO_VECTOR;
	matrix	camera_orient = IDENTITY_MATRIX;
	debris	*debrisp;
	vec3d	orient_vec, up_vector;
	int		target_team;
	float		factor;	
	int flags=0;

	debrisp = &Debris[target_objp->instance];

	target_team = obj_team(target_objp);

	if ( Detail.targetview_model )	{
		// take the forward orientation to be the vector from the player to the current target
		vm_vec_sub(&orient_vec, &target_objp->pos, &Player_obj->pos);
		vm_vec_normalize(&orient_vec);

		factor = 2*target_objp->radius;

		// use the player's up vector, and construct the viewers orientation matrix
		up_vector = Player_obj->orient.vec.uvec;
		vm_vector_2_matrix(&camera_orient,&orient_vec,&up_vector,NULL);

		// normalize the vector from the player to the current target, and scale by a factor to calculate
		// the objects position
		vm_vec_copy_scale(&obj_pos,&orient_vec,factor);

		renderTargetSetup(&camera_eye, &camera_orient, 0.5f);
		model_clear_instance(debrisp->model_num);

		switch (Targetbox_wire) {
			case 0:
				flags |= MR_NO_LIGHTING;

				break;
			case 1:
				model_set_outline_color(255,255,255);

				flags = (Cmdline_nohtl) ? MR_SHOW_OUTLINE : MR_SHOW_OUTLINE_HTL;
				flags |= MR_NO_POLYS | MR_NO_LIGHTING;

				break;
			case 2:
				break;
			case 3:
				model_set_outline_color(255,255,255);

				flags |= MR_NO_LIGHTING | MR_NO_TEXTURING;

				break;
		}

		if(Targetbox_shader_effect > -1) {
			flags |= MR_ANIMATED_SHADER;

			opengl_shader_set_animated_effect(Targetbox_shader_effect);
		}

		// This calls the colour that doesn't get reset
		submodel_render( debrisp->model_num, debrisp->submodel_num, &target_objp->orient, &obj_pos, flags | MR_LOCK_DETAIL | MR_NO_FOGGING );
		renderTargetClose();
	}
	renderTargetForeground();
	renderTargetIntegrity(1);

	// print out ship class that debris came from
	char printable_ship_class[NAME_LENGTH];
	if (debrisp->parent_alt_name >= 0)
		mission_parse_lookup_alt_index(debrisp->parent_alt_name, printable_ship_class);
	else
		strcpy_s(printable_ship_class, Ship_info[debrisp->ship_info_index].name);

	end_string_at_first_hash_symbol(printable_ship_class);
	
	renderString(position[0] + Class_offsets[0], position[1] + Class_offsets[1], EG_TBOX_CLASS, printable_ship_class);	
	renderString(position[0] + Name_offsets[0], position[1] + Name_offsets[1], EG_TBOX_NAME, XSTR("Debris", 348));	
}

/**
 * @note Formerly hud_render_target_weapon(object *target_objp)
 */
void HudGaugeTargetBox::renderTargetWeapon(object *target_objp)
{
	vec3d		obj_pos = ZERO_VECTOR;
	vec3d		camera_eye = ZERO_VECTOR;
	matrix		camera_orient = IDENTITY_MATRIX;
	vec3d		orient_vec, up_vector;
	weapon_info	*target_wip = NULL;
	weapon		*wp = NULL;
	object		*viewer_obj, *viewed_obj;
	int *replacement_textures = NULL;
	int			target_team, is_homing, is_player_missile, missile_view, viewed_model_num, hud_target_lod, w, h;
	float			factor;
	char			outstr[100];				// temp buffer
	int flags=0;

	target_team = obj_team(target_objp);

	wp = &Weapons[target_objp->instance];
	target_wip = &Weapon_info[wp->weapon_info_index];

	if (target_wip->model_num == -1)
		return;

	is_homing = FALSE;
	if ( target_wip->wi_flags & WIF_HOMING && wp->homing_object != &obj_used_list )
		is_homing = TRUE;

	is_player_missile = FALSE;
	if ( target_objp->parent_sig == Player_obj->signature ) {
		is_player_missile = TRUE;
	}

	if ( Detail.targetview_model )	{

		viewer_obj			= Player_obj;
		viewed_obj			= target_objp;
		missile_view		= FALSE;
		viewed_model_num	= target_wip->model_num;
		hud_target_lod		= target_wip->hud_target_lod;
		if ( is_homing && is_player_missile ) {
			ship *homing_shipp = &Ships[wp->homing_object->instance];
			ship_info *homing_sip = &Ship_info[homing_shipp->ship_info_index];

			viewer_obj			= target_objp;
			viewed_obj			= wp->homing_object;
			missile_view		= TRUE;
			viewed_model_num	= homing_sip->model_num;
			replacement_textures = homing_shipp->ship_replacement_textures;
			hud_target_lod		= homing_sip->hud_target_lod;
		}

		// take the forward orientation to be the vector from the player to the current target
		vm_vec_sub(&orient_vec, &viewed_obj->pos, &viewer_obj->pos);
		vm_vec_normalize(&orient_vec);

		if ( missile_view == FALSE )
			factor = 2*target_objp->radius;
		else
			factor = vm_vec_dist_quick(&viewer_obj->pos, &viewed_obj->pos);

		// use the viewer's up vector, and construct the viewers orientation matrix
		up_vector = viewer_obj->orient.vec.uvec;
		vm_vector_2_matrix(&camera_orient,&orient_vec,&up_vector,NULL);

		// normalize the vector from the viewer to the viwed target, and scale by a factor to calculate
		// the objects position
		vm_vec_copy_scale(&obj_pos,&orient_vec,factor);

		renderTargetSetup(&camera_eye, &camera_orient, View_zoom/3);
		model_clear_instance(viewed_model_num);

		switch (Targetbox_wire) {
			case 0:
				flags |= MR_NO_LIGHTING;

				break;
			case 1:
				model_set_outline_color_fast(iff_get_color_by_team_and_object(target_team, Player_ship->team, 0, target_objp));

				flags = (Cmdline_nohtl) ? MR_SHOW_OUTLINE : MR_SHOW_OUTLINE_HTL;
				flags |= MR_NO_POLYS | MR_NO_LIGHTING;

				break;
			case 2:
				break;
			case 3:
				model_set_outline_color_fast(iff_get_color_by_team_and_object(target_team, Player_ship->team, 0, target_objp));

				flags |= MR_NO_LIGHTING | MR_NO_TEXTURING;

				break;
		}

		if (hud_target_lod >= 0) {
			model_set_detail_level(hud_target_lod);
		}

		if(Targetbox_shader_effect > -1) {
			flags |= MR_ANIMATED_SHADER;

			opengl_shader_set_animated_effect(Targetbox_shader_effect);
		}

		model_render( viewed_model_num, &viewed_obj->orient, &obj_pos, flags | MR_LOCK_DETAIL | MR_AUTOCENTER | MR_IS_MISSILE | MR_NO_FOGGING, -1, -1, replacement_textures);
		renderTargetClose();
	}
	renderTargetForeground(); 

	renderTargetIntegrity(1);
	setGaugeColor();

	// print out the weapon class name
	sprintf( outstr,"%s", target_wip->name );
	gr_get_string_size(&w,&h,outstr);

	// drop name past the # sign
	end_string_at_first_hash_symbol(outstr);			

	renderString(position[0] + Name_offsets[0], position[1] + Name_offsets[1], EG_TBOX_NAME, outstr);	

	// If a homing weapon, show time to impact
	if ( is_homing ) {
		float dist, speed;

		dist = vm_vec_dist(&target_objp->pos, &wp->homing_object->pos);
		speed = vm_vec_mag(&target_objp->phys_info.vel);
		if ( speed > 0 ) {
			sprintf(outstr, NOX("impact: %.1f sec"), dist/speed);
		} else {
			sprintf(outstr, XSTR( "unknown", 349));
		}

		renderString(position[0] + Class_offsets[0], position[1] + Class_offsets[1], EG_TBOX_CLASS, outstr);		
	}
}

/**
 * @note Formerly hud_render_target_asteroid(object *target_objp)
 */
void HudGaugeTargetBox::renderTargetAsteroid(object *target_objp)
{
	vec3d		obj_pos = ZERO_VECTOR;
	vec3d		camera_eye = ZERO_VECTOR;
	matrix		camera_orient = IDENTITY_MATRIX;
	asteroid		*asteroidp;
	vec3d		orient_vec, up_vector;
	int			target_team;
	float			time_to_impact, factor;	
	int			pof;

	int flags=0;									//draw flags for wireframe
	asteroidp = &Asteroids[target_objp->instance];

	target_team = obj_team(target_objp);

	pof = asteroidp->asteroid_subtype;
	
	time_to_impact = asteroid_time_to_impact(target_objp);

	if ( Detail.targetview_model )	{
		// take the forward orientation to be the vector from the player to the current target
		vm_vec_sub(&orient_vec, &target_objp->pos, &Player_obj->pos);
		vm_vec_normalize(&orient_vec);

		factor = 2*target_objp->radius;

		// use the player's up vector, and construct the viewers orientation matrix
		up_vector = Player_obj->orient.vec.uvec;
		vm_vector_2_matrix(&camera_orient,&orient_vec,&up_vector,NULL);

		// normalize the vector from the player to the current target, and scale by a factor to calculate
		// the objects position
		vm_vec_copy_scale(&obj_pos,&orient_vec,factor);

		renderTargetSetup(&camera_eye, &camera_orient, 0.5f);
		model_clear_instance(Asteroid_info[asteroidp->asteroid_type].model_num[pof]);
		
		switch (Targetbox_wire) {
			case 0:
				flags |= MR_NO_LIGHTING;

				break;
			case 1:
				if (time_to_impact>=0)
					model_set_outline_color(255,255,255);
				else
					model_set_outline_color(64,64,0);

				flags = (Cmdline_nohtl) ? MR_SHOW_OUTLINE : MR_SHOW_OUTLINE_HTL;
				flags |= MR_NO_POLYS | MR_NO_LIGHTING;

				break;
			case 2:
				break;
			case 3:
				if (time_to_impact>=0)
					model_set_outline_color(255,255,255);
				else
					model_set_outline_color(64,64,0);

				flags |= MR_NO_LIGHTING | MR_NO_TEXTURING;

				break;
		}

		if(Targetbox_shader_effect > -1) {
			flags |= MR_ANIMATED_SHADER;

			opengl_shader_set_animated_effect(Targetbox_shader_effect);
		}

		model_render(Asteroid_info[asteroidp->asteroid_type].model_num[pof], &target_objp->orient, &obj_pos, flags | MR_LOCK_DETAIL | MR_NO_FOGGING );
		renderTargetClose();
	}
	renderTargetForeground();
	renderTargetIntegrity(1);
	setGaugeColor();

	// hud print type of Asteroid (debris)
	char hud_name[64];
	switch (asteroidp->asteroid_type) {
		case ASTEROID_TYPE_SMALL:
		case ASTEROID_TYPE_MEDIUM:
		case ASTEROID_TYPE_LARGE:
			strcpy_s(hud_name, NOX("asteroid"));
			break;

		default:
			sprintf(hud_name, NOX("%s debris"), Species_info[(asteroidp->asteroid_type / NUM_DEBRIS_SIZES) - 1].species_name);
			break;
	}

	renderString(position[0] + Name_offsets[0], position[1] + Name_offsets[1], EG_TBOX_NAME, hud_name);
	

	if ( time_to_impact >= 0.0f ) {
		renderPrintf(position[0] + Class_offsets[0], position[1] + Class_offsets[1], EG_TBOX_CLASS, NOX("impact: %.1f sec"), time_to_impact);	
	}
}

/**
 * Render a jump node on the target monitor
 * @note Formerly hud_render_target_jump_node(object *target_objp)
 */
void HudGaugeTargetBox::renderTargetJumpNode(object *target_objp)
{
	char			outstr[256];
	vec3d		obj_pos = ZERO_VECTOR;
	vec3d		camera_eye = ZERO_VECTOR;
	matrix		camera_orient = IDENTITY_MATRIX;
	vec3d		orient_vec, up_vector;
	float			factor, dist;
	int			hx, hy, w, h;
	SCP_list<jump_node>::iterator jnp;
	
	for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
		if(jnp->get_obj() != target_objp)
			continue;
	
		if ( jnp->is_hidden() ) {
			set_target_objnum( Player_ai, -1 );
			return;
		}

		if ( Detail.targetview_model )	{
			// take the forward orientation to be the vector from the player to the current target
			vm_vec_sub(&orient_vec, &target_objp->pos, &Player_obj->pos);
			vm_vec_normalize(&orient_vec);

			factor = target_objp->radius*4.0f;

			// use the player's up vector, and construct the viewers orientation matrix
			up_vector = Player_obj->orient.vec.uvec;
			vm_vector_2_matrix(&camera_orient,&orient_vec,&up_vector,NULL);

			// normalize the vector from the player to the current target, and scale by a factor to calculate
			// the objects position
			vm_vec_copy_scale(&obj_pos,&orient_vec,factor);

			renderTargetSetup(&camera_eye, &camera_orient, 0.5f);
			jnp->render( &obj_pos );
			renderTargetClose();
		}

		renderTargetForeground();
		renderTargetIntegrity(1);
		setGaugeColor();

		strcpy_s(outstr, jnp->get_name_ptr());
		end_string_at_first_hash_symbol(outstr);
		renderString(position[0] + Name_offsets[0], position[1] + Name_offsets[1], EG_TBOX_NAME, outstr);	

		dist = vm_vec_dist_quick(&target_objp->pos, &Player_obj->pos);
		if ( Hud_unit_multiplier > 0.0f ) {	// use a different displayed distance scale
			dist = dist * Hud_unit_multiplier;
		}

		// account for hud shaking
		hx = fl2i(HUD_offset_x);
		hy = fl2i(HUD_offset_y);

		sprintf(outstr,XSTR( "d: %.0f", 340), dist);
		hud_num_make_mono(outstr);
		gr_get_string_size(&w,&h,outstr);
	
		renderPrintf(position[0] + Dist_offsets[0]+hx, position[1] + Dist_offsets[1]+hy, EG_TBOX_DIST, outstr);
	}
}

/**
 * Toggle through the valid targetbox modes
 *
 * @note 0==standard
 * @note 1==wireframe only
 * @note 2==standard with lighting
 */
void hud_targetbox_switch_wireframe_mode()
{

	Targetbox_wire++;
		if (Targetbox_wire==3)
			Targetbox_wire=0;
}

/**
 * Init a specific targetbox timer
 */
void hud_targetbox_init_flash_timer(int index)
{
	Targetbox_flash_timers[index] = 1;
}

/**
 * Init the timers used to flash different parts of the targetbox.
 *
 * @note This needs to get called whenever the current target changes.
 * @note Need to call initFlashTimers for any TargetBox gauges and call initDockFlashTimer() for Extra Target Info gauges (Switfty)
 */
void hud_targetbox_init_flash()
{
	for(int i = 0; i < NUM_TBOX_FLASH_TIMERS; i++) {
		hud_targetbox_init_flash_timer(i);
	}

	Last_ts = -1;
	Current_ts = -1;
}

int HudGaugeTargetBox::maybeFlashElement(int index, int flash_fast)
{
	int draw_bright=0;

	setGaugeColor();
	if ( !timestamp_elapsed(Targetbox_flash_timers[index]) ) {
		if ( timestamp_elapsed(Next_flash_timers[index]) ) {
			if ( flash_fast ) {
				Next_flash_timers[index] = timestamp(fl2i(TBOX_FLASH_INTERVAL/2.0f));
			} else {
				Next_flash_timers[index] = timestamp(TBOX_FLASH_INTERVAL);
			}
			flash_flags ^= (1<<index);	// toggle between default and bright frames
		}

		if ( flash_flags & (1<<index) ) {
			setGaugeColor(HUD_C_BRIGHT);
			draw_bright=1;
		} else {			
			setGaugeColor(HUD_C_DIM);
		}
	}

	return draw_bright;
}

void HudGaugeTargetBox::renderTargetClose()
{
	if (!Cmdline_nohtl) {
		gr_end_view_matrix();
		gr_end_proj_matrix();
	}

	g3_end_frame();
	hud_save_restore_camera_data(0);
	resetClip();
}

/**
 * Get the shield and hull percentages for a given ship object
 *
 * @param objp		Pointer to ship object that you want strength values for
 * @param shields	OUTPUT parameter:	percentage value of shields (0->1.0)
 * @param integrity OUTPUT parameter: percentage value of integrity (0->1.0)
 */
void hud_get_target_strength(object *objp, float *shields, float *integrity)
{
	*shields = get_shield_pct(objp);
	*integrity = get_hull_pct(objp);
}

HudGaugeExtraTargetData::HudGaugeExtraTargetData():
HudGauge(HUD_OBJECT_EXTRA_TARGET_DATA, HUD_TARGET_MONITOR_EXTRA_DATA, true, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY), 255, 255, 255)
{
	initDockFlashTimer();
}

void HudGaugeExtraTargetData::initialize()
{
	initDockFlashTimer();
}

void HudGaugeExtraTargetData::initBracketOffsets(int x, int y)
{
	bracket_offsets[0] = x;
	bracket_offsets[1] = y;
}

void HudGaugeExtraTargetData::initDockOffsets(int x, int y)
{
	dock_offsets[0] = x;
	dock_offsets[1] = y;
}

void HudGaugeExtraTargetData::initTimeOffsets(int x, int y)
{
	time_offsets[0] = x;
	time_offsets[1] = y;
}

void HudGaugeExtraTargetData::initOrderOffsets(int x, int y)
{
	order_offsets[0] = x;
	order_offsets[1] = y;
}

void HudGaugeExtraTargetData::initBitmaps(char *fname)
{
	bracket.first_frame = bm_load_animation(fname, &bracket.num_frames);
	if ( bracket.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname);
	}
}

void HudGaugeExtraTargetData::pageIn()
{
	bm_page_in_aabitmap( bracket.first_frame, bracket.num_frames );
}

/**
 * @note Formerly hud_targetbox_show_extra_ship_info(target_shipp, target_objp) (Swifty)
 */
void HudGaugeExtraTargetData::render(float frametime)
{
	char outstr[256], tmpbuf[256];
	int has_orders = 0;
	int not_training;
	int extra_data_shown=0;

	if(!canRender())
		return;

	if ( Player_ai->target_objnum == -1)
		return;
	
	if ( Target_static_playing ) 
		return;

	object	*target_objp;
	target_objp = &Objects[Player_ai->target_objnum];

	// only render if this the current target is type OBJ_SHIP
	if(target_objp->type != OBJ_SHIP)
		return;

	ship* target_shipp	= &Ships[target_objp->instance];

	setGaugeColor();

	not_training = !(The_mission.game_type & MISSION_TYPE_TRAINING);
	if ( not_training) {
		// Print out current orders if the targeted ship is friendly
		// AL 12-26-97: only show orders and time to target for friendly ships
		// Backslash: actually let's consult the IFF table.  Maybe we want to show orders for certain teams, or hide orders for friendlies
		if ( ((Player_ship->team == target_shipp->team) || ((Iff_info[target_shipp->team].flags & IFFF_ORDERS_SHOWN) && !(Iff_info[target_shipp->team].flags & IFFF_ORDERS_HIDDEN)) ) && !(ship_get_SIF(target_shipp) & SIF_NOT_FLYABLE) ) {
			extra_data_shown=1;
			if ( ship_return_orders(outstr, target_shipp) ) {
				gr_force_fit_string(outstr, 255, 162);
				has_orders = 1;
			} else {
				strcpy_s(outstr, XSTR( "no orders", 337));
			}
			
			renderString(position[0] + order_offsets[0], position[1] + order_offsets[1], EG_TBOX_EXTRA1, outstr);			
		}

		if ( has_orders ) {
			sprintf(outstr, XSTR( "time to: ", 338));
			if ( ship_return_time_to_goal(tmpbuf, target_shipp) ) {
				strcat_s(outstr, tmpbuf);
				
				renderString(position[0] + time_offsets[0], position[1] + time_offsets[1], EG_TBOX_EXTRA2, outstr);				
			}
		}
	}

	if (Player_ai->last_target != Player_ai->target_objnum) {
		endFlashDock();
	}

	// Print out dock status
	if ( object_is_docked(target_objp) )
	{
		startFlashDock(2000);
		// count the objects directly docked to me
		int dock_count = dock_count_direct_docked_objects(target_objp);

		// docked to only one object
		if (dock_count == 1)
		{
			sprintf(outstr, XSTR("Docked: %s", 339), Ships[dock_get_first_docked_object(target_objp)->instance].ship_name);
			end_string_at_first_hash_symbol(outstr);
		}
		// docked to multiple objects
		else
		{
			sprintf(outstr, XSTR("Docked: %d objects", -1), dock_count);
		}

		gr_force_fit_string(outstr, 255, 173);
		maybeFlashDock();
			
		renderString(position[0] + dock_offsets[0], position[1] + dock_offsets[1], EG_TBOX_EXTRA3, outstr);			
		extra_data_shown=1;
	}

	if ( extra_data_shown ) {	
		renderBitmap(bracket.first_frame, position[0] + bracket_offsets[0], position[1] + bracket_offsets[1]);		
	}
}

void HudGaugeExtraTargetData::initDockFlashTimer()
{
	flash_timer[0] = 1;
	flash_timer[1] = 1;
	flash_flags = false;
}

void HudGaugeExtraTargetData::startFlashDock(int duration)
{
	flash_timer[0] = timestamp(duration);
}

int HudGaugeExtraTargetData::maybeFlashDock(int flash_fast)
{
	int draw_bright=0;

	setGaugeColor();
	if ( !timestamp_elapsed(flash_timer[0]) ) {
		if ( timestamp_elapsed(flash_timer[1]) ) {
			if ( flash_fast ) {
				flash_timer[1] = timestamp(fl2i(TBOX_FLASH_INTERVAL/2.0f));
			} else {
				flash_timer[1] = timestamp(TBOX_FLASH_INTERVAL);
			}

			// toggle between default and bright frames
			if(flash_flags)
				flash_flags = false;
			else
				flash_flags = true;
		}

		if (flash_flags) {
			setGaugeColor(HUD_C_BRIGHT);
			draw_bright=1;
		} else {			
			setGaugeColor(HUD_C_DIM);
		}
	}

	return draw_bright;
}

void HudGaugeExtraTargetData::endFlashDock()
{
	flash_timer[0] = timestamp(0);
}

//from aicode.cpp. Less include...problems...this way.
extern bool turret_weapon_has_flags(ship_weapon *swp, int flags);
extern bool turret_weapon_has_flags2(ship_weapon *swp, int flags);
extern bool turret_weapon_has_subtype(ship_weapon *swp, int subtype);
void get_turret_subsys_name(ship_weapon *swp, char *outstr)
{
	Assert(swp != NULL);	// Goober5000 //WMC

	//WMC - find the first weapon, if there is one
	if (swp->num_primary_banks || swp->num_secondary_banks) {
		// check if beam or flak using weapon flags
		if (turret_weapon_has_flags(swp, WIF_BEAM)) {
			sprintf(outstr, "%s", XSTR("Beam turret", 1567));
		}else if (turret_weapon_has_flags(swp, WIF_FLAK)) {
			sprintf(outstr, "%s", XSTR("Flak turret", 1566));
		} else {

			if (!turret_weapon_has_subtype(swp, WP_MISSILE) && turret_weapon_has_subtype(swp, WP_LASER)) {
				// ballistic too! - Goober5000
				if (turret_weapon_has_flags2(swp, WIF2_BALLISTIC))
				{
					sprintf(outstr, "%s", XSTR("Turret", 1487));
				}
				// the TVWP has some primaries flagged as bombs
				else if (turret_weapon_has_flags(swp, WIF_BOMB))
				{
					sprintf(outstr, "%s", XSTR("Missile lnchr", 1569));
				}
				else
				{
					sprintf(outstr, "%s", XSTR("Laser turret", 1568));
				}
			} else if (turret_weapon_has_subtype(swp, WP_MISSILE)) {
				sprintf(outstr, "%s", XSTR("Missile lnchr", 1569));
			} else {
				// Illegal subtype
				Int3();
				sprintf(outstr, "%s", XSTR("Turret", 1487));
			}
		}
	} else if(swp->num_tertiary_banks) {
		//TODO: add tertiary turret code stuff here
		sprintf(outstr, "%s", NOX("Unknown"));
	} else {
		// This should not happen
		sprintf(outstr, "%s", NOX("Unused"));
	}
}

void HudGaugeTargetBox::renderTargetShipInfo(object *target_objp)
{
	ship			*target_shipp;
	ship_info	*target_sip;
	int			w, h, screen_integrity = 1;
	char			outstr[NAME_LENGTH];
	char			outstr_name[NAME_LENGTH*2+3];
	char			outstr_class[NAME_LENGTH];
	float			ship_integrity, shield_strength;

	Assert(target_objp);	// Goober5000
	Assert(target_objp->type == OBJ_SHIP);
	target_shipp = &Ships[target_objp->instance];
	target_sip = &Ship_info[target_shipp->ship_info_index];

	// set up colors
	if ( HudGauge::maybeFlashSexp() == 1 ) {
		hud_set_iff_color(target_objp, 1);
	} else {
		// Print out ship name, with wing name if it exists
		if ( maybeFlashElement(TBOX_FLASH_NAME) ) {
			hud_set_iff_color(target_objp, 1);
		} else {
			hud_set_iff_color(target_objp);
		}
	}

	// set up lines
	hud_stuff_ship_name(outstr_name, target_shipp);
	hud_stuff_ship_class(outstr_class, target_shipp);

	// maybe concatenate the callsign
	if (*outstr_name)
	{
		char outstr_callsign[NAME_LENGTH];

		hud_stuff_ship_callsign(outstr_callsign, target_shipp);
		if (*outstr_callsign)
			sprintf(&outstr_name[strlen(outstr_name)], " (%s)", outstr_callsign);
	}
	// maybe substitute the callsign
	else
	{
		hud_stuff_ship_callsign(outstr_name, target_shipp);
	}

	// print lines based on current coords
	renderString(position[0] + Name_offsets[0], position[1] + Name_offsets[1], EG_TBOX_NAME, outstr_name);	
	renderString(position[0] + Class_offsets[0], position[1] + Class_offsets[1], EG_TBOX_CLASS, outstr_class);

	// ----------

	ship_integrity = 1.0f;
	shield_strength = 1.0f;
	hud_get_target_strength(target_objp, &shield_strength, &ship_integrity);

	// convert to values of 0->100
	shield_strength *= 100.0f;
	ship_integrity *= 100.0f;

	screen_integrity = fl2i(ship_integrity+0.5f);
	if ( screen_integrity == 0 ) {
		if ( ship_integrity > 0 ) {
			screen_integrity = 1;
		}
	}
	// Print out right-justified integrity
	sprintf(outstr, XSTR( "%d%%", 341), screen_integrity);
	gr_get_string_size(&w,&h,outstr);

	if ( HudGauge::maybeFlashSexp() == 1 ) {
		setGaugeColor(HUD_C_BRIGHT);
	} else {
		maybeFlashElement(TBOX_FLASH_HULL);
	}

	renderPrintf(position[0] + Hull_offsets[0]-w, position[1] + Hull_offsets[1], EG_TBOX_HULL, "%s", outstr);	
	setGaugeColor();

	// print out the targeted sub-system and % integrity
	if (Player_ai->targeted_subsys != NULL) {
		shield_strength = Player_ai->targeted_subsys->current_hits/Player_ai->targeted_subsys->max_hits * 100.0f;
		screen_integrity = fl2i(shield_strength+0.5f);

		if ( screen_integrity < 0 ) {
			screen_integrity = 0;
		}

		if ( screen_integrity == 0 ) {
			if ( shield_strength > 0 ) {
				screen_integrity = 1;
			}
		}

		maybeFlashElement(TBOX_FLASH_SUBSYS);

		// get turret subsys name
		if (Player_ai->targeted_subsys->system_info->type == SUBSYSTEM_TURRET && !ship_subsys_has_instance_name(Player_ai->targeted_subsys)) {
			get_turret_subsys_name(&Player_ai->targeted_subsys->weapons, outstr);
		} else {
			sprintf(outstr, "%s", ship_subsys_get_name(Player_ai->targeted_subsys));
		}

		char *p_line;
		// hence pipe shall be the linebreak
		char linebreak[2] = "|";
		int n_linebreaks = 0;
		p_line = strpbrk(outstr,linebreak);
		
		// figure out how many linebreaks we actually have
		while (p_line != NULL) {
			n_linebreaks++;
			p_line = strpbrk(p_line+1,linebreak);
		}

		if (n_linebreaks) {
			p_line = strtok(outstr,linebreak);
			while (p_line != NULL) {
				gr_printf(position[0] + Viewport_offsets[0]+2, position[1] + Viewport_offsets[1]+Viewport_h-h-(10*n_linebreaks), p_line);
				p_line = strtok(NULL,linebreak);
				n_linebreaks--;
			}
		} else {
			hud_targetbox_truncate_subsys_name(outstr);
			renderPrintf(position[0] + Viewport_offsets[0]+2, position[1] + Viewport_offsets[1]+Viewport_h-h, outstr);
		}

		// AL 23-3-98: Fighter bays are a special case.  Player cannot destroy them, so don't
		//					show the subsystem strength
		// Goober5000: don't display any strength if we can't destroy this subsystem - but sometimes
		// fighterbays can be destroyed
		if ( ship_subsys_takes_damage(Player_ai->targeted_subsys) )
		{
			sprintf(outstr,XSTR( "%d%%", 341),screen_integrity);
			gr_get_string_size(&w,&h,outstr);
			renderPrintf(position[0] + Viewport_offsets[0] + Viewport_w - w - 1, position[1] + Viewport_offsets[1] + Viewport_h - h, "%s", outstr);
		}

		setGaugeColor();
	}

	// print out 'disabled' on the monitor if the target is disabled
	if ( (target_shipp->flags & SF_DISABLED) || (ship_subsys_disrupted(target_shipp, SUBSYSTEM_ENGINE)) ) {
		if ( target_shipp->flags & SF_DISABLED ) {
			sprintf(outstr, XSTR( "DISABLED", 342));
		} else {
			sprintf(outstr, XSTR( "DISRUPTED", 343));
		}
		gr_get_string_size(&w,&h,outstr);
		renderPrintf(position[0] + Viewport_offsets[0] + Viewport_w/2 - w/2 - 1, position[1] + Viewport_offsets[1] + Viewport_h - 2*h, "%s", outstr);
	}
}

/**
 * Determine if the subsystem is in line-of sight, without taking into account whether the player ship is
 * facing the subsystem
 */
int hud_targetbox_subsystem_in_view(object *target_objp, int *sx, int *sy)
{
	ship_subsys	*subsys;
	vec3d		subobj_pos;
	vertex		subobj_vertex;
	int			rval = -1;
	polymodel	*pm;

	subsys = Player_ai->targeted_subsys;
	if (subsys != NULL ) {
		find_submodel_instance_point(&subobj_pos, target_objp, subsys->system_info->subobj_num);
		vm_vec_add2(&subobj_pos, &target_objp->pos);

		// is it subsystem in view
		if ( Player->subsys_in_view == -1 ) {
			rval = ship_subsystem_in_sight(target_objp, subsys, &View_position, &subobj_pos, 0);
		} else {
			rval =  Player->subsys_in_view;
		}

		// get screen coords, adjusting for autocenter
		Assert(target_objp->type == OBJ_SHIP);
		if (target_objp->type == OBJ_SHIP) {
			pm = model_get(Ship_info[Ships[target_objp->instance].ship_info_index].model_num);
			if (pm->flags & PM_FLAG_AUTOCEN) {
				vec3d temp, delta;
				vm_vec_copy_scale(&temp, &pm->autocenter, -1.0f);
				vm_vec_unrotate(&delta, &temp, &target_objp->orient);
				vm_vec_add2(&subobj_pos, &delta);
			}
		}

		g3_rotate_vertex(&subobj_vertex, &subobj_pos);
		g3_project_vertex(&subobj_vertex);
		*sx = (int) subobj_vertex.screen.xyw.x;
		*sy = (int) subobj_vertex.screen.xyw.y;
	}

	return rval;
}

void hud_cargo_scan_update(object *targetp, float frametime)
{
	// update cargo inspection status
	Cargo_string[0] = 0;
	if ( targetp->type == OBJ_SHIP ) {
		Target_display_cargo = player_inspect_cargo(frametime, Cargo_string);
		if ( Target_display_cargo ) {
			if ( Player->cargo_inspect_time > 0 ) {
				hud_targetbox_start_flash(TBOX_FLASH_CARGO);
			}
		}
	}
}

void hud_update_cargo_scan_sound()
{
	if ( Player->cargo_inspect_time <= 0  ) {
		player_stop_cargo_scan_sound();
		return;
	}
	player_maybe_start_cargo_scan_sound();

}

/**
 * If the player is scanning for cargo, draw some cool scanning lines on the target monitor
 */
void HudGaugeTargetBox::maybeRenderCargoScan(ship_info *target_sip)
{
	int x1, y1, x2, y2;
	int scan_time;				// time required to scan ship

	if ( Player->cargo_inspect_time <= 0  ) {
		return;
	}

	scan_time = target_sip->scan_time;
	setGaugeColor(HUD_C_BRIGHT);

	// draw horizontal scan line
	x1 = position[0] + Cargo_scan_start_offsets[0]; // Cargo_scan_coords[gr_screen.res][0];
	y1 = fl2i(0.5f + position[1] + Cargo_scan_start_offsets[1] + ( (i2fl(Player->cargo_inspect_time) / scan_time) * Cargo_scan_h ));
	x2 = x1 + Cargo_scan_w;

	renderLine(x1, y1, x2, y1);

	// RT Changed this to be optional
	if(Cmdline_dualscanlines) {
		// added 2nd horizontal scan line - phreak
		y1 = fl2i(position[1] + Cargo_scan_start_offsets[1] + Cargo_scan_h - ( (i2fl(Player->cargo_inspect_time) / scan_time) * Cargo_scan_h ));
		renderLine(x1, y1, x2, y1);
	}

	// draw vertical scan line
	x1 = fl2i(0.5f + position[0] + Cargo_scan_start_offsets[0] + ( (i2fl(Player->cargo_inspect_time) / scan_time) * Cargo_scan_w ));
	y1 = position[1] + Cargo_scan_start_offsets[1];
	y2 = y1 + Cargo_scan_h;

	renderLine(x1, y1-3, x1, y2-1);

	// RT Changed this to be optional
	if(Cmdline_dualscanlines) {
		// added 2nd vertical scan line - phreak
		x1 = fl2i(0.5f + Cargo_scan_w + position[0] + Cargo_scan_start_offsets[0] - ( (i2fl(Player->cargo_inspect_time) / scan_time) * Cargo_scan_w ));
		renderLine(x1, y1-3, x1, y2-1);
	}
}

void HudGaugeTargetBox::showTargetData(float frametime)
{
	char outstr[256];						// temp buffer for sprintf'ing hud output
	int w,h;									// width and height of string about to print
	object		*target_objp;
	ship			*shipp = NULL;
	debris		*debrisp = NULL;
	ship_info	*sip = NULL;
	int is_ship = 0;
	float		displayed_target_distance, displayed_target_speed, current_target_distance, current_target_speed;

	setGaugeColor();

	target_objp = &Objects[Player_ai->target_objnum];

	current_target_distance = Player_ai->current_target_distance;

	if ( Hud_unit_multiplier > 0.0f ) {	// use a different displayed distance scale
		displayed_target_distance = current_target_distance * Hud_unit_multiplier;
	} else {
		displayed_target_distance = current_target_distance;
	}

	switch( Objects[Player_ai->target_objnum].type ) {
		case OBJ_SHIP:
			shipp = &Ships[target_objp->instance];
			sip = &Ship_info[shipp->ship_info_index];
			is_ship = 1;
			break;

		case OBJ_DEBRIS:
			debrisp = &Debris[target_objp->instance]; 
			sip = &Ship_info[debrisp->ship_info_index];
			break;

		case OBJ_WEAPON:
			sip = NULL;
			break;

		case OBJ_ASTEROID:
			sip = NULL;
			break;

		case OBJ_JUMP_NODE:
			return;

		default:
			Int3();	// can't happen
			break;
	}

	int hx, hy;

	// Account for HUD shaking
	hx = fl2i(HUD_offset_x);
	hy = fl2i(HUD_offset_y);

	// print out the target distance and speed
	sprintf(outstr,XSTR( "d: %.0f%s", 350), displayed_target_distance, modifiers[Player_ai->current_target_dist_trend]);

	hud_num_make_mono(outstr);
	gr_get_string_size(&w,&h,outstr);

	renderString(position[0] + Dist_offsets[0]+hx, position[1] + Dist_offsets[1]+hy, EG_TBOX_DIST, outstr);	

#if 0
	current_target_speed = vm_vec_dist(&target_objp->pos, &target_objp->last_pos) / frametime;
#endif
	// 7/28/99 DKA: Do not use vec_mag_quick -- the error is too big
	current_target_speed = vm_vec_mag(&target_objp->phys_info.vel);
	if ( current_target_speed < 0.1f ) {
		current_target_speed = 0.0f;
	}
	// if the speed is 0, determine if we are docked with something -- if so, get the docked velocity
	if ( (current_target_speed == 0.0f) && is_ship ) {
		current_target_speed = dock_calc_docked_fspeed(&Objects[shipp->objnum]);

		if ( current_target_speed < 0.1f ) {
			current_target_speed = 0.0f;
		}
	}

	if ( Hud_speed_multiplier > 0.0f ) {	// use a different displayed speed scale
		displayed_target_speed = current_target_speed * Hud_speed_multiplier;
	} else {
		displayed_target_speed = current_target_speed;
	}

	sprintf(outstr, XSTR( "s: %.0f%s", 351), displayed_target_speed, (displayed_target_speed>1)?modifiers[Player_ai->current_target_speed_trend]:"");
	hud_num_make_mono(outstr);

	renderString(position[0] + Speed_offsets[0]+hx, position[1] + Speed_offsets[1]+hy, EG_TBOX_SPEED, outstr);

	//
	// output target info for debug purposes only, this will be removed later
	//

#ifndef NDEBUG
	//XSTR:OFF
	char outstr2[256];	
	if ( Show_target_debug_info && (is_ship == 1) ) {
		int sx, sy, dy;
		sx = 5;
		dy = gr_get_font_height() + 1;
		sy = 300 - 7*dy;

		gr_set_color_fast(&HUD_color_debug);

		if ( shipp->ai_index >= 0 ) {
			ai_info	*aip = &Ai_info[shipp->ai_index];

			sprintf(outstr,"AI: %s",Ai_behavior_names[aip->mode]);

			switch (aip->mode) {
			case AIM_CHASE:
				Assert(aip->submode <= SM_BIG_PARALLEL);	//	Must be <= largest chase submode value.
				sprintf(outstr2," / %s",Submode_text[aip->submode]);
				strcat_s(outstr,outstr2);
				break;
			case AIM_STRAFE:
				Assert(aip->submode <= AIS_STRAFE_POSITION);	//	Must be <= largest chase submode value.
				sprintf(outstr2," / %s",Strafe_submode_text[aip->submode-AIS_STRAFE_ATTACK]);
				strcat_s(outstr,outstr2);
				break;
			case AIM_WAYPOINTS:
				break;
			default:
				break;
			}

			gr_printf(sx, sy, outstr);
			sy += dy;

			gr_printf(sx, sy, "Max speed = %d, (%d%%)", (int) shipp->current_max_speed, (int) (100.0f * vm_vec_mag(&target_objp->phys_info.vel)/shipp->current_max_speed));
			sy += dy;
			
			// data can be found in target montior
			if (aip->target_objnum != -1) {
				char	target_str[32];
				float	dot, dist;
				vec3d	v2t;

				if (aip->target_objnum == Player_obj-Objects)
					strcpy_s(target_str, "Player!");
				else
					sprintf(target_str, "%s", Ships[Objects[aip->target_objnum].instance].ship_name);

				gr_printf(sx, sy, "Targ: %s", target_str);
				sy += dy;

				dist = vm_vec_dist_quick(&Objects[Player_ai->target_objnum].pos, &Objects[aip->target_objnum].pos);
				vm_vec_normalized_dir(&v2t,&Objects[aip->target_objnum].pos, &Objects[Player_ai->target_objnum].pos);

				dot = vm_vec_dot(&v2t, &Objects[Player_ai->target_objnum].orient.vec.fvec);

				// data can be found in target monitor
				gr_printf(sx, sy, "Targ dot: %3.2f", dot);
				sy += dy;
				gr_printf(sx, sy, "Targ dst: %3.2f", dist);
				sy += dy;

				if ( aip->targeted_subsys != NULL ) {
					sprintf(outstr, "Subsys: %s", aip->targeted_subsys->system_info->subobj_name);
					gr_printf(sx, sy, outstr);
				}
				sy += dy;
			}

			// print out energy transfer information on the ship
			sy = 70;

			sprintf(outstr,"MAX G/E: %.0f/%.0f",shipp->weapon_energy,shipp->current_max_speed);
			gr_printf(sx, sy, outstr);
			sy += dy;
			 
			sprintf(outstr,"G/S/E: %.2f/%.2f/%.2f",Energy_levels[shipp->weapon_recharge_index],Energy_levels[shipp->shield_recharge_index],Energy_levels[shipp->engine_recharge_index]);
			gr_printf(sx, sy, outstr);
			sy += dy;

			//	Show information about attacker.
			{
				int	found = 0;

				if (Enemy_attacker != NULL)
					if (Enemy_attacker->type == OBJ_SHIP) {
						ship		*eshipp;
						ai_info	*eaip;
						float		dot, dist;
						vec3d	v2t;

						eshipp = &Ships[Enemy_attacker->instance];
						eaip = &Ai_info[eshipp->ai_index];

						if (eaip->target_objnum == Player_obj-Objects) {
							found = 1;
							dist = vm_vec_dist_quick(&Enemy_attacker->pos, &Player_obj->pos);
							vm_vec_normalized_dir(&v2t,&Objects[eaip->target_objnum].pos, &Enemy_attacker->pos);

							dot = vm_vec_dot(&v2t, &Enemy_attacker->orient.vec.fvec);

							gr_printf(sx, sy, "#%i: %s", Enemy_attacker-Objects, Ships[Enemy_attacker->instance].ship_name);
							sy += dy;
							gr_printf(sx, sy, "Targ dist: %5.1f", dist);
							sy += dy;
							gr_printf(sx, sy, "Targ dot: %3.2f", dot);
							sy += dy;
						}
					}

				if (Player_ai->target_objnum == Enemy_attacker - Objects)
					found = 0;

				if (!found) {
					int	i;

					Enemy_attacker = NULL;
					for (i=0; i<MAX_OBJECTS; i++)
						if (Objects[i].type == OBJ_SHIP) {
							int	enemy;

							if (i != Player_ai->target_objnum) {
								enemy = Ai_info[Ships[Objects[i].instance].ai_index].target_objnum;

								if (enemy == Player_obj-Objects) {
									Enemy_attacker = &Objects[i];
									break;
								}
							}
						}
				}
			}

			// Show target size
			// hud_target_w
			gr_printf(sx, sy, "Targ size: %dx%d", Hud_target_w, Hud_target_h );
			sy += dy;

			polymodel *pm = model_get(sip->model_num);
			gr_printf(sx, sy, "POF:%s", pm->filename );
			sy += dy;

			gr_printf(sx, sy, "Mass: %.2f\n", pm->mass);
			sy += dy;
		}
	}

	// display the weapons for the target on the HUD.  Include ammo counts.
	if ( Show_target_weapons && (is_ship == 1) ) {
		int sx, sy, dy, i;
		ship_weapon *swp;

		swp = &shipp->weapons;
		sx = 400;
		sy = 100;
		dy = gr_get_font_height();

		sprintf(outstr,"Num primaries: %d", swp->num_primary_banks);
		gr_printf(sx,sy,outstr);
		sy += dy;
		for ( i = 0; i < swp->num_primary_banks; i++ ) {
			sprintf(outstr,"%d. %s", i+1, Weapon_info[swp->primary_bank_weapons[i]].name);
			gr_printf(sx,sy,outstr);
			sy += dy;
		}

		sy += dy;
		sprintf(outstr,"Num secondaries: %d", swp->num_secondary_banks);
		gr_printf(sx,sy,outstr);
		sy += dy;
		for ( i = 0; i < swp->num_secondary_banks; i++ ) {
			sprintf(outstr,"%d. %s", i+1, Weapon_info[swp->secondary_bank_weapons[i]].name);
			gr_printf(sx,sy,outstr);
			sy += dy;
		}
	}
	//XSTR:ON

#endif
}

/**
 * Called at the start of each level
 */
void hud_init_target_static()
{
	Target_static_next = 0;
	Target_static_playing = 0;
}

/**
 * Determine if we should draw static on top of the target box
 */
void hud_update_target_static()
{
	float	sensors_str;

	// on lowest skill level, don't show static on target monitor
	if ( Game_skill_level == 0 ) 
		return;

	// if multiplayer observer, don't show static
	if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER))
		return;

	sensors_str = ship_get_subsystem_strength( Player_ship, SUBSYSTEM_SENSORS );

	if ( ship_subsys_disrupted(Player_ship, SUBSYSTEM_SENSORS) ) {
		sensors_str = SENSOR_STR_TARGET_NO_EFFECTS-1;
	}

	if ( sensors_str > SENSOR_STR_TARGET_NO_EFFECTS ) {
		Target_static_playing = 0;
		Target_static_next = 0;
	} else {
		if ( Target_static_next == 0 )
			Target_static_next = 1;
	}

	if ( timestamp_elapsed(Target_static_next) ) {
		Target_static_playing ^= 1;
		Target_static_next = timestamp_rand(50, 750);
	}

	if ( Target_static_playing ) {
		if ( Target_static_looping == -1 ) {
			Target_static_looping = snd_play_looping(&Snds[SND_STATIC]);
		}
	} else {
		if ( Target_static_looping != -1 ) {
			snd_stop(Target_static_looping);
			Target_static_looping = -1;
		}
	}
}

void hud_update_ship_status(object *targetp)
{
	// print out status of ship for the targetbox
	if ( (Ships[targetp->instance].flags & SF_DISABLED) || (ship_subsys_disrupted(&Ships[targetp->instance], SUBSYSTEM_ENGINE)) ) {
		Current_ts = TS_DIS;
	} else {
		if ( Pl_target_integrity > 0.9 ) {
			Current_ts = TS_OK;
		} else if ( Pl_target_integrity > 0.2 ) {
			Current_ts = TS_DMG;
		} else {
			Current_ts = TS_CRT;
		}
	}

	if ( Last_ts != -1 && Current_ts != Last_ts ) {
		hud_targetbox_start_flash(TBOX_FLASH_STATUS);
	}

	Last_ts = Current_ts;
}

/**
 * Start the targetbox item flashing for duration ms
 *
 * @param index		TBOX_FLASH_ define
 * @param duration	optional param (default value TBOX_FLASH_DURATION), how long to flash in ms
 */
void hud_targetbox_start_flash(int index, int duration)
{
	Targetbox_flash_timers[index] = timestamp(duration);
}

/**
 * Stop flashing a specific targetbox item
 */
void hud_targetbox_end_flash(int index)
{
	Targetbox_flash_timers[index] = timestamp(0);
}

void HudGaugeTargetBox::pageIn()
{
	bm_page_in_aabitmap( Monitor_frame.first_frame, Monitor_frame.num_frames);

	bm_page_in_aabitmap( Integrity_bar.first_frame, Integrity_bar.num_frames );
}

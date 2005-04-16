#include "wmcgui.h"
#include "gamesequence/gamesequence.h"
#include "io/key.h"
#include "freespace2/freespace.h"

#include "cmdline/cmdline.h"
#include "ship/ship.h"
#include "render/3d.h"
#include "lighting/lighting.h"
#include "model/model.h"
#include "missionui/missionscreencommon.h"
#include "weapon/beam.h"

//All sorts of globals

static GUIScreen *Lab_screen = NULL;

static int ShipSelectShipIndex = -1;
static int ShipSelectModelNum = -1;
static int ModelLOD = 0;
static int ModelFlags = MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING;
void change_lod(Tree* caller);

//*****************************Ship Options Window*******************************
static Window* ShipOptWin = NULL;

#define NUM_SHIP_OPTIONS	50
Checkbox *soc[NUM_SHIP_OPTIONS];

void set_ship_options_ship(ship_info *sip)
{
	unsigned int i=0;
	soc[i++]->SetFlag(&sip->flags, SIF_SUPPORT);
	soc[i++]->SetFlag(&sip->flags, SIF_CARGO);
	soc[i++]->SetFlag(&sip->flags, SIF_FIGHTER);
	soc[i++]->SetFlag(&sip->flags, SIF_BOMBER);
	soc[i++]->SetFlag(&sip->flags, SIF_CRUISER);
	soc[i++]->SetFlag(&sip->flags, SIF_CORVETTE);
	soc[i++]->SetFlag(&sip->flags, SIF_FREIGHTER);
	soc[i++]->SetFlag(&sip->flags, SIF_CAPITAL);
	soc[i++]->SetFlag(&sip->flags, SIF_TRANSPORT);
	soc[i++]->SetFlag(&sip->flags, SIF_NAVBUOY);
	soc[i++]->SetFlag(&sip->flags, SIF_SENTRYGUN);
	soc[i++]->SetFlag(&sip->flags, SIF_ESCAPEPOD);
	soc[i++]->SetFlag(&sip->flags, SIF_GAS_MINER);
	soc[i++]->SetFlag(&sip->flags, SIF_AWACS);
	soc[i++]->SetFlag(&sip->flags, SIF_SHIP_CLASS_STEALTH);
	soc[i++]->SetFlag(&sip->flags, SIF_SUPERCAP);
	soc[i++]->SetFlag(&sip->flags, SIF_KNOSSOS_DEVICE);
	soc[i++]->SetFlag(&sip->flags, SIF_DRYDOCK);
	soc[i++]->SetFlag(&sip->flags, SIF_SHIP_COPY);
	soc[i++]->SetFlag(&sip->flags, SIF_BIG_DAMAGE);
	soc[i++]->SetFlag(&sip->flags, SIF_HAS_AWACS);
	soc[i++]->SetFlag(&sip->flags, SIF_SHIP_CLASS_DONT_COLLIDE_INVIS);
	soc[i++]->SetFlag(&sip->flags, SIF_DO_COLLISION_CHECK);
	soc[i++]->SetFlag(&sip->flags, SIF_PLAYER_SHIP);
	soc[i++]->SetFlag(&sip->flags, SIF_DEFAULT_PLAYER_SHIP);
	soc[i++]->SetFlag(&sip->flags, SIF_BALLISTIC_PRIMARIES);
	soc[i++]->SetFlag(&sip->flags, SIF2_FLASH);
	soc[i++]->SetFlag(&sip->flags, SIF2_SURFACE_SHIELDS);
	soc[i++]->SetFlag(&sip->flags, SIF2_SHOW_SHIP_MODEL);
	soc[i++]->SetFlag(&sip->flags, SIF_IN_TECH_DATABASE);
	soc[i++]->SetFlag(&sip->flags, SIF_IN_TECH_DATABASE_M);

	//ShipOptWin->SetCaption(sip->name);
}

void zero_ship_opt_win(GUIObject *caller)
{
	ShipOptWin = NULL;
}

void ship_options_window(Button *caller)
{
	if(ShipOptWin != NULL)
		return;

	GUIObject* cwp = Lab_screen->Add(new Window("Ship options", gr_screen.max_w - 200, 200));
	ShipOptWin = (Window*)cwp;
	unsigned int i=0;
	int y = 0;

	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("SUPPORT", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("CARGO", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("FIGHTER", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("BOMBER", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("CRUISER", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("CORVETTE", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("FREIGHTER", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("CAPITAL", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("TRANSPORT", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("NAVBUOY", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("SENTRYGUN", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("ESCAPEPOD", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("GAS MINER", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("AWACS", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("STEALTH", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("SUPERCAP", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("KNOSSOS DEVICE", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("DRYDOCK", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("SHIP COPY", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("BIG DAMAGE", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("HAS AWACS", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("NO COLLIDE INVISIBLE", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("DO COLLISION CHECK", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("PLAYER SHIP", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("DEFAULT PLAYER SHIP", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("BALLISTIC PRIMARIES", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("FLASH", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("SURFACE SHIELDS", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("SHOW SHIP MODEL", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("IN TECH DATABASE", 0, y));
	y += soc[i++]->GetHeight() + 10;
	soc[i] = (Checkbox*) cwp->AddChild(new Checkbox("IN TECH DATABASE MULTI", 0, y));

	if(ShipSelectShipIndex != -1)
		set_ship_options_ship(&Ship_info[ShipSelectShipIndex]);

	cwp->SetCloseFunction(zero_ship_opt_win);
}
//*****************************Ship Variables Window*******************************
static Window* ShipVarWin = NULL;

#define NUM_SHIP_VARIABLES		25
Text *svt[NUM_SHIP_OPTIONS];

#define SVW_SET_SI_VAR(var)	svt[i]->SetText(sip->var);	\
	svt[i++]->SetSaveLoc(&sip->var, T_ST_ONENTER)

#define SVW_SET_VAR(var)	svt[i]->SetText(var);	\
	svt[i++]->SetSaveLoc(&var, T_ST_ONENTER)

extern int Hud_shield_filename_count;
extern int True_NumSpecies;

void set_ship_variables_ship(ship_info *sip)
{
	unsigned int i=0;
	svt[i++]->SetText(sip->name);
	svt[i]->SetText(sip->species);
	svt[i++]->SetSaveLoc(&sip->species, T_ST_ONENTER, True_NumSpecies-1, 0);

	SVW_SET_SI_VAR(density);
	SVW_SET_SI_VAR(damp);
	SVW_SET_SI_VAR(rotdamp);
	SVW_SET_SI_VAR(max_vel.xyz.x);
	SVW_SET_SI_VAR(max_vel.xyz.y);
	SVW_SET_SI_VAR(max_vel.xyz.z);

	SVW_SET_SI_VAR(max_shield_strength);
	SVW_SET_SI_VAR(max_hull_strength);
	SVW_SET_SI_VAR(subsys_repair_rate_percent);
	SVW_SET_SI_VAR(hull_repair_rate_percent);
	SVW_SET_SI_VAR(cmeasure_max);
	svt[i]->SetText(sip->shield_icon_index);
	svt[i++]->SetSaveLoc(&sip->shield_icon_index, T_ST_ONENTER, Hud_shield_filename_count-1, 0);

	SVW_SET_SI_VAR(power_output);
	SVW_SET_SI_VAR(max_overclocked_speed);
	SVW_SET_SI_VAR(max_weapon_reserve);

	SVW_SET_SI_VAR(afterburner_fuel_capacity);
	SVW_SET_SI_VAR(afterburner_burn_rate);
	SVW_SET_SI_VAR(afterburner_recover_rate);

	SVW_SET_SI_VAR(inner_rad);
	SVW_SET_SI_VAR(outer_rad);
	SVW_SET_SI_VAR(damage);
	SVW_SET_SI_VAR(blast);
	SVW_SET_SI_VAR(explosion_propagates);
	SVW_SET_SI_VAR(shockwave_speed);
	SVW_SET_SI_VAR(shockwave_count);

	SVW_SET_SI_VAR(closeup_zoom);
	SVW_SET_SI_VAR(closeup_pos.xyz.x);
	SVW_SET_SI_VAR(closeup_pos.xyz.y);
	SVW_SET_SI_VAR(closeup_pos.xyz.z);

	//Test
	/*
	SVW_SET_VAR(Objects[Ships[0].objnum].pos.xyz.x);
	SVW_SET_VAR(Objects[Ships[0].objnum].pos.xyz.y);
	SVW_SET_VAR(Objects[Ships[0].objnum].pos.xyz.z);

	SVW_SET_VAR(Objects[Ships[0].objnum].phys_info.vel.xyz.x);
	SVW_SET_VAR(Objects[Ships[0].objnum].phys_info.vel.xyz.y);
	SVW_SET_VAR(Objects[Ships[0].objnum].phys_info.vel.xyz.z);*/
}

#define SVW_LEFTWIDTH		150
#define SVW_RIGHTWIDTH		100
#define SVW_RIGHTX			160

void zero_ship_var_win(GUIObject *caller)
{
	ShipVarWin = NULL;
}
#define SVW_ADD_TEXT_HEADER(name) y += (cwp->AddChild(new Text(name, name, SVW_RIGHTX/2, y + 10, SVW_RIGHTWIDTH))->GetHeight() + 15)

#define SVW_ADD_TEXT(name) cwp->AddChild(new Text(name, name, 0, y, SVW_LEFTWIDTH));			\
	svt[i] = (Text*) cwp->AddChild(new Text(std::string (name) + std::string("Editbox"), "", SVW_RIGHTX, y, SVW_RIGHTWIDTH, 12, T_EDITTABLE));	\
	y += svt[i++]->GetHeight() + 5														\

void ship_variables_window(Button *caller)
{
	if(ShipVarWin != NULL)
		return;

	GUIObject* cwp = Lab_screen->Add(new Window("Ship variables", gr_screen.max_w - (SVW_RIGHTX + SVW_RIGHTWIDTH + 25), 200));
	ShipVarWin = (Window*)cwp;
	unsigned int i = 0;
	int y = 0;

	cwp->AddChild(new Text("Name", "", y, SVW_LEFTWIDTH));
	svt[i] = (Text*) cwp->AddChild(new Text("Ship name", "<None>", SVW_RIGHTX, y, SVW_RIGHTWIDTH, 12));
	y += svt[i++]->GetHeight() + 5;
	SVW_ADD_TEXT("Species");

	//Physics
	SVW_ADD_TEXT_HEADER("Physics");
	SVW_ADD_TEXT("Density");
	SVW_ADD_TEXT("Damp");
	SVW_ADD_TEXT("Rotdamp");
	SVW_ADD_TEXT("Max vel (x)");
	SVW_ADD_TEXT("Max vel (y)");
	SVW_ADD_TEXT("Max vel (z)");

	//Other
	SVW_ADD_TEXT_HEADER("Stats");
	SVW_ADD_TEXT("Shields");
	SVW_ADD_TEXT("Hull");
	SVW_ADD_TEXT("Subsys repair rate");
	SVW_ADD_TEXT("Hull repair rate");
	SVW_ADD_TEXT("Countermeasures");
	SVW_ADD_TEXT("HUD Icon");

	SVW_ADD_TEXT_HEADER("Power");
	SVW_ADD_TEXT("Power output");
	SVW_ADD_TEXT("Max oclk speed");
	SVW_ADD_TEXT("Max weapon reserve");

	SVW_ADD_TEXT_HEADER("Afterburner");
	SVW_ADD_TEXT("Fuel");
	SVW_ADD_TEXT("Burn rate");
	SVW_ADD_TEXT("Recharge rate");

	SVW_ADD_TEXT_HEADER("Explosion");
	SVW_ADD_TEXT("Inner radius");
	SVW_ADD_TEXT("Outer radius");
	SVW_ADD_TEXT("Damage");
	SVW_ADD_TEXT("Blast");
	SVW_ADD_TEXT("Propagates");
	SVW_ADD_TEXT("Shockwave speed");
	SVW_ADD_TEXT("Shockwave count");

	//Techroom
	SVW_ADD_TEXT_HEADER("Techroom");
	SVW_ADD_TEXT("Closeup zoom");
	SVW_ADD_TEXT("Closeup pos (x)");
	SVW_ADD_TEXT("Closeup pos (y)");
	SVW_ADD_TEXT("Closeup pos (z)");

	//Test
	/*
	SVW_ADD_TEXT_HEADER(Ships[0].ship_name);
	SVW_ADD_TEXT("Mission pos (x)");
	SVW_ADD_TEXT("Mission pos (y)");
	SVW_ADD_TEXT("Mission pos (z)");
	SVW_ADD_TEXT("Mission vel (x)");
	SVW_ADD_TEXT("Mission vel (y)");
	SVW_ADD_TEXT("Mission vel (z)");
	*/

	if(ShipSelectShipIndex != -1)
		set_ship_variables_ship(&Ship_info[ShipSelectShipIndex]);

	ShipVarWin->SetCloseFunction(zero_ship_var_win);
}


//*****************************Options Window*******************************
void make_options_window(Button *caller)
{
	GUIObject* ccp;
	GUIObject* cwp = Lab_screen->Add(new Window("Options", gr_screen.max_w - 300, 200));
	int y = 0;
	ccp = cwp->AddChild(new Checkbox("No lighting", 0,  y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_NO_LIGHTING);

	/*y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("No texturing", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_NO_TEXTURING);*/

	y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("No smoothing", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_NO_SMOOTHING);

	y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("No Z-buffer", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_NO_ZBUFFER);

	y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("No culling", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_NO_CULL);

	y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("No Fogging", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_NO_FOGGING);

	y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("Wireframe", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_SHOW_OUTLINE | MR_NO_POLYS);

	y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("Transparent", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_ALL_XPARENT);

	y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("Away norms transparent", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_EDGE_ALPHA);

	y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("Toward norms transparent", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_CENTER_ALPHA);

	y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("Show pivots", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_SHOW_PIVOTS);

	y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("Show paths", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_SHOW_PATHS);

	y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("Show bay paths", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_BAY_PATHS);

	y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("Show radius", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_SHOW_RADIUS);

	/*y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("Show damage", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_SHOW_DAMAGE);*/

	y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("Show shields", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_SHOW_SHIELDS);

	/*y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("Show thrusters", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_SHOW_THRUSTERS);*/

	y += ccp->GetHeight() + 10;
	ccp = cwp->AddChild(new Checkbox("Show invisible faces", 0, y));
	((Checkbox*)ccp)->SetFlag(&ModelFlags, MR_SHOW_INVISIBLE_FACES);
}

//*****************************Shiplist Window*******************************
void make_new_window(Button* caller)
{
	static int total = 0;
	GUIObject* cgp;
	char buf[8];
	itoa(total, buf, 10);

	std::string caption = "Ship list ";
	caption += buf;
	cgp = Lab_screen->Add(new Window(caption, 50 + total*15, 50 + total*15));
	total++;

	Tree* cmp = (Tree*)cgp->AddChild(new Tree("Ship tree", 0, 0));
	TreeItem *ctip;
	int j;
	for(int i = 0; i < Num_ship_types; i++)
	{
		ctip = cmp->AddItem(NULL, Ship_info[i].name, (void*)i, false);
		for(j = 0; j < Ship_info[i].num_detail_levels; j++)
		{
			itoa(j, buf, 10);
			caption = "LOD ";
			caption += buf;

			cmp->AddItem(ctip, caption, (void*)j, false, change_lod);
		}
	}
}

//*****************************Description Window*******************************
static Window* DescWin = NULL;
static Text* DescText = NULL;

void zero_descwin(GUIObject* caller)
{
	DescWin = NULL;
	DescText = NULL;
}

void make_another_window(Button* caller)
{
	if(DescWin != NULL)
		return;

	DescWin = (Window*)Lab_screen->Add(new Window("Description", gr_screen.max_w - gr_screen.max_w/3 - 50, gr_screen.max_h - gr_screen.max_h/6 - 50, gr_screen.max_w/3, gr_screen.max_h/6));
	DescText = (Text*)DescWin->AddChild(new Text("Description Text", "No ship selected.", 0, 0));
	if(ShipSelectShipIndex != -1)
	{
		DescText->SetText(Ship_info[ShipSelectShipIndex].tech_desc);
		DescWin->SetCaption(Ship_info[ShipSelectShipIndex].name);
		//DescText->SetSaveStringPtr(&Ship_info[ShipSelectShipIndex].tech_desc, T_ST_ONENTER, T_ST_MALLOC);
	}
		
	DescText->SetCloseFunction(zero_descwin);
}

//*****************************Ship display stuff*******************************

void change_lod(Tree* caller)
{
	if(ShipSelectModelNum != -1)
	{
		model_page_out_textures(ShipSelectModelNum);
		//model_unload(ShipSelectModelNum);
	}
	ShipSelectShipIndex = (int)(caller->GetSelectedItem()->Parent->Data);
	ModelLOD = (int)(caller->GetSelectedItem()->Data);
	ShipSelectModelNum = model_load(Ship_info[ShipSelectShipIndex].pof_file, 0, NULL);

	if(DescText != NULL && DescWin != NULL)
	{
		DescWin->SetCaption(Ship_info[ShipSelectShipIndex].name);
		DescText->SetText(Ship_info[ShipSelectShipIndex].tech_desc);
		//DescText->SetSaveStringPtr(&Ship_info[ShipSelectShipIndex].tech_desc, T_ST_ONENTER, T_ST_MALLOC);
	}

	if(ShipOptWin != NULL)
	{
		set_ship_options_ship(&Ship_info[ShipSelectShipIndex]);
	}

	if(ShipVarWin != NULL)
	{
		set_ship_variables_ship(&Ship_info[ShipSelectShipIndex]);
	}
}

extern float View_zoom;
static int Trackball_mode = 1;
static int Trackball_active = 0;
void show_ship(float frametime)
{
	static float ShipSelectScreenShipRot = 0.0f;
	static matrix ShipScreenOrient = IDENTITY_MATRIX;

	if(ShipSelectModelNum == -1)
		return;

	float rev_rate;
	angles rot_angles, view_angles;
	int z;
	ship_info *sip = &Ship_info[ShipSelectShipIndex];

	// get correct revolution rate

	rev_rate = REVOLUTION_RATE;
	z = sip->flags;
	if (z & SIF_BIG_SHIP) {
		rev_rate *= 1.7f;
	}
	if (z & SIF_HUGE_SHIP) {
		rev_rate *= 3.0f;
	}

	// rotate the ship as much as required for this frame
	if (Trackball_active) {
		int dx, dy;
		matrix mat1, mat2;

			mouse_get_delta(&dx, &dy);
			if (dx || dy) {
				vm_trackball(-dx, -dy, &mat1);
				vm_matrix_x_matrix(&mat2, &mat1, &ShipScreenOrient);
				ShipScreenOrient = mat2;
			}
	}
	else
	{
		ShipSelectScreenShipRot += PI2 * frametime / rev_rate;
		while (ShipSelectScreenShipRot > PI2){
			ShipSelectScreenShipRot -= PI2;	
		}

		// setup stuff needed to render the ship
		view_angles.p = -0.6f;
		view_angles.b = 0.0f;
		view_angles.h = 0.0f;
		vm_angles_2_matrix(&ShipScreenOrient, &view_angles);

		rot_angles.p = 0.0f;
		rot_angles.b = 0.0f;
		rot_angles.h = ShipSelectScreenShipRot;
		vm_rotate_matrix_by_angles(&ShipScreenOrient, &rot_angles);
	}

//	gr_set_clip(Tech_ship_display_coords[gr_screen.res][0], Tech_ship_display_coords[gr_screen.res][1], Tech_ship_display_coords[gr_screen.res][2], Tech_ship_display_coords[gr_screen.res][3]);	
	//gr_set_clip(Ship_anim_coords[gr_screen.res][0], Ship_anim_coords[gr_screen.res][1], Tech_ship_display_coords[gr_screen.res][2], Tech_ship_display_coords[gr_screen.res][3]);		

	// render the ship
	g3_start_frame(1);
	g3_set_view_matrix(&sip->closeup_pos, &vmd_identity_matrix, sip->closeup_zoom * 1.3f);
	if (!Cmdline_nohtl) gr_set_proj_matrix( (4.0f/9.0f) * 3.14159f * View_zoom, gr_screen.aspect*(float)gr_screen.clip_width/(float)gr_screen.clip_height, Min_draw_distance, Max_draw_distance);
	if (!Cmdline_nohtl)	gr_set_view_matrix(&Eye_position, &Eye_matrix);

	// lighting for techroom
	light_reset();
	vec3d light_dir = vmd_zero_vector;
	light_dir.xyz.y = 1.0f;	
	light_add_directional(&light_dir, 0.65f, 1.0f, 1.0f, 1.0f);
	// light_filter_reset();
	light_rotate_all();
	// lighting for techroom

	model_set_outline_color(255, 255, 255);
	model_clear_instance(ShipSelectModelNum);
	model_set_detail_level(ModelLOD);
	model_render(ShipSelectModelNum, &ShipScreenOrient, &vmd_zero_vector, ModelFlags);

	if (!Cmdline_nohtl) 
	{
		gr_end_view_matrix();
		gr_end_proj_matrix();
	}

	g3_end_frame();
	//gr_reset_clip();
}

void get_out_of_lab(Button* caller)
{
	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
}

//***************************************
static bool Lab_in_mission;

void lab_init()
{
	if(gameseq_get_pushed_state() == GS_STATE_GAME_PLAY)
		Lab_in_mission = true;
	else
		Lab_in_mission = false;

	beam_pause_sounds();

	//If we were viewing a ship, load 'er up
	if(ShipSelectShipIndex != -1)
	{
		ShipSelectModelNum = model_load(Ship_info[ShipSelectShipIndex].pof_file, 0, NULL);
	}

	//If you want things to stay as you left them, uncomment this and "delete Lab_screen"
	//of course, you still need to delete it somewhere else (ie when Freespace closes)
	
	if(Lab_screen != NULL)
	{
		GUI_system->PushScreen(Lab_screen);
		return;
	}

	//We start by creating the screen/toolbar
	Lab_screen = GUI_system->PushScreen(new GUIScreen("Lab"));
	GUIObject *cwp;
	GUIObject *cbp;
	cwp = Lab_screen->Add(new Window("Toolbar", 0, 0, -1, -1, WS_NOTITLEBAR | WS_NONMOVEABLE));
//	cwp->AddChild(new Button("File", 0, 0, file_menu));
	int x = 0;
	cbp = cwp->AddChild(new Button("Ship classes", 0, 0, make_new_window));
	x += cbp->GetWidth() + 10;
	cbp = cwp->AddChild(new Button("Class Description", x, 0, make_another_window));
	x += cbp->GetWidth() + 10;
	if(!Lab_in_mission)
	{
		cbp = cwp->AddChild(new Button("Render options", x, 0, make_options_window));
		x += cbp->GetWidth() + 10;
	}
	cbp = cwp->AddChild(new Button("Class options", x, 0, ship_options_window));
	x += cbp->GetWidth() + 10;
	cbp = cwp->AddChild(new Button("Class variables", x, 0, ship_variables_window));
	x += cbp->GetWidth() + 10;
	cbp = cwp->AddChild(new Button("Exit", x, 0, get_out_of_lab));
}

extern void game_render_frame_setup(vec3d *eye_pos, matrix *eye_orient);
extern void game_render_frame(vec3d *eye_pos, matrix *eye_orient);
void lab_do_frame(float frametime)
{
	gr_clear();
	if(Lab_in_mission)
	{
		vec3d eye_pos;
		matrix eye_orient;
		game_render_frame_setup(&eye_pos, &eye_orient);
		game_render_frame( &eye_pos, &eye_orient );
	}
	else
	{
		show_ship(frametime);
	}
	if(GUI_system->OnFrame(frametime, Trackball_active==0 ? true : false, false) == GSOF_NOTHINGPRESSED && mouse_down(MOUSE_LEFT_BUTTON))
	{
		Trackball_active = 1;
		Trackball_mode = 1;
	}
	else
	{
		Trackball_active = 0;
	}
	gr_flip();
}

void lab_close()
{
	delete Lab_screen;

	if(ShipSelectModelNum != -1)
	{
		model_page_out_textures(ShipSelectModelNum);
		//model_unload(ShipSelectModelNum);
	}
	ShipSelectModelNum = -1;
	beam_unpause_sounds();
	//audiostream_unpause_all();
	game_flush();
}

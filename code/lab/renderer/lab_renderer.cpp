#include "lab/renderer/lab_renderer.h"
#include "lab/labv2_internal.h"
#include "lab/wmcgui.h"
#include "graphics/2d.h"
#include "graphics/light.h"
#include "lighting/lighting_profiles.h"
#include "starfield/starfield.h"
#include "starfield/nebula.h"
#include "nebula/neb.h"
#include "freespace.h"
#include "tracing/tracing.h"

void LabRenderer::onFrame(float frametime) {
	GR_DEBUG_SCOPE("Lab Frame");

	gr_reset_clip();
	gr_clear();

	if (!renderFlags[LabRenderFlag::TimeStopped])
		Missiontime += Frametime;
	
	// render our particular thing
	if (getLabManager()->CurrentObject >= 0) {
		int w, h;
		renderModel(frametime);

		// print out the current pof filename, to help with... something
		if (strlen(getLabManager()->ModelFilename.c_str())) {
			gr_get_string_size(&w, &h, getLabManager()->ModelFilename.c_str());
			gr_set_color_fast(&Color_white);
			gr_string(gr_screen.center_offset_x + gr_screen.center_w - w,
				gr_screen.center_offset_y + gr_screen.center_h - h, getLabManager()->ModelFilename.c_str(), GR_RESIZE_NONE);
		}
	}

	renderHud(frametime);

	// Normally, we would call gr_flip here, but because wmcgui conflates rendering and input gathering, this is done at the end of 
	// the LabManager::onFrame method
}

void LabRenderer::renderModel(float frametime) {
	GR_DEBUG_SCOPE("Lab Render Model");

	auto lab_debris_override_save = Motion_debris_enabled;
	auto lab_envmap_override_save = Envmap_override;
	auto lab_emissive_light_save = Cmdline_emissive;

	light_reset();

	Cmdline_emissive = renderFlags[LabRenderFlag::ShowEmissiveLighting];

	object* obj = &Objects[getLabManager()->CurrentObject];

	obj->pos = getLabManager()->CurrentPosition;
	obj->orient = getLabManager()->CurrentOrientation;

	Envmap_override = renderFlags[LabRenderFlag::NoEnvMap];
	Glowpoint_override = renderFlags[LabRenderFlag::NoGlowpoints];
	PostProcessing_override = renderFlags[LabRenderFlag::HidePostProcessing];

	if (obj->type == OBJ_SHIP) {
		Ships[obj->instance].flags.set(Ship::Ship_Flags::Subsystem_movement_locked, !renderFlags[LabRenderFlag::MoveSubsystems]);
		Ships[obj->instance].flags.set(Ship::Ship_Flags::Draw_as_wireframe, renderFlags[LabRenderFlag::ShowWireframe]);
		Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_full_detail, renderFlags[LabRenderFlag::ShowFullDetail]);
		Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_light,
			renderFlags[LabRenderFlag::NoLighting] || currentMissionBackground == LAB_MISSION_NONE_STRING);
		Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_diffuse, renderFlags[LabRenderFlag::NoDiffuseMap]);
		Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_glowmap, renderFlags[LabRenderFlag::NoGlowMap]);
		Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_normalmap, renderFlags[LabRenderFlag::NoNormalMap]);
		Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_specmap, renderFlags[LabRenderFlag::NoSpecularMap]);
		Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_reflectmap, renderFlags[LabRenderFlag::NoReflectMap]);
		Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_heightmap, renderFlags[LabRenderFlag::NoHeightMap]);
		Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_miscmap, renderFlags[LabRenderFlag::NoMiscMap]);
		Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_weapons, !renderFlags[LabRenderFlag::ShowWeapons]);
		Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_ambientmap, renderFlags[LabRenderFlag::NoAOMap]);

		Ships[obj->instance].team_name = currentTeamColor;

		if (renderFlags[LabRenderFlag::ShowDamageLightning]) {
			obj->hull_strength = 1.0f;
		}
		else {
			obj->hull_strength = Ship_info[Ships[obj->instance].ship_info_index].max_hull_strength;
		}
	}

	if (renderFlags[LabRenderFlag::ShowWireframe])
		model_render_set_wireframe_color(&Color_white);

	if (renderFlags[LabRenderFlag::ShowThrusters] || renderFlags[LabRenderFlag::ShowAfterburners]) {
		obj->phys_info.linear_thrust.xyz.z = 1.0f;
		if (obj->type == OBJ_SHIP) {
			Ships[obj->instance].flags.remove(Ship::Ship_Flags::No_thrusters);
		}
		if (renderFlags[LabRenderFlag::ShowAfterburners]) {
			obj->phys_info.flags |= PF_AFTERBURNER_ON;
			// Keep the AB topped off
			Ships[obj->instance].afterburner_fuel = Ship_info[Ships[obj->instance].ship_info_index].afterburner_fuel_capacity;
		}
		else
			obj->phys_info.flags &= ~PF_AFTERBURNER_ON;
	}
	else {
		obj->phys_info.linear_thrust.xyz.z = 0.0f;

		if (obj->type == OBJ_SHIP)
			Ships[obj->instance].flags.set(Ship::Ship_Flags::No_thrusters);
	}

	obj_move_all(frametime);

	particle::move_all(frametime);
	particle::ParticleManager::get()->doFrame(frametime);
	shockwave_move_all(frametime);

	Trail_render_override = true;
	game_render_frame(labCamera->FS_camera);
	Trail_render_override = false;

	Motion_debris_enabled = lab_debris_override_save;
	Envmap_override = lab_envmap_override_save;
	Cmdline_emissive = lab_emissive_light_save;

	gr_reset_clip();
	gr_set_color_fast(&HUD_color_debug);
	if (Cmdline_frame_profile) {
		tracing::frame_profile_process_frame();
		gr_string(gr_screen.center_offset_x + 20, gr_screen.center_offset_y + 100 + gr_get_font_height() + 1,
			tracing::get_frame_profile_output().c_str(), GR_RESIZE_NONE);
	}
}

SCP_string get_rot_mode_string(LabRotationMode rotmode)
{
	switch (rotmode) {
	case LabRotationMode::Both:
		return "Manual rotation mode: Pitch and Yaw";
	case LabRotationMode::Pitch:
		return "Manual rotation mode: Pitch";
	case LabRotationMode::Yaw:
		return "Manual rotation mode: Yaw";
	case LabRotationMode::Roll:
		return "Manual rotation mode: Roll";
	default:
		return "HOW DID THIS HAPPEN? Ask a coder!";
	}
}

SCP_string get_rot_speed_string(float speed_divisor)
{
	auto exp = std::lroundf(log10f(speed_divisor));

	switch (exp) {
	case 2:
		return "Fast";
	case 3:
		return "Slow";
	case 4:
		return "Slowest";
	default:
		return "HOW DID THIS HAPPEN? Ask a coder!";
	}
}

void LabRenderer::renderHud(float) {
	GR_DEBUG_SCOPE("Lab Render HUD");

	// print FPS at bottom left, might be helpful
	extern void game_get_framerate();
	extern float frametotal;
	extern float Framerate;

	game_get_framerate();

	gr_set_color_fast(&Color_white);

	if (frametotal != 0.0f) {
		gr_printf_no_resize(gr_screen.center_offset_x + 2,
			gr_screen.center_offset_y + gr_screen.center_h - gr_get_font_height(),
			"FPS: %3i %s", (int)std::lround(Framerate), labCamera->getOnFrameInfo().c_str());
	}
	else {
		gr_string(gr_screen.center_offset_x + 10, gr_screen.center_offset_y + gr_screen.center_h - gr_get_font_height(),
			"FPS: ?", GR_RESIZE_NONE);
	}

	//Print FXAA preset
	if ((gr_is_fxaa_mode(Gr_aa_mode) || gr_is_smaa_mode(Gr_aa_mode)) && !PostProcessing_override) {
		const char* aa_mode;
		switch (Gr_aa_mode) {
		case AntiAliasMode::FXAA_Low:
			aa_mode = "FXAA Low";
			break;
		case AntiAliasMode::FXAA_Medium:
			aa_mode = "FXAA Medium";
			break;
		case AntiAliasMode::FXAA_High:
			aa_mode = "FXAA High";
			break;
		case AntiAliasMode::SMAA_Low:
			aa_mode = "SMAA Low";
			break;
		case AntiAliasMode::SMAA_Medium:
			aa_mode = "SMAA Medium";
			break;
		case AntiAliasMode::SMAA_High:
			aa_mode = "SMAA High";
			break;
		case AntiAliasMode::SMAA_Ultra:
			aa_mode = "SMAA Ultra";
			break;
		default:
			aa_mode = "None";
			break;
		}

		gr_printf_no_resize(gr_screen.center_offset_x + 2, gr_screen.center_offset_y + gr_screen.center_h - (gr_get_font_height() * 2) - 3, "AA Preset: %s", aa_mode);
	}

	//Print current Team Color setting, if any
	if (currentTeamColor != LAB_TEAM_COLOR_NONE) {
		gr_printf_no_resize(gr_screen.center_offset_x + 2,
			gr_screen.center_offset_y + gr_screen.center_h - (gr_get_font_height() * 3) - 3,
			"Use T and Y to cycle through available Team Color settings. Current: %s",
			currentTeamColor.c_str());
	}

	// Camera usage info
	gr_printf_no_resize(gr_screen.center_offset_x + 2,
		gr_screen.center_offset_y + gr_screen.center_h - (gr_get_font_height() * 4) - 3,
		"%s Use number keys to switch between AA presets. R to cycle model rotation "
		"modes, S to cycle model rotation speeds, V to reset view, "
		"M to export environment map.", labCamera->getUsageInfo().c_str());

	// Rotation mode
	SCP_string text = get_rot_mode_string(getLabManager()->RotationMode);
	gr_printf_no_resize(gr_screen.center_offset_x + 2,
		gr_screen.center_offset_y + gr_screen.center_h - (gr_get_font_height() * 5) - 3,
		"%s Rotation speed: %s", get_rot_mode_string(getLabManager()->RotationMode).c_str(),
		get_rot_speed_string(getLabManager()->RotationSpeedDivisor).c_str());
}

void LabRenderer::useBackground(const SCP_string& mission_name) {
	matrix skybox_orientation;
	char skybox_model[MAX_FILENAME_LEN];
	int skybox_flags;

	int ambient_light_level;
	extern const char* Neb2_filenames[];

	char envmap_name[MAX_FILENAME_LEN] = {0};

	currentMissionBackground = mission_name;

	stars_pre_level_init(true);
	vm_set_identity(&skybox_orientation);

	// (DahBlount) - Remember to load the debris anims
	stars_load_debris(false);

	if (mission_name != "None") {
		read_file_text((mission_name + ".fs2").c_str(), CF_TYPE_MISSIONS);
		reset_parse();

		flagset<Mission::Mission_Flags> flags;
		skip_to_start_of_string("+Flags");
		if (optional_string("+Flags:"))
			stuff_flagset(&flags);

		// Are we using a skybox?
		skip_to_start_of_string_either("$Skybox Model:", "#Background bitmaps");

		strcpy_s(skybox_model, "");
		if (optional_string("$Skybox Model:")) {
			stuff_string(skybox_model, F_NAME, MAX_FILENAME_LEN);

			if (optional_string("+Skybox Orientation:")) {
				stuff_matrix(&skybox_orientation);
			}

			if (optional_string("+Skybox Flags:")) {
				skybox_flags = 0;
				stuff_int(&skybox_flags);
			}
			else {
				skybox_flags = DEFAULT_NMODEL_FLAGS;
			}

			stars_set_background_model(skybox_model, nullptr, skybox_flags);
			stars_set_background_orientation(&skybox_orientation);

			skip_to_start_of_string("#Background bitmaps");
		}

		if (optional_string("#Background bitmaps")) {
			required_string("$Num stars:");
			stuff_int(&Num_stars);
			if (Num_stars >= MAX_STARS)
				Num_stars = MAX_STARS;

			required_string("$Ambient light level:");
			stuff_int(&ambient_light_level);

			if (ambient_light_level == 0) {
				ambient_light_level = DEFAULT_AMBIENT_LIGHT_LEVEL;
			}

			gr_set_ambient_light(ambient_light_level & 0xff, (ambient_light_level >> 8) & 0xff,
				(ambient_light_level >> 16) & 0xff);

			strcpy_s(Neb2_texture_name, "");
			Neb2_poof_flags = ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));
			bool nebula = false;
			if (optional_string("+Neb2:")) {
				nebula = true;
				stuff_string(Neb2_texture_name, F_NAME, MAX_FILENAME_LEN);
			}
			if (optional_string("+Neb2Color:")) {
				nebula = true;
				int neb_colors[3];
				stuff_int_list(neb_colors, 3, RAW_INTEGER_TYPE);
				Neb2_fog_color[0] = (ubyte)neb_colors[0];
				Neb2_fog_color[1] = (ubyte)neb_colors[1];
				Neb2_fog_color[2] = (ubyte)neb_colors[2];
				flags |= Mission::Mission_Flags::Neb2_fog_color_override;
			}

			if (nebula){
				required_string("+Neb2Flags:");
				stuff_int(&Neb2_poof_flags);

				if (flags[Mission::Mission_Flags::Fullneb]) {
					neb2_post_level_init(flags[Mission::Mission_Flags::Neb2_fog_color_override]);
				}
			}

			if (flags[Mission::Mission_Flags::Fullneb]) {
				// no regular nebula stuff
				nebula_close();
			}
			else {
				Nebula_index = -1;
				if (optional_string("+Nebula:")) {
					char str[MAX_FILENAME_LEN];
					int z;
					stuff_string(str, F_NAME, MAX_FILENAME_LEN);

					// parse the proper nebula type (full or not)
					for (z = 0; z < NUM_NEBULAS; z++) {
						if (flags[Mission::Mission_Flags::Fullneb]) {
							if (!stricmp(str, Neb2_filenames[z])) {
								Nebula_index = z;
								break;
							}
						}
						else {
							if (!stricmp(str, Nebula_filenames[z])) {
								Nebula_index = z;
								break;
							}
						}
					}

					if (z == NUM_NEBULAS)
						WarningEx(LOCATION, "Unknown nebula %s!", str);

					if (optional_string("+Color:")) {
						stuff_string(str, F_NAME, MAX_FILENAME_LEN);
						for (z = 0; z < NUM_NEBULA_COLORS; z++) {
							if (!stricmp(str, Nebula_colors[z])) {
								Mission_palette = z;
								break;
							}
						}
					}

					if (z == NUM_NEBULA_COLORS)
						WarningEx(LOCATION, "Unknown nebula color %s!", str);

					if (optional_string("+Pitch:")) {
						stuff_int(&Nebula_pitch);
					}
					else {
						Nebula_pitch = 0;
					}

					if (optional_string("+Bank:")) {
						stuff_int(&Nebula_bank);
					}
					else {
						Nebula_bank = 0;
					}

					if (optional_string("+Heading:")) {
						stuff_int(&Nebula_heading);
					}
					else {
						Nebula_heading = 0;
					}
				}

				if (Nebula_index >= 0) {
					nebula_init(Nebula_filenames[Nebula_index], Nebula_pitch, Nebula_bank, Nebula_heading);
				}
				else {
					nebula_close();
				}
			}

			stars_load_debris(flags[Mission::Mission_Flags::Fullneb]);


			extern void parse_one_background(background_t * background);
			while (optional_string("$Bitmap List:") || check_for_string("$Sun:") || check_for_string("$Starbitmap:")) {
				stars_add_blank_background(false);
				parse_one_background(&Backgrounds.back());
			}

			stars_load_first_valid_background();

			if (optional_string("$Environment Map:")) {
				stuff_string(envmap_name, F_NAME, MAX_FILENAME_LEN);
			}

			const int size = 512;
			int gen_flags = (BMP_FLAG_RENDER_TARGET_STATIC | BMP_FLAG_CUBEMAP | BMP_FLAG_RENDER_TARGET_MIPMAP);

			if (!Cmdline_env) {
				return;
			}

			if (gr_screen.envmap_render_target >= 0) {
				if (!bm_release(gr_screen.envmap_render_target, 1)) {
					Warning(LOCATION, "Unable to release environment map render target.");
				}

				gr_screen.envmap_render_target = -1;
			}

			if (strlen(envmap_name)) {
				// Load the mission map so we can use it later
				ENVMAP = bm_load(The_mission.envmap_name);
				// Load may fail, if so, don't exit early. Proceed to make render target.
				if (ENVMAP > 1) {
					return;
				}
			}

			gr_screen.envmap_render_target = bm_make_render_target(size, size, gen_flags);
		}
	}
	else {
		// (DahBlount) - This spot should be used to disable rendering features that only apply to missions.
		Motion_debris_override = true;
		Num_stars = 0;
	}
}

std::unique_ptr<LabCamera> &LabRenderer::getCurrentCamera() {
	return labCamera;
}

void LabRenderer::setCurrentCamera(std::unique_ptr<LabCamera> &newcam) {
	labCamera = std::move(newcam);
}
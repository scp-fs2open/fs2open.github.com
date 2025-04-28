#include "lab/labv2_internal.h"
#include "lab/manager/lab_manager.h"
#include "lab/renderer/lab_renderer.h"
#include "io/key.h"
#include "asteroid/asteroid.h"
#include "math/staticrand.h"
#include "missionui/missionscreencommon.h"
#include "debris/debris.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "particle/particle.h"
#include "weapon/muzzleflash.h"
#include "weapon/beam.h"

#include "freespace.h"

#include "extensions/ImGuizmo.h"
#include "io/mouse.h"
#include "weapon/weapon.h"

//Turret firing forward declarations
void ai_turret_execute_behavior(const ship* shipp, ship_subsys* ss);


void lab_exit() {
	getLabManager()->notify_close();
}

namespace ltp = lighting_profiles;
LabManager::LabManager() {
	The_mission.Reset();

	if (The_mission.ai_profile == nullptr)
		The_mission.ai_profile = &Ai_profiles[Default_ai_profile];

	Renderer.reset(new LabRenderer());
	labUi = LabUi();

	obj_init();

	fireball_init();
	debris_init();
	extern void debris_page_in();
	debris_page_in();
	asteroid_level_init();
	shockwave_level_init();
	ship_level_init();
	shipfx_flash_init();
	mflash_page_in(true);
	weapon_level_init();
	beam_level_init();
	particle::init();

	ai_paused = 1;

	// do some other setup
	// External weapon displays require a call to weapons_page_in, which in turn requires team data to be set
	Num_teams = 1;

	team_data* teamp = &Team_data[0];

	// In the lab, all ships are valid
	for (size_t i = 0; i < Ship_info.size(); ++i) {
		teamp->ship_list[i] = static_cast<int>(i);
		strcpy_s(teamp->ship_list_variables[i], "");
		teamp->ship_count[i] = 1;
		teamp->loadout_total += 1;
		strcpy_s(teamp->ship_count_variables[i], "");
	}
	teamp->default_ship = 0;
	teamp->num_ship_choices = static_cast<int>(Ship_info.size());

	// you want guns? you get guns.
	for (size_t i = 0; i < Weapon_info.size(); ++i) {
		teamp->weaponry_pool[i] = static_cast<int>(i);
		teamp->weaponry_count[i] = 640; // should be enough for everyone
		strcpy_s(teamp->weaponry_amount_variable[i], "");
		strcpy_s(teamp->weaponry_pool_variable[i], "");
	}
	teamp->num_weapon_choices = static_cast<int>(Weapon_info.size());

	Game_mode |= GM_LAB;

	using namespace ltp;
	graphicsSettings = gfx_options();
	graphicsSettings.ppcv = ltp::lab_get_ppc();
	graphicsSettings.ambient_factor = ltp::lab_get_ambient();
	graphicsSettings.light_factor = ltp::lab_get_light();
	graphicsSettings.emissive_factor = ltp::lab_get_emissive();
	graphicsSettings.exposure_level = ltp::current_exposure();
	graphicsSettings.tonemapper = ltp::current_tonemapper();
	graphicsSettings.bloom_level = gr_bloom_intensity();
	graphicsSettings.aa_mode = Gr_aa_mode;
}

LabManager::~LabManager()
{
	obj_delete_all();
}

void LabManager::resetGraphicsSettings() {
	Renderer->resetGraphicsSettings(graphicsSettings);
}

void LabManager::onFrame(float frametime) {
	if (gr_screen.mode == GR_OPENGL)
		ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(gr_screen.max_w, gr_screen.max_h);
	ImGui::NewFrame();

	Renderer->onFrame(frametime);

	labUi.create_ui();

	int key = game_check_key();

	int dx, dy;
	mouse_get_delta(&dx, &dy);
	Renderer->getCurrentCamera()->handleInput(dx, dy, mouse_down(MOUSE_LEFT_BUTTON) != 0, mouse_down(MOUSE_RIGHT_BUTTON) != 0, key_get_shift_status());

	if (!Renderer->getCurrentCamera()->handlesObjectPlacement()) {
		if (mouse_down(MOUSE_LEFT_BUTTON)) {
			angles rot_angle;
			vm_extract_angles_matrix_alternate(&rot_angle, &CurrentOrientation);

			if (RotationMode == LabRotationMode::Yaw) {
				rot_angle.h += dx / RotationSpeedDivisor;
			}
			if (RotationMode == LabRotationMode::Pitch) {
				rot_angle.p += dy / RotationSpeedDivisor;
			}
			if (RotationMode == LabRotationMode::Both) {
				rot_angle.h += dx / RotationSpeedDivisor;
				rot_angle.p += dy / RotationSpeedDivisor;
			}
			if (RotationMode == LabRotationMode::Roll) {
				rot_angle.b += dx / RotationSpeedDivisor;
			}

			if (rot_angle.h < -PI)
				rot_angle.h = PI - 0.001f;
			if (rot_angle.h > PI)
				rot_angle.h = -PI + 0.001f;

			CLAMP(rot_angle.p, -PI_2, PI_2);

			if (rot_angle.b < -PI)
				rot_angle.b = PI - 0.001f;
			if (rot_angle.b > PI)
				rot_angle.b = -PI + 0.001f;

			vm_angles_2_matrix(&CurrentOrientation, &rot_angle);
		}
	}

	if (key != 0) {
		// handle any key presses
		switch (key) {
			// Adjust AA presets

		case KEY_T:
			Renderer->useNextTeamColorPreset();
			break;

		case KEY_Y:
			Renderer->usePreviousTeamColorPreset();
			break;

		case KEY_V:
			Renderer->resetView();
			break;

		case KEY_UP:
			break;

		case KEY_DOWN:
			break;

		case KEY_R:
			switch (RotationMode) {
			case LabRotationMode::Both:
				RotationMode = LabRotationMode::Yaw;
				break;
			case LabRotationMode::Yaw:
				RotationMode = LabRotationMode::Pitch;
				break;
			case LabRotationMode::Pitch:
				RotationMode = LabRotationMode::Roll;
				break;
			case LabRotationMode::Roll:
				RotationMode = LabRotationMode::Both;
				break;
			}
			break;

		case KEY_S:
			RotationSpeedDivisor *= 10.f;
			if (RotationSpeedDivisor > 10000.f)
				RotationSpeedDivisor = 100.f;
			break;

		case KEY_M:
			gr_dump_envmap(Renderer->currentMissionBackground.c_str());
			break;

		case KEY_ESC:
			notify_close();
			break;

		default:
			// check for game-specific controls
			if (CurrentMode == LabMode::Ship) {
				if (check_control(PLUS_5_PERCENT_THROTTLE, key))
					Lab_thrust_len += 0.05f;
				else if (check_control(MINUS_5_PERCENT_THROTTLE, key))
					Lab_thrust_len -= 0.05f;

				CLAMP(Lab_thrust_len, 0.0f, 1.0f);

				if (check_control(AFTERBURNER, key))
					Lab_thrust_afterburn = !Lab_thrust_afterburn;
			}
			break;
		}
	}

	float rev_rate;
	ship_info* sip = nullptr;

	if (CurrentObject != -1 && (Objects[CurrentObject].type == OBJ_SHIP)) {
		sip = &Ship_info[Ships[Objects[CurrentObject].instance].ship_info_index];

		auto obj = &Objects[CurrentObject];
		bool weapons_firing = false;
		for (auto i = 0; i < Ships[obj->instance].weapons.num_primary_banks; ++i) {
			if (FirePrimaries & (1 << i)) {
				weapons_firing = true;
				Ships[obj->instance].weapons.current_primary_bank = i;

				ship_fire_primary(obj);

				Ships[obj->instance].weapon_energy = sip->max_weapon_reserve;
			}
		}

		Ships[obj->instance].flags.set(Ship::Ship_Flags::Trigger_down, weapons_firing);

		for (auto i = 0; i < Ships[obj->instance].weapons.num_secondary_banks; ++i) {
			if (FireSecondaries & (1 << i)) {
				Ships[obj->instance].weapons.current_secondary_bank = i;

				ship_fire_secondary(obj);
			}
		}

		ship_process_post(obj, frametime);
		ai_process_subobjects(CurrentObject); // So that animations get reset

		if (!getLabManager()->FireTurrets.empty()) {
			for (auto& [subsys, mode, fire] : getLabManager()->FireTurrets) {
				if (!fire || subsys == nullptr)
					continue;

				vec3d new_pos, new_vec;
				ship_get_global_turret_info(&Objects[subsys->parent_objnum], subsys->system_info, &new_pos, &new_vec);

				bool multipart = false;
				// Turret is multipart
				if (subsys->system_info->turret_gun_sobj >= 0 && subsys->system_info->subobj_num != subsys->system_info->turret_gun_sobj) {
					multipart = true;
				}

				switch (mode) {
					case LabTurretAimType::UVEC: {
						subsys->last_aim_enemy_pos = new_pos + new_vec * 500.0f;
						break;
					}
					case LabTurretAimType::INITIAL: {
						subsys->last_aim_enemy_pos = vmd_zero_vector;
						break;
					}
					case LabTurretAimType::RANDOM: {
						bool gen_new_vec = !multipart || subsys->points_to_target <= 0.010f;
						if (gen_new_vec && timestamp_elapsed(subsys->turret_next_fire_stamp)) {
							vec3d rand_vec;
							const int MAX_ATTEMPTS = 100;
							bool valid_vec_found = false;

							// You get 100 tries to find a set of random coords to fire at
							for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
								float full_fov_degrees = 2.0f * acosf(subsys->system_info->turret_fov) * (180.0f / PI);
								vm_vec_random_cone(&rand_vec, &new_vec, full_fov_degrees);

								vec3d target_point;
								vm_vec_scale_add(&target_point, &new_pos, &rand_vec, 1000.0f);

								//  Create a vector from the turret to the random point.
								vec3d turret_to_target;
								vm_vec_sub(&turret_to_target, &target_point, &new_pos);
								vm_vec_normalize(&turret_to_target);

								// Test if the generated vector is within the FOV
								if (turret_fov_test(subsys, &new_vec, &turret_to_target, 0.0f)) {
									valid_vec_found = true;
									rand_vec = target_point;
									break;
								}
							}

							if (valid_vec_found) {
								subsys->last_aim_enemy_pos = rand_vec;
							} else {
								subsys->last_aim_enemy_pos = new_pos + new_vec * 500.0f;
							}
						}
						break;
					}
					default:
						Assertion(false, "Invalid Lab Turret Aim Type!");
						break;
				}

				ai_turret_execute_behavior(&Ships[obj->instance], subsys);
			}
		}

	}

	// get correct revolution rate
	rev_rate = REVOLUTION_RATE;

	if (sip != nullptr) {
		if (sip->is_big_ship()) {
			rev_rate *= 1.7f;
		}
		else if (sip->is_huge_ship()) {
			rev_rate *= 3.0f;
		}
	}

	if (Flags[ManagerFlags::ModelRotationEnabled]) {
		angles rot_angles;

		rot_angles.p = 0.0f;
		rot_angles.b = 0.0f;
		rot_angles.h = PI2 * frametime / rev_rate;
		vm_rotate_matrix_by_angles(&CurrentOrientation, &rot_angles);
	}
	
	if (Cmdline_show_imgui_debug)
		ImGui::ShowDemoWindow();
	ImGui::Render();
	if (gr_screen.mode == GR_OPENGL)
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (CloseThis)
		close();

	gr_flip();
}

void LabManager::changeDisplayedObject(LabMode mode, int info_index) {
	if (mode == CurrentMode && info_index == CurrentClass)
		return;

	CurrentMode = mode;
	CurrentClass = info_index;

	if (CurrentObject != -1) {
		// Stop any firing turrets
		FireTurrets.clear();

		obj_delete_all();
		CurrentObject = -1;
	}

	switch (CurrentMode) {
	case LabMode::Ship:
		CurrentObject = ship_create(&CurrentOrientation, &CurrentPosition, CurrentClass);
		changeShipInternal();
		break;
	case LabMode::Weapon:
		CurrentObject = weapon_create(&CurrentPosition, &CurrentOrientation, CurrentClass, -1);
		if (Weapon_info[CurrentClass].model_num != -1) {
			ModelFilename = model_get(Weapon_info[CurrentClass].model_num)->filename;
		}
		break;
	default:
		UNREACHABLE("Unhandled lab mode %d", (int)mode);
		ModelFilename = "";
		break;
	}

	Assert(CurrentObject != -1);

	Renderer->getCurrentCamera()->displayedObjectChanged();
}

void LabManager::changeShipInternal() {
	auto ship_objp = &Ships[Objects[CurrentObject].instance];
	auto ship_infop = &Ship_info[ship_objp->ship_info_index];

	// This is normally set during mission parse
	ship_objp->special_exp_damage = -1;

	// If the ship class defines replacement textures, load them and apply them to the ship
	// load the texture
	auto replacements = ship_infop->replacement_textures;
	for (auto& tr : replacements) {
		if (!stricmp(tr.new_texture, "invisible"))
		{
			// invisible is a special case
			tr.new_texture_id = REPLACE_WITH_INVISIBLE;
		}
		else
		{
			// try to load texture or anim as normal
			tr.new_texture_id = bm_load_either(tr.new_texture);
		}
	}

	ship_objp->apply_replacement_textures(replacements);
	ship_page_in_textures(ship_objp->ship_info_index);

	if (!ship_infop->default_team_name.empty())
		Renderer->setTeamColor(ship_infop->default_team_name);

	ModelFilename = model_get(ship_infop->model_num)->filename;
}
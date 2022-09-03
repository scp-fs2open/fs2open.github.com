#include "lab/labv2_internal.h"
#include "lab/manager/lab_manager.h"
#include "lab/renderer/lab_renderer.h"
#include "lab/dialogs/ship_classes.h"
#include "lab/dialogs/weapon_classes.h"
#include "lab/dialogs/render_options.h"
#include "lab/dialogs/backgrounds.h"
#include "lab/dialogs/actions.h"
#include "io/key.h"
#include "asteroid/asteroid.h"
#include "missionui/missionscreencommon.h"
#include "debris/debris.h"
#include "ship/shipfx.h"
#include "weapon/muzzleflash.h"
#include "weapon/beam.h"


void lab_exit(Button* /*caller*/) {
	getLabManager()->notify_close();
}

LabManager::LabManager() {
	Screen.reset(GUI_system.PushScreen(new GUIScreen("Lab")));
	Toolbar = (Window*)Screen->Add(new Window("Toolbar", gr_screen.center_offset_x, gr_screen.center_offset_y,
		-1, -1, WS_NOTITLEBAR | WS_NONMOVEABLE));

	Renderer.reset(new LabRenderer());

	Dialogs.emplace_back(std::make_shared<ShipClasses>());
	Dialogs.emplace_back(std::make_shared<WeaponClasses>());
	Dialogs.emplace_back(std::make_shared<BackgroundDialog>());
	Dialogs.emplace_back(std::make_shared<Actions>());
	Dialogs.emplace_back(std::make_shared<RenderOptions>());

	int x = 0;
	for (auto &dialog : Dialogs) {
		auto *dgo = new DialogOpener(dialog, x, 0);
		dialog->setOpener(dgo);
		auto *cbp = Toolbar->AddChild(dgo);
		x += cbp->GetWidth() + 10;
	}

	Toolbar->AddChild(new Button("Exit", x, 0, lab_exit));

	if (The_mission.ai_profile == nullptr)
		The_mission.ai_profile = &Ai_profiles[Default_ai_profile];

	obj_init();

	fireball_init();
	debris_init();
	extern void debris_page_in();
	debris_page_in();
	asteroid_level_init();
	shockwave_level_init();
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
}

LabManager::~LabManager()
{
	for (auto &dialog : Dialogs) {
		dialog->close();
	}

	obj_delete_all();
	Toolbar = nullptr;
}

void LabManager::onFrame(float frametime) {
	
	Renderer->onFrame(frametime);

	if (Screen != nullptr) {
		bool keyPressed = (GUI_system.OnFrame(frametime, true, false) == GSOF_NOTHINGPRESSED);

		if (keyPressed) {
			int key = GUI_system.GetKeyPressed();
			//int status = GUI_system.GetStatus();

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

			// handle any key presses
			switch (key) {
				// Adjust AA presets
			case KEY_0:
				if (!PostProcessing_override)
					LabRenderer::setAAMode(AntiAliasMode::FXAA_Low);
				break;
			case KEY_1:
				if (!PostProcessing_override)
					LabRenderer::setAAMode(AntiAliasMode::FXAA_Medium);
				break;
			case KEY_2:
				if (!PostProcessing_override)
					LabRenderer::setAAMode(AntiAliasMode::FXAA_High);
				break;
			case KEY_3:
				if (!PostProcessing_override)
					LabRenderer::setAAMode(AntiAliasMode::SMAA_Low);
				break;
			case KEY_4:
				if (!PostProcessing_override)
					LabRenderer::setAAMode(AntiAliasMode::SMAA_Medium);
				break;
			case KEY_5:
				if (!PostProcessing_override)
					LabRenderer::setAAMode(AntiAliasMode::SMAA_High);
				break;
			case KEY_6:
				if (!PostProcessing_override)
					LabRenderer::setAAMode(AntiAliasMode::SMAA_Ultra);
				break;

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
				// Dumping the environment map only makes sense if we actually have a background set
				if (Renderer->currentMissionBackground != LAB_MISSION_NONE_STRING)
					gr_dump_envmap(Renderer->currentMissionBackground.c_str());
				break;

				// bail...
			case KEY_ESC:
				close();
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

		if (CloseThis)
			close();
	}

	gr_flip();
}

void LabManager::changeDisplayedObject(LabMode mode, int info_index) {
	if (mode == CurrentMode && info_index == CurrentClass)
		return;

	CurrentMode = mode;
	CurrentClass = info_index;

	if (CurrentObject != -1) {
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

	for (auto &dialog : Dialogs) {
		dialog->update(CurrentMode, CurrentClass);
	}

	Renderer->getCurrentCamera()->displayedObjectChanged();
}

void LabManager::changeShipInternal() {
	auto ship_objp = &Ships[Objects[CurrentObject].instance];
	auto ship_infop = &Ship_info[ship_objp->ship_info_index];

	// This is normally set during mission parse
	ship_objp->special_exp_damage = -1;

	// If the ship class defines replacement textures, load them and apply them to the ship
	// load the texture
	auto replacements = SCP_vector<texture_replace>();
	for (auto tr : ship_infop->replacement_textures) {
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
		replacements.push_back(tr);
	}

	ship_objp->apply_replacement_textures(replacements);
	ship_page_in_textures(ship_objp->ship_info_index);

	Renderer->setTeamColor(ship_infop->default_team_name);

	ModelFilename = model_get(ship_infop->model_num)->filename;
}
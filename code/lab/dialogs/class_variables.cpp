#include "lab/dialogs/class_variables.h"
#include "lab/labv2_internal.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "weapon/weapon.h"

void Variables::open() {
	/*if (dialogWindow == nullptr) {
		dialogWindow = (DialogWindow*)getLabManager()->Screen->Add(new DialogWindow("Class Variables", gr_screen.center_offset_x + gr_screen.center_w - 285,
			gr_screen.center_offset_y + 200));
		Assert(Opener != nullptr);
		dialogWindow->SetOwner(Opener->getDialog());
	}
	update(getLabManager()->CurrentMode, getLabManager()->CurrentClass);*/
}

void Variables::update(LabMode newLabMode, int classIndex) {
	if (dialogWindow == nullptr)
		return;

	int y = 0;
	dialogWindow->DeleteChildren();

	ship_info* sip = nullptr;
	weapon_info* wip = nullptr;

	if (classIndex > -1) {
		switch (newLabMode) {
		case LabMode::Ship:
			sip = &Ship_info[classIndex];

			addVariable(&y, "Name", sip->name);
			addVariable(&y, "Species", sip->species);
			addVariable(&y, "Type", sip->class_type);
			addVariable(&y, "Default Team Color", sip->default_team_name);

			addHeader(y, "Physics");
			addVariable(&y, "Density", sip->density);
			addVariable(&y, "Damp", sip->damp);
			addVariable(&y, "Rotdamp", sip->rotdamp);
			addVariable(&y, "Max vel (x)", sip->max_vel.xyz.x);
			addVariable(&y, "Max vel (y)", sip->max_vel.xyz.y);
			addVariable(&y, "Max vel (z)", sip->max_vel.xyz.z);
			addVariable(&y, "Warp in speed", Warp_params[sip->warpin_params_index].speed);
			addVariable(&y, "Warp out speed", Warp_params[sip->warpout_params_index].speed);

			addHeader(y, "Stats");
			addVariable(&y, "Shields", sip->max_shield_strength);
			addVariable(&y, "Hull", sip->max_hull_strength);
			addVariable(&y, "Subsys repair rate", sip->subsys_repair_rate);
			addVariable(&y, "Subsys repair max", sip->subsys_repair_max);
			addVariable(&y, "Hull repair rate", sip->hull_repair_rate);
			addVariable(&y, "Hull repair max", sip->hull_repair_max);
			addVariable(&y, "Countermeasures", sip->cmeasure_max);
			addVariable(&y, "HUD Icon", sip->shield_icon_index);

			addHeader(y, "Power");
			addVariable(&y, "Power output", sip->power_output);
			addVariable(&y, "Max oclk speed", sip->max_overclocked_speed);
			addVariable(&y, "Max weapon reserve", sip->max_weapon_reserve);

			addHeader(y, "Afterburner");
			addVariable(&y, "Fuel", sip->afterburner_fuel_capacity);
			addVariable(&y, "Burn rate", sip->afterburner_burn_rate);
			addVariable(&y, "Recharge rate", sip->afterburner_recover_rate);

			addHeader(y, "Explosion");
			addVariable(&y, "Inner radius", sip->shockwave.inner_rad);
			addVariable(&y, "Outer radius", sip->shockwave.outer_rad);
			addVariable(&y, "Damage", sip->shockwave.damage);
			addVariable(&y, "Blast", sip->shockwave.blast);
			addVariable(&y, "Propagates", sip->explosion_propagates);
			addVariable(&y, "Shockwave speed", sip->shockwave.speed);
			addVariable(&y, "Shockwave count", sip->shockwave_count);

			// techroom
			addHeader(y, "Techroom");
			addVariable(&y, "Closeup zoom", sip->closeup_zoom);
			addVariable(&y, "Closeup pos (x)", sip->closeup_pos.xyz.x);
			addVariable(&y, "Closeup pos (y)", sip->closeup_pos.xyz.y);
			addVariable(&y, "Closeup pos (z)", sip->closeup_pos.xyz.z);
			break;
		case LabMode::Weapon:
			wip = &Weapon_info[classIndex];

			addVariable(&y, "Name", wip->name);
			addVariable(&y, "Subtype", wip->subtype);

			// physics
			addHeader(y, "Physics");
			addVariable(&y, "Mass", wip->mass);
			addVariable(&y, "Max speed", wip->max_speed);
			addVariable(&y, "Lifetime", wip->lifetime);
			addVariable(&y, "Range", wip->weapon_range);
			addVariable(&y, "Min Range", wip->weapon_min_range);

			addHeader(y, "Damage");
			addVariable(&y, "Fire wait", wip->fire_wait);
			addVariable(&y, "Damage", wip->damage);
			addVariable(&y, "Armor factor", wip->armor_factor);
			addVariable(&y, "Shield factor", wip->shield_factor);
			addVariable(&y, "Subsys factor", wip->subsystem_factor);

			addHeader(y, "Armor");
			addVariable(&y, "Damage type", wip->damage_type_idx);

			addHeader(y, "Shockwave");
			addVariable(&y, "Speed", wip->shockwave.speed);

			addHeader(y, "Missiles");
			addVariable(&y, "Turn time", wip->turn_time);
			addVariable(&y, "FOV", wip->fov);
			addVariable(&y, "Min locktime", wip->min_lock_time);
			addVariable(&y, "Pixels/sec", wip->lock_pixels_per_sec);
			addVariable(&y, "Catchup pixels/sec", wip->catchup_pixels_per_sec);
			addVariable(&y, "Catchup pixel pen.", wip->catchup_pixel_penalty);
			addVariable(&y, "Swarm count", wip->swarm_count);
			addVariable(&y, "Swarm wait", wip->SwarmWait);
			break;
		default:
			return;
		}
	}
}

template<typename T>
void Variables::addVariable(int* Y, const char* var_name, T &value) {
	int y = 0;
	Text* new_text;

	if (Y) {
		y = *Y;
	}

	// variable
	dialogWindow->AddChild(new Text((var_name), (var_name), 0, y, 150));
	// edit box
	SCP_stringstream value_str;
	value_str << value;
	new_text = (Text*)dialogWindow->AddChild(new Text(SCP_string((var_name)) + SCP_string("Editbox"), value_str.str(),
		160, y, 100, gr_get_font_height() + 2));

	if (Y) {
		*Y += new_text->GetHeight() + 2;
	}
}

Text* Variables::addHeader(int& y, const SCP_string& text) {
	auto retval = (Text*)dialogWindow->AddChild(new Text(text, text, 80, y + 8, 100));
	y += retval->GetHeight() + 10;

	return retval;
}

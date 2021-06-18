#include "lab/dialogs/actions.h"
#include "lab/labv2_internal.h"
#include "ship/shiphit.h"

#include <array>

/**********************Click event handlers********************************************/

void destroy_subsystem(Tree* caller) {
	auto selected_subsys_index = caller->GetSelectedItem()->GetData();

	if (getLabManager()->isSafeForShips()) {
		auto sp = &Ships[Objects[getLabManager()->CurrentObject].instance];
		auto ssp = GET_FIRST(&sp->subsys_list);
		auto subsys_index = 0;
		while (ssp != END_OF_LIST(&sp->subsys_list)) {
			if (subsys_index == selected_subsys_index) {
				ssp->current_hits = 0;
				do_subobj_destroyed_stuff(sp, ssp, nullptr);
			}

			ssp = GET_NEXT(ssp);
			++subsys_index;
		}

		// recalculate when done
		ship_recalc_subsys_strength(sp);
	}
}

void change_primary(Tree* caller) {
	auto weapon_name = caller->GetSelectedItem()->Name;
	auto bank_num = caller->GetSelectedItem()->GetData();

	auto weapon_index = weapon_info_lookup(weapon_name.c_str());

	auto sp = &Ships[Objects[getLabManager()->CurrentObject].instance];
	sp->weapons.primary_bank_weapons[bank_num] = weapon_index;

	if (Weapon_info[weapon_index].wi_flags[Weapon::Info_Flags::Ballistic]) {
		sp->weapons.primary_bank_ammo[bank_num] = 10000;
	}
}

void change_secondary(Tree* caller) {
	auto weapon_name = caller->GetSelectedItem()->Name;
	auto bank_num = caller->GetSelectedItem()->GetData();

	auto weapon_index = weapon_info_lookup(weapon_name.c_str());

	auto sp = &Ships[Objects[getLabManager()->CurrentObject].instance];
	sp->weapons.secondary_bank_weapons[bank_num] = weapon_index;
}

void destroy_ship(Button* /*caller*/) {
	if (getLabManager()->isSafeForShips()) {
		auto obj = &Objects[getLabManager()->CurrentObject];

		obj->flags.remove(Object::Object_Flags::Player_ship);
		ship_self_destruct(obj);
	}
}

std::map<AnimationTriggerType, std::map<int, bool>> manual_animation_triggers = {};
std::map<AnimationTriggerType, bool> manual_animations = {};

std::array<bool, MAX_SHIP_PRIMARY_BANKS> triggered_primary_banks;
std::array<bool, MAX_SHIP_SECONDARY_BANKS> triggered_secondary_banks;

void reset_animations(Tree*) {
	if (getLabManager()->isSafeForShips()) {
		auto shipp = &Ships[Objects[getLabManager()->CurrentObject].instance];

		for (auto i = 0; i < MAX_SHIP_PRIMARY_BANKS; ++i) {
			if (triggered_primary_banks[i]) {
				model_anim_start_type(shipp, AnimationTriggerType::PrimaryBank, i, -1, false);
				triggered_primary_banks[i] = false;
			}
		}

		for (auto i = 0; i < MAX_SHIP_SECONDARY_BANKS; ++i) {
			if (triggered_secondary_banks[i]) {
				model_anim_start_type(shipp, AnimationTriggerType::SecondaryBank, i, -1, false);
				triggered_secondary_banks[i] = false;
			}
		}

		for (auto entry : manual_animations) {
			if (manual_animations[entry.first]) {
				model_anim_start_type(shipp, entry.first, 0, -1, false);
				manual_animations[entry.first] = false;
			}
		}

		for (const auto& entry : manual_animation_triggers) {
			auto animation_type = entry.first;
			auto manual_trigger_map = entry.second;

			for (auto manual_trigger : manual_trigger_map) {
				if (manual_trigger.second) {
					model_anim_start_type(shipp, animation_type, manual_trigger.first, -1, false);
				}
			}
		}
	}
}

void trigger_primary_bank(Tree* caller) {
	if (getLabManager()->isSafeForShips()) {
		auto shipp = &Ships[Objects[getLabManager()->CurrentObject].instance];
		auto bank = caller->GetSelectedItem()->GetData();
		model_anim_start_type(shipp, AnimationTriggerType::PrimaryBank, bank, triggered_primary_banks[bank] ? -1 : 1, false);
		triggered_primary_banks[bank] = !triggered_primary_banks[bank];
	}
}

void trigger_secondary_bank(Tree* caller) {
	if (getLabManager()->isSafeForShips()) {
		auto shipp = &Ships[Objects[getLabManager()->CurrentObject].instance];
		auto bank = caller->GetSelectedItem()->GetData();
		model_anim_start_type(shipp, AnimationTriggerType::SecondaryBank, bank, triggered_primary_banks[bank] ? -1 : 1, false);
		triggered_primary_banks[bank] = !triggered_primary_banks[bank];
	}
}

void labviewer_actions_do_triggered_anim(AnimationTriggerType type, int subobj_num, int direction) {
	if (getLabManager()->isSafeForShips()) {
		auto shipp = &Ships[Objects[getLabManager()->CurrentObject].instance];
		model_anim_start_type(shipp, type, subobj_num, direction);
	}
}

void trigger_animation(Tree* caller) {
	if (getLabManager()->isSafeForShips()) {
		auto shipp = &Ships[Objects[getLabManager()->CurrentObject].instance];

		auto anim_type = static_cast<AnimationTriggerType>(caller->GetSelectedItem()->GetData());

		// bool model_anim_start_type(ship *shipp, AnimationTriggerType animation_type, int subtype, int direction, bool instant = false);		// for all valid subsystems
		model_anim_start_type(shipp, anim_type, 0, manual_animations[anim_type] ? -1 : 1, false);
		manual_animations[anim_type] = !manual_animations[anim_type];
	}
}

void trigger_scripted(Tree* caller) {
	auto subobj_num = caller->GetSelectedItem()->GetData();
	auto scripted_anim_triggers = manual_animation_triggers[AnimationTriggerType::Scripted];
	auto direction = scripted_anim_triggers[subobj_num] ? -1 : 1;

	labviewer_actions_do_triggered_anim(AnimationTriggerType::Scripted, subobj_num, direction);
	scripted_anim_triggers[subobj_num] = !scripted_anim_triggers[subobj_num];
}

void trigger_turret_firing(Tree* caller) {
	auto subobj_num = caller->GetSelectedItem()->GetData();
	auto turret_firing_triggers = manual_animation_triggers[AnimationTriggerType::TurretFiring];
	auto direction = turret_firing_triggers[subobj_num] ? -1 : 1;

	labviewer_actions_do_triggered_anim(AnimationTriggerType::TurretFiring, subobj_num, direction);
	turret_firing_triggers[subobj_num] = !turret_firing_triggers[subobj_num];
}

void trigger_turret_fired(Tree* caller) {
	auto subobj_num = caller->GetSelectedItem()->GetData();
	auto turret_fired_triggers = manual_animation_triggers[AnimationTriggerType::TurretFired];
	auto direction = turret_fired_triggers[subobj_num] ? -1 : 1;

	labviewer_actions_do_triggered_anim(AnimationTriggerType::TurretFired, subobj_num, direction);
	turret_fired_triggers[subobj_num] = !turret_fired_triggers[subobj_num];
}

/***********************Dialog implementations***************************************/

void Actions::open(Button* /*caller*/) {
	if (dialogWindow != nullptr)
		return;
	
	getLabManager()->loadWeapons();

	dialogWindow = (DialogWindow*)getLabManager()->Screen->Add(new DialogWindow("Actions", gr_screen.center_offset_x + 250, gr_screen.center_offset_y + 50));
	Assert(Opener != nullptr);
	dialogWindow->SetOwner(Opener->getDialog());

	int y = 0;

	auto btn = new Button("Destroy Ship", 0, y, destroy_ship);
	dialogWindow->AddChild(btn);
	y += btn->GetHeight();

	for (auto dialog : subDialogs) {
		auto *dgo = new DialogOpener(dialog, 0, y);
		dialog->setOpener(dgo);
		dialogWindow->AddChild(dgo);
		y += dgo->GetHeight();
	}

	update(getLabManager()->CurrentMode, getLabManager()->CurrentClass);
}

void Actions::update(LabMode newLabMode, int classIndex) {
	for (auto dialog : subDialogs) {
		dialog->update(newLabMode, classIndex);
	}
}

void DestroySubsystems::open(Button* /*caller*/) {
	if (dialogWindow != nullptr)
		return;

	dialogWindow = (DialogWindow*)getLabManager()->Screen->Add(new DialogWindow("Destroy Subsystems", gr_screen.center_offset_x + 400, gr_screen.center_offset_y + 50));
	Assert(Opener != nullptr);
	dialogWindow->SetOwner(Opener->getDialog());

	update(getLabManager()->CurrentMode, getLabManager()->CurrentClass);
}

void DestroySubsystems::update(LabMode, int) {
	if (dialogWindow == nullptr)
		return;

	dialogWindow->DeleteChildren();

	auto subsys_tree = (Tree*)dialogWindow->AddChild(new Tree("Subsystem List", 0, 0));

	if (getLabManager()->isSafeForShips()) {
		auto sp = &Ships[Objects[getLabManager()->CurrentObject].instance];
		auto ssp = GET_FIRST(&sp->subsys_list);
		auto subsys_index = 0;
		while (ssp != END_OF_LIST(&sp->subsys_list)) {
			subsys_tree->AddItem(nullptr, ssp->system_info->name, subsys_index, true, destroy_subsystem);
			ssp = GET_NEXT(ssp);
			++subsys_index;
		}
	}
}

void ChangeLoadout::open(Button* /*caller*/) {
	if (dialogWindow != nullptr)
		return;

	dialogWindow = (DialogWindow*)getLabManager()->Screen->Add(new DialogWindow("Change Loadout", gr_screen.center_offset_x + 400, gr_screen.center_offset_y + 50));
	Assert(Opener != nullptr);
	dialogWindow->SetOwner(Opener->getDialog());

	update(getLabManager()->CurrentMode, getLabManager()->CurrentClass);
}

void ChangeLoadout::update(LabMode, int) {
	if (dialogWindow == nullptr)
		return;

	dialogWindow->DeleteChildren();

	auto weapons_tree = (Tree*)dialogWindow->AddChild(new Tree("Weapons stuff", 0, 0));

	if (getLabManager()->isSafeForShips()) {
		auto sp = &Ships[Objects[getLabManager()->CurrentObject].instance];
		auto sip = &Ship_info[sp->ship_info_index];

		if (sip->is_flyable()) {
			auto primaries_head = weapons_tree->AddItem(nullptr, "Primaries", 0, false);
			auto secondaries_head = weapons_tree->AddItem(nullptr, "Secondaries", 0, false);

			for (auto bank_num = 0; bank_num < sip->num_primary_banks; ++bank_num) {
				SCP_string bank_string;
				sprintf(bank_string, "Bank %i", bank_num);
				auto bank_head = weapons_tree->AddItem(primaries_head, bank_string, 0, false);
				auto tabled_weapons_head = weapons_tree->AddItem(bank_head, "Tabled weapons", 0, false);
				auto others_head = weapons_tree->AddItem(bank_head, "Other Primaries", 0, false);
				auto beams_head = weapons_tree->AddItem(others_head, "Beams", 0, false);
				auto ballistics_head = weapons_tree->AddItem(others_head, "Ballistics", 0, false);
				auto player_allowed_ballistics = weapons_tree->AddItem(ballistics_head, "Player Allowed", 0, false);
				auto lasers_head = weapons_tree->AddItem(others_head, "Lasers", 0, false);
				auto player_allowed_lasers = weapons_tree->AddItem(lasers_head, "Player Allowed", 0, false);
				auto capitals_head = weapons_tree->AddItem(bank_head, "Capital Ship weapons", 0, false);

				auto n_weapons = MIN(Weapon_info.size(), MAX_WEAPON_TYPES);
				for (size_t j = 0; j < n_weapons; ++j) {
					auto wip = &Weapon_info[j];
					if (wip->subtype == WP_LASER || wip->subtype == WP_BEAM) {
						if (sip->allowed_weapons[j] != 0) {
							weapons_tree->AddItem(tabled_weapons_head, wip->name, bank_num, true, change_primary);
						}
						else {
							TreeItem* head_item = nullptr;

							if (wip->wi_flags[Weapon::Info_Flags::Beam] || wip->subtype == WP_BEAM) {
								head_item = beams_head;
							}
							else if (wip->wi_flags[Weapon::Info_Flags::Huge, Weapon::Info_Flags::Supercap]) {
								head_item = capitals_head;
							}
							else if (wip->subtype == WP_LASER) {
								if (wip->wi_flags[Weapon::Info_Flags::Ballistic]) {
									if (wip->wi_flags[Weapon::Info_Flags::Player_allowed])
										head_item = player_allowed_ballistics;
									else
										head_item = ballistics_head;
								}
								else {
									if (wip->wi_flags[Weapon::Info_Flags::Player_allowed])
										head_item = player_allowed_lasers;
									else
										head_item = lasers_head;
								}
							}

							if (head_item != nullptr) {
								weapons_tree->AddItem(head_item, wip->name, bank_num, true, change_primary);
							}
						}
					}
				}
			}

			for (auto i = 0; i < sip->num_secondary_banks; ++i) {
				SCP_string bank_string;
				sprintf(bank_string, "Bank %i", i);
				auto bank_head = weapons_tree->AddItem(secondaries_head, bank_string, 0, false);
				auto tabled_weapons_head = weapons_tree->AddItem(bank_head, "Tabled weapons", 0, false);
				auto others_head = weapons_tree->AddItem(bank_head, "Others", 0, false);
				auto missiles_head = weapons_tree->AddItem(others_head, "Missiles", 0, false);
				auto bombs_head = weapons_tree->AddItem(others_head, "Bombs", 0, false);

				auto n_weapons = MIN(Weapon_info.size(), MAX_WEAPON_TYPES);
				for (size_t j = 0; j < n_weapons; ++j) {
					auto wip = &Weapon_info[j];
					if (wip->subtype == WP_MISSILE) {
						TreeItem* head_item = nullptr;

						if (sip->allowed_weapons[j] != 0) {
							head_item = tabled_weapons_head;
						}
						else {
							if (wip->wi_flags[Weapon::Info_Flags::Bomb]) {
								head_item = bombs_head;
							}
							else {
								head_item = missiles_head;
							}
						}

						weapons_tree->AddItem(head_item, wip->name, i, true, change_secondary);
					}
				}
			}
		}
	}
}

void WeaponFire::open(Button* /*caller*/) {
	if (dialogWindow != nullptr)
		return;

	dialogWindow = (DialogWindow*)getLabManager()->Screen->Add(
		new DialogWindow("Fire weapons", gr_screen.center_offset_x + 400, gr_screen.center_offset_y + 50)
	);
	Assert(Opener != nullptr);
	dialogWindow->SetOwner(Opener->getDialog());

	update(getLabManager()->CurrentMode, getLabManager()->CurrentClass);
}

void WeaponFire::update(LabMode, int) {
	if (dialogWindow == nullptr) return;

	if (getLabManager()->isSafeForShips()) {
		auto sp = &Ships[Objects[getLabManager()->CurrentObject].instance];

		int y = 0;

		for (auto i = 0; i < sp->weapons.num_primary_banks; ++i) {
			SCP_string bank_string;
			sprintf(bank_string, "Primary bank %i", i);
			auto cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox((bank_string), 2, y));
			cbp->SetFlag(&getLabManager()->FirePrimaries, 1 << i);
			y += cbp->GetHeight() + 1;
		}

		for (auto i = 0; i < sp->weapons.num_secondary_banks; ++i) {
			SCP_string bank_string;
			sprintf(bank_string, "Secondary bank %i", i);
			auto cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox((bank_string), 2, y));
			cbp->SetFlag(&getLabManager()->FireSecondaries, 1 << i);
			y += cbp->GetHeight() + 1;
		}
	}
}

void AnimationTrigger::open(Button* /*caller*/) {
	if (dialogWindow != nullptr)
		return;

	dialogWindow = (DialogWindow*)getLabManager()->Screen->Add(
		new DialogWindow("Trigger animations", gr_screen.center_offset_x + 400, gr_screen.center_offset_y + 50)
	);
	Assert(Opener != nullptr);
	dialogWindow->SetOwner(Opener->getDialog());

	update(getLabManager()->CurrentMode, getLabManager()->CurrentClass);
}

void AnimationTrigger::update(LabMode, int) {
	if (dialogWindow == nullptr)
		return;

	if (getLabManager()->isSafeForShips()) {
		auto shipp = &Ships[Objects[getLabManager()->CurrentObject].instance];

		auto animations_tree = (Tree*)dialogWindow->AddChild(new Tree("Animations stuff", 0, 0));

		std::map<AnimationTriggerType, TreeItem*> subsystem_headers;

		animations_tree->AddItem(nullptr, "Reset animation state", 0, true, reset_animations);

		auto subsystems_head = animations_tree->AddItem(nullptr, "Subsystem triggers");

		manual_animation_triggers.clear();

		for (const auto& entry : Animation_type_names) {
			if (entry.first == AnimationTriggerType::Initial)
				continue;

			subsystem_headers[entry.first] = animations_tree->AddItem(subsystems_head, entry.second);
		}

		for (auto i = 0; i < shipp->weapons.num_primary_banks; ++i) {
			SCP_string bank_string;
			sprintf(bank_string, "Trigger animations for Bank %i", i);
			animations_tree->AddItem(subsystem_headers[AnimationTriggerType::PrimaryBank], bank_string, i, true, trigger_primary_bank);
		}

		for (bool& triggered_primary_bank : triggered_primary_banks)
			triggered_primary_bank = false;

		for (auto i = 0; i < shipp->weapons.num_secondary_banks; ++i) {
			SCP_string bank_string;
			sprintf(bank_string, "Trigger animations for Bank %i", i);
			animations_tree->AddItem(subsystem_headers[AnimationTriggerType::SecondaryBank], bank_string, i, true, trigger_secondary_bank);
		}

		for (bool& triggered_secondary_bank : triggered_secondary_banks)
			triggered_secondary_bank = false;

		auto ssp = GET_FIRST(&shipp->subsys_list);
		auto subsys_index = 0;

		// We use these vectors to handle cases where several subsystems have animations defined for the same turret firing/having fired
		SCP_vector<int> turret_firing_subsystem_triggers;
		SCP_vector<int> turret_fired_subsystem_triggers;

		std::map<int, model_subsystem*> stupid_workaround_map; // this is a stupid workaround to avoid having to iterate over the subsys list a billion times

		while (ssp != END_OF_LIST(&shipp->subsys_list)) {
			stupid_workaround_map[ssp->system_info->subobj_num] = ssp->system_info;

			if (ssp->system_info->n_triggers != 0) {
				for (auto i = 0; i < ssp->system_info->n_triggers; ++i) {
					auto trigger = ssp->system_info->triggers[i];
					if (trigger.type == AnimationTriggerType::Initial)
						continue;

					switch (trigger.type) {
					case AnimationTriggerType::TurretFiring:
						if (!SCP_vector_contains(turret_firing_subsystem_triggers, trigger.subtype)) {
							// For "turret-firing" animations, subtype contains the subobject number of the firing turret
							turret_firing_subsystem_triggers.push_back(trigger.subtype);
							manual_animation_triggers[trigger.type][trigger.subtype] = false;

						}
						break;
					case AnimationTriggerType::TurretFired:
						if (!SCP_vector_contains(turret_fired_subsystem_triggers, trigger.subtype)) {
							// Same as above
							turret_fired_subsystem_triggers.push_back(trigger.subtype);
							manual_animation_triggers[trigger.type][ssp->system_info->subobj_num] = false;
						}
						break;
					case AnimationTriggerType::Scripted:
						manual_animation_triggers[trigger.type][ssp->system_info->subobj_num] = false;
						animations_tree->AddItem(subsystem_headers[AnimationTriggerType::Scripted], ssp->system_info->name, ssp->system_info->subobj_num, true, trigger_scripted);
						break;
					default:
						break;
					}
				}
			}

			ssp = GET_NEXT(ssp);
			++subsys_index;
		}

		for (auto subobj_num : turret_firing_subsystem_triggers) {
			animations_tree->AddItem(subsystem_headers[AnimationTriggerType::TurretFiring], stupid_workaround_map[subobj_num]->subobj_name, subobj_num, true, trigger_turret_firing);
		}

		for (auto subobj_num : turret_fired_subsystem_triggers) {
			animations_tree->AddItem(subsystem_headers[AnimationTriggerType::TurretFired], stupid_workaround_map[subobj_num]->subobj_name, subobj_num, true, trigger_turret_fired);
		}

		auto shipwide_head = animations_tree->AddItem(nullptr, "Shipwide triggers");

		for (const auto& entry : Animation_type_names) {
			manual_animations[entry.first] = false;
			animations_tree->AddItem(shipwide_head, entry.second, static_cast<int>(entry.first), false, trigger_animation);
		}
	}
}

void AnimationTrigger::close() {
	if (dialogWindow != nullptr) {
		dialogWindow->DeleteChildren();
		dialogWindow = nullptr;
	}
	manual_animation_triggers.clear();
	manual_animations.clear();

	for (auto& trigger : triggered_primary_banks)
		trigger = false;
	for (auto& trigger : triggered_secondary_banks)
		trigger = false;
}
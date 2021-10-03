#include "lab/dialogs/actions.h"
#include "lab/labv2_internal.h"
#include "ship/shiphit.h"
#include "model/modelanimation.h"

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

std::map<animation::ModelAnimationTriggerType, std::map<SCP_string, bool>> manual_animation_triggers = {};
std::map<animation::ModelAnimationTriggerType, bool> manual_animations = {};

std::array<bool, MAX_SHIP_PRIMARY_BANKS> triggered_primary_banks;
std::array<bool, MAX_SHIP_SECONDARY_BANKS> triggered_secondary_banks;

void reset_animations(Tree*) {
	if (getLabManager()->isSafeForShips()) {
		auto shipp = &Ships[Objects[getLabManager()->CurrentObject].instance];

		polymodel_instance* shipp_pmi = model_get_instance(shipp->model_instance_num);

		for (auto i = 0; i < MAX_SHIP_PRIMARY_BANKS; ++i) {
			if (triggered_primary_banks[i]) {
				Ship_info[shipp->ship_info_index].animations.startAll(shipp_pmi, AnimationTriggerType::PrimaryBank, animation::ModelAnimationDirection::RWD, false, false, i);
				triggered_primary_banks[i] = false;
			}
		}

		for (auto i = 0; i < MAX_SHIP_SECONDARY_BANKS; ++i) {
			if (triggered_secondary_banks[i]) {
				Ship_info[shipp->ship_info_index].animations.startAll(shipp_pmi, AnimationTriggerType::SecondaryBank, animation::ModelAnimationDirection::RWD, false, false, i);
				triggered_secondary_banks[i] = false;
			}
		}

		for (auto entry : manual_animations) {
			if (manual_animations[entry.first]) {
				Ship_info[shipp->ship_info_index].animations.startAll(shipp_pmi, entry.first, animation::ModelAnimationDirection::RWD, false, false);
				manual_animations[entry.first] = false;
			}
		}

		for (const auto& entry : manual_animation_triggers) {
			auto animation_type = entry.first;
			Ship_info[shipp->ship_info_index].animations.startAll(shipp_pmi, animation_type, animation::ModelAnimationDirection::RWD);
		}
	}
}

void trigger_primary_bank(Tree* caller) {
	if (getLabManager()->isSafeForShips()) {
		auto shipp = &Ships[Objects[getLabManager()->CurrentObject].instance];
		auto bank = caller->GetSelectedItem()->GetData();
		Ship_info[shipp->ship_info_index].animations.startAll(model_get_instance(shipp->model_instance_num), AnimationTriggerType::PrimaryBank, triggered_primary_banks[bank] ? animation::ModelAnimationDirection::RWD : animation::ModelAnimationDirection::FWD, false, false, bank);
		triggered_primary_banks[bank] = !triggered_primary_banks[bank];
	}
}

void trigger_secondary_bank(Tree* caller) {
	if (getLabManager()->isSafeForShips()) {
		auto shipp = &Ships[Objects[getLabManager()->CurrentObject].instance];
		auto bank = caller->GetSelectedItem()->GetData();
		Ship_info[shipp->ship_info_index].animations.startAll(model_get_instance(shipp->model_instance_num), AnimationTriggerType::SecondaryBank, triggered_primary_banks[bank] ? animation::ModelAnimationDirection::RWD : animation::ModelAnimationDirection::FWD, false, false, bank);
		triggered_primary_banks[bank] = !triggered_primary_banks[bank];
	}
}

void labviewer_actions_do_triggered_anim(AnimationTriggerType type, const SCP_string& name, bool direction, int subtype = animation::ModelAnimationSet::SUBTYPE_DEFAULT) {
	if (getLabManager()->isSafeForShips()) {
		auto shipp = &Ships[Objects[getLabManager()->CurrentObject].instance];

		if(subtype != animation::ModelAnimationSet::SUBTYPE_DEFAULT)
			Ship_info[shipp->ship_info_index].animations.startAll(model_get_instance(shipp->model_instance_num), type, direction ? animation::ModelAnimationDirection::RWD : animation::ModelAnimationDirection::FWD, false, false, subtype, true);
		else
			Ship_info[shipp->ship_info_index].animations.start(model_get_instance(shipp->model_instance_num), type, name, direction ? animation::ModelAnimationDirection::RWD : animation::ModelAnimationDirection::FWD);
	}
}

void trigger_animation(Tree* caller) {
	if (getLabManager()->isSafeForShips()) {
		auto shipp = &Ships[Objects[getLabManager()->CurrentObject].instance];

		auto anim_type = static_cast<AnimationTriggerType>(caller->GetSelectedItem()->GetData());

		Ship_info[shipp->ship_info_index].animations.startAll(model_get_instance(shipp->model_instance_num), anim_type, manual_animations[anim_type] ? animation::ModelAnimationDirection::RWD : animation::ModelAnimationDirection::FWD, false, false);
		manual_animations[anim_type] = !manual_animations[anim_type];
	}
}

void trigger_scripted(Tree* caller) {
	auto& scripted_anim_triggers = manual_animation_triggers[AnimationTriggerType::Scripted];
	auto direction = scripted_anim_triggers[caller->GetSelectedItem()->Name];

	labviewer_actions_do_triggered_anim(AnimationTriggerType::Scripted, caller->GetSelectedItem()->Name, direction);
	scripted_anim_triggers[caller->GetSelectedItem()->Name] = !scripted_anim_triggers[caller->GetSelectedItem()->Name];
}

void trigger_turret_firing(Tree* caller) {
	auto subobj_num = caller->GetSelectedItem()->GetData();
	auto& turret_firing_triggers = manual_animation_triggers[AnimationTriggerType::TurretFiring];
	auto direction = turret_firing_triggers[caller->GetSelectedItem()->Name];

	labviewer_actions_do_triggered_anim(AnimationTriggerType::TurretFiring, caller->GetSelectedItem()->Name, direction, subobj_num);
	turret_firing_triggers[caller->GetSelectedItem()->Name] = !turret_firing_triggers[caller->GetSelectedItem()->Name];
}

void trigger_turret_fired(Tree* caller) {
	auto& turret_fired_triggers = manual_animation_triggers[AnimationTriggerType::TurretFired];
	auto direction = turret_fired_triggers[caller->GetSelectedItem()->Name];

	labviewer_actions_do_triggered_anim(AnimationTriggerType::TurretFired, caller->GetSelectedItem()->Name, direction);
	turret_fired_triggers[caller->GetSelectedItem()->Name] = !turret_fired_triggers[caller->GetSelectedItem()->Name];
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

	for (auto &dialog : subDialogs) {
		auto *dgo = new DialogOpener(dialog, 0, y);
		dialog->setOpener(dgo);
		dialogWindow->AddChild(dgo);
		y += dgo->GetHeight();
	}

	update(getLabManager()->CurrentMode, getLabManager()->CurrentClass);
}

void Actions::update(LabMode newLabMode, int classIndex) {
	for (auto &dialog : subDialogs) {
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

		std::map<animation::ModelAnimationTriggerType, TreeItem*> subsystem_headers;

		animations_tree->AddItem(nullptr, "Reset animation state", 0, true, reset_animations);

		auto subsystems_head = animations_tree->AddItem(nullptr, "Subsystem triggers");

		manual_animation_triggers.clear();

		for (const auto& entry : animation::Animation_type_names) {
			if (entry.first == animation::ModelAnimationTriggerType::Initial)
				continue;

			subsystem_headers[entry.first] = animations_tree->AddItem(subsystems_head, entry.second);
		}

		for (auto i = 0; i < shipp->weapons.num_primary_banks; ++i) {
			SCP_string bank_string;
			sprintf(bank_string, "Trigger animations for Bank %i", i);
			animations_tree->AddItem(subsystem_headers[animation::ModelAnimationTriggerType::PrimaryBank], bank_string, i, true, trigger_primary_bank);
		}

		for (bool& triggered_primary_bank : triggered_primary_banks)
			triggered_primary_bank = false;

		for (auto i = 0; i < shipp->weapons.num_secondary_banks; ++i) {
			SCP_string bank_string;
			sprintf(bank_string, "Trigger animations for Bank %i", i);
			animations_tree->AddItem(subsystem_headers[animation::ModelAnimationTriggerType::SecondaryBank], bank_string, i, true, trigger_secondary_bank);
		}

		for (bool& triggered_secondary_bank : triggered_secondary_banks)
			triggered_secondary_bank = false;

		const animation::ModelAnimationSet& anims = Ship_info[shipp->ship_info_index].animations;

		for (const auto& animList : anims.m_animationSet) {
			for (const auto& animation : animList.second) {
				switch (animList.first.first) {
				case animation::ModelAnimationTriggerType::TurretFiring: {
					SCP_string name = animation.first;
					if (animList.first.second != animation::ModelAnimationSet::SUBTYPE_DEFAULT) {
						//Legacy layer
						sprintf(name, "Turret Model #%i", animList.first.second);
					}
					
					animations_tree->AddItem(subsystem_headers[animation::ModelAnimationTriggerType::TurretFiring], name, animList.first.second, true, trigger_turret_firing);
					break;
				}
				case animation::ModelAnimationTriggerType::TurretFired:
					animations_tree->AddItem(subsystem_headers[animation::ModelAnimationTriggerType::TurretFired], animation.first, animList.first.second, true, trigger_turret_fired);
					break;
				case animation::ModelAnimationTriggerType::Scripted:
					animations_tree->AddItem(subsystem_headers[animation::ModelAnimationTriggerType::Scripted], animation.first, animList.first.second, true, trigger_scripted);
					break;
				default:
					break;
				}

			}
		}

		auto shipwide_head = animations_tree->AddItem(nullptr, "Shipwide triggers");

		for (const auto& entry : animation::Animation_type_names) {
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
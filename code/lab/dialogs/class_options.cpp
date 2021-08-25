#include "lab/dialogs/class_options.h"
#include "lab/labv2_internal.h"
#include "ship/ship.h"
#include "weapon/weapon.h"

void Options::open(Button* /*caller*/) {
	if (dialogWindow == nullptr) {
		dialogWindow = (DialogWindow*)getLabManager()->Screen->Add(new DialogWindow(
			"Flags Window", gr_screen.center_offset_x + gr_screen.center_w - 205, gr_screen.center_offset_y + 200));
		Assert(Opener != nullptr);
		dialogWindow->SetOwner(Opener->getDialog());
	}
	update(getLabManager()->CurrentMode, getLabManager()->CurrentClass);
}

void Options::update(LabMode newLabMode, int classIndex) {
	if (dialogWindow == nullptr)
		return;

	int y = 0;
	dialogWindow->DeleteChildren();

	ship_info* sip;
	weapon_info* wip;

	if (classIndex > -1) {
		switch (newLabMode) {
		case LabMode::Ship:
			sip = &Ship_info[classIndex];
			for (size_t i = 0; i < Num_ship_flags; ++i) {
				addFlags<Ship::Info_Flags>(nullptr, &y, Ship_flags[i].name, Ship_flags[i].def, Ship_Class_Flags);
			}

			for (auto flag_def : Ship_Class_Flags) {
				if (flag_def.flag == Ship::Info_Flags::NUM_VALUES)
					continue;
				flag_def.cb->SetFlag(sip->flags, flag_def.flag, sip);
			}
			break;
		case LabMode::Weapon:
			wip = &Weapon_info[classIndex];

			for (size_t i = 0; i < num_weapon_info_flags; ++i) {
				addFlags<Weapon::Info_Flags>(nullptr, &y, Weapon_Info_Flags[i].name, Weapon_Info_Flags[i].def,
					Weapon_Class_Flags);
			}

			for (auto flag_def : Weapon_Class_Flags) {
				if (flag_def.flag == Weapon::Info_Flags::NUM_VALUES)
					continue;
				flag_def.cb->SetFlag(wip->wi_flags, flag_def.flag, wip);
			}
			break;
		default:
			return;
		}
	}
}


// The readability-non-const-parameter check is disabled here since it apparently gets confused by int* const -- The E, 23/10/2020
template <class T>
void Options::addFlags(int* const X, int* const Y, const char* flag_name, T flag, SCP_vector<lab_flag<T>>& flag_list)  // NOLINT(readability-non-const-parameter)
{
	int x = 0, y = 0;

	lab_flag<T> new_flag;

	if (X) {
		x = *X;
	}

	if (Y) {
		y = *Y;
	}

	new_flag.cb = (Checkbox*)dialogWindow->AddChild(new Checkbox(flag_name, x, y));
	new_flag.flag = flag;
	flag_list.push_back(new_flag);

	if (X) {
		*X += new_flag.cb->GetWidth() + 2;
	}

	if (Y) {
		*Y += new_flag.cb->GetHeight() + 1;
	}
}
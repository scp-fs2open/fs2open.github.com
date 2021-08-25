#include "lab/dialogs/weapon_classes.h"
#include "lab/labv2_internal.h"
#include "weapon/weapon.h"

void changeWeapon(Tree* caller) {
	auto class_index = caller->GetSelectedItem()->GetData();

	getLabManager()->changeDisplayedObject(LabMode::Weapon, class_index);
}

void WeaponClasses::open(Button* /*caller*/) {
	if (dialogWindow != nullptr)
		return;

	dialogWindow = (DialogWindow*)getLabManager()->Screen->Add(new DialogWindow("Weapon Classes", gr_screen.center_offset_x + 50, gr_screen.center_offset_y + 50));
	Assert(Opener != nullptr);
	dialogWindow->SetOwner(Opener->getDialog());
	auto tree = (Tree*)dialogWindow->AddChild(new Tree("Weapon Tree", 0, 0));

	SCP_vector<TreeItem*> typeHeaders;

	for (auto i = 0; i < Num_weapon_subtypes; ++i) {
		typeHeaders.push_back(tree->AddItem(nullptr, Weapon_subtype_names[i], 0, false));
	}

	auto idx = 0;
	for (auto wip : Weapon_info) {
		if (wip.subtype == WP_UNUSED)
			continue;

		if (wip.subtype >= Num_weapon_subtypes) {
			Error(LOCATION, "Unknown weapon subtype in weapon %s!!!", wip.name);
		}

		TreeItem* header = nullptr;
		if (wip.wi_flags[Weapon::Info_Flags::Beam])
			header = typeHeaders[WP_BEAM];
		else
			header = typeHeaders[wip.subtype];

		tree->AddItem(header, wip.get_display_name(), idx, false, changeWeapon);
		++idx;
	}
}

void WeaponClasses::update(LabMode newLabMode, int classIndex) {
	// if the incoming mode is not LabMode::Weapon, close this dialog
	if (newLabMode != LabMode::Weapon) {
		close();
		return;
	}

	if (Class_toolbar == nullptr) {
		Class_toolbar = (DialogWindow*)getLabManager()->Screen->Add(new DialogWindow("Class Toolbar", gr_screen.center_offset_x + 0,
			gr_screen.center_offset_y + getLabManager()->Toolbar->GetHeight(), -1, -1, WS_NOTITLEBAR | WS_NONMOVEABLE));
		Assert(Opener != nullptr);
		Class_toolbar->SetOwner(Opener->getDialog());
	}
	Class_toolbar->DeleteChildren();

	auto x = 0;
	for (auto &subdialog : Subdialogs) {
		auto *dgo = new DialogOpener(subdialog, x, 0);
		subdialog->setOpener(dgo);
		auto *cbp = Class_toolbar->AddChild(dgo);
		x += cbp->GetWidth();

		subdialog->update(newLabMode, classIndex);
	}
}
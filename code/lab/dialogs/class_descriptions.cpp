#include "lab/dialogs/class_descriptions.h"
#include "lab/labv2_internal.h"

void Descriptions::open(Button* /*caller*/) {
	if (dialogWindow == nullptr) {
		dialogWindow = (DialogWindow*)getLabManager()->Screen->Add(
			new DialogWindow("Description", gr_screen.center_offset_x + gr_screen.center_w - gr_screen.center_w / 3 - 50,
				gr_screen.center_offset_y + gr_screen.center_h - gr_screen.center_h / 6 - 50, gr_screen.center_w / 3,
				gr_screen.center_h / 6)
		);
		Assert(Opener != nullptr);
		dialogWindow->SetOwner(Opener->getDialog());
	}

	dialogWindow->DeleteChildren();
	descriptionText = (Text*)dialogWindow->AddChild(new Text("Description Text", "No description available.", 0, 0));

	update(getLabManager()->CurrentMode, getLabManager()->CurrentClass);
}

void Descriptions::update(LabMode newLabMode, int classIndex) {
	if (dialogWindow == nullptr)
		return;

	if (classIndex > -1) {
		switch (newLabMode) {
		case LabMode::Ship:
			dialogWindow->SetCaption(Ship_info[classIndex].name);

			if (Ship_info[classIndex].tech_desc != nullptr) {
				descriptionText->SetText(Ship_info[classIndex].tech_desc);
			}
			else {
				descriptionText->SetText("No description available.");
			}
			break;
		case LabMode::Weapon:
			dialogWindow->SetCaption(Weapon_info[classIndex].get_display_name());

			if (Weapon_info[classIndex].tech_desc != nullptr) {
				descriptionText->SetText(Weapon_info[classIndex].tech_desc);
			}
			else {
				descriptionText->SetText("No description available.");
			}
			break;
		default:
			return;
		}
	}
	else {
		descriptionText->SetText("No description available.");
	}
}
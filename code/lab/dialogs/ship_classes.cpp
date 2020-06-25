#include "lab/dialogs/ship_classes.h"
#include "lab/labv2_internal.h"

void changeShip(Tree* caller) {
	auto class_index = caller->GetSelectedItem()->GetData();

	LMGR->changeDisplayedObject(LabMode::Ship, class_index);
}

void ShipClasses::open(Button* caller) {
	if (already_opened)
		return;

	List_window = (Window*)LMGR->Screen->Add(new Window("Ship Classes", gr_screen.center_offset_x + 50, gr_screen.center_offset_y + 50));
	Class_tree = (Tree*)List_window->AddChild(new Tree("Ship Tree", 0, 0));

	/* Ship tree layout
		Species1
		Species2
		Species3
		   |--Ship
		   |--Ship
		   |--Ship
	*/

	SCP_vector<TreeItem*> species_headers;

	for (auto const& species_def : Species_info) {
		auto header = Class_tree->AddItem(nullptr, species_def.species_name);
		species_headers.push_back(header);
	}

	auto idx = 0;
	for (auto const& class_def : Ship_info) {
		auto header = species_headers[class_def.species];
		Class_tree->AddItem(header, class_def.name, idx, true, changeShip);
		++idx;
	}

	already_opened = true;
}

void ShipClasses::update(LabMode newLabMode, int classIndex) {
	Class_toolbar->DeleteChildren();

	Class_toolbar = (Window*)LMGR->Screen->Add(new Window("Class Toolbar", gr_screen.center_offset_x + 0,
		gr_screen.center_offset_y + LMGR->Toolbar->GetHeight(), -1, -1, WS_NOTITLEBAR | WS_NONMOVEABLE));

	auto x = 0;
	for (auto subdialog : Subdialogs) {
		auto cbp = Class_toolbar->AddChild(new DialogOpener(subdialog, x, 0));
		x += cbp->GetWidth();

		subdialog->update(newLabMode, classIndex);
	}
}
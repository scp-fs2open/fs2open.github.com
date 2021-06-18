#include "lab/dialogs/ship_classes.h"
#include "lab/labv2_internal.h"

void changeShip(Tree* caller) {
	auto class_index = caller->GetSelectedItem()->GetData();

	getLabManager()->changeDisplayedObject(LabMode::Ship, class_index);
}

void ShipClasses::open(Button* /*caller*/) {
	if (dialogWindow != nullptr)
		return;

	dialogWindow = (DialogWindow*)getLabManager()->Screen->Add(new DialogWindow("Ship Classes", gr_screen.center_offset_x + 50, gr_screen.center_offset_y + 50));
	Assert(Opener != nullptr);
	dialogWindow->SetOwner(Opener->getDialog());
	auto tree = (Tree*)dialogWindow->AddChild(new Tree("Ship Tree", 0, 0));

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
		auto header = tree->AddItem(nullptr, species_def.species_name);
		species_headers.push_back(header);
	}

	auto idx = 0;
	for (auto const& class_def : Ship_info) {
		auto header = species_headers[class_def.species];
		tree->AddItem(header, class_def.name, idx, true, changeShip);
		++idx;
	}
}

void ShipClasses::update(LabMode newLabMode, int classIndex) {
	// if the incoming mode is not LabMode::Ship, close this dialog
	if (newLabMode != LabMode::Ship) {
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
	for (auto subdialog : Subdialogs) {
		auto *dgo = new DialogOpener(subdialog, x, 0);
		subdialog->setOpener(dgo);
		auto *cbp = Class_toolbar->AddChild(dgo);
		x += cbp->GetWidth();

		subdialog->update(newLabMode, classIndex);
	}
}
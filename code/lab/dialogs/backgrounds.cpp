#include "lab/dialogs/backgrounds.h"
#include "lab/labv2_internal.h"

SCP_string get_directory_or_vp(const char* path)
{
	SCP_string result(path);

	// Is this a mission in a directory?
	size_t found = result.find("data" DIR_SEPARATOR_STR "missions");

	if (found == std::string::npos) // Guess not
	{
		found = result.find(".vp");
	}

	auto directory_name_pos = result.rfind(DIR_SEPARATOR_CHAR, found - strlen(DIR_SEPARATOR_STR) - 1);

	result = result.substr(directory_name_pos, found - directory_name_pos);

	found = result.find(DIR_SEPARATOR_CHAR);
	// Strip directory separators
	while (found != std::string::npos) {
		result.erase(found, strlen(DIR_SEPARATOR_STR));
		found = result.find(DIR_SEPARATOR_CHAR);
	}

	return result;
}

void labviewer_change_background(Tree* caller) {
	getLabManager()->Renderer->useBackground(caller->GetSelectedItem()->Name);
}

void BackgroundDialog::open(Button* /*caller*/) {
	if (dialogWindow != nullptr)
		return;

	dialogWindow = (DialogWindow*)getLabManager()->Screen->Add(new DialogWindow("Mission Backgrounds", gr_screen.center_offset_x + 250, gr_screen.center_offset_y + 50));
	Assert(Opener != nullptr);
	dialogWindow->SetOwner(Opener->getDialog());

	SCP_vector<SCP_string> missions;

	cf_get_file_list(missions, CF_TYPE_MISSIONS, NOX("*.fs2"));

	SCP_map<SCP_string, SCP_vector<SCP_string>> directories;

	for (const auto& filename : missions) {
		auto res = cf_find_file_location((filename + ".fs2").c_str(), CF_TYPE_MISSIONS);

		if (res.found) {
			auto location = get_directory_or_vp(res.full_name.c_str());

			directories[location].push_back(filename);
		}
	}

	auto Num_mission_directories = directories.size();
	auto Mission_directories = new TreeItem * [Num_mission_directories];

	Tree* missiontree = (Tree*)dialogWindow->AddChild(new Tree("Missions", 0, 0));
	missiontree->AddItem(nullptr, "None", 0, true, labviewer_change_background);

	int i = 0;
	for (auto const &directory : directories) {
		auto directoryItem = Mission_directories[i];
		directoryItem = missiontree->AddItem(nullptr, directory.first);

		for (const auto& Lab_mission : directory.second) {
			missiontree->AddItem(directoryItem, Lab_mission, 0, true, labviewer_change_background);
		}
	}
}
#include "WeaponsTBLViewerModel.h"

#include <weapon/weapon.h>
namespace fso::fred::dialogs {
WeaponsTBLViewerModel::WeaponsTBLViewerModel(QObject* parent, EditorViewport* viewport, int wc)
	: AbstractDialogModel(parent, viewport)
{
	initializeData(wc);
}
bool WeaponsTBLViewerModel::apply()
{
	return true;
}
void WeaponsTBLViewerModel::reject() {}
void WeaponsTBLViewerModel::initializeData(const int wc)
{
	char line[256], line2[256]{}, file_text[82]{};
	const weapon_info* sip = &Weapon_info[wc];
	int i, j, n, found = 0, comment = 0, num_files = 0;
	CFILE* fp = nullptr;
	SCP_vector<SCP_string> tbl_file_names;
	text.clear();

	if (!sip) {
		return;
	}

	fp = cfopen("weapons.tbl", "r");
	Assert(fp);

	while (cfgets(line, 255, fp)) {
		while (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = 0;

		for (i = j = 0; line[i]; i++) {
			if (line[i] == '/' && line[i + 1] == '/')
				break;
			if (line[i] == '/' && line[i + 1] == '*') {
				comment = 1;
				i++;
				continue;
			}

			if (line[i] == '*' && line[i + 1] == '/') {
				comment = 0;
				i++;
				continue;
			}

			if (!comment) {
				line2[j++] = line[i];
			}
		}

		line2[j] = 0;
		if (!strnicmp(line2, "$Name:", 6)) {
			drop_trailing_white_space(line2);
			found = 0;
			i = 6;

			while (line2[i] == ' ' || line2[i] == '\t' || line2[i] == '@')
				i++;

			if (!stricmp(line2 + i, sip->name)) {
				text += "-- weapons.tbl  -------------------------------\r\n";
				found = 1;
			}
		}

		if (found) {
			text += line;
			text += "\r\n";
		}
	}

	cfclose(fp);

	// done with ships.tbl, so now check all modular ship tables...
	num_files = cf_get_file_list(tbl_file_names, CF_TYPE_TABLES, NOX("*-wep.tbm"), CF_SORT_REVERSE);

	for (n = 0; n < num_files; n++) {
		tbl_file_names[n] += ".tbm";

		fp = cfopen(tbl_file_names[n].c_str(), "r");
		Assert(fp);

		memset(line, 0, sizeof(line));
		memset(line2, 0, sizeof(line2));
		found = 0;
		comment = 0;

		while (cfgets(line, 255, fp)) {
			while (line[strlen(line) - 1] == '\n')
				line[strlen(line) - 1] = 0;

			for (i = j = 0; line[i]; i++) {
				if (line[i] == '/' && line[i + 1] == '/')
					break;

				if (line[i] == '/' && line[i + 1] == '*') {
					comment = 1;
					i++;
					continue;
				}

				if (line[i] == '*' && line[i + 1] == '/') {
					comment = 0;
					i++;
					continue;
				}

				if (!comment)
					line2[j++] = line[i];
			}

			line2[j] = 0;
			if (!strnicmp(line2, "$Name:", 6)) {
				drop_trailing_white_space(line2);
				found = 0;
				i = 6;

				while (line2[i] == ' ' || line2[i] == '\t' || line2[i] == '@')
					i++;

				if (!stricmp(line2 + i, sip->name)) {
					memset(file_text, 0, sizeof(file_text));
					snprintf(file_text,
						sizeof(file_text) - 1,
						"--  %s  -------------------------------\r\n",
						tbl_file_names[n].c_str());
					text += file_text;
					found = 1;
				}
			}

			if (found) {
				text += line;
				text += "\r\n";
			}
		}

		cfclose(fp);
	}
	modelChanged();
}
SCP_string WeaponsTBLViewerModel::getText() const
{
	return text;
}
} // namespace fso::fred::dialogs
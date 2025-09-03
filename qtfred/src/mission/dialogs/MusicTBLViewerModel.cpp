#include "MusicTBLViewerModel.h"

#include <weapon/weapon.h>
namespace fso::fred::dialogs {
MusicTBLViewerModel::MusicTBLViewerModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	initializeData();
}
bool MusicTBLViewerModel::apply()
{
	return true;
}
void MusicTBLViewerModel::reject() {}
void MusicTBLViewerModel::initializeData()
{
	char line[256]{};
	CFILE* fp = nullptr;
	SCP_vector<SCP_string> tbl_file_names;

	text.clear();

	// Base table
	text += "-- music.tbl  -------------------------------\r\n";
	fp = cfopen("music.tbl", "r");
	Assert(fp);
	while (cfgets(line, 255, fp)) {
		text += line;
		text += "\r\n";
	}
	cfclose(fp);

	// Modular tables (*-mus.tbm), reverse sorted to match legacy behavior
	const int num_files = cf_get_file_list(tbl_file_names, CF_TYPE_TABLES, NOX("*-mus.tbm"), CF_SORT_REVERSE);
	for (int n = 0; n < num_files; ++n) {
		tbl_file_names[n] += ".tbm";

		text += "--  ";
		text += tbl_file_names[n];
		text += "  -------------------------------\r\n";

		fp = cfopen(tbl_file_names[n].c_str(), "r");
		Assert(fp);

		memset(line, 0, sizeof(line));
		while (cfgets(line, 255, fp)) {
			text += line;
			text += "\r\n";
		}
		cfclose(fp);
	}

	modelChanged();
}
SCP_string MusicTBLViewerModel::getText() const
{
	return text;
}
} // namespace fso::fred::dialogs
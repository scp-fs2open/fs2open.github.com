#include "MusicTBLViewerModel.h"

#include <utils/table_viewer.h>
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
	text = table_viewer::get_complete_table_text("music.tbl", "*-mus.tbm");
	modelChanged();
}
SCP_string MusicTBLViewerModel::getText() const
{
	return text;
}
} // namespace fso::fred::dialogs

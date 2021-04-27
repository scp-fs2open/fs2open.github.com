#include "CampaignEditorDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {



CampaignEditorDialogModel::CampaignMissionData::CampaignMissionData() {
	branches.emplace_back();
	it_branches = branches.begin();
}

CampaignEditorDialogModel::CampaignEditorDialogModel(CampaignEditorDialog* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport),
	_parent(parent),
	_campaignTechReset(false)
{
	connect(this, &AbstractDialogModel::modelChanged, this, &CampaignEditorDialogModel::flagModified);

	_missionData.emplace_back();
	_it_missionData = _missionData.begin();
}

bool CampaignEditorDialogModel::apply() {
	return saveTo(_currentFile);
}

void CampaignEditorDialogModel::reject() {
	// nothing to do if the dialog is created each time it's opened
}

bool CampaignEditorDialogModel::loadCurrentFile() {
	if (_currentFile.isEmpty())
		return false;
	//QFile f(_currentFile);

	return true;
}

bool CampaignEditorDialogModel::saveTo(const QString &file) {
	bool success = _saveTo(file);
	QMessageBox::information(_parent, file, success ? tr("Successfully saved") : tr("Error saving"));
	return success;
}

bool CampaignEditorDialogModel::_saveTo(const QString &file) {
	if (file.isEmpty())
		return false;
	//QFile f(file);

	return false;
}

}
}
}

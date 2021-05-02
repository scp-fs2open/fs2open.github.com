#include "CampaignEditorDialogModel.h"
#include <mission/missioncampaign.h>

namespace fso {
namespace fred {
namespace dialogs {



CampaignEditorDialogModel::CampaignMissionData::CampaignMissionData() {
	branches.emplace_back();
	it_branches = branches.begin();
}

CampaignEditorDialogModel::CampaignEditorDialogModel(QString file, CampaignEditorDialog* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport),
	_parent(parent),
	_currentFile(file)

{
	_fileLoaded = loadCurrentFile();

	_missionData.emplace_back();
	_it_missionData = _missionData.begin();
}

bool CampaignEditorDialogModel::apply() {
	return saveTo(_currentFile);
}

void CampaignEditorDialogModel::reject() {
	// nothing to do if the dialog is created each time it's opened
}

bool CampaignEditorDialogModel::saveTo(const QString &file) {
	bool success = _saveTo(file);
	QMessageBox::information(_parent, file, success ? tr("Successfully saved") : tr("Error saving"));
	return success;
}

//TODO retrieve constants
const QStringList CampaignEditorDialogModel::campaignTypes {
	campaign_types, &campaign_types[MAX_CAMPAIGN_TYPES]		//missioncampaign.h global
};

bool CampaignEditorDialogModel::loadCurrentFile() {
	if (_currentFile.isEmpty())
		return false;
	if (mission_campaign_load(qPrintable(_currentFile), nullptr, 0))
		return false;

	//missioncampaign.h globals
	_campaignName = Campaign.name;
	_campaignType = campaignTypes[Campaign.type];
	_campaignTechReset = Campaign.flags & CF_CUSTOM_TECH_DATABASE;
	_campaignDescr = Campaign.desc;
	_numPlayers = Campaign.num_players;

	return true;
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

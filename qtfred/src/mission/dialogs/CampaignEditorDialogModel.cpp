#include "CampaignEditorDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

CampaignEditorDialogModel::CampaignMissionData::CampaignMissionData() {
	branches.emplace_back();
	it_branches = branches.begin();
}

CampaignEditorDialogModel::CampaignEditorDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {
	connect(this, &AbstractDialogModel::modelChanged, this, &CampaignEditorDialogModel::flagModified);

	initializeData();
}

bool CampaignEditorDialogModel::apply() {

	return false;
}

void CampaignEditorDialogModel::reject() {
	// nothing to do if the dialog is created each time it's opened
}

void CampaignEditorDialogModel::initializeData() {
	_campaignName = "";
	_campaignType = "";
	_campaignTechReset = false;

	_missionData.emplace_back();
	_it_missionData = _missionData.begin();
}


}
}
}

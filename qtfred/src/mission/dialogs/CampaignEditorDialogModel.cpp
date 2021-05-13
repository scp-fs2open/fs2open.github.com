#include "CampaignEditorDialogModel.h"
#include <mission/missioncampaign.h>
#include "weapon/weapon.h"

namespace fso {
namespace fred {
namespace dialogs {

CampaignEditorDialogModel::CampaignMissionData::CampaignMissionData() {
	branches.emplace_back();
	it_branches = branches.begin();
}


static const QString loadFile(QString file) {
	if (file.isEmpty()) {
		mission_campaign_clear();
		return QString();
	}
	if (mission_campaign_load(qPrintable(file.replace('/',DIR_SEPARATOR_CHAR)), nullptr, 0))
		return QString();

	return Campaign.filename;
}

static CheckedDataListModel<int>::RowData initShips(SCP_vector<ship_info>::const_iterator &s_it){
	auto shpIdx = std::distance(Ship_info.cbegin(), s_it);
	return s_it->flags[Ship::Info_Flags::Player_ship] ?
			CheckedDataListModel<int>::RowData(s_it->name,
				shpIdx,
				Campaign.ships_allowed[shpIdx])
			: CheckedDataListModel<int>::RowData();

}

static CheckedDataListModel<int>::RowData initWeps(SCP_vector<weapon_info>::const_iterator &w_it){
	auto wepIdx = std::distance(Weapon_info.cbegin(), w_it);
	for (auto s_it = Ship_info.cbegin(); s_it != Ship_info.cend(); ++s_it)
		if (s_it->flags[Ship::Info_Flags::Player_ship]
				&& s_it->allowed_weapons[static_cast<size_t>(wepIdx)])
			return CheckedDataListModel<int>::RowData(w_it->name,
													  wepIdx,
													  Campaign.weapons_allowed[wepIdx]);
	return CheckedDataListModel<int>::RowData();
}


CampaignEditorDialogModel::CampaignEditorDialogModel(const QString &file, CampaignEditorDialog* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport),
	_parent(parent),
	_currentFile(loadFile(file)),
	initialShips(Ship_info.cbegin(), Ship_info.cend(), &initShips, this),
	initialWeapons(Weapon_info.cbegin(), Weapon_info.cend(), &initWeps, this)
{
	connect(&initialShips, &QAbstractListModel::dataChanged, this, &CampaignEditorDialogModel::flagModified);
	connect(&initialWeapons, &QAbstractListModel::dataChanged, this, &CampaignEditorDialogModel::flagModified);

	//missioncampaign.h globals
	_campaignName = Campaign.name;
	_campaignType = campaignTypes[Campaign.type];
	_campaignTechReset = Campaign.flags & CF_CUSTOM_TECH_DATABASE;
	_campaignDescr = Campaign.desc;
	_numPlayers = Campaign.num_players;

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
static QStringList initCampaignTypes(){
	QStringList ret;
	for (auto& tp: campaign_types) {  //missioncampaign.h global
		ret << tp;
	}
	return ret;
}
const QStringList CampaignEditorDialogModel::campaignTypes { initCampaignTypes() };

bool CampaignEditorDialogModel::_saveTo(QString file) {
	if (file.isEmpty())
		return false;
	//QFile f(file);

	return false;
}

}
}
}

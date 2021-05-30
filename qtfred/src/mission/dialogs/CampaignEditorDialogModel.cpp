#include "CampaignEditorDialogModel.h"
#include "weapon/weapon.h"
#include "cutscene/cutscenes.h"
#include "menuui/mainhallmenu.h"
#include "stats/scoring.h"

extern int Skip_packfile_search;
#define CMISSIONS_END &Campaign.missions[Campaign.num_missions]

namespace fso {
namespace fred {
namespace dialogs {

static QStringList initCutscenes() {
	QStringList ret{""};
	for (auto& cs: Cutscenes)
		ret << cs.filename;
	return ret;
}

static QStringList initMainhalls() {
	QStringList ret;
	for (auto& vec_mh: Main_hall_defines)
		for (auto& mh: vec_mh){
			QString name{ mh.name.c_str() };
			if (! ret.contains(name))
				ret << name;
		}
	return ret;
}

static QStringList initDebriefingPersonas() {
	QStringList ret{""};
	for (auto& rn: Ranks)
		for (auto& p: rn.promotion_text){
			QString persona{ QString::number(p.first) };
			if (p.first >= 0 && ! ret.contains(persona))
				ret << persona;
		}
	return ret;
}

static const QString loadFile(QString file, const QString& campaignType) {
	if (file.isEmpty()) {
		mission_campaign_clear();
		Campaign.type = campaignType.isEmpty() ? CAMPAIGN_TYPE_SINGLE :
												 CampaignEditorDialogModel::campaignTypes.indexOf(campaignType);
		return QString();
	}
	Assert(campaignType.isEmpty());
	if (mission_campaign_load(qPrintable(file.replace('/',DIR_SEPARATOR_CHAR)), nullptr, 0))
		return QString();

	return Campaign.filename;
}

static CheckedDataListModel<std::ptrdiff_t>::RowData initShips(const SCP_vector<ship_info>::const_iterator &s_it){
	std::ptrdiff_t shpIdx{ std::distance(Ship_info.cbegin(), s_it) };
	return CheckedDataListModel<std::ptrdiff_t>::RowData{
			s_it->flags[Ship::Info_Flags::Player_ship] ? s_it->name : "",
			shpIdx,
			static_cast<bool>(Campaign.ships_allowed[static_cast<size_t>(shpIdx)])};
}

static CheckedDataListModel<std::ptrdiff_t>::RowData initWeps(const SCP_vector<weapon_info>::const_iterator &w_it){
	std::ptrdiff_t wepIdx{ std::distance(Weapon_info.cbegin(), w_it) };
	for (auto s_it = Ship_info.cbegin(); s_it != Ship_info.cend(); ++s_it)
		if (s_it->flags[Ship::Info_Flags::Player_ship]
			&& s_it->allowed_weapons[static_cast<size_t>(wepIdx)])
			return CheckedDataListModel<std::ptrdiff_t>::RowData{
					w_it->name,
					wepIdx,
					static_cast<bool>(Campaign.weapons_allowed[static_cast<size_t>(wepIdx)])};
	return CheckedDataListModel<std::ptrdiff_t>::RowData();
}

static SCP_vector<SCP_string> getMissions(){
	Skip_packfile_search = 1;
	SCP_vector<SCP_string> missions;
	cf_get_file_list(missions, CF_TYPE_MISSIONS, "*fs2", CF_SORT_NAME);
	Skip_packfile_search = 0;
	return missions;
}

static inline bool isCampaignCompatible(const mission &fsoMission){
	return (Campaign.type == CAMPAIGN_TYPE_SINGLE && fsoMission.game_type & (MISSION_TYPE_SINGLE|MISSION_TYPE_TRAINING))
			|| (Campaign.type == CAMPAIGN_TYPE_MULTI_COOP && fsoMission.game_type & MISSION_TYPE_MULTI_COOP)
			|| (Campaign.type == CAMPAIGN_TYPE_MULTI_TEAMS && fsoMission.game_type & MISSION_TYPE_MULTI_TEAMS);
}

CampaignEditorDialogModel::CampaignMissionData::CampaignMissionData(const QString &file, const cmission &cm) :
	filename(file),
	briefingCutscene(cm.briefing_cutscene),
	mainhall(cm.main_hall.c_str()),
	debriefingPersona(QString::number(cm.debrief_persona_index))
{
	branches.emplace_back();
	it_branches = branches.begin();
}

CampaignEditorDialogModel::CampaignMissionData::CampaignMissionData(const QString &file) :
	filename(file)
{
	branches.emplace_back();
	it_branches = branches.begin();
}

CheckedDataListModel<std::unique_ptr<CampaignEditorDialogModel::CampaignMissionData>>::RowData
CampaignEditorDialogModel::CampaignMissionData::initMissions(const SCP_vector<SCP_string>::const_iterator &m_it){

	const QString filename{ QString::fromStdString(*m_it).append(".fs2") };
	const cmission *cm_it{
		std::find_if(Campaign.missions, CMISSIONS_END,
				[&](const cmission &cm){ return filename == cm.name; })};

	std::unique_ptr<CampaignMissionData> ret_data{
		cm_it == CMISSIONS_END
				? new CampaignMissionData{filename}
				: new CampaignMissionData{filename, *cm_it}};

	ret_data->fredable = !get_mission_info(m_it->c_str(), &ret_data->fsoMission);
	if (! ret_data->fredable)
		QMessageBox::warning(nullptr, "Error loading mission", "Could not get info from mission: " + ret_data->filename +"\nFile corrupted?");

	return CheckedDataListModel<std::unique_ptr<CampaignMissionData>>::RowData{
		isCampaignCompatible(ret_data->fsoMission) ? filename : "",
		ret_data,
		cm_it != CMISSIONS_END,
		ret_data->fredable ? Qt::color0 : Qt::red};
}

CampaignEditorDialogModel::CampaignEditorDialogModel(CampaignEditorDialog* parent, EditorViewport* viewport, const QString &file, const QString& newCampaignType) :
	AbstractDialogModel(parent, viewport),
	_parent(parent),
	cutscenes(initCutscenes()),
	mainhalls(initMainhalls()),
	debriefingPersonas(initDebriefingPersonas()),
	campaignFile(loadFile(file, newCampaignType)),
	campaignType(campaignTypes[Campaign.type]),
	initialShips(Ship_info, &initShips, this),
	initialWeapons(Weapon_info, &initWeps, this),
	missionData(getMissions(), &CampaignMissionData::initMissions, this),
	_campaignName(Campaign.name),		//missioncampaign.h globals
	_campaignDescr(Campaign.desc),
	_campaignTechReset(Campaign.flags & CF_CUSTOM_TECH_DATABASE)
{
	for (int i=0; i<Campaign.num_missions; i++)
		if (! missionData.contains(Campaign.missions[i].name)) {
			std::unique_ptr<CampaignMissionData> ptr{
				new CampaignMissionData{Campaign.missions[i].name}
			};

			bool loaded = !get_mission_info(Campaign.missions[i].name, &ptr->fsoMission);
			ptr->fredable = false;

			missionData.addRow(Campaign.missions[i].name, ptr, true,
							   loaded ? Qt::darkYellow : Qt::red);
		}

	connect(&initialShips, &QAbstractListModel::dataChanged, this, &CampaignEditorDialogModel::flagModified);
	connect(&initialWeapons, &QAbstractListModel::dataChanged, this, &CampaignEditorDialogModel::flagModified);
	connect(&missionData, &QAbstractListModel::dataChanged, this, &CampaignEditorDialogModel::modelChanged);
	connect(&missionData, &QAbstractListModel::dataChanged, this, &CampaignEditorDialogModel::flagModified);
	connect(&missionData, &QAbstractListModel::dataChanged, this, &CampaignEditorDialogModel::checkMissionDrop);
}

bool CampaignEditorDialogModel::apply() {

	return saveTo(campaignFile);
}

void CampaignEditorDialogModel::reject() {
	// nothing to do if the dialog is created each time it's opened
}


bool CampaignEditorDialogModel::saveTo(const QString &file) {
	bool success = _saveTo(file);
	QMessageBox::information(_parent, file, success ? tr("Successfully saved") : tr("Error saving"));
	return success;
}

const CampaignEditorDialogModel::CampaignMissionData CampaignEditorDialogModel::mdEmpty{""};

static QStringList initCampaignTypes() {
	QStringList ret;
	for (auto& tp: campaign_types) {  //missioncampaign.h global
		ret << tp;
	}
	return ret;
}
const QStringList CampaignEditorDialogModel::campaignTypes { initCampaignTypes() };

void CampaignEditorDialogModel::checkMissionDrop(const QModelIndex &idx, const QModelIndex &bottomRight, const QVector<int> &roles) {
	Assert(idx == bottomRight);

	if (roles.contains(Qt::CheckStateRole)	&& ! missionData.internalData(idx)->fredable) {
		if (missionData.data(idx, Qt::CheckStateRole).toBool()) {
			droppedMissions.removeAll(missionData.data(idx).toString());
		} else {
			droppedMissions.append(missionData.data(idx).toString());
		}
	}
}

void CampaignEditorDialogModel::missionSelectionChanged(const QModelIndex &changed) {
	mnData_it = missionData.internalData(changed).get();
	mnData_idx = changed;
	_parent->updateUI();
}

bool CampaignEditorDialogModel::_saveTo(QString file) {
	if (file.isEmpty())
		return false;
	//auto path = qPrintable(file.replace('/',DIR_SEPARATOR_CHAR));

	return false;
}

}
}
}

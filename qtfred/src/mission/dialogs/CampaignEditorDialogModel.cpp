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
	for (auto& vec_mh: Main_hall_defines) {
		for (auto& mh: vec_mh){
			QString name{ mh.name.c_str() };
			if (! ret.contains(name))
				ret << name;
		}
}
	return ret;
}

static QStringList initDebriefingPersonas() {
	QStringList ret{""};
	for (auto& rn: Ranks) {
		for (auto& p: rn.promotion_text){
			QString persona{ QString::number(p.first) };
			if (p.first >= 0 && ! ret.contains(persona))
				ret << persona;
		}
}
	return ret;
}

static QString loadFile(QString file, const QString& campaignType) {
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

static void initShips(const SCP_vector<ship_info>::const_iterator &s_it, CheckedDataListModel<std::ptrdiff_t> &model){
	std::ptrdiff_t shpIdx{ std::distance(Ship_info.cbegin(), s_it) };
	if (s_it->flags[Ship::Info_Flags::Player_ship]) {
		model.initRow(
				s_it->name,
				shpIdx,
				static_cast<bool>(Campaign.ships_allowed[static_cast<size_t>(shpIdx)]));
}
}

static void initWeps(const SCP_vector<weapon_info>::const_iterator &w_it, CheckedDataListModel<std::ptrdiff_t> &model){
	std::ptrdiff_t wepIdx{ std::distance(Weapon_info.cbegin(), w_it) };
	for (const ship_info& shp: Ship_info) {
		if (shp.flags[Ship::Info_Flags::Player_ship]
			&& shp.allowed_weapons[static_cast<size_t>(wepIdx)]
			&& !model.contains(w_it->name)) {
			model.initRow(
					w_it->name,
					wepIdx,
					static_cast<bool>(Campaign.weapons_allowed[static_cast<size_t>(wepIdx)]));
}
}
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

CampaignEditorDialogModel::CampaignLoopData::CampaignLoopData(const cmission *loop) :
	is(loop)
{
	if (loop) {
		descr = loop->mission_branch_desc;
		anim = loop->mission_branch_brief_anim;
		voice = loop->mission_branch_brief_sound;
	}
}

CampaignEditorDialogModel::CampaignBranchData::CampaignBranchData(const int &sexp_branch, const QString &from, const cmission *_loop) :
	sexp(CAR(sexp_branch)),
	loop(_loop)
{
	int node_next = CADR(sexp_branch);
	if (!stricmp(CTEXT(node_next), "end-of-campaign")) {
		type = END;
	} else if (!stricmp(CTEXT(node_next), "next-mission")) {
		next = CTEXT(CDR(node_next));
		type = (from == next) ? REPEAT : NEXT_NOT_FOUND;
	}
}

void CampaignEditorDialogModel::CampaignBranchData::connect(const SCP_unordered_set<const CampaignMissionData*>& missions) {
	if (std::find_if(missions.cbegin(), missions.cend(),
					 [&](const CampaignMissionData* mn){
						return next == mn->filename; })
			== missions.cend())
		type = NEXT_NOT_FOUND;
	else
		type = NEXT;
}


const SCP_map<CampaignEditorDialogModel::CampaignBranchData::BranchType, QString> CampaignEditorDialogModel::CampaignBranchData::branchTexts{
	{INVALID, ""}, {REPEAT, "Repeat mission "}, {NEXT, "Branch to "}, {NEXT_NOT_FOUND, "Branch to "}, {END, "End of Campaign"}
};

CampaignEditorDialogModel::CampaignMissionData::CampaignMissionData(QString file, const cmission &cm) :
	filename(std::move(file)),
	briefingCutscene(cm.briefing_cutscene),
	mainhall(cm.main_hall.c_str()),
	debriefingPersona(QString::number(cm.debrief_persona_index))
{
	branchesFromFormula(cm.formula);
	branchesFromFormula(cm.mission_loop_formula, &cm);
}

CampaignEditorDialogModel::CampaignMissionData::CampaignMissionData(QString file) :
	filename(std::move(file))
{

}

void CampaignEditorDialogModel::CampaignMissionData::initMissions(
		const SCP_vector<SCP_string>::const_iterator &m_it,
		CheckedDataListModel<CampaignEditorDialogModel::CampaignMissionData> &model)
{
	const QString filename{ QString::fromStdString(*m_it).append(".fs2") };
	const cmission *cm_it{
		std::find_if(Campaign.missions, CMISSIONS_END,
				[&](const cmission &cm){ return filename == cm.name; })};

	CampaignMissionData* ret_data{
		cm_it == CMISSIONS_END
				? new CampaignMissionData{filename}
				: new CampaignMissionData{filename, *cm_it} };

	ret_data->fredable = !get_mission_info(m_it->c_str(), &ret_data->fsoMission);
	if (! ret_data->fredable)
		QMessageBox::warning(nullptr, "Error loading mission", "Could not get info from mission: " + ret_data->filename +"\nFile corrupted?");

	if(isCampaignCompatible(ret_data->fsoMission)) {
		model.initRow(
			filename,
			ret_data,
			cm_it != CMISSIONS_END,
			ret_data->fredable ? Qt::color0 : Qt::red);
	}
}

void CampaignEditorDialogModel::CampaignMissionData::branchesFromFormula(int formula, const cmission *loop) {
	if ( formula < 0 || stricmp(CTEXT(formula), "cond"))
		return;

	for (int it_cond_arm = CDR(formula);
		 it_cond_arm != -1;
		 it_cond_arm = CDR(it_cond_arm) )
		branches.emplace_back(CAR(it_cond_arm), filename, loop);
}

CampaignEditorDialogModel::CampaignEditorDialogModel(CampaignEditorDialog* _parent, EditorViewport* viewport, const QString &file, const QString& newCampaignType) :
	AbstractDialogModel(_parent, viewport),
	parent(_parent),
	cutscenes(initCutscenes()),
	mainhalls(initMainhalls()),
	debriefingPersonas(initDebriefingPersonas()),
	campaignFile(loadFile(file, newCampaignType)),
	campaignType(campaignTypes[Campaign.type]),
	initialShips(Ship_info, &initShips, this),
	initialWeapons(Weapon_info, &initWeps, this),
	missionData(getMissions(), &CampaignMissionData::initMissions, this),
	campaignName(Campaign.name),		//missioncampaign.h globals
	campaignDescr(Campaign.desc),
	campaignTechReset(Campaign.flags & CF_CUSTOM_TECH_DATABASE)
{
	for (int i=0; i<Campaign.num_missions; i++) {
		if (! missionData.contains(Campaign.missions[i].name)) {
			CampaignMissionData* ptr{ new CampaignMissionData{Campaign.missions[i].name} };

			bool loaded = !get_mission_info(Campaign.missions[i].name, &ptr->fsoMission);
			ptr->fredable = false;

			missionData.initRow(Campaign.missions[i].name, ptr, true,
							   loaded ? Qt::darkYellow : Qt::red);
		}
	}

	connectBranches(false);

	connect(&initialShips, &QAbstractListModel::dataChanged, this, &CampaignEditorDialogModel::flagModified);
	connect(&initialWeapons, &QAbstractListModel::dataChanged, this, &CampaignEditorDialogModel::flagModified);
	connect(&missionData, &QAbstractListModel::dataChanged, this, [&](){connectBranches();});
	connect(&missionData, &QAbstractListModel::dataChanged, this, &CampaignEditorDialogModel::flagModified);
	connect(&missionData, &QAbstractListModel::dataChanged, this, &CampaignEditorDialogModel::checkMissionDrop);
}

bool CampaignEditorDialogModel::apply() {

	return saveTo(campaignFile);
}

void CampaignEditorDialogModel::reject() {
	// nothing to do if the dialog is created each time it's opened
}

void CampaignEditorDialogModel::setCurBrIsLoop(bool isLoop) {
	if (! (mnData_it && mnData_it->brData_it)) return;
	modify<bool>(mnData_it->brData_it->loop.is, isLoop);
	parent->updateUIMission();}

void CampaignEditorDialogModel::setCurLoopAnim(const QString &anim) {
	if (! (mnData_it && mnData_it->brData_it)) return;
	modify<QString>(mnData_it->brData_it->loop.anim, anim);
	parent->updateUIBranch();}

void CampaignEditorDialogModel::setCurLoopVoice(const QString &voice) {
	if (! (mnData_it && mnData_it->brData_it)) return;
	modify<QString>(mnData_it->brData_it->loop.voice, voice);
	parent->updateUIBranch();}

bool CampaignEditorDialogModel::saveTo(const QString &file) {
	bool success = _saveTo(file);
	QMessageBox::information(parent, file, success ? tr("Successfully saved") : tr("Error saving"));
	return success;
}

const CampaignEditorDialogModel::CampaignBranchData CampaignEditorDialogModel::CampaignMissionData::bdEmpty{};
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

	if (roles.contains(Qt::CheckStateRole)	&& ! missionData.internalData(idx).fredable) {
		if (missionData.data(idx, Qt::CheckStateRole).toBool()) {
			droppedMissions.removeAll(missionData.data(idx).toString());
		} else {
			droppedMissions.append(missionData.data(idx).toString());
		}
	}
}

void CampaignEditorDialogModel::connectBranches(bool uiUpdate) {
	for (auto& mn: missionData.getCheckedData())
		for (auto& br: mn->branches)
			if (br.type == CampaignBranchData::NEXT || br.type == CampaignBranchData::NEXT_NOT_FOUND)
				br.connect(missionData.getCheckedDataConst());
	if (uiUpdate)
		parent->updateUIMission();
}

void CampaignEditorDialogModel::missionSelectionChanged(const QModelIndex &changed) {
	if (mnData_it)
		mnData_it->brData_it = nullptr;
	mnData_it = &missionData.internalData(changed);
	mnData_idx = changed;
	parent->updateUIMission();
}

void CampaignEditorDialogModel::selectCurBr(const CampaignBranchData *br) {
	if (! mnData_it) return;
	mnData_it->brData_it = const_cast<CampaignBranchData*>(br);
	parent->updateUIBranch();
}

//placeholder
bool CampaignEditorDialogModel::_saveTo(QString file) const {
	if (file.isEmpty())
		return false;
	qPrintable(file.replace('/',DIR_SEPARATOR_CHAR));
	getCampaignDescr();
	return false;
}

}
}
}

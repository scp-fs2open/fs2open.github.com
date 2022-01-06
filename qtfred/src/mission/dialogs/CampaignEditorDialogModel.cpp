#include "CampaignEditorDialogModel.h"
#include "weapon/weapon.h"
#include "cutscene/cutscenes.h"
#include "menuui/mainhallmenu.h"
#include "stats/scoring.h"
#include "mission/missiongoals.h"
#include "mission/missionsave.h"

#include <QPlainTextDocumentLayout>

extern int Skip_packfile_search;

extern void parse_init(bool basic);
extern void parse_mission_info(mission *mn, bool basic);
extern void parse_events(mission *mn);
extern void parse_goals(mission* mn);
extern int Subsys_status_size;

namespace fso {
namespace fred {
namespace dialogs {

static QString loadFile(QString file, const QString& campaignType) {
	if (file.isEmpty()) {
		mission_campaign_clear();
		Campaign.type = campaignType.isEmpty() ? CAMPAIGN_TYPE_SINGLE :
												 CampaignEditorDialogModel::campaignTypes.indexOf(campaignType);
		return QString();
	}
	//FRED is to enforce that only on new campaigns a campaign type may be given
	Assert(campaignType.isEmpty());
	parse_init(false);
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

static bool parseMnPart(mission *mn, const char *filename){
	try {
		read_file_text(filename, CF_TYPE_MISSIONS);

		parse_init(false);
		Subsys_index = 0;
		Subsys_status_size = 0;
		if (Subsys_status != nullptr) {
			vm_free( Subsys_status );
			Subsys_status = nullptr;
		}

		parse_mission_info(mn, true);
		skip_to_start_of_string("#Events");
		Num_mission_events = 0;
		parse_events(mn);
		Num_goals = 0;
		parse_goals(mn);
	}  catch (const parse::ParseException& ) {
		return false;
	}
	return true;
}

CampaignEditorDialogModel::CampaignEditorDialogModel(CampaignEditorDialog* _parent, EditorViewport* viewport, const QString &file, const QString& newCampaignType) :
	AbstractDialogModel(_parent, viewport),
	parent(_parent),
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
			mission temp{};
			bool loaded{parseMnPart(&temp, qPrintable(Campaign.missions[i].name))};

			CampaignMissionData* ptr{
				new CampaignMissionData{
					Campaign.missions[i].name, loaded, &temp, &Campaign.missions[i]
				}
			};

			ptr->fredable = false;

			missionData.initRow(Campaign.missions[i].name, ptr, true,
							   loaded ? Qt::darkYellow : Qt::red);
		}
	}

	//reparse campaign after parsing missions
	parse_init(false);
	if (!campaignFile.isEmpty()){
		QString temp = campaignFile;
		bool reloaded = !mission_campaign_load(qPrintable(temp.replace('/',DIR_SEPARATOR_CHAR)), nullptr, 0);
		Assertion(reloaded, "Campaign file should still be loadable");
		connectBranches(false, &Campaign);
	}

	connect(&campaignDescr, &QTextDocument::contentsChanged, this, &CampaignEditorDialogModel::flagModified);
	connect(&initialShips, &QAbstractListModel::dataChanged, this, &CampaignEditorDialogModel::flagModified);
	connect(&initialWeapons, &QAbstractListModel::dataChanged, this, &CampaignEditorDialogModel::flagModified);
	connect(&missionData, &QAbstractListModel::rowsAboutToBeInserted, this, [&](){mnData_it = nullptr; mnData_idx = QModelIndex();});
	connect(&missionData, &QAbstractListModel::dataChanged, this, [&](){connectBranches();});
	connect(&missionData, &QAbstractListModel::dataChanged, this, &CampaignEditorDialogModel::flagModified);
	connect(&missionData, &QAbstractListModel::dataChanged, this, &CampaignEditorDialogModel::checkMissionDrop);
}

void CampaignEditorDialogModel::supplySubModels(QListView &ships, QListView &weps, QListView &missions, QPlainTextEdit &descr) {
	ships.setModel(&initialShips);
	weps.setModel(&initialWeapons);
	missions.setModel(&missionData);

	campaignDescr.associateEdit(&descr);
}

QStringList CampaignEditorDialogModel::getMissionGoals(const QString& /*model knows best*/) {
	QString reference_name;
	if (getCurBr())
		reference_name = Sexp_nodes[CDR(getCurBr()->sexp)].text;
	else
		return QStringList{};
	for (auto mn : missionData)
		if (reference_name == mn.first.filename)
			return mn.first.goals;
	return QStringList{};
}

QStringList CampaignEditorDialogModel::getMissionEvents(const QString& /*model knows best*/) {
	QString reference_name;
	if (getCurBr())
		reference_name = Sexp_nodes[CDR(getCurBr()->sexp)].text;
	else
		return QStringList{};
	for (auto mn : missionData)
		if (reference_name == mn.first.filename)
			return mn.first.events;
	return QStringList{};
}

QStringList CampaignEditorDialogModel::getMissionNames() {
	QStringList ret{};
	ret.reserve(static_cast<int>(missionData.getCheckedData().size()));
	for (auto mn : missionData)
		if (mn.second)
			ret << mn.first.filename;
	return ret;
}

QList<QAction *> CampaignEditorDialogModel::getContextMenuExtras(QObject *menu_parent) {
	QList<QAction *> ret{};

	int curBrIdx = getCurBrIdx();
	QAction *moveUpAct{new QAction{tr("Move Up"), menu_parent}};
	moveUpAct->setEnabled(curBrIdx > 0);
	connect(moveUpAct, &QAction::triggered, this, [this](){moveCurBr(true);});
	ret << moveUpAct;

	QAction *moveDownAct{new QAction{tr("Move Down"), menu_parent}};
	moveDownAct->setEnabled(curBrIdx >= 0 && curBrIdx + 1 < getCurMnBrCnt());
	connect(moveDownAct, &QAction::triggered, this, [this](){moveCurBr(false);});
	ret << moveDownAct;

	QAction *toggleLoopAct{new QAction{tr("Toggle Loop"), menu_parent}};
	toggleLoopAct->setEnabled(curBrIdx >= 0);
	toggleLoopAct->setCheckable(true);
	toggleLoopAct->setChecked(getCurBrIsLoop());
	connect(toggleLoopAct, &QAction::toggled, this, &CampaignEditorDialogModel::setCurBrIsLoop);
	ret << toggleLoopAct;

	return ret;
}

int CampaignEditorDialogModel::getCampaignNumPlayers() const {
	if (campaignType == campaignTypes[0])
		return 0;
	auto checked = missionData.getCheckedData();
	if (checked.empty())
		return 0;
	return (**checked.cbegin()).nPlayers;
}

void CampaignEditorDialogModel::connectBranches(bool uiUpdate, const campaign *cpgn) {
	for (auto& mn: missionData.getCheckedData()) {
		if (cpgn) {
			const cmission *const cm_it{
				std::find_if(cpgn->missions, &cpgn->missions[cpgn->num_missions],
						[&](const cmission &cm){ return mn->filename == cm.name; })};
			if (cm_it != &cpgn->missions[cpgn->num_missions]) {
				mn->branchesFromFormula(this, cm_it->formula);
				mn->branchesFromFormula(this, cm_it->mission_loop_formula, cm_it);
			}
		}
		for (auto& br: mn->branches)
			if (br.type == CampaignBranchData::NEXT || br.type == CampaignBranchData::NEXT_NOT_FOUND)
				br.connect(const_cast<const CheckedDataListModel<CampaignMissionData>&>(missionData).getCheckedData());
	}
	if (uiUpdate)
		parent->updateUIMission();
}

bool CampaignEditorDialogModel::fillTree(sexp_tree& sxt) const {
	if (!mnData_it) return false;
	int i = 0;
	for (const CampaignBranchData& br : mnData_it->branches) {
		NodeImage img;
		if (br.type == CampaignBranchData::NEXT_NOT_FOUND)
			img = NodeImage::ROOT_DIRECTIVE;
		else if (br.loop)
			img = NodeImage::ROOT;
		else
			img = NodeImage::BLACK_DOT;
		QTreeWidgetItem *h = sxt.insert(CampaignBranchData::branchTexts.at(br.type) + br.next, img);
		h->setData(0, Qt::UserRole, i++);
		sxt.add_sub_tree(sxt.load_sub_tree(br.sexp, true, "do-nothing"), h);
	}
	mnData_it->brData_it = nullptr;
	mnData_it->brData_idx = -1;
	return true;
}

void CampaignEditorDialogModel::supplySubModelLoop(QPlainTextEdit &descr) {
	if (getCurBr())
		getCurBr()->loopDescr->associateEdit(&descr);
	else
		descr.setDocument(nullptr);
}

bool CampaignEditorDialogModel::saveTo(const QString &file) {
	bool success = _saveTo(file);
	QMessageBox::information(parent, file, success ? tr("Successfully saved") : tr("Error saving"));
	modified = ! success;
	//TODO error checker
	return success;
}

void CampaignEditorDialogModel::checkMissionDrop(const QModelIndex &idx, const QModelIndex &bottomRight, const QVector<int> &roles) {
	Assert(idx == bottomRight);
	Assert(missionData.internalData(idx));

	if (roles.contains(Qt::CheckStateRole)	&& ! missionData.internalData(idx)->fredable) {
		if (missionData.data(idx, Qt::CheckStateRole).toBool()) {
			droppedMissions.removeAll(missionData.data(idx).toString());
		} else {
			droppedMissions.append(missionData.data(idx).toString());
		}
	}
}

bool CampaignEditorDialogModel::_saveTo(QString file) const {
	if (file.isEmpty())
		return false;

	mission_campaign_clear();

	qstrncpy(Campaign.name, qPrintable(campaignName), NAME_LENGTH);

	Campaign.type = campaignTypes.indexOf(campaignType);

	if (! campaignDescr.isEmpty())
		Campaign.desc = vm_strdup(qPrintable(campaignDescr.toPlainText()));

	//Defined as number of players of first mission
	Campaign.num_players = getCampaignNumPlayers();

	Campaign.flags = CF_DEFAULT_VALUE;
	if (campaignTechReset)
		Campaign.flags |= CF_CUSTOM_TECH_DATABASE;

	for (auto shp_idx_ptr : initialShips.getCheckedData()) {
		Assertion(shp_idx_ptr, "NULL ship class index in initial ships");
		Assertion(*shp_idx_ptr < MAX_SHIP_CLASSES, "Illegal ship class index in initial ships: %ld", *shp_idx_ptr);
		Campaign.ships_allowed[*shp_idx_ptr] = true;
	}

	for (auto wep_idx_ptr : initialWeapons.getCheckedData()) {
		Assertion(wep_idx_ptr, "NULL weapon class index in initial weapons");
		Assertion(*wep_idx_ptr < MAX_WEAPON_TYPES, "Illegal weapon class index in initial weapons: %ld", *wep_idx_ptr);
		Campaign.weapons_allowed[*wep_idx_ptr] = true;
	}

	CFred_mission_save save;
	return !save.save_campaign_file(qPrintable(file.replace('/',DIR_SEPARATOR_CHAR)));
}

void CampaignEditorDialogModel::missionSelectionChanged(const QItemSelection & selected) {
	if (mnData_it)
		mnData_it->brData_it = nullptr;
	if (selected.empty()) {
		mnData_it = nullptr;
		mnData_idx = QPersistentModelIndex();
	} else {
		const QPersistentModelIndex &changed = selected.first().topLeft();
		mnData_it = missionData.internalData(changed);
		mnData_idx = changed;
	}
	parent->updateUIMission();
}

int CampaignEditorDialogModel::addCurMnBranchTo(const QModelIndex *other, bool flip) {
	if (! mnData_it)
		return -1;
	if (! other) {
		mnData_it->branches.emplace_back(this, mnData_it->filename);

		flagModified();
		return static_cast<int>(mnData_it->branches.size() -1);
	}
	CampaignMissionData *otherMn = missionData.internalData(*other);
	if (! otherMn)
		return -1;
	CampaignMissionData &from = flip ? *otherMn : *mnData_it;
	const CampaignMissionData &to = flip ? *mnData_it : *otherMn;
	from.branches.emplace_back(this, from.filename, to.filename);

	flagModified();
	return static_cast<int>(mnData_it->branches.size() -1);
}

void CampaignEditorDialogModel::delCurMnBranch(int node) {
	if (! mnData_it) return;

	mnData_it->branches.erase(mnData_it->branches.cbegin() + node);
	flagModified();

	parent->updateUIMission();
}

void CampaignEditorDialogModel::selectCurBr(const QTreeWidgetItem *selected) {
	if (! mnData_it) return;
	mnData_it->brData_it = nullptr;
	mnData_it->brData_idx = -1;

	if (!selected) return;

	const QTreeWidgetItem *parent_node;
	while ((parent_node = selected->parent()))
		selected = parent_node;
	mnData_it->brData_idx = selected->data(0, Qt::UserRole).toInt();
	auto idx = static_cast<size_t>(mnData_it->brData_idx);
	Assert(idx < mnData_it->branches.size());
	mnData_it->brData_it = &mnData_it->branches[idx];

	parent->updateUIBranch();
}

int CampaignEditorDialogModel::setCurBrCond(const QString &sexp, const QString &mn, const QString &arg) {
	if (! getCurBr()) return -1;

	mnData_it->brData_it->sexp =
			alloc_sexp(qPrintable(sexp), SEXP_ATOM, SEXP_ATOM_OPERATOR, -1,
					   alloc_sexp(qPrintable(mn), SEXP_ATOM, SEXP_ATOM_STRING, -1,
								  alloc_sexp(qPrintable(arg), SEXP_ATOM, SEXP_ATOM_STRING, -1, -1)));
	flagModified();
	return getCurBrIdx();
}

bool CampaignEditorDialogModel::setCurBrSexp(int sexp) {
	if (! getCurBr()) return false;

	mnData_it->brData_it->sexp = sexp;
	flagModified();
	return true;
}

void CampaignEditorDialogModel::setCurBrIsLoop(bool isLoop) {
	if (! getCurBr()) return;
	if (isLoop)
		for (auto& br : mnData_it->branches)
			if (br.loop) {
				if (QMessageBox::question(parent, "Change loop", "This will make branch to "+br.next+" no longer a loop. Continue?") == QMessageBox::StandardButton::No)
					return;
				modify<bool>(br.loop, false);
			}

	modify<bool>(mnData_it->brData_it->loop, isLoop);
	int idx = getCurBrIdx();
	parent->updateUIMission(false);
	parent->updateUIBranch(idx);
}

void CampaignEditorDialogModel::moveCurBr(bool up) {
	if (! getCurBr()) return;

	auto idx = static_cast<size_t>(mnData_it->brData_idx);
	if (idx == 0 && up) return;

	size_t other_idx = up ? idx - 1 : idx + 1;
	auto& brs = mnData_it->branches;
	if (brs.size() == other_idx) return;

	std::swap(brs[idx], brs[other_idx]);
	flagModified();

	parent->updateUIMission(false);
	parent->updateUIBranch(static_cast<int>(other_idx));
}

void CampaignEditorDialogModel::setCurLoopAnim(const QString &anim) {
	if (! (mnData_it && mnData_it->brData_it)) return;
	modify<QString>(mnData_it->brData_it->loopAnim, anim);
	parent->updateUIBranch();
}

void CampaignEditorDialogModel::setCurLoopVoice(const QString &voice) {
	if (! (mnData_it && mnData_it->brData_it)) return;
	modify<QString>(mnData_it->brData_it->loopVoice, voice);
	parent->updateUIBranch();
}

static inline QStringList getParsedEvts() {
	QStringList ret;
	for (int i = 0; i < Num_mission_events; i++)
		ret << Mission_events[i].name;
	return ret;
}

static inline QStringList getParsedGoals() {
	QStringList ret;
	for (int i = 0; i < Num_goals; i++)
		ret << Mission_goals[i].name;
	return ret;
}

static inline bool isCampaignCompatible(const mission &fsoMission) {
	return (Campaign.type == CAMPAIGN_TYPE_SINGLE && fsoMission.game_type & (MISSION_TYPE_SINGLE|MISSION_TYPE_TRAINING))
			|| (Campaign.type == CAMPAIGN_TYPE_MULTI_COOP && fsoMission.game_type & MISSION_TYPE_MULTI_COOP)
			|| (Campaign.type == CAMPAIGN_TYPE_MULTI_TEAMS && fsoMission.game_type & MISSION_TYPE_MULTI_TEAMS);
}

CampaignEditorDialogModel::CampaignMissionData::CampaignMissionData(QString file, bool loaded, const mission *fsoMn, const cmission *cm) :
	filename(std::move(file)),
	fredable(loaded),
	nPlayers(loaded ? fsoMn->num_players : 0),
	notes(loaded ? fsoMn->notes : ""),
	events(loaded ? getParsedEvts() : QStringList{}),
	goals(loaded ? getParsedGoals() : QStringList{}),
	briefingCutscene(cm ? cm->briefing_cutscene : ""),
	mainhall(cm ? cm->main_hall.c_str() : ""),
	debriefingPersona(cm ? QString::number(cm->debrief_persona_index) : "")
{
	if (cm && (cm->flags & CMISSION_FLAG_HAS_FORK))
		QMessageBox::warning(nullptr, "Unsupported campaign feature", "This campaign uses scpFork, which is nonfunctional in FSO and unsupported in FRED. Use axemFork instead.\nAffected mission: " + filename);
	if (cm && (cm->flags & CMISSION_FLAG_BASTION))
		QMessageBox::warning(nullptr, "Unsupported campaign feature", "This campaign uses Bastion mainhall flag, which is outdated. Use explicit mainhall settings.\nAffected mission: " + filename);
}

void CampaignEditorDialogModel::CampaignMissionData::initMissions(
		const SCP_vector<SCP_string>::const_iterator &m_it,
		CheckedDataListModel<CampaignEditorDialogModel::CampaignMissionData> &model)
{
	const QString filename{ QString::fromStdString(*m_it).append(".fs2") };
	const cmission * cm_it{
		std::find_if(Campaign.missions, &Campaign.missions[Campaign.num_missions],
				[&](const cmission &cm){ return filename == cm.name; })};
	if (cm_it == &Campaign.missions[Campaign.num_missions])
		cm_it = nullptr;

	mission temp{};
	bool loaded{parseMnPart(&temp, qPrintable(filename))};

	if (! isCampaignCompatible(temp))
		return;

	CampaignMissionData* data{
		new CampaignMissionData{ filename, loaded, &temp, cm_it}
	};

	if (! loaded)
		QMessageBox::warning(nullptr, "Error loading mission", "Could not get info from mission: " + filename +"\nFile corrupted?");

	model.initRow(filename,	data, cm_it, loaded ? Qt::color0 : Qt::red);
}

void CampaignEditorDialogModel::CampaignMissionData::branchesFromFormula(CampaignEditorDialogModel *model, int formula, const cmission *loop) {
	if ( formula < 0 || stricmp(CTEXT(formula), "cond"))
		return;

	for (int it_cond_arm = CDR(formula);
		 it_cond_arm != -1;
		 it_cond_arm = CDR(it_cond_arm) )
		branches.emplace_back(model, CAR(it_cond_arm), filename, loop);
}

CampaignEditorDialogModel::CampaignBranchData::CampaignBranchData(CampaignEditorDialogModel *model, int sexp_branch, const QString &from, const cmission *_loop) :
	sexp(CAR(sexp_branch)),
	loop(_loop),
	loopDescr(new AssociatedPlainTextDocument(_loop ? _loop->mission_branch_desc : "", model))
{
	int node_next = CADR(sexp_branch);
	if (!stricmp(CTEXT(node_next), "end-of-campaign")) {
		type = END;
	} else if (!stricmp(CTEXT(node_next), "next-mission")) {
		next = CTEXT(CDR(node_next));
		type = (from == next) ? REPEAT : NEXT_NOT_FOUND;
	}

	QObject::connect(loopDescr, &QTextDocument::contentsChanged, model, &CampaignEditorDialogModel::flagModified);
	if (loop) {
		loopAnim = _loop->mission_branch_brief_anim;
		loopVoice = _loop->mission_branch_brief_sound;
	}

}

CampaignEditorDialogModel::CampaignBranchData::CampaignBranchData(CampaignEditorDialogModel *model, const QString &from, QString to) :
	type(to.isEmpty() ? END : to == from ? REPEAT : NEXT),
	sexp(Locked_sexp_true),
	next(std::move(to)),
	loop(false),
	loopDescr(new AssociatedPlainTextDocument("", model))
{
	QObject::connect(loopDescr, &QTextDocument::contentsChanged, model, &CampaignEditorDialogModel::flagModified);
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

static inline QStringList initCutscenes() {
	QStringList ret{""};
	for (auto& cs: Cutscenes)
		ret << cs.filename;
	return ret;
}

const QStringList& CampaignEditorDialogModel::cutscenes() {
	static QStringList ret{ initCutscenes() };
	return ret;
}

static inline QStringList initMainhalls() {
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

const QStringList& CampaignEditorDialogModel::mainhalls() {
	static QStringList ret{ initMainhalls() };
	return ret;
}

static inline QStringList initDebriefingPersonas() {
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

const QStringList& CampaignEditorDialogModel::debriefingPersonas() {
	static QStringList ret{ initDebriefingPersonas() };
	return ret;
}

static inline QStringList initFilelist(const int type) {
	QStringList ret{""};
	SCP_vector<SCP_string> fl;
	cf_get_file_list(fl, type, "*", CF_SORT_NAME, nullptr, CF_LOCATION_TYPE_PRIMARY_MOD);
	for (auto& f: fl)
		ret << QString::fromStdString(f);
	return ret;
}

const QStringList& CampaignEditorDialogModel::loopAnims() {
	static QStringList ret{ initFilelist(CF_TYPE_INTERFACE) }; // as per campaigneditordlg.cpp:810
	return ret;
}

const QStringList& CampaignEditorDialogModel::loopVoices() {
	static QStringList ret{ initFilelist(CF_TYPE_VOICE_CMD_BRIEF) }; // as per campaigneditordlg.cpp:832
	return ret;
}

static inline QStringList initCampaignTypes() {
	QStringList ret;
	for (auto& tp: campaign_types) {  //missioncampaign.h global
		ret << tp;
	}
	return ret;
}

const QStringList CampaignEditorDialogModel::campaignTypes { initCampaignTypes() };

}
}
}

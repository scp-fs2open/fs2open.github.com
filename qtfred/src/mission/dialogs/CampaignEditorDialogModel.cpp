#include "CampaignEditorDialogModel.h"
#include "weapon/weapon.h"
#include "cutscene/cutscenes.h"
#include "menuui/mainhallmenu.h"
#include "stats/scoring.h"
#include "mission/missiongoals.h"
#include "mission/missionsave.h"

#include <QMessageBox>
#include <QPlainTextDocumentLayout>

//from cfilesystem.cpp, to find loose (non-vp) mission files, as oldfred did (campaignfilelistbox.cpp:58)
extern int Skip_packfile_search;

//from missionparse.cpp, to parse relevant parts of available mission files only (parseMnPart)
extern void parse_init(bool basic);
extern void parse_mission_info(mission *mn, bool basic);
extern void parse_events(mission *mn);
extern void parse_goals(mission* mn);
extern int Subsys_status_size;

namespace fso {
namespace fred {
namespace dialogs {

namespace  {
QString loadOrCreateFile(QString file, const QString& campaignType) {
	if (file.isEmpty()) {
		mission_campaign_clear();
		Campaign.type = campaignType.isEmpty() ? CAMPAIGN_TYPE_SINGLE :
												 CampaignEditorDialogModel::campaignTypes.indexOf(campaignType);
		return {};
	}
	//FRED is to enforce that only on new campaigns a campaign type may be given
	Assertion(campaignType.isEmpty(), "The editor should only allow setting a campaign type when a new campaign is created. Please report.");
	parse_init(false);
	if (mission_campaign_load(qPrintable(file.replace('/',DIR_SEPARATOR_CHAR)), nullptr, 0))
		return {};

	return Campaign.filename;
}

void initShips(const SCP_vector<ship_info>::const_iterator &s_it, CheckedDataListModel<std::ptrdiff_t> &model){
	Assertion(s_it < Ship_info.cend(), "Attempting to access a value outside of Ship_info. Please report.");
	std::ptrdiff_t shpIdx{ std::distance(Ship_info.cbegin(), s_it) };
	if (s_it->flags[Ship::Info_Flags::Player_ship]) {
		model.initRow(
				s_it->name,
				shpIdx,
				static_cast<bool>(Campaign.ships_allowed[static_cast<size_t>(shpIdx)]));
	}
}

void initWeps(const SCP_vector<weapon_info>::const_iterator &w_it, CheckedDataListModel<std::ptrdiff_t> &model){
	Assertion(w_it < Weapon_info.cend(), "Attempting to access a value outside of Weapon_info. Please report.");
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

SCP_vector<SCP_string> getMissions(){
	Skip_packfile_search = 1;
	SCP_vector<SCP_string> missions;
	// #8a check: duplicate mission: get loose mission files once
	cf_get_file_list(missions, CF_TYPE_MISSIONS, "*fs2", CF_SORT_NAME);
	Skip_packfile_search = 0;
	return missions;
}

bool parseMnPart(mission *mn, const char *filename){
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
	} catch ( const parse::ParseException& ) {
		return false;
	}
	return true;
}
} //namespace

CampaignEditorDialogModel::CampaignEditorDialogModel(CampaignEditorDialog* _parent, EditorViewport* viewport, const QString &file, const QString& newCampaignType, int _numPlayers) :
	AbstractDialogModel(_parent, viewport),
	campaignFile(loadOrCreateFile(file, newCampaignType)),
	campaignType(campaignTypes[Campaign.type]),
	numPlayers(campaignFile.isEmpty() ? (Campaign.num_players = _numPlayers) : Campaign.num_players),
	parent(_parent),
	initialShips(Ship_info, &initShips, this),
	initialWeapons(Weapon_info, &initWeps, this),
	missionData(getMissions(), &CampaignMissionData::initMissions, this),
	campaignName(Campaign.name),		//missioncampaign.h globals
	campaignDescr(Campaign.desc),
	campaignTechReset(Campaign.flags & CF_CUSTOM_TECH_DATABASE)
{
	Assertion(_numPlayers == 0 || campaignFile.isEmpty(), "The editor should only allow setting a player number when a new campaign is created. Please report.");

	for (int i=0; i<Campaign.num_missions; i++) {
		// #8 check: duplicate mission: don't add mission if name already present
		if (! missionData.contains(Campaign.missions[i].name)) {
			mission temp{};
			bool loaded{parseMnPart(&temp, qPrintable(Campaign.missions[i].name))};

			CampaignMissionData* mnDataPtr{ // temporary handle, until the missionData submodel takes ownership
				new CampaignMissionData{
					Campaign.missions[i].name, loaded, &temp, &Campaign.missions[i]
				}
			};

			mnDataPtr->fredable = false;

			// #11 check: multi player number: skip and warn on load
			if (campaignTypes.indexOf(campaignType) != CAMPAIGN_TYPE_SINGLE && numPlayers != temp.num_players) {
				CampaignEditorDialog::uiWarn(tr("Potential Campaign Bug"), tr("Mission %1 of campaign has wrong player number: %2 and was removed.").arg(temp.name, temp.num_players));
				delete mnDataPtr;
				continue;
			}

			missionData.initRow(Campaign.missions[i].name, mnDataPtr, true,
							   loaded ? Qt::darkYellow : Qt::red);
		}
	}

	// #9 check: no first mission: enforced here
	if (Campaign.num_missions > 0)
		firstMission = Campaign.missions[0].name;

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
	connect(&missionData, &QAbstractListModel::dataChanged, this, &CampaignEditorDialogModel::trackMissionUncheck);
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
		return {};
	for (auto mn : missionData)
		if (reference_name == mn.first.filename)
			return mn.first.goals;
	return {};
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
	return {};
}

QStringList CampaignEditorDialogModel::getMissionNames() {
	QStringList ret;
	ret.reserve(static_cast<int>(missionData.collectCheckedData().size()));
	for (auto mn : missionData)
		if (mn.second)
			ret << mn.first.filename;
	return ret;
}

QList<QAction *> CampaignEditorDialogModel::getContextMenuExtras(QObject *menu_parent) {
	QList<QAction *> ret;

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
	toggleLoopAct->setChecked(isCurBrLoop());
	connect(toggleLoopAct, &QAction::toggled, this, &CampaignEditorDialogModel::setCurBrIsLoop);
	ret << toggleLoopAct;

	return ret;
}

bool CampaignEditorDialogModel::fillTree(sexp_tree& sxt) const {
	if (!mnData_it)
		return false;
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
	CampaignEditorDialog::uiWarn(file, success ? tr("Successfully saved") : tr("Error saving"));
	modified = ! success;
	return success;
}

void CampaignEditorDialogModel::missionSelectionChanged(const QItemSelection & selected) {
	if (mnData_it)
		mnData_it->brData_it = nullptr;
	if (selected.empty()) {
		mnData_it = nullptr;
		mnData_idx = QPersistentModelIndex();
	} else {
		const QPersistentModelIndex &changed = selected.first().topLeft();
		mnData_it = missionData.managedData(changed);
		mnData_idx = changed;
	}
	parent->updateUIMission();
}

void CampaignEditorDialogModel::setCurMnFirst(){
	if (! mnData_it)
		return;
	QMessageBox::StandardButton resBtn = QMessageBox::Yes;
	if (! firstMission.isEmpty()) {
		resBtn = QMessageBox::question( parent, tr("Change first mission?"),
										tr("Do you want to replace\n%1\nas first mission?").arg(firstMission),
										QMessageBox::Yes | QMessageBox::No,
										QMessageBox::Yes );
	}

	if (resBtn == QMessageBox::Yes)
		modify<QString>(firstMission, mnData_it->filename);
}

int CampaignEditorDialogModel::addCurMnBranchTo(const QModelIndex *other, bool flip) {
	if (! mnData_it)
		return -1;
	if (! other) {
		mnData_it->branches.emplace_back(this, mnData_it->filename);

		flagModified();
		return static_cast<int>(mnData_it->branches.size() -1);
	}
	CampaignMissionData *otherMn = missionData.managedData(*other);
	if (! otherMn)
		return -1;
	CampaignMissionData &from = flip ? *otherMn : *mnData_it;
	const CampaignMissionData &to = flip ? *mnData_it : *otherMn;
	from.branches.emplace_back(this, from.filename, to.filename);

	flagModified();
	return static_cast<int>(mnData_it->branches.size() -1);
}

void CampaignEditorDialogModel::delCurMnBranch(int node) {
	if (! mnData_it)
		return;

	mnData_it->branches.erase(mnData_it->branches.cbegin() + node);
	flagModified();

	parent->updateUIMission();
}

void CampaignEditorDialogModel::selectCurBr(const QTreeWidgetItem *selected) {
	if (! mnData_it)
		return;
	mnData_it->brData_it = nullptr;
	mnData_it->brData_idx = -1;

	if (!selected)
		return;

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
	if (! getCurBr())
		return -1;

	mnData_it->brData_it->sexp =
			alloc_sexp(qPrintable(sexp), SEXP_ATOM, SEXP_ATOM_OPERATOR, -1,
					   alloc_sexp(qPrintable(mn), SEXP_ATOM, SEXP_ATOM_STRING, -1,
								  alloc_sexp(qPrintable(arg), SEXP_ATOM, SEXP_ATOM_STRING, -1, -1)));
	flagModified();
	return getCurBrIdx();
}

bool CampaignEditorDialogModel::setCurBrSexp(int sexp) {
	if (! getCurBr())
		return false;

	// #5 check: always false branch: reject bad manual edit
	if (sexp == Locked_sexp_false) {
		CampaignEditorDialog::uiWarn(tr("Potential Campaign Bug"), tr("Attempt to set campaign branch condition to false rejected"));
		return false;
	}

	mnData_it->brData_it->sexp = sexp;
	flagModified();
	return true;
}

void CampaignEditorDialogModel::setCurBrIsLoop(bool isLoop) {
	if (! getCurBr())
		return;
	if (isLoop) {
		for (auto& br : mnData_it->branches) {
			if (br.loop) {
				if (QMessageBox::question(parent, tr("Change loop"), tr("This will make branch to %1 no longer a loop. Continue?").arg(br.next)) == QMessageBox::StandardButton::No)
					return;
				modify<bool>(br.loop, false);
			}
		}
	}

	modify<bool>(mnData_it->brData_it->loop, isLoop);
	int idx = getCurBrIdx();
	parent->updateUIMission(false);
	parent->updateUIBranch(idx);
}

void CampaignEditorDialogModel::moveCurBr(bool up) {
	if (! getCurBr())
		return;

	auto idx = static_cast<size_t>(mnData_it->brData_idx);
	if (idx == 0 && up)
		return;

	size_t other_idx = up ? idx - 1 : idx + 1;
	auto& brs = mnData_it->branches;
	if (brs.size() == other_idx)
		return;

	std::swap(brs[idx], brs[other_idx]);
	flagModified();

	parent->updateUIMission(false);
	parent->updateUIBranch(static_cast<int>(other_idx));
}

void CampaignEditorDialogModel::setCurLoopAnim(const QString &anim) {
	if (! (mnData_it && mnData_it->brData_it))
		return;
	modify<QString>(mnData_it->brData_it->loopAnim, anim);
	parent->updateUIBranch();
}

void CampaignEditorDialogModel::setCurLoopVoice(const QString &voice) {
	if (! (mnData_it && mnData_it->brData_it))
		return;
	modify<QString>(mnData_it->brData_it->loopVoice, voice);
	parent->updateUIBranch();
}

void CampaignEditorDialogModel::connectBranches(bool uiUpdate, const campaign *cpgn) {
	for (auto *mn: missionData.collectCheckedData()) {
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
				br.connect(const_cast<const CheckedDataListModel<CampaignMissionData>&>(missionData).collectCheckedData());
	}
	if (uiUpdate)
		parent->updateUIMission();
}

bool CampaignEditorDialogModel::_saveTo(QString file) const {
	if (file.isEmpty())
		return false;

	mission_campaign_clear();

	strncpy(Campaign.name, qPrintable(campaignName), NAME_LENGTH);

	Campaign.type = campaignTypes.indexOf(campaignType);

	if (! campaignDescr.isEmpty())
		Campaign.desc = vm_strdup(qPrintable(campaignDescr.toPlainText()));

	Campaign.num_players = numPlayers;

	Campaign.flags = CF_DEFAULT_VALUE;
	if (campaignTechReset)
		Campaign.flags |= CF_CUSTOM_TECH_DATABASE;

	for (auto shp_idx_ptr : initialShips.collectCheckedData()) {
		Assertion(shp_idx_ptr, "NULL ship class index in initial ships");
		Assertion(*shp_idx_ptr < MAX_SHIP_CLASSES, "Illegal ship class index in initial ships: %ld", *shp_idx_ptr);
		Campaign.ships_allowed[*shp_idx_ptr] = true;
	}

	for (auto wep_idx_ptr : initialWeapons.collectCheckedData()) {
		Assertion(wep_idx_ptr, "NULL weapon class index in initial weapons");
		Assertion(*wep_idx_ptr < MAX_WEAPON_TYPES, "Illegal weapon class index in initial weapons: %ld", *wep_idx_ptr);
		Campaign.weapons_allowed[*wep_idx_ptr] = true;
	}

	if (firstMission.length() > 0) {
		static const QString PAST_BRANCHES{};
		SCP_unordered_set<const CampaignMissionData*> unsaved{missionData.collectCheckedData()};
		SCP_queue<const QString*> saveNext{};
		int i=0, lvl=0, pos=0;

		saveNext.push(&firstMission);
		do {
			// traversal
			if (saveNext.empty())
				saveNext.push(&(*unsaved.cbegin())->filename);
			while (! saveNext.empty()) {
				const QString *mnName = saveNext.front();
				saveNext.pop();

				if (mnName == &PAST_BRANCHES) {
					if (pos > 0) {
						++lvl;
						pos = 0;}
					continue;
				}

				auto it = std::find_if(unsaved.cbegin(), unsaved.cend(),
									   [&](const CampaignMissionData *mn_ptr) {
					 return mn_ptr->filename == *mnName;	});
				const CampaignMissionData *mn;
				if (it != unsaved.cend()) {
					mn = *it;
					unsaved.erase(it);
				}
				else {
					continue;
				}

				for (const auto &br : mn->branches)
					saveNext.push(&br.next);
				saveNext.push(&PAST_BRANCHES);

				// saving
				cmission &cm = Campaign.missions[i++];

				cm.name = vm_strdup(qPrintable(*mnName));
				strncpy(cm.briefing_cutscene, qPrintable(mn->briefingCutscene), NAME_LENGTH);
				cm.main_hall = mn->mainhall.toStdString();

				//Bastion flag unsupported in missionLoad
				cm.flags = 0;
				cm.debrief_persona_index = static_cast<ubyte>(mn->debriefingPersona.toUShort());

				using BranchType = CampaignBranchData::BranchType;
				cm.formula =
						alloc_sexp("cond", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
				int *cond_arms_ptr = &Sexp_nodes[cm.formula].rest;
				bool flag_last_branch = false;
				for (const auto &br : mn->branches) {
					Assertion(br.type != BranchType::INVALID, "UI should not let any branch remain invalid");
					// #2 check: Illegal target mission
					if (br.type == BranchType::NEXT_NOT_FOUND)
						CampaignEditorDialog::uiWarn(tr("Potential Campaign Bug"), tr("Saving branch to unknown mission:\n%1 to %2").arg(mn->filename, br.next));
					if (! br.loop) { //build formula from nonloop branches
						// #6 check: True middle branch
						if (flag_last_branch)
							CampaignEditorDialog::uiWarn(tr("Potential Campaign Bug"), tr("Branch is unreachable due to previous \"true\" condition:\n%1 to %2").arg(mn->filename, br.next));
						if (br.sexp == Locked_sexp_true)
							flag_last_branch = true;

						int nextsexp;
						if (br.type != BranchType::END) {
							nextsexp = alloc_sexp("next-mission", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1,
												  alloc_sexp(qPrintable(br.next), SEXP_ATOM, SEXP_ATOM_STRING, -1, -1));
						} else {
							nextsexp =  alloc_sexp("end-of-campaign", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
						}

						*cond_arms_ptr =
								alloc_sexp("", SEXP_LIST, -1,
										alloc_sexp("", SEXP_LIST, -1, br.sexp,
												   alloc_sexp("", SEXP_LIST, -1, nextsexp, -1)), -1);

						cond_arms_ptr = &Sexp_nodes[*cond_arms_ptr].rest;
					} else { //flag & save for loop
						Assertion(cm.flags ^ CMISSION_FLAG_HAS_LOOP, "UI should have stopped attempt at multiple loops");
						// #4 check: always true loop
						if (br.sexp == Locked_sexp_true)
							CampaignEditorDialog::uiWarn(tr("Potential Campaign Bug"), tr("Loop is always true from mission: %1 to %2").arg(mn->filename, br.next));
						cm.flags |= CMISSION_FLAG_HAS_LOOP;

						QString descr = br.loopDescr->toPlainText();
						if (descr.isEmpty())
							cm.mission_branch_desc = nullptr;
						else
							cm.mission_branch_desc = vm_strdup(qPrintable(descr));

						if (br.loopAnim.isEmpty())
							cm.mission_branch_brief_anim = nullptr;
						else
							cm.mission_branch_brief_anim = vm_strdup(qPrintable(br.loopAnim));

						if (br.loopVoice.isEmpty())
							cm.mission_branch_brief_sound = nullptr;
						else
							cm.mission_branch_brief_sound = vm_strdup(qPrintable(br.loopVoice));

						int nextsexp =
								alloc_sexp("next-mission", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1,
										   alloc_sexp(qPrintable(br.next), SEXP_ATOM, SEXP_ATOM_STRING, -1, -1));
						int cond_arms =
								alloc_sexp("", SEXP_LIST, -1,
										alloc_sexp("", SEXP_LIST, -1, br.sexp,
												   alloc_sexp("", SEXP_LIST, -1, nextsexp, -1)), -1);
						cm.mission_loop_formula =
								alloc_sexp("cond", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, cond_arms);
					}
				}
				// #7 check: not always true last branch
				if (!flag_last_branch)
					CampaignEditorDialog::uiWarn(tr("Potential Campaign Bug"), tr("Last branch is not always true from mission: %1").arg(mn->filename));

				cm.level = lvl;
				cm.pos = pos++;
			}
		} while (! unsaved.empty());
		Campaign.num_missions = i;
	}

	CFred_mission_save save;
	return !save.save_campaign_file(qPrintable(file.replace('/',DIR_SEPARATOR_CHAR)));
}

bool CampaignEditorDialogModel::deleteLinksTo(const CampaignMissionData &target) {
	SCP_vector<std::pair<CampaignMissionData *, int>> del;
	QString msg = tr("The following missions have links to the removed mission (%1):\n").arg(target.filename);
	for (auto mn_it: missionData) {
		auto &mn = mn_it.first;
		if (&mn == &target)
			continue;
		for (auto br_it = mn.branches.cbegin(); br_it != mn.branches.cend(); ++br_it) {
			if (br_it->next == target.filename) {
				del.emplace_back(&mn, br_it - mn.branches.cbegin());
				msg.append(mn.filename).append('\n');
			}
		}
	}

	if (del.empty())
		return true;
	if (QMessageBox::StandardButton::No ==
			QMessageBox::question(parent, tr("Remove links to mission?"), msg + tr("Do you want to remove them?")))
		return false;

	CampaignMissionData *bup_mnData_it = mnData_it;
	for (auto del_it = del.rbegin(); del_it != del.rend(); ++del_it) {
		mnData_it = del_it->first;
		delCurMnBranch(del_it->second);
	}
	mnData_it = bup_mnData_it;
	return true;
}

void CampaignEditorDialogModel::trackMissionUncheck(const QModelIndex &idx, const QModelIndex &bottomRight, const QVector<int> &roles) {
	Assert(idx == bottomRight);
	Assert(missionData.managedData(idx));

	if (roles.contains(Qt::CheckStateRole)) {
		const auto mn = missionData.managedData(idx);
		bool checked = missionData.data(idx, Qt::CheckStateRole).toBool();

		if (! missionData.collectCheckedData().empty()) {
			//must always have first mission, or no missions
			// #9 check: no first mission: enforced here
			if (! checked) {
				if (mn->filename == firstMission) {
					CampaignEditorDialog::uiWarn(tr("First Mission"), tr("You cannot remove the first mission of a campaign,\nunless it is the only one. Choose another to be first."));
					missionData.setData(idx, Qt::Checked, Qt::CheckStateRole);
					return;
				}
				// #3 check: illegal target mission: remove illegal links
				if (! deleteLinksTo(*mn)){
					CampaignEditorDialog::uiWarn(tr("Target Mission"), tr("A mission cannot be removed unless all links to it are deleted."));
					missionData.setData(idx, Qt::Checked, Qt::CheckStateRole);
					return;
				}
			} else if (firstMission == "" && missionData.collectCheckedData().size() == 1){
				firstMission = mn->filename;
			}
		} else {
			firstMission = "";
		}

		//as unfredable (=packaged/missing) missions are only loaded when specified
		//in the campaign file, unchecking them will make them unreachable after save/reload.
		//Warn on save if that happens.
		if(! mn->fredable) {
			if (checked) {
				droppedMissions.removeAll(mn->filename);
			} else {
				droppedMissions.append(mn->filename);
			}
		}
	}
}

namespace { //helpers for CampaignMissionData construction
	inline QStringList getParsedEvts() {
		QStringList ret;
		for (int i = 0; i < Num_mission_events; i++)
			ret << Mission_events[i].name;
		return ret;
	}

	inline QStringList getParsedGoals() {
		QStringList ret;
		for (int i = 0; i < Num_goals; i++)
			ret << Mission_goals[i].name;
		return ret;
	}

	// #11 check: multi player number: exclude wrong number
	inline bool isCampaignCompatible(const mission &fsoMission) {
		return (Campaign.type == CAMPAIGN_TYPE_SINGLE && fsoMission.game_type & (MISSION_TYPE_SINGLE|MISSION_TYPE_TRAINING))
				|| (Campaign.type == CAMPAIGN_TYPE_MULTI_COOP && fsoMission.game_type & MISSION_TYPE_MULTI_COOP && Campaign.num_players == fsoMission.num_players)
				|| (Campaign.type == CAMPAIGN_TYPE_MULTI_TEAMS && fsoMission.game_type & MISSION_TYPE_MULTI_TEAMS && Campaign.num_players == fsoMission.num_players);
	}
} //namespace

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
		CampaignEditorDialog::uiWarn(tr("Unsupported campaign feature"), tr("This campaign uses scpFork, which is nonfunctional in FSO and unsupported in FRED. Use axemFork instead.\nAffected mission: %1").arg(filename));
	if (cm && (cm->flags & CMISSION_FLAG_BASTION))
		CampaignEditorDialog::uiWarn(tr("Unsupported campaign feature"), tr("This campaign uses Bastion mainhall flag, which is outdated. Use explicit mainhall settings.\nAffected mission: %1").arg(filename));
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

	CampaignMissionData* data{  // temporary handle, until the missionData submodel takes ownership
		new CampaignMissionData{ filename, loaded, &temp, cm_it}
	};

	if (! loaded)
		CampaignEditorDialog::uiWarn(tr("Error loading mission"), tr("Could not get info from mission: %1\nFile corrupted?").arg(filename));

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

	// #5a check: always false branch: Warn & prevent on load
	if (sexp == Locked_sexp_false) {
		sexp = Locked_sexp_true;
		CampaignEditorDialog::uiWarn(tr("Potential Campaign Bug"), tr("Branch from %1 to %2 was always false. Set to always true, please fix.").arg(from, next));
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

/*
 * Other campaign checks:
 * #1 check: illegal source mission: does not apply, the mission owns the branch
 * #3 check: branch sexp is assigned results of alloc_sexp (with safe params), sexp_tree::save_tree or loading a file exclusively
 * #10 check: duplicate first mission: does not apply, only one possible value
 *
*/

} // namespace dialogs
} // namespace fred
} // namespace fso

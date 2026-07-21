#include "CampaignEditorDialog.h"
#include "ui_CampaignEditorDialog.h"
#include "ui/Theme.h"

#include <globalincs/globals.h>
#include "mission/commands/FredCommands.h"
#include "mission/missioncampaign.h"
#include "ui/widgets/sexp_tree_view.h"
#include "ui/widgets/SimpleListSelectDialog.h"
#include "ui/util/default_dir.h"
#include "ui/util/DialogUndo.h"
#include "ui/util/SignalBlockers.h"
#include "mission/util.h"
#include <ui/dialogs/MissionSpecs/CustomDataDialog.h>
#include <QFileInfo>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QAbstractItemView>
#include <ui/FredView.h>

namespace fso::fred::dialogs {

CampaignEditorDialog::CampaignEditorDialog(FredView* parent, EditorViewport* viewport)
	: QMainWindow(parent), SexpTreeEditorInterface({TreeFlags::LabeledRoot, TreeFlags::RootDeletable}),
	  ui(new Ui::CampaignEditorDialog), _viewport(viewport), _fredView(parent)
{
	ui->setupUi(this);

	// The campaign editor is file-scoped: no apply step, so the stack is the
	// only undo history and is cleared on New/Open/Save/close. setupDialogUndo
	// adds Edit -> Undo/Redo to the existing menu bar.
	_dialogStack = new QUndoStack(this);
	_fredView->undoGroup()->addStack(_dialogStack);
	util::setupDialogUndo(this, _fredView->undoGroup(), _dialogStack, tr("Campaign"));

	ui->nameLineEdit->setMaxLength(NAME_LENGTH - 1);
	ui->loopAnimLineEdit->setMaxLength(MAX_FILENAME_LEN - 1);
	ui->loopVoiceLineEdit->setMaxLength(MAX_FILENAME_LEN - 1);

	// Build the Qt adapter for our data model
	// This is kinda messy but the sexp_tree_view widget owns both the ui and the data for the tree
	// Simultaneously our tree model needs to be able to tell the tree when things change and also
	// be able to read data from the tree as needed. So we pass in this small adapter object with
	// the relevant tree operations allowing the model to do all the cross talk it needs
	struct QtCampaignTreeOps final : ICampaignEditorTreeOps {
		explicit QtCampaignTreeOps(sexp_tree_view& t) : tree(t) {}
		sexp_tree_view& tree;

		int loadSexp(int formula_index) override
		{
			// The sexp_tree_view's load_sub_tree is what creates the internal model for a branch
			return tree._model.load_sub_tree(formula_index, true, "true");
		}

		int saveSexp(int internal_node_id) override
		{
			// First, find the root of the branch that contains the edited node.
			int root_node = tree.get_root(internal_node_id);

			// Now, save the entire branch starting from its root.
			return tree._model.save_tree(root_node);
		}

		int createDefaultSexp() override
		{
			// A default branch is just a "true" condition. We load it from an invalid index.
			return tree._model.load_sub_tree(-1, true, "true");
		}

		void clearTree() override
		{
			// Drops both the internal nodes and the visuals; used before a
			// working-state restore reloads every branch of every mission.
			tree.clear_tree("");
		}

		void rebuildBranchTree(const SCP_vector<CampaignBranchData>& branches, const SCP_string& currentMissionName) override
		{
			// Reset the visual tree
			tree.clear();

			for (const auto& branch : branches) {
				// Determine caption (original FRED uses a generic "Branch to ..." except for END/self-loop)
				QString rootText;
				if (branch.next_mission_name.empty()) {
					rootText = QStringLiteral("End of Campaign");
				} else {
					rootText = QStringLiteral("Branch to %1").arg(QString::fromStdString(branch.next_mission_name));
				}

				// For selfloops, use a special caption
				if (!branch.next_mission_name.empty() && branch.next_mission_name == currentMissionName) {
					rootText = QStringLiteral("Repeat mission");
				}

				// Pick the icon
				const auto icon = (branch.is_loop || branch.is_fork) ? fso::fred::NodeImage::ROOT : fso::fred::NodeImage::BLACK_DOT;

				// Insert the visual root row with icon and add
				QTreeWidgetItem* rootItem = tree.insert(rootText, icon);
				rootItem->setData(0, sexp_tree_view::FormulaDataRole, branch.sexp_formula);
				tree.add_sub_tree(branch.sexp_formula, rootItem);
			}
		}

		void expandBranch(int internal_node_id) override
		{
			// Find the visual tree item corresponding to the internal node
			for (int i = 0; i < tree.topLevelItemCount(); ++i) {
				auto* item = tree.topLevelItem(i);
				if (item && item->data(0, sexp_tree_view::FormulaDataRole).toInt() == internal_node_id) {
					// Call the widget's expand_branch method
					tree.expand_branch(item);
					break;
				}
			}
		}
	};

	_treeOps = std::make_unique<QtCampaignTreeOps>(QtCampaignTreeOps{*ui->sxtBranches});

	ui->sxtBranches->initializeEditor(_viewport->editor, this, _viewport, parent);
	ui->sxtBranches->clear_tree();
	ui->sxtBranches->_model.post_load();

	// Now construct the model with reference to tree ops
	_model = std::make_unique<CampaignEditorDialogModel>(this, _viewport, *_treeOps);

	ui->mainTabs->setCurrentIndex(0); // Ensure the first tab is selected

	ui->graphView->setModel(_model.get());

	// Connect sexp tree signals. A branch delete is handled in two stages:
	// this direct handler runs at emit time, before the widget frees the
	// branch, so it can capture the pre-delete state for the undo snapshot
	// and mute the modified() the widget emits after freeing.
	connect(ui->sxtBranches, &sexp_tree_view::rootNodeDeleted, this, [this](int) {
		_branchDeleteBefore = _model->captureWorkingState();
		_branchDeletePending = true;
	});

	// Stage two runs queued, outside the widget's delete handler, and does
	// the model/UI work before pushing the snapshot.
	connect(
		ui->sxtBranches,
		&sexp_tree_view::rootNodeDeleted,
		this,
		[this](int formulaNodeId) {
			int mission_selection = _model->getCurrentMissionSelection(); // save now because rebuild clears it

			// Remove the branch
			_model->removeBranchByTreeId(formulaNodeId);

			// Rebuild the graph view
			ui->graphView->rebuildAll();
			ui->graphView->setSelectedMission(mission_selection);

			updateLoopDetails();

			_branchDeletePending = false;
			pushWorkingStateSnapshot(_branchDeleteBefore, tr("Delete Branch"));
			_branchDeleteBefore.clear();
		},
		Qt::QueuedConnection);

	// A tree edit can replace a branch condition's root node; keep the
	// branch's stored tree id in step.
	connect(ui->sxtBranches, &sexp_tree_view::rootNodeFormulaChanged, this,
		[this](int old_id, int new_id) { _model->changeBranchFormula(old_id, new_id); });

	connect(ui->sxtBranches,
		&QTreeWidget::currentItemChanged,
		this,
		[this](QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/) {
			if (!current || current->parent()) {
				_model->setCurrentBranchSelection(-1);
				return;
			}

			if (auto* tw = current->treeWidget()) {
				const int idx = tw->indexOfTopLevelItem(current);
				_model->setCurrentBranchSelection(idx >= 0 ? idx : -1);
			} else {
				_model->setCurrentBranchSelection(-1);
			}

			updateLoopDetails();
		});

	connect(ui->sxtBranches, &sexp_tree_view::modified, this, &CampaignEditorDialog::onBranchTreeModified);

	initializeUi();
	updateUi();

	// The before-state for the next tree edit: the tree mutates before
	// modified() fires, so a fresh capture at handler time would already
	// contain the edit. indexChanged fires on every push/undo/redo, keeping
	// the cache aligned with the current working state.
	_workingStateCache = _model->captureWorkingState();
	connect(_dialogStack, &QUndoStack::indexChanged, this, [this](int) {
		if (_model)
			_workingStateCache = _model->captureWorkingState();
	});

	// Saving marks the stack's current index clean rather than clearing the
	// history; undoing or redoing back onto that index means the working
	// copy matches the file on disk again.
	connect(_dialogStack, &QUndoStack::cleanChanged, this, [this](bool clean) {
		if (clean && _model)
			_model->setUnmodified();
	});
}

void CampaignEditorDialog::onBranchTreeModified()
{
	_model->setModified();
	if (_suppressTreeUndo || _branchDeletePending)
		return;

	pushWorkingStateSnapshot(_workingStateCache, tr("Edit Branch Formula"));
}

void CampaignEditorDialog::pushWorkingStateSnapshot(const QByteArray& before, const QString& label)
{
	const QByteArray after = _model->captureWorkingState();
	if (after == before)
		return;

	_dialogStack->push(new DialogSnapshotCommand(before, after,
		[this](const QByteArray& blob) {
			_suppressTreeUndo = true;
			// Selections are not part of the blob; re-select the same mission
			// by filename if it still exists after the restore.
			const SCP_string selected = _model->getCurrentMissionFilename();
			const auto expanded = ui->sxtBranches->captureExpansionState();
			_model->restoreWorkingState(blob);
			updateUi(); // specs, tech lists, available missions, graph
			const int idx = _model->findMissionIndexByFilename(selected);
			if (idx >= 0) {
				ui->graphView->setSelectedMission(idx);
				_model->setCurrentMissionSelection(idx); // rebuilds the branch tree
				ui->sxtBranches->restoreExpansionState(expanded);
			}
			updateMissionDetails();
			updateLoopDetails();
			_suppressTreeUndo = false;
		},
		label));
}
CampaignEditorDialog::~CampaignEditorDialog() = default;

SCP_vector<SCP_string> CampaignEditorDialog::getMissionNames()
{
	SCP_vector<SCP_string> list;
	if (!_model) {
		return list;
	}
	for (const auto& mission : _model->getCampaignMissions()) {
		list.emplace_back(mission.filename);
	}
	return list;
}

bool CampaignEditorDialog::hasDefaultMissionName()
{
	return _model && !_model->getCampaignMissions().empty();
}

// Resolve a mission filename (as it appears in the sexp tree) to its index in the
// global Campaign.missions[] table that syncCampaignMissionList() mirrors from
// _model->getCampaignMissions(). Lazy-loads the goal/event list from disk if the
// FRED_LOAD_PENDING flag is still set. Returns -1 if the mission isn't in the
// campaign yet (e.g. the user is typing the name from scratch).
static int loadAndFindCampaignMission(const SCP_string& reference_name)
{
	if (reference_name.empty()) {
		return -1;
	}
	int idx = mission_campaign_find_mission(reference_name.c_str());
	if (idx < 0) {
		return -1;
	}
	if (Campaign.missions[idx].flags & CMISSION_FLAG_FRED_LOAD_PENDING) {
		read_mission_goal_list(idx);
		Campaign.missions[idx].flags &= ~CMISSION_FLAG_FRED_LOAD_PENDING;
	}
	return idx;
}

SCP_vector<SCP_string> CampaignEditorDialog::getMissionGoals(const SCP_string& reference_name)
{
	SCP_vector<SCP_string> list;
	const int idx = loadAndFindCampaignMission(reference_name);
	if (idx < 0) {
		return list;
	}
	for (const auto& goal : Campaign.missions[idx].goals) {
		list.emplace_back(goal.name);
	}
	return list;
}

SCP_vector<SCP_string> CampaignEditorDialog::getMissionEvents(const SCP_string& reference_name)
{
	SCP_vector<SCP_string> list;
	const int idx = loadAndFindCampaignMission(reference_name);
	if (idx < 0) {
		return list;
	}
	for (const auto& event : Campaign.missions[idx].events) {
		list.emplace_back(event.name);
	}
	return list;
}


void CampaignEditorDialog::closeEvent(QCloseEvent* e)
{
	// First, ask the user if they want to save any pending changes.
	if (questionSaveChanges()) {
		// If the user didn't cancel, it's safe to accept the close event.
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		e->accept();
	} else {
		// If the user cancelled, we ignore the close event to keep the window open.
		e->ignore();
	}
}

void CampaignEditorDialog::initializeUi()
{
	util::SignalBlockers blocker(this);

	fso::fred::bindCustomIcon(ui->moveBranchTopButton,     CustomIcon::MoveToTop);
	fso::fred::bindStandardIcon(ui->moveBranchUpButton,    QStyle::SP_ArrowUp);
	fso::fred::bindStandardIcon(ui->moveBranchDownButton,  QStyle::SP_ArrowDown);
	fso::fred::bindCustomIcon(ui->moveBranchBottomButton,  CustomIcon::MoveToBottom);
	fso::fred::bindStandardIcon(ui->testVoiceButton,      QStyle::SP_MediaPlay);

	// setup the types combo box
	auto types = _model->getCampaignTypes();
	QStringList typeList;
	for (const auto& type : types) {
		typeList.append(QString::fromStdString(type));
	}

	ui->typeComboBox->clear();
	ui->typeComboBox->addItems(typeList);

	int font_height = ui->availableMissionsListWidget->fontMetrics().height();
	ui->availableMissionsListWidget->setGridSize(QSize(125, font_height + 2));

	auto disableDnD = [](QAbstractItemView* v) { // no dungeons OR dragons allowed!
		v->setDragEnabled(false);
		v->setAcceptDrops(false);
		v->setDragDropMode(QAbstractItemView::NoDragDrop);
		v->setDefaultDropAction(Qt::IgnoreAction);
		v->setDropIndicatorShown(false);
		v->setEditTriggers(QAbstractItemView::NoEditTriggers);
	};

	disableDnD(ui->availableMissionsListWidget);

	// setup the cutscene list
	auto cutscenes = _model->getCutsceneList();
	QStringList cutsceneList;
	for (const auto& cs : cutscenes) {
		cutsceneList.append(QString::fromStdString(cs));
	}

	ui->briefCutsceneComboBox->clear();
	ui->briefCutsceneComboBox->addItems(cutsceneList);

	// setup the main hall lists
	refreshMainhallCombos();

	ui->fredMissionButton->setHidden(true); // TODO activate this when QtFRED is closer to completion
}

// The main hall choices depend on the save format (retail campaigns only
// know halls "0" and "1"), so the combos are rebuilt when the format toggles.
void CampaignEditorDialog::refreshMainhallCombos()
{
	util::SignalBlockers blocker(this);

	auto mainhalls = _model->getMainhallList();
	QStringList mainhallList;
	for (const auto& mh : mainhalls) {
		mainhallList.append(QString::fromStdString(mh));
	}

	ui->mainhallComboBox->clear();
	ui->mainhallComboBox->addItems(mainhallList);
	ui->substituteMainhallComboBox->clear();
	ui->substituteMainhallComboBox->addItems(mainhallList);
}

void CampaignEditorDialog::syncShipListItem(int shipClassIndex)
{
	QSignalBlocker blocker(ui->shipsListWidget);
	for (int i = 0; i < ui->shipsListWidget->count(); ++i) {
		auto* item = ui->shipsListWidget->item(i);
		if (item && item->data(Qt::UserRole).toInt() == shipClassIndex) {
			item->setCheckState(_model->getAllowedShip(shipClassIndex) ? Qt::Checked : Qt::Unchecked);
			break;
		}
	}
}

void CampaignEditorDialog::syncWeaponListItem(int weaponClassIndex)
{
	QSignalBlocker blocker(ui->weaponsListWidget);
	for (int i = 0; i < ui->weaponsListWidget->count(); ++i) {
		auto* item = ui->weaponsListWidget->item(i);
		if (item && item->data(Qt::UserRole).toInt() == weaponClassIndex) {
			item->setCheckState(_model->getAllowedWeapon(weaponClassIndex) ? Qt::Checked : Qt::Unchecked);
			break;
		}
	}
}

void CampaignEditorDialog::updateUi()
{
	util::SignalBlockers blocker(this);

	ui->nameLineEdit->setText(QString::fromStdString(_model->getCampaignName()));
	ui->typeComboBox->setCurrentIndex(_model->getCampaignType());
	ui->resetTechAtStartCheckBox->setChecked(_model->getCampaignTechReset());
	ui->descriptionPlainTextEdit->setPlainText(QString::fromStdString(_model->getCampaignDescription()));

	ui->retailFormatCheckbox->setChecked(_model->getSaveFormat() == CampaignFormat::Retail);
	
	updateTechLists();
	updateAvailableMissionsList();
	updateMissionDetails();
	enableDisableControls();

	ui->graphView->rebuildAll();
}

void CampaignEditorDialog::updateTechLists()
{
	util::SignalBlockers blocker(this);
	ui->shipsListWidget->clear();
	ui->weaponsListWidget->clear();

	// Get the pre-filtered list of ships from the model
	for (const auto& [name, index, is_allowed] : _model->getAllowedShips()) {
		auto* item = new QListWidgetItem(QString::fromStdString(name), ui->shipsListWidget);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(is_allowed ? Qt::Checked : Qt::Unchecked);
		item->setData(Qt::UserRole, index); // Store the original index
	}

	// Get the pre-filtered list of weapons from the model
	for (const auto& [name, index, is_allowed] : _model->getAllowedWeapons()) {
		auto* item = new QListWidgetItem(QString::fromStdString(name), ui->weaponsListWidget);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(is_allowed ? Qt::Checked : Qt::Unchecked);
		item->setData(Qt::UserRole, index); // Store the original index
	}
}

void CampaignEditorDialog::updateAvailableMissionsList()
{
	util::SignalBlockers blocker(this);

	ui->availableMissionsListWidget->clear();

	const QColor packagedColor(128, 128, 0); // dark yellow

	for (const auto& [name, isEditable] : _model->getAvailableMissionFiles()) {
		auto* item = new QListWidgetItem(QString::fromStdString(name));
		if (!isEditable) {
			item->setForeground(packagedColor);
			item->setToolTip("This mission is packaged in a VP and cannot be modified.");
		}
		item->setData(Qt::UserRole, isEditable); // save whether it's editable
		ui->availableMissionsListWidget->addItem(item);
	}
}

void CampaignEditorDialog::updateMissionDetails()
{
	util::SignalBlockers blocker(this);
	
	ui->briefCutsceneComboBox->setCurrentIndex(ui->briefCutsceneComboBox->findText(_model->getCurrentMissionBriefingCutscene().c_str(), Qt::MatchFixedString));
	ui->mainhallComboBox->setCurrentIndex(ui->mainhallComboBox->findText(_model->getCurrentMissionMainhall().c_str(), Qt::MatchFixedString));
	ui->substituteMainhallComboBox->setCurrentIndex(ui->substituteMainhallComboBox->findText(_model->getCurrentMissionSubstituteMainhall().c_str(), Qt::MatchFixedString));
	ui->debriefingPersonaSpinBox->setValue(_model->getCurrentMissionDebriefingPersona());

	enableDisableControls();
}

void CampaignEditorDialog::updateLoopDetails()
{
	util::SignalBlockers blocker(this);

	ui->loopDescriptionPlainTextEdit->setPlainText(QString::fromStdString(_model->getCurrentBranchLoopDescription()));
	ui->loopAnimLineEdit->setText(QString::fromStdString(_model->getCurrentBranchLoopAnim()));
	ui->loopVoiceLineEdit->setText(QString::fromStdString(_model->getCurrentBranchLoopVoice()));

	enableDisableControls();
}

void CampaignEditorDialog::enableDisableControls()
{
	bool mission_selected = (_model->getCurrentMissionSelection() >= 0) || (ui->availableMissionsListWidget->currentItem() != nullptr);
	ui->fredMissionButton->setEnabled(mission_selected);

	bool has_mission = (_model->getCurrentMissionSelection() >= 0);

	ui->sxtBranches->setEnabled(has_mission);
	ui->briefCutsceneComboBox->setEnabled(has_mission);
	ui->mainhallComboBox->setEnabled(has_mission);
	ui->substituteMainhallComboBox->setEnabled(has_mission);
	ui->debriefingPersonaSpinBox->setEnabled(has_mission);

	bool branch_selected = (_model->getCurrentBranchSelection() >= 0);
	bool can_move_up = branch_selected && _model->getCurrentBranchSelection() > 0;
	bool can_move_down = branch_selected && _model->getCurrentBranchSelection() < _model->getNumBranches() - 1;
	ui->moveBranchTopButton->setEnabled(can_move_up);
	ui->moveBranchUpButton->setEnabled(can_move_up);
	ui->moveBranchDownButton->setEnabled(can_move_down);
	ui->moveBranchBottomButton->setEnabled(can_move_down);

	bool special_branch_selected = _model->getCurrentBranchIsSpecial();
	ui->loopDescriptionPlainTextEdit->setEnabled(special_branch_selected);
	ui->loopAnimBrowseButton->setEnabled(special_branch_selected);
	ui->loopAnimLineEdit->setEnabled(special_branch_selected);
	ui->loopVoiceBrowseButton->setEnabled(special_branch_selected);
	ui->loopVoiceLineEdit->setEnabled(special_branch_selected);

	bool enable_playback = special_branch_selected && !_model->getCurrentBranchLoopVoice().empty();
	ui->testVoiceButton->setEnabled(enable_playback);
}

bool CampaignEditorDialog::questionSaveChanges()
{
	if (!_model->query_modified()) {
		return true; // No changes, safe to proceed.
	}

	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this,
		"Unsaved Changes",
		"This campaign has been modified.\n\nSave changes?",
		QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

	if (reply == QMessageBox::Cancel) {
		return false; // User cancelled, abort the operation.
	}

	if (reply == QMessageBox::Save) {
		// Abort the operation if the save fails (validity errors, a canceled
		// Save As, a write failure) — proceeding would discard the changes.
		return doSave();
	}

	// If we get here, the user chose Discard.
	return true;
}

void CampaignEditorDialog::on_actionNew_triggered()
{
	// Check if there are unsaved changes.
	if (!questionSaveChanges()) {
		return; // User cancelled.
	}

	_model->createNewCampaign();
	_dialogStack->clear(); // the history refers to the previous campaign
	updateUi();
	ui->graphView->zoomToFitAll();
}

void CampaignEditorDialog::on_actionOpen_triggered()
{
	// Check if there are unsaved changes.
	if (!questionSaveChanges()) {
		return; // User cancelled the operation.
	}

	// Open a file dialog to let the user select a campaign file.
	const QString lastDir = util::getLastDir("campaign/loadCampaign", CF_TYPE_MISSIONS);

	QString pathName = QFileDialog::getOpenFileName(this, "Load Campaign", lastDir, "FS2 Campaigns (*.fc2)");

	if (pathName.isEmpty()) {
		return; // User cancelled the file dialog.
	}

	util::saveLastDir("campaign/loadCampaign", pathName);
	QString nativePath = QDir::toNativeSeparators(pathName);

	_model->loadCampaignFromFile(nativePath.toUtf8().constData());
	_dialogStack->clear(); // the history refers to the previous campaign
	updateUi();
	ui->graphView->zoomToFitAll();
}

void CampaignEditorDialog::on_actionSave_triggered()
{
	doSave();
}

void CampaignEditorDialog::on_actionSave_As_triggered()
{
	doSaveAs();
}

bool CampaignEditorDialog::doSave()
{
	// This saves to the currently known filename. If the filename is empty
	// (because it's a new campaign), this delegates to the Save As logic.
	if (_model->getCampaignFilename().empty()) {
		return doSaveAs();
	}

	const bool saved = _model->saveCampaign(""); // Empty string = use the current filename.
	if (saved) {
		// Keep the history; mark this point as the saved state so the
		// modified flag tracks undo/redo across the save point.
		_dialogStack->setClean();
	}
	return saved;
}

bool CampaignEditorDialog::doSaveAs()
{
	// Open a file dialog to let the user choose a save location and filename.
	const QString lastDir = util::getLastDir("campaign/saveCampaign", CF_TYPE_MISSIONS);

	QString pathName = QFileDialog::getSaveFileName(this, "Save Campaign As", lastDir, "FS2 Campaigns (*.fc2)");

	if (pathName.isEmpty()) {
		return false; // User cancelled the file dialog.
	}

	util::saveLastDir("campaign/saveCampaign", pathName);

	// The model will handle the actual save operation.
	const bool saved = _model->saveCampaign(pathName.toUtf8().constData());
	if (saved) {
		_dialogStack->setClean();
	}
	return saved;
}

void CampaignEditorDialog::on_actionExit_triggered()
{
	this->close();
}

void CampaignEditorDialog::on_nameLineEdit_textChanged(const QString& arg1)
{
	const SCP_string before = _model->getCampaignName();
	_model->setCampaignName(arg1.toUtf8().constData());
	// The setter truncates, so read the stored value back.
	const SCP_string after = _model->getCampaignName();
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<SCP_string>(FieldId::Camp_Name, nullptr, tr("Change Campaign Name"), true);
	cmd->addEntry(before, after, [this](const SCP_string& v) {
		_model->setCampaignName(v);
		QSignalBlocker blocker(ui->nameLineEdit);
		ui->nameLineEdit->setText(QString::fromStdString(v));
	});
	_dialogStack->push(cmd);
}

void CampaignEditorDialog::on_typeComboBox_currentIndexChanged(int index)
{
	// The model should have a list of campaign types matching the combo box.
	if (!SCP_vector_inbounds(_model->getCampaignTypes(), index)) {
		return;
	}

	const int before = _model->getCampaignType();
	_model->setCampaignType(index); // refuses (with an error dialog) once missions exist
	const int after = _model->getCampaignType();
	if (before == after) {
		// Rejected or no-op: put the combo back in step with the model.
		QSignalBlocker blocker(ui->typeComboBox);
		ui->typeComboBox->setCurrentIndex(after);
		return;
	}

	updateAvailableMissionsList(); // the compatibility filter changed

	auto* cmd = new FieldEditCommand<int>(FieldId::Camp_Type, nullptr, tr("Change Campaign Type"), true);
	cmd->addEntry(before, after, [this](const int& v) {
		_model->setCampaignType(v);
		{
			QSignalBlocker blocker(ui->typeComboBox);
			ui->typeComboBox->setCurrentIndex(v);
		}
		updateAvailableMissionsList();
	});
	_dialogStack->push(cmd);
}

void CampaignEditorDialog::on_resetTechAtStartCheckBox_toggled(bool checked)
{
	const bool before = _model->getCampaignTechReset();
	if (before == checked)
		return;

	_model->setCampaignTechReset(checked);

	auto* cmd = new FieldEditCommand<bool>(FieldId::Camp_TechReset, nullptr, tr("Toggle Tech Database Reset"), true);
	cmd->addEntry(before, checked, [this](const bool& v) {
		_model->setCampaignTechReset(v);
		QSignalBlocker blocker(ui->resetTechAtStartCheckBox);
		ui->resetTechAtStartCheckBox->setChecked(v);
	});
	_dialogStack->push(cmd);
}

void CampaignEditorDialog::on_campaignCustomDataButton_clicked()
{
	CustomDataDialog dlg(this, _viewport);
	dlg.setInitial(_model->getCustomData());

	if (dlg.exec() != QDialog::Accepted) {
		return;
	}

	const SCP_map<SCP_string, SCP_string> before = _model->getCustomData();
	_model->setCustomData(dlg.items());
	const SCP_map<SCP_string, SCP_string> after = _model->getCustomData();
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<SCP_map<SCP_string, SCP_string>>(
	    FieldId::Camp_CustomData, nullptr, tr("Edit Custom Data"), true);
	cmd->setNoMerge(); // each subdialog visit is a discrete action
	cmd->addEntry(before, after, [this](const SCP_map<SCP_string, SCP_string>& v) { _model->setCustomData(v); });
	_dialogStack->push(cmd);
}

void CampaignEditorDialog::on_descriptionPlainTextEdit_textChanged()
{
	const SCP_string before = _model->getCampaignDescription();
	_model->setCampaignDescription(ui->descriptionPlainTextEdit->toPlainText().toUtf8().constData());
	const SCP_string after = _model->getCampaignDescription();
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<SCP_string>(FieldId::Camp_Description, nullptr, tr("Change Campaign Description"), true);
	cmd->addEntry(before, after, [this](const SCP_string& v) {
		_model->setCampaignDescription(v);
		QSignalBlocker blocker(ui->descriptionPlainTextEdit);
		ui->descriptionPlainTextEdit->setPlainText(QString::fromStdString(v));
	});
	_dialogStack->push(cmd);
}

void CampaignEditorDialog::on_shipsListWidget_itemChanged(QListWidgetItem* item)
{
	if (!item) {
		return;
	}

	const int ship_class_index = item->data(Qt::UserRole).toInt();
	const bool is_allowed = (item->checkState() == Qt::Checked);
	const bool before = _model->getAllowedShip(ship_class_index);
	if (before == is_allowed)
		return;

	_model->setAllowedShip(ship_class_index, is_allowed);

	auto* cmd = new FieldEditCommand<bool>(
	    FieldId::Camp_ShipAllowed + ship_class_index, nullptr, tr("Toggle Allowed Ship"), true);
	cmd->addEntry(before, is_allowed, [this, ship_class_index](const bool& v) {
		_model->setAllowedShip(ship_class_index, v);
		syncShipListItem(ship_class_index);
	});
	_dialogStack->push(cmd);
}

void CampaignEditorDialog::on_weaponsListWidget_itemChanged(QListWidgetItem* item)
{
	if (!item) {
		return;
	}

	const int weapon_class_index = item->data(Qt::UserRole).toInt();
	const bool is_allowed = (item->checkState() == Qt::Checked);
	const bool before = _model->getAllowedWeapon(weapon_class_index);
	if (before == is_allowed)
		return;

	_model->setAllowedWeapon(weapon_class_index, is_allowed);

	auto* cmd = new FieldEditCommand<bool>(
	    FieldId::Camp_WeaponAllowed + weapon_class_index, nullptr, tr("Toggle Allowed Weapon"), true);
	cmd->addEntry(before, is_allowed, [this, weapon_class_index](const bool& v) {
		_model->setAllowedWeapon(weapon_class_index, v);
		syncWeaponListItem(weapon_class_index);
	});
	_dialogStack->push(cmd);
}

void CampaignEditorDialog::on_errorCheckerButton_clicked()
{
	if (_model->checkValidity()) {
		QMessageBox ::information(this, "No Issues Found", "No issues were found in the campaign.");
	}
}

void CampaignEditorDialog::on_availableMissionsFilterLineEdit_textChanged(const QString& arg1)
{
	_model->setAvailableMissionsFilter(arg1.toUtf8().constData());
	updateAvailableMissionsList();
}

void CampaignEditorDialog::on_availableMissionsListWidget_itemSelectionChanged()
{
	// Since we share some of the UI we need to clear it first
	// so that it's clear what the user is working on
	ui->graphView->clearSelectedMission();
	updateMissionDetails();
	
	// Get the currently selected item
	QListWidgetItem* selected_item = ui->availableMissionsListWidget->currentItem();
	if (!selected_item) {
		ui->missionNameLineEdit->clear();
		ui->missionDescriptionPlainTextEdit->clear();
		return;
	}

	// Get the filename from the item's text
	SCP_string filename = selected_item->text().toUtf8().constData();

	mission mission_info;
	if (get_mission_info(filename.c_str(), &mission_info) != 0) {
		// Failed to retrieve mission info, clear fields and return
		ui->missionNameLineEdit->clear();
		ui->missionDescriptionPlainTextEdit->clear();
		return;
	}

	if (!mission_info.name.empty()) {
		ui->missionNameLineEdit->setText(QString::fromStdString(mission_info.name));
	} else {
		ui->missionNameLineEdit->clear();
	}

	if (mission_info.notes[0] != '\0') {
		ui->missionDescriptionPlainTextEdit->setPlainText(QString::fromUtf8(mission_info.notes));
	} else {
		ui->missionDescriptionPlainTextEdit->clear();
	}
}

void CampaignEditorDialog::on_graphView_missionSelected(int missionIndex) {
	_model->setCurrentMissionSelection(missionIndex);

	SCP_string filename = _model->getCurrentMissionFilename();
	mission mission_info;
	if (get_mission_info(filename.c_str(), &mission_info) != 0) {
		// Failed to retrieve mission info, clear fields and return
		ui->missionNameLineEdit->clear();
		ui->missionDescriptionPlainTextEdit->clear();
		return;
	}

	if (!mission_info.name.empty()) {
		ui->missionNameLineEdit->setText(QString::fromStdString(mission_info.name));
	} else {
		ui->missionNameLineEdit->clear();
	}

	if (mission_info.notes[0] != '\0') {
		ui->missionDescriptionPlainTextEdit->setPlainText(QString::fromUtf8(mission_info.notes));
	} else {
		ui->missionDescriptionPlainTextEdit->clear();
	}

	updateMissionDetails();
}

void CampaignEditorDialog::on_graphView_specialModeToggleRequested(int missionIndex)
{
	const QByteArray before = _model->captureWorkingState();
	_model->toggleMissionSpecialMode(missionIndex);
	ui->graphView->rebuildAll();
	pushWorkingStateSnapshot(before, tr("Toggle Special Branch Mode"));
}

void CampaignEditorDialog::on_graphView_addMissionHereRequested(QPointF sceneTopLeft)
{
	const auto selections = ui->availableMissionsListWidget->selectedItems();
	SCP_string filename;
	// Only one item should be selected
	if (!selections.empty()) {
		filename = selections[0]->text().toUtf8().constData();
	}

	if (filename.empty()) {
		QList<QString> availableMissions;
		for (const auto& [name, isEditable] : _model->getAvailableMissionFiles()) {
			availableMissions.append(QString::fromStdString(name));
		}

		SimpleListSelectDialog dlg(availableMissions, this);
		dlg.setTitle("Choose mission");
		dlg.setPlaceholder("Filter missions...");

		if (dlg.exec() == QDialog::Accepted) {
			filename = dlg.selectedText().toUtf8().constData();
		}

		if (filename.empty())
			return; // user canceled
	}

	// add
	const QByteArray before = _model->captureWorkingState();
	_model->addMission(filename, 0, 0);

	// New mission index is the last element now
	const int idx = static_cast<int>(_model->getCampaignMissions().size() - 1);

	// persist graph placement
	_model->setMissionGraphX(idx, static_cast<int>(std::lround(sceneTopLeft.x())));
	_model->setMissionGraphY(idx, static_cast<int>(std::lround(sceneTopLeft.y())));

	// refresh graph and select the new node
	ui->graphView->rebuildAll();
	ui->graphView->setSelectedMission(idx);

	updateAvailableMissionsList();
	pushWorkingStateSnapshot(before, tr("Add Mission"));
}

void CampaignEditorDialog::on_graphView_deleteMissionRequested(int missionIndex)
{
	const QByteArray before = _model->captureWorkingState();
	ui->graphView->clearSelectedMission();
	updateMissionDetails();
	_model->removeMission(missionIndex);
	ui->graphView->rebuildAll();
	updateAvailableMissionsList();
	pushWorkingStateSnapshot(before, tr("Delete Mission"));
}

void CampaignEditorDialog::on_graphView_addRepeatBranchRequested(int missionIndex)
{
	const QByteArray before = _model->captureWorkingState();
	_model->addBranch(missionIndex, missionIndex);
	ui->graphView->rebuildAll();
	pushWorkingStateSnapshot(before, tr("Add Repeat Branch"));
}

void CampaignEditorDialog::on_graphView_createMissionAtAndConnectRequested(QPointF sceneTopLeft, int fromIndex, bool isSpecial)
{
	// Ask user via available missions dialog
	QList<QString> availableMissions;
	for (const auto& [name, isEditable] : _model->getAvailableMissionFiles()) {
		availableMissions.append(QString::fromStdString(name));
	}

	SimpleListSelectDialog dlg(availableMissions, this);
	dlg.setTitle("Choose mission");
	dlg.setPlaceholder("Filter missions...");

	SCP_string picked;
	if (dlg.exec() == QDialog::Accepted) {
		picked = dlg.selectedText().toUtf8().constData();
	}

	if (picked.empty())
		return; // user canceled

	// Add the mission to the model
	const QByteArray before = _model->captureWorkingState();
	_model->addMission(picked, /*level*/ 0, /*position*/ 0);

	// New mission index (last)
	const auto& ms = _model->getCampaignMissions();
	const int newIdx = static_cast<int>(ms.size()) - 1;

	// Persist graph placement
	_model->setMissionGraphX(newIdx, static_cast<int>(std::lround(sceneTopLeft.x())));
	_model->setMissionGraphY(newIdx, static_cast<int>(std::lround(sceneTopLeft.y())));

	// Connect from source to the new mission
	if (isSpecial) {
		_model->addSpecialBranch(fromIndex, newIdx);
	} else {
		_model->addBranch(fromIndex, newIdx);
	}

	// Rebuild
	ui->graphView->rebuildAll();
	ui->graphView->setSelectedMission(newIdx);
	updateAvailableMissionsList();
	pushWorkingStateSnapshot(before, tr("Add Mission"));
}

void CampaignEditorDialog::on_graphView_setFirstMissionRequested(int missionIndex)
{
	const QByteArray before = _model->captureWorkingState();
	int current_selection = _model->getCurrentMissionSelection(); // save now because rebuild clears it
	_model->setMissionAsFirst(missionIndex);
	ui->graphView->rebuildAll();

	if (current_selection == missionIndex) {
		// If we changed the first mission, the selection index changed too
		current_selection = 0;
	}

	ui->graphView->setSelectedMission(current_selection);
	pushWorkingStateSnapshot(before, tr("Set First Mission"));
}

void CampaignEditorDialog::on_graphView_branchConnectRequested(int fromIndex, int toIndex, bool isSpecial)
{
	const QByteArray before = _model->captureWorkingState();
	if (isSpecial) {
		_model->addSpecialBranch(fromIndex, toIndex);
	} else {
		_model->addBranch(fromIndex, toIndex);
	}
	// Duplicate/conflicting connections are rejected by the model; the
	// snapshot equality check absorbs those into no-ops.
	pushWorkingStateSnapshot(before, tr("Add Branch"));
}

void CampaignEditorDialog::on_graphView_endBranchConnectRequested(int fromIndex)
{
	const QByteArray before = _model->captureWorkingState();
	_model->addEndBranch(fromIndex);
	pushWorkingStateSnapshot(before, tr("Add Branch"));
}

void CampaignEditorDialog::on_graphView_nodeDragStarted(int missionIndex)
{
	// Only the first movement of each node in a gesture records its position.
	if (_nodeDragBefore.find(missionIndex) == _nodeDragBefore.end()) {
		_nodeDragBefore[missionIndex] = {_model->getMissionGraphX(missionIndex),
		                                 _model->getMissionGraphY(missionIndex)};
	}
}

void CampaignEditorDialog::on_graphView_nodeDragFinished(int /*missionIndex*/)
{
	// Collect every node moved during the gesture — a multi-select drag moves
	// all selected nodes but only the cursor node emits nodeDragFinished.
	SCP_vector<std::pair<int, std::pair<int, int>>> moved; // (missionIndex, before)
	for (const auto& [idx, before] : _nodeDragBefore) {
		const std::pair<int, int> after{_model->getMissionGraphX(idx), _model->getMissionGraphY(idx)};
		if (before != after) {
			moved.emplace_back(idx, before);
		}
	}
	_nodeDragBefore.clear();
	if (moved.empty())
		return;

	auto* cmd = new FieldEditCommand<std::pair<int, int>>(
	    FieldId::Camp_MissionNodePos + moved.front().first * FieldId::Camp_MissionFieldStride,
	    nullptr, tr("Move Mission Node"), true);
	cmd->setNoMerge(); // one command per drag gesture
	for (const auto& [idx, before] : moved) {
		const std::pair<int, int> after{_model->getMissionGraphX(idx), _model->getMissionGraphY(idx)};
		cmd->addEntry(before, after, [this, idx = idx](const std::pair<int, int>& v) {
			_model->setMissionGraphX(idx, v.first);
			_model->setMissionGraphY(idx, v.second);
			const int sel = _model->getCurrentMissionSelection();
			ui->graphView->rebuildAll();
			ui->graphView->setSelectedMission(sel);
		});
	}
	_dialogStack->push(cmd);
}

void CampaignEditorDialog::on_briefCutsceneComboBox_currentIndexChanged(const QString& arg1)
{
	if (_model->getCurrentMissionSelection() < 0)
		return;

	const int missionIndex  = _model->getCurrentMissionSelection();
	const SCP_string before = _model->getCurrentMissionBriefingCutscene();
	_model->setCurrentMissionBriefingCutscene(arg1.toUtf8().constData());
	const SCP_string after = _model->getCurrentMissionBriefingCutscene();
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::Camp_MissionCutscene + missionIndex * FieldId::Camp_MissionFieldStride,
	    nullptr, tr("Change Briefing Cutscene"), true);
	cmd->addEntry(before, after, [this, missionIndex](const SCP_string& v) {
		_model->setMissionBriefingCutsceneAt(missionIndex, v);
		updateMissionDetails();
	});
	_dialogStack->push(cmd);
}

void CampaignEditorDialog::on_debriefingPersonaSpinBox_valueChanged(int arg1)
{
	if (_model->getCurrentMissionSelection() < 0)
		return;

	const int missionIndex = _model->getCurrentMissionSelection();
	const int before       = _model->getCurrentMissionDebriefingPersona();
	_model->setCurrentMissionDebriefingPersona(arg1);
	const int after = _model->getCurrentMissionDebriefingPersona();
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::Camp_MissionPersona + missionIndex * FieldId::Camp_MissionFieldStride,
	    nullptr, tr("Change Debriefing Persona"), true);
	cmd->addEntry(before, after, [this, missionIndex](const int& v) {
		_model->setMissionDebriefingPersonaAt(missionIndex, v);
		updateMissionDetails();
	});
	_dialogStack->push(cmd);
}

void CampaignEditorDialog::on_mainhallComboBox_currentIndexChanged(const QString& arg1)
{
	if (_model->getCurrentMissionSelection() < 0)
		return;

	const int missionIndex  = _model->getCurrentMissionSelection();
	const SCP_string before = _model->getCurrentMissionMainhall();
	_model->setCurrentMissionMainhall(arg1.toUtf8().constData());
	const SCP_string after = _model->getCurrentMissionMainhall();
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::Camp_MissionMainhall + missionIndex * FieldId::Camp_MissionFieldStride,
	    nullptr, tr("Change Main Hall"), true);
	cmd->addEntry(before, after, [this, missionIndex](const SCP_string& v) {
		_model->setMissionMainhallAt(missionIndex, v);
		updateMissionDetails();
	});
	_dialogStack->push(cmd);
}

void CampaignEditorDialog::on_substituteMainhallComboBox_currentIndexChanged(const QString& arg1)
{
	if (_model->getCurrentMissionSelection() < 0)
		return;

	const int missionIndex  = _model->getCurrentMissionSelection();
	const SCP_string before = _model->getCurrentMissionSubstituteMainhall();
	_model->setCurrentMissionSubstituteMainhall(arg1.toUtf8().constData());
	const SCP_string after = _model->getCurrentMissionSubstituteMainhall();
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::Camp_MissionSubMainhall + missionIndex * FieldId::Camp_MissionFieldStride,
	    nullptr, tr("Change Substitute Main Hall"), true);
	cmd->addEntry(before, after, [this, missionIndex](const SCP_string& v) {
		_model->setMissionSubstituteMainhallAt(missionIndex, v);
		updateMissionDetails();
	});
	_dialogStack->push(cmd);
}

void CampaignEditorDialog::on_moveBranchTopButton_clicked()
{
	int mission_selection = _model->getCurrentMissionSelection(); // save now because rebuild clears it

	_model->moveBranchToTop();
	ui->graphView->rebuildAll();

	ui->graphView->setSelectedMission(mission_selection);
}

void CampaignEditorDialog::on_moveBranchUpButton_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	int mission_selection = _model->getCurrentMissionSelection(); // save now because rebuild clears it

	_model->moveBranchUp();
	ui->graphView->rebuildAll();

	ui->graphView->setSelectedMission(mission_selection);
	pushWorkingStateSnapshot(before, tr("Move Branch"));
}

void CampaignEditorDialog::on_moveBranchDownButton_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	int mission_selection = _model->getCurrentMissionSelection(); // save now because rebuild clears it

	_model->moveBranchDown();
	ui->graphView->rebuildAll();

	ui->graphView->setSelectedMission(mission_selection);
	pushWorkingStateSnapshot(before, tr("Move Branch"));
}

void CampaignEditorDialog::on_moveBranchBottomButton_clicked()
{
	int mission_selection = _model->getCurrentMissionSelection(); // save now because rebuild clears it

	_model->moveBranchToBottom();
	ui->graphView->rebuildAll();

	ui->graphView->setSelectedMission(mission_selection);
}

void CampaignEditorDialog::on_loopDescriptionPlainTextEdit_textChanged()
{
	if (!_model->getCurrentBranchIsSpecial())
		return;

	const int missionIndex  = _model->getCurrentMissionSelection();
	const int branchIndex   = _model->getCurrentBranchSelection();
	const SCP_string before = _model->getCurrentBranchLoopDescription();
	_model->setCurrentBranchLoopDescription(ui->loopDescriptionPlainTextEdit->toPlainText().toUtf8().constData());
	const SCP_string after = _model->getCurrentBranchLoopDescription();
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::Camp_LoopDescription +
	        (missionIndex * FieldId::Camp_BranchesPerMission + branchIndex) * FieldId::Camp_BranchFieldStride,
	    nullptr, tr("Change Loop Description"), true);
	cmd->addEntry(before, after, [this, missionIndex, branchIndex](const SCP_string& v) {
		_model->setBranchLoopDescriptionAt(missionIndex, branchIndex, v);
		updateLoopDetails();
	});
	_dialogStack->push(cmd);
}

void CampaignEditorDialog::on_loopAnimLineEdit_textChanged(const QString& arg1)
{
	if (!_model->getCurrentBranchIsSpecial())
		return;

	const int missionIndex  = _model->getCurrentMissionSelection();
	const int branchIndex   = _model->getCurrentBranchSelection();
	const SCP_string before = _model->getCurrentBranchLoopAnim();
	_model->setCurrentBranchLoopAnim(arg1.toUtf8().constData());
	const SCP_string after = _model->getCurrentBranchLoopAnim();
	enableDisableControls();
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::Camp_LoopAnim +
	        (missionIndex * FieldId::Camp_BranchesPerMission + branchIndex) * FieldId::Camp_BranchFieldStride,
	    nullptr, tr("Change Loop Animation"), true);
	cmd->addEntry(before, after, [this, missionIndex, branchIndex](const SCP_string& v) {
		_model->setBranchLoopAnimAt(missionIndex, branchIndex, v);
		updateLoopDetails();
	});
	_dialogStack->push(cmd);
}

void CampaignEditorDialog::on_loopVoiceLineEdit_textChanged(const QString& arg1)
{
	if (!_model->getCurrentBranchIsSpecial())
		return;

	const int missionIndex  = _model->getCurrentMissionSelection();
	const int branchIndex   = _model->getCurrentBranchSelection();
	const SCP_string before = _model->getCurrentBranchLoopVoice();
	_model->setCurrentBranchLoopVoice(arg1.toUtf8().constData());
	const SCP_string after = _model->getCurrentBranchLoopVoice();
	enableDisableControls();
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::Camp_LoopVoice +
	        (missionIndex * FieldId::Camp_BranchesPerMission + branchIndex) * FieldId::Camp_BranchFieldStride,
	    nullptr, tr("Change Loop Voice"), true);
	cmd->addEntry(before, after, [this, missionIndex, branchIndex](const SCP_string& v) {
		_model->setBranchLoopVoiceAt(missionIndex, branchIndex, v);
		updateLoopDetails();
	});
	_dialogStack->push(cmd);
}

void CampaignEditorDialog::on_loopAnimBrowseButton_clicked()
{
	const QString lastDir = util::getLastDir("campaign/loopAnim", CF_TYPE_INTERFACE);

	const QString filter = "FSO Animations (*.ani *.eff *.png);;All Files (*.*)";
	const QString fileName = QFileDialog::getOpenFileName(this, "Select Loop Animation", lastDir, filter);
	if (!fileName.isEmpty()) {
		util::saveLastDir("campaign/loopAnim", fileName);
		// Store the bare filename; the resulting textChanged updates the
		// model and pushes the undo entry.
		ui->loopAnimLineEdit->setText(QFileInfo(fileName).fileName());
	}
}

void CampaignEditorDialog::on_loopVoiceBrowseButton_clicked()
{
	const QString lastDir = util::getLastDir("campaign/loopVoice", CF_TYPE_VOICE_SPECIAL);

	const QString filter = "Audio Files (*.wav *.ogg);;All Files (*.*)";
	const QString fileName = QFileDialog::getOpenFileName(this, "Select Loop Voice", lastDir, filter);
	if (!fileName.isEmpty()) {
		util::saveLastDir("campaign/loopVoice", fileName);
		ui->loopVoiceLineEdit->setText(QFileInfo(fileName).fileName());
	}
}

void CampaignEditorDialog::on_testVoiceButton_clicked()
{
	_model->testCurrentBranchLoopVoice();
}

void CampaignEditorDialog::on_retailFormatCheckbox_toggled(bool checked)
{
	const bool before = (_model->getSaveFormat() == CampaignFormat::Retail);
	if (before == checked)
		return;

	_model->setSaveFormat(checked ? CampaignFormat::Retail : CampaignFormat::FSO);
	refreshMainhallCombos(); // the format decides which main hall names are valid
	updateMissionDetails();

	auto* cmd = new FieldEditCommand<bool>(FieldId::Camp_RetailFormat, nullptr, tr("Toggle Retail Format"), true);
	cmd->addEntry(before, checked, [this](const bool& v) {
		_model->setSaveFormat(v ? CampaignFormat::Retail : CampaignFormat::FSO);
		{
			QSignalBlocker blocker(ui->retailFormatCheckbox);
			ui->retailFormatCheckbox->setChecked(v);
		}
		refreshMainhallCombos();
		updateMissionDetails();
	});
	_dialogStack->push(cmd);
}

} // namespace fso::fred::dialogs

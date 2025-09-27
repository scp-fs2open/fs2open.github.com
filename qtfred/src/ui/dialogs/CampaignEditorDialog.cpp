#include "CampaignEditorDialog.h"
#include "ui_CampaignEditorDialog.h"

#include "ui/widgets/sexp_tree.h"
#include "ui/widgets/SimpleListSelectDialog.h"
#include "ui/util/SignalBlockers.h"
#include "mission/util.h"
#include <ui/dialogs/MissionSpecs/CustomDataDialog.h>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QAbstractItemView>
#include <ui/FredView.h>

namespace fso::fred::dialogs {

CampaignEditorDialog::CampaignEditorDialog(QWidget* parent, EditorViewport* viewport)
	: QMainWindow(parent), SexpTreeEditorInterface({TreeFlags::LabeledRoot, TreeFlags::RootDeletable}),
	  ui(new Ui::CampaignEditorDialog), _viewport(viewport)
{
	ui->setupUi(this);

	// Build the Qt adapter for our data model
	// This is kinda messy but the sexp_tree widget owns both the ui and the data for the tree
	// Simultaneously our tree model needs to be able to tell the tree when things change and also
	// be able to read data from the tree as needed. So we pass in this small adapter object with
	// the relevant tree operations allowing the model to do all the cross talk it needs
	struct QtCampaignTreeOps final : ICampaignEditorTreeOps {
		explicit QtCampaignTreeOps(sexp_tree& t) : tree(t) {}
		sexp_tree& tree;

		int loadSexp(int formula_index) override
		{
			// The sexp_tree's load_sub_tree is what creates the internal model for a branch
			return tree.load_sub_tree(formula_index, true, "true");
		}

		int saveSexp(int internal_node_id) override
		{
			// First, find the root of the branch that contains the edited node.
			int root_node = tree.get_root(internal_node_id);

			// Now, save the entire branch starting from its root.
			return tree.save_tree(root_node);
		}

		int createDefaultSexp() override
		{
			// A default branch is just a "true" condition. We load it from an invalid index.
			return tree.load_sub_tree(-1, true, "true");
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
				rootItem->setData(0, sexp_tree::FormulaDataRole, branch.sexp_formula);
				tree.add_sub_tree(branch.sexp_formula, rootItem);
			}
		}

		void expandBranch(int internal_node_id) override
		{
			// Find the visual tree item corresponding to the internal node
			for (int i = 0; i < tree.topLevelItemCount(); ++i) {
				auto* item = tree.topLevelItem(i);
				if (item && item->data(0, sexp_tree::FormulaDataRole).toInt() == internal_node_id) {
					// Call the widget's expand_branch method
					tree.expand_branch(item);
					break;
				}
			}
		}
	};

	_treeOps = std::make_unique<QtCampaignTreeOps>(QtCampaignTreeOps{*ui->sxtBranches});

	ui->sxtBranches->initializeEditor(_viewport->editor, this);
	ui->sxtBranches->clear_tree();
	ui->sxtBranches->post_load();

	// Now construct the model with reference to tree ops
	_model = std::make_unique<CampaignEditorDialogModel>(this, _viewport, *_treeOps);

	ui->mainTabs->setCurrentIndex(0); // Ensure the first tab is selected

	_model = std::make_unique<CampaignEditorDialogModel>(this, _viewport, *_treeOps); // model exists now
	ui->graphView->setModel(_model.get());

	// Connect sexp tree signals
	connect(
		ui->sxtBranches,
		&sexp_tree::rootNodeDeleted,
		this,
		[this](int formulaNodeId) {
			int mission_selection = _model->getCurrentMissionSelection(); // save now because rebuild clears it

			// Remove the branch
			_model->removeBranchByTreeId(formulaNodeId);

			// Rebuild the graph view
			ui->graphView->rebuildAll();
			ui->graphView->setSelectedMission(mission_selection);

			updateLoopDetails();
		},
		Qt::QueuedConnection);

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

	connect(ui->sxtBranches, &sexp_tree::nodeChanged, this, [this](int node) {
		_model->updateCurrentBranch(node);

		updateLoopDetails();
	});

	initializeUi();
	updateUi();
}
CampaignEditorDialog::~CampaignEditorDialog() = default;

void CampaignEditorDialog::closeEvent(QCloseEvent* e)
{
	// First, ask the user if they want to save any pending changes.
	if (questionSaveChanges()) {
		// If the user didn't cancel, it's safe to accept the close event.
		e->accept();
	} else {
		// If the user cancelled, we ignore the close event to keep the window open.
		e->ignore();
	}
}

void CampaignEditorDialog::initializeUi()
{
	util::SignalBlockers blocker(this);

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
	auto mainhalls = _model->getMainhallList();
	QStringList mainhallList;
	for (const auto& mh : mainhalls) {
		mainhallList.append(QString::fromStdString(mh));
	}

	ui->mainhallComboBox->clear();
	ui->mainhallComboBox->addItems(mainhallList);
	ui->substituteMainhallComboBox->clear();
	ui->substituteMainhallComboBox->addItems(mainhallList);

	ui->fredMissionButton->setHidden(true); // TODO activate this when QtFRED is closer to completion
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
	ui->moveBranchUpButton->setEnabled(branch_selected && _model->getCurrentBranchSelection() > 0);
	ui->moveBranchDownButton->setEnabled(branch_selected && _model->getCurrentBranchSelection() < _model->getNumBranches() -1);

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
		on_actionSave_triggered();
	}

	// If we get here, the user chose Save or Discard.
	return true;
}

void CampaignEditorDialog::on_actionNew_triggered()
{
	// Check if there are unsaved changes.
	if (!questionSaveChanges()) {
		return; // User cancelled.
	}

	_model->createNewCampaign();
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
	QString pathName = QFileDialog::getOpenFileName(this, "Load Campaign", "", "FS2 Campaigns (*.fc2)");

	if (pathName.isEmpty()) {
		return; // User cancelled the file dialog.
	}

	QString nativePath = QDir::toNativeSeparators(pathName);

	_model->loadCampaignFromFile(nativePath.toUtf8().constData());
	updateUi();
	ui->graphView->zoomToFitAll();
}

void CampaignEditorDialog::on_actionSave_triggered()
{
	// This action saves to the currently known filename.
	// If the filename is empty (because it's a new campaign), this will
	// delegate to the Save As logic.
	if (_model->getCampaignFilename().empty()) {
		on_actionSave_As_triggered();
		return;
	}

	_model->saveCampaign(""); // Pass empty string to use the current filename.
}

void CampaignEditorDialog::on_actionSave_As_triggered()
{
	// Open a file dialog to let the user choose a save location and filename.
	QString pathName = QFileDialog::getSaveFileName(this, "Save Campaign As", "", "FS2 Campaigns (*.fc2)");

	if (pathName.isEmpty()) {
		return; // User cancelled the file dialog.
	}

	// The model will handle the actual save operation.
	_model->saveCampaign(pathName.toUtf8().constData());
}

void CampaignEditorDialog::on_actionExit_triggered()
{
	this->close();
}

void CampaignEditorDialog::on_nameLineEdit_textChanged(const QString& arg1)
{
	_model->setCampaignName(arg1.toUtf8().constData());
}

void CampaignEditorDialog::on_typeComboBox_currentIndexChanged(int index)
{
	// The model should have a list of campaign types matching the combo box.
	if (SCP_vector_inbounds(_model->getCampaignTypes(), index)) {
		_model->setCampaignType(index);
	}
}

void CampaignEditorDialog::on_resetTechAtStartCheckBox_toggled(bool checked)
{
	_model->setCampaignTechReset(checked);
}

void CampaignEditorDialog::on_campaignCustomDataButton_clicked()
{
	CustomDataDialog dlg(this, _viewport);
	dlg.setInitial(_model->getCustomData());

	if (dlg.exec() == QDialog::Accepted) {
		_model->setCustomData(dlg.items());
	}
}

void CampaignEditorDialog::on_descriptionPlainTextEdit_textChanged()
{
	_model->setCampaignDescription(ui->descriptionPlainTextEdit->toPlainText().toUtf8().constData());
}

void CampaignEditorDialog::on_shipsListWidget_itemChanged(QListWidgetItem* item)
{
	if (!item) {
		return;
	}

	const int ship_class_index = item->data(Qt::UserRole).toInt();
	const bool is_allowed = (item->checkState() == Qt::Checked);
	_model->setAllowedShip(ship_class_index, is_allowed);
}

void CampaignEditorDialog::on_weaponsListWidget_itemChanged(QListWidgetItem* item)
{
	if (!item) {
		return;
	}

	const int weapon_class_index = item->data(Qt::UserRole).toInt();
	const bool is_allowed = (item->checkState() == Qt::Checked);
	_model->setAllowedWeapon(weapon_class_index, is_allowed);
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

	// Get the filename from the item's text
	SCP_string filename = selected_item->text().toUtf8().constData();

	mission mission_info;
	if (get_mission_info(filename.c_str(), &mission_info) != 0) {
		// Failed to retrieve mission info, clear fields and return
		ui->missionNameLineEdit->clear();
		ui->missionDescriptionPlainTextEdit->clear();
		return;
	}

	if (mission_info.name[0] != '\0') {
		ui->missionNameLineEdit->setText(QString::fromUtf8(mission_info.name));
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

	if (mission_info.name[0] != '\0') {
		ui->missionNameLineEdit->setText(QString::fromUtf8(mission_info.name));
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
	_model->toggleMissionSpecialMode(missionIndex);
	ui->graphView->rebuildAll();
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
}

void CampaignEditorDialog::on_graphView_deleteMissionRequested(int missionIndex)
{
	ui->graphView->clearSelectedMission();
	updateMissionDetails();
	_model->removeMission(missionIndex);
	ui->graphView->rebuildAll();
	updateAvailableMissionsList();
}

void CampaignEditorDialog::on_graphView_addRepeatBranchRequested(int missionIndex)
{
	_model->addBranch(missionIndex, missionIndex);
	ui->graphView->rebuildAll();
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
}

void CampaignEditorDialog::on_graphView_setFirstMissionRequested(int missionIndex)
{
	int current_selection = _model->getCurrentMissionSelection(); // save now because rebuild clears it
	_model->setMissionAsFirst(missionIndex);
	ui->graphView->rebuildAll();

	if (current_selection == missionIndex) {
		// If we changed the first mission, the selection index changed too
		current_selection = 0;
	}

	ui->graphView->setSelectedMission(current_selection);
}

void CampaignEditorDialog::on_briefCutsceneComboBox_currentIndexChanged(const QString& arg1)
{
	_model->setCurrentMissionBriefingCutscene(arg1.toUtf8().constData());
}

void CampaignEditorDialog::on_debriefingPersonaSpinBox_valueChanged(int arg1)
{
	_model->setCurrentMissionDebriefingPersona(arg1);
}

void CampaignEditorDialog::on_mainhallComboBox_currentIndexChanged(const QString& arg1)
{
	_model->setCurrentMissionMainhall(arg1.toUtf8().constData());
}

void CampaignEditorDialog::on_substituteMainhallComboBox_currentIndexChanged(const QString& arg1)
{
	_model->setCurrentMissionSubstituteMainhall(arg1.toUtf8().constData());
}

void CampaignEditorDialog::on_moveBranchUpButton_clicked()
{
	int mission_selection = _model->getCurrentMissionSelection(); // save now because rebuild clears it

	_model->moveBranchUp();
	ui->graphView->rebuildAll();

	ui->graphView->setSelectedMission(mission_selection);
}

void CampaignEditorDialog::on_moveBranchDownButton_clicked()
{
	int mission_selection = _model->getCurrentMissionSelection(); // save now because rebuild clears it
	
	_model->moveBranchDown();
	ui->graphView->rebuildAll();

	ui->graphView->setSelectedMission(mission_selection);
}

void CampaignEditorDialog::on_loopDescriptionPlainTextEdit_textChanged()
{
	_model->setCurrentBranchLoopDescription(ui->loopDescriptionPlainTextEdit->toPlainText().toUtf8().constData());
}

void CampaignEditorDialog::on_loopAnimLineEdit_textChanged(const QString& arg1)
{
	_model->setCurrentBranchLoopAnim(arg1.toUtf8().constData());

	bool enable_playback = _model->getCurrentBranchIsSpecial() && !_model->getCurrentBranchLoopVoice().empty();
	ui->testVoiceButton->setEnabled(enable_playback);
}

void CampaignEditorDialog::on_loopVoiceLineEdit_textChanged(const QString& arg1)
{
	_model->setCurrentBranchLoopVoice(arg1.toUtf8().constData());
}

void CampaignEditorDialog::on_loopAnimBrowseButton_clicked()
{
	util::SignalBlockers blocker(this);
	
	QString filter = "FSO Animations (*.ani *.eff *.png);;All Files (*.*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Select Loop Animation", "", filter);
	if (!fileName.isEmpty()) {
		ui->loopAnimLineEdit->setText(fileName);
	}
}

void CampaignEditorDialog::on_loopVoiceBrowseButton_clicked()
{
	util::SignalBlockers blocker(this);
	
	QString filter = "Audio Files (*.wav *.ogg);;All Files (*.*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Select Loop Voice", "", filter);
	if (!fileName.isEmpty()) {
		ui->loopVoiceLineEdit->setText(fileName);
	}

	bool enable_playback = _model->getCurrentBranchIsSpecial() && !_model->getCurrentBranchLoopVoice().empty();
	ui->testVoiceButton->setEnabled(enable_playback);
}

void CampaignEditorDialog::on_testVoiceButton_clicked()
{
	_model->testCurrentBranchLoopVoice();
}

void CampaignEditorDialog::on_retailFormatCheckbox_toggled(bool checked)
{
	_model->setSaveFormat(checked ? CampaignFormat::Retail : CampaignFormat::FSO);
}

} // namespace fso::fred::dialogs

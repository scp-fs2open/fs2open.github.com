#include "CampaignEditorDialog.h"
#include "ui_CampaignEditorDialog.h"

#include "ui/widgets/sexp_tree.h"
#include "ui/widgets/SimpleListSelectDialog.h"
#include "ui/util/SignalBlockers.h"
#include "mission/util.h"
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QAbstractItemView>
#include <ui/FredView.h>

namespace fso::fred::dialogs {

CampaignEditorDialog::CampaignEditorDialog(QWidget* _parent, EditorViewport* _viewport)
	: QMainWindow(_parent), SexpTreeEditorInterface({TreeFlags::LabeledRoot, TreeFlags::RootDeletable}),
	  ui(new Ui::CampaignEditorDialog), _viewport(_viewport)
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

				// For self-loops, use a special caption
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
		});

	connect(ui->sxtBranches,
		&sexp_tree::nodeChanged,
		_model.get(),
		&CampaignEditorDialogModel::updateCurrentBranch,
		Qt::QueuedConnection);

	initializeUi();
	updateUi();

	/*connect(&warnings, &CampaignEditorUtil::WarningVec::gotMsg, this, [&]() {
		for (auto warn_it = warnings.begin(); warn_it != warnings.end(); warn_it = warnings.erase(warn_it)) {
			if (warn_it->type.isEmpty()) {
				QMessageBox::warning(this, warn_it->title, warn_it->msg);
			}
		}
	});
	warnings.gotMsg();

	QPalette p = ui->lblMissionDescr1->palette();
	p.setColor(QPalette::WindowText, Qt::darkYellow);
	ui->lblMissionDescr1->setPalette(p);
	p = ui->lblMissionDescr2->palette();
	p.setColor(QPalette::WindowText, Qt::red);
	ui->lblMissionDescr2->setPalette(p);

	connect(ui->lstMissions, &QListView::clicked, this, &CampaignEditorDialog::lstMissionsClicked);

	setModel();

	connect(ui->lstMissions, &QListView::customContextMenuRequested, this, &CampaignEditorDialog::mnLinkMenu);

	connect(ui->btnErrorChecker, &QPushButton::clicked, []() {
		uiWarn(tr("Error Checker"), tr("Error checker not implemented. Try saving a copy."));
	});
	connect(ui->btnFredMission, &QPushButton::clicked, this, [&]() {
		reject();
		if (result() == Rejected)
			qobject_cast<FredView*>(parent)->loadMissionFile(model->getCurMnFilename());
	});
	connect(ui->btnFirstMission, &QPushButton::clicked, this, [&](bool toggledOn) {
		Assertion(toggledOn, "Should not be able to unset first mission");
		model->setCurMnFirst();
		updateUIMission(false);
	});
	connect(ui->btnExit, &QPushButton::clicked, this, &CampaignEditorDialog::reject);

	QMenu* menFile{new QMenu(this)};
	ui->btnMenu->setMenu(menFile);
	QMenu* menFileNew{menFile->addMenu(tr("&New"))};
	for (auto& campaignType : model->campaignTypes)
		menFileNew->addAction(campaignType, this, &CampaignEditorDialog::fileNew);
	menFile->addSeparator();

	menFile->addAction(tr("&Open..."), this, &CampaignEditorDialog::fileOpen, tr("Ctrl+O"));
	menFile->addAction(tr("&Save"), this, &CampaignEditorDialog::fileSave, tr("Ctrl+S"));
	menFile->addAction(tr("Save &as..."), this, &CampaignEditorDialog::fileSaveAs, tr("Ctrl+Shift+S"));
	menFile->addAction(tr("Save &Copy as..."), this, &CampaignEditorDialog::fileSaveCopyAs);
	menFile->addSeparator();
	menFile->addAction(tr("E&xit"), this, &QDialog::reject);

	updateUIAll();*/
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
}

void CampaignEditorDialog::updateUi()
{
	util::SignalBlockers blocker(this);

	ui->nameLineEdit->setText(QString::fromStdString(_model->getCampaignName()));
	ui->typeComboBox->setCurrentIndex(_model->getCampaignType());
	ui->resetTechAtStartCheckBox->setChecked(_model->getCampaignTechReset());
	ui->descriptionPlainTextEdit->setPlainText(QString::fromStdString(_model->getCampaignDescription()));
	
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
	// TODO fill in mission details from model

	enableDisableControls();
}

void CampaignEditorDialog::enableDisableControls()
{
	bool has_mission = (!_model->getCurrentMissionFilename().empty()); // TODO Add an actual validator to the model??

	ui->sxtBranches->setEnabled(has_mission);

	// TODO enable/disable other controls based on context
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

	if (mission_info.name) {
		ui->missionNameLineEdit->setText(QString::fromUtf8(mission_info.name));
	} else {
		ui->missionNameLineEdit->clear();
	}

	if (mission_info.notes) {
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

	if (mission_info.name) {
		ui->missionNameLineEdit->setText(QString::fromUtf8(mission_info.name));
	} else {
		ui->missionNameLineEdit->clear();
	}

	if (mission_info.notes) {
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
	if (selections.size() > 0) {
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

	// New mission index is the last element now (or look it up by filename)
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
	// 1) Ask user via your available-missions listwidget
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

	// 2) Add the mission to the model
	_model->addMission(picked, /*level*/ 0, /*position*/ 0);

	// New mission index (last)
	const auto& ms = _model->getCampaignMissions();
	const int newIdx = static_cast<int>(ms.size()) - 1;

	// 3) Persist graph placement
	_model->setMissionGraphX(newIdx, static_cast<int>(std::lround(sceneTopLeft.x())));
	_model->setMissionGraphY(newIdx, static_cast<int>(std::lround(sceneTopLeft.y())));

	// 4) Connect from source to the new mission
	if (isSpecial) {
		_model->addSpecialBranch(fromIndex, newIdx);
	} else {
		_model->addBranch(fromIndex, newIdx);
	}

	// 5) Rebuild & focus new node
	ui->graphView->rebuildAll();
	ui->graphView->setSelectedMission(newIdx);
	updateAvailableMissionsList();
}

/*void CampaignEditorDialog::setModel(CampaignEditorDialogModel* new_model)
{
	if (new_model)
		model = std::unique_ptr<CampaignEditorDialogModel>(new_model);

	ui->cmbBriefingCutscene->setModel(new QStringListModel{CampaignEditorDialogModel::cutscenes(), model.get()});
	ui->cmbMainhall->setModel(new QStringListModel{CampaignEditorDialogModel::mainhalls(), model.get()});
	ui->cmbDebriefingPersona->setModel(
		new QStringListModel{CampaignEditorDialogModel::debriefingPersonas(), model.get()});
	ui->cmbLoopAnim->setModel(new QStringListModel{CampaignEditorDialogModel::loopAnims(), model.get()});
	ui->cmbLoopVoice->setModel(new QStringListModel{CampaignEditorDialogModel::loopVoices(), model.get()});

	model->supplySubModels(*(ui->lstShips), *(ui->lstWeapons), *(ui->lstMissions), *(ui->txaDescr));

	connect(ui->lstMissions->selectionModel(),
		&QItemSelectionModel::selectionChanged,
		model.get(),
		&CampaignEditorDialogModel::missionSelectionChanged);

	connect(ui->txtName, &QLineEdit::textChanged, model.get(), &CampaignEditorDialogModel::setCampaignName);
	connect(ui->chkTechReset, &QCheckBox::stateChanged, model.get(), [&](int changed) {
		model->setCampaignTechReset(changed == Qt::Checked);
	});

	connect(ui->cmbBriefingCutscene,
		&QComboBox::currentTextChanged,
		model.get(),
		&CampaignEditorDialogModel::setCurMnBriefingCutscene);
	connect(ui->cmbMainhall,
		&QComboBox::currentTextChanged,
		model.get(),
		&CampaignEditorDialogModel::setCurMnMainhall);
	connect(ui->cmbDebriefingPersona,
		&QComboBox::currentTextChanged,
		model.get(),
		&CampaignEditorDialogModel::setCurMnDebriefingPersona);

	ui->sxtBranches->initializeEditor(nullptr, model.get());
	connect(ui->sxtBranches,
		&sexp_tree::rootNodeDeleted,
		model.get(),
		&CampaignEditorDialogModel::delCurMnBranch,
		Qt::QueuedConnection); // will call removeBranch()
	connect(ui->sxtBranches,
		&QTreeWidget::currentItemChanged,
		model.get(),
		&CampaignEditorDialogModel::selectCurBr); // will call setCurrentBranchSelection()
	connect(
		ui->sxtBranches,
		&sexp_tree::nodeChanged,
		model.get(),
		[&](int node) { // will call updateCurrentBranch()
			bool success = model->setCurBrSexp(ui->sxtBranches->save_tree(ui->sxtBranches->get_root(node)));
			if (!success) {
				int old = model->getCurBrIdx();
				restoreBranchOpen(old);
			}
		},
		Qt::QueuedConnection);

	connect(ui->cmbLoopAnim,
		&QComboBox::currentTextChanged,
		model.get(),
		&CampaignEditorDialogModel::setCurLoopAnim);
	connect(ui->cmbLoopVoice,
		&QComboBox::currentTextChanged,
		model.get(),
		&CampaignEditorDialogModel::setCurLoopVoice);
}

void CampaignEditorDialog::reject()
{ // merely means onClose
	if (!questionSaveChanges())
		return;

	QDialog::reject();
	deleteLater();
}

void CampaignEditorDialog::updateUISpec()
{
	util::SignalBlockers blockers(this);

	setWindowTitle(model->campaignFile.isEmpty() ? "Untitled" : model->campaignFile + ".fc2");

	ui->txtName->setText(model->getCampaignName());
	if (CampaignEditorDialogModel::campaignTypes.indexOf(model->campaignType))
		ui->txtType->setText(model->campaignType + ": " + QString::number(model->numPlayers));
	else
		ui->txtType->setText(model->campaignType);
	ui->chkTechReset->setChecked(model->getCampaignTechReset());
}

void CampaignEditorDialog::updateUIMission(bool updateBranch)
{
	util::SignalBlockers blockers(this);

	ui->btnFirstMission->setEnabled(model->getCurMnIncluded() && !model->isCurMnFirst());
	ui->btnFirstMission->setChecked(model->isCurMnFirst());
	ui->btnFredMission->setEnabled(model->getCurMnFredable());
	ui->txbMissionDescr->setText(model->getCurMnDescr());

	ui->cmbBriefingCutscene->setCurrentText(model->getCurMnBriefingCutscene());
	ui->cmbMainhall->setCurrentText(model->getCurMnMainhall());
	ui->cmbDebriefingPersona->setCurrentText(model->getCurMnDebriefingPersona());

	bool included{model->getCurMnIncluded()};
	ui->cmbBriefingCutscene->setEnabled(included);
	ui->cmbMainhall->setEnabled(included);
	ui->cmbDebriefingPersona->setEnabled(included);

	ui->sxtBranches->setEnabled(included);
	ui->sxtBranches->clear_tree("");

	if (included)
		model->fillTree(*ui->sxtBranches);

	if (updateBranch)
		updateUIBranch();
}

void CampaignEditorDialog::updateUIBranch(int selectedIdx)
{
	util::SignalBlockers blockers(this);

	if (selectedIdx >= 0)
		model->selectCurBr(ui->sxtBranches->topLevelItem(selectedIdx));
	else
		selectedIdx = model->getCurBrIdx();

	bool sel = selectedIdx >= 0;

	if (sel) {
		ui->sxtBranches->selectionModel()->select(ui->sxtBranches->model()->index(selectedIdx, 0),
			QItemSelectionModel::Select);
	}

	bool loop = model->isCurBrLoop();

	model->supplySubModelLoop(*ui->txaLoopDescr);
	ui->txaLoopDescr->setEnabled(loop);

	ui->cmbLoopAnim->setCurrentText(model->getCurLoopAnim());
	ui->cmbLoopAnim->setEnabled(loop);

	ui->cmbLoopVoice->setCurrentText(model->getCurLoopVoice());
	ui->cmbLoopVoice->setEnabled(loop);
}

void fso::fred::dialogs::CampaignEditorDialog::restoreBranchOpen(int branch)
{
	updateUIMission(false);
	updateUIBranch(branch);
	ui->sxtBranches->expand_branch(ui->sxtBranches->topLevelItem(branch));
}

bool CampaignEditorDialog::questionSaveChanges()
{
	QMessageBox::StandardButton resBtn = QMessageBox::Discard;
	if (model->query_modified()) {
		QString msg = tr("This campaign has been modified.\n") +
						(model->missionDropped() ? tr("Additionally, packaged/missing\nmission(s) have been "
													"removed,\nwhich cannot be added again.\n")
												: "") +
						tr("\nSave changes?");
		resBtn = QMessageBox::question(this,
			tr("Unsaved changes"),
			msg,
			QMessageBox::Cancel | QMessageBox::Discard | QMessageBox::Save,
			QMessageBox::Save);
	}

	switch (resBtn) {
	case QMessageBox::Discard:
		model->reject();
		return true;
	case QMessageBox::Save:
		return fileSave();
	case QMessageBox::Cancel:
		return false;
	default:
		UNREACHABLE("An unhandled button was pressed. A coder must handle the buttons they provide.");
		return false;
	}
}

void CampaignEditorDialog::fileNew()
{
	if (!questionSaveChanges())
		return;

	auto* act = qobject_cast<QAction*>(sender());
	if (!act)
		return;

	setModel(new CampaignEditorDialogModel(this,
		viewport,
		"",
		act->text(),
		act->text().contains("multi")
			? QInputDialog::getInt(this, "Campaign Player Number", "Enter campaign player number", 2, 2)
			: 0));
	updateUIAll();
}

void CampaignEditorDialog::fileOpen()
{
	if (!questionSaveChanges())
		return;

	QString pathName =
		QFileDialog::getOpenFileName(this, tr("Load campaign"), model->campaignFile, tr("FS2 campaigns (*.fc2)"));

	if (pathName.isEmpty())
		return;

	auto newModel = new CampaignEditorDialogModel(this, viewport, pathName);
	if (newModel->isFileLoaded()) {
		setModel(newModel);
	} else {
		delete newModel;
		uiWarn(tr("Error opening file"), pathName);
		setModel(new CampaignEditorDialogModel(this, viewport, ""));
	}

	updateUIAll();
}

bool CampaignEditorDialog::fileSave()
{
	if (model->campaignFile.isEmpty())
		return fileSaveAs();

	bool res = model->apply();

	updateUIAll();
	return res;
}

bool CampaignEditorDialog::fileSaveAs()
{
	QString pathName = QFileDialog::getSaveFileName(this,
		tr("Save campaign as"),
		model->campaignFile,
		tr("FS2 campaigns (*.fc2)"));
	if (pathName.isEmpty())
		return false;

	bool res = model->saveTo(pathName);
	if (res)
		setModel(new CampaignEditorDialogModel(this, viewport, pathName));

	updateUIAll();
	return res;
}

void CampaignEditorDialog::fileSaveCopyAs()
{
	QString pathName =
		QFileDialog::getSaveFileName(this, tr("Save copy as"), model->campaignFile, tr("FS2 campaigns (*.fc2)"));
	if (pathName.isEmpty())
		return;

	model->saveTo(pathName);
}

void CampaignEditorDialog::lstMissionsClicked(const QModelIndex& idx)
{
	QItemSelectionModel& sel = *ui->lstMissions->selectionModel();
	bool same = sel.selectedIndexes().contains(idx);
	sel.clearSelection();
	if (!same)
		sel.select(idx, QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
}

void CampaignEditorDialog::mnLinkMenu(const QPoint& pos)
{
	QModelIndex here = ui->lstMissions->indexAt(pos);
	if (here.data(Qt::CheckStateRole) != Qt::Checked)
		return;

	const QString* mnName = model->missionName(here);
	if (!mnName)
		return;
	const QStringList* goals{model->missionGoals(here)};
	if (!goals)
		return;
	const QStringList* evts{model->missionEvents(here)};
	if (!evts)
		return;

	QMenu menu{ui->lstMissions};

	QAction* to = menu.addAction(tr("Add branch to ") + mnName);
	QAction* from = menu.addAction(tr("Add branch from ") + mnName);
	QAction* end = menu.addAction(tr("Add campaign end"));
	bool mnSel{model->isCurMnSelected()};
	to->setEnabled(mnSel);
	from->setEnabled(mnSel);
	end->setEnabled(mnSel);
	if (!mnSel) {
		menu.exec(ui->lstMissions->mapToGlobal(pos));
		return;
	}

	QMenu* par{nullptr};

	if (model->getCurBrIdx() < 0) {
		menu.addSection(tr("Select a branch to choose conditions!"));
		menu.addAction("")->setEnabled(false);
	} else {
		menu.addSection(tr("Branch Conditions"));

		QMenu* gt = menu.addMenu("is-previous-goal-true");
		QMenu* gf = menu.addMenu("is-previous-goal-false");
		for (auto& g : *goals) {
			gt->addAction(g);
			gf->addAction(g);
		}
		connect(gt, &QMenu::aboutToShow, this, [&]() { par = gt; });
		connect(gf, &QMenu::aboutToShow, this, [&]() { par = gf; });

		QMenu* et = menu.addMenu("is-previous-event-true");
		QMenu* ef = menu.addMenu("is-previous-event-false");
		for (auto& e : *evts) {
			et->addAction(e);
			ef->addAction(e);
		}
		connect(et, &QMenu::aboutToShow, this, [&]() { par = et; });
		connect(ef, &QMenu::aboutToShow, this, [&]() { par = ef; });
	}

	const QAction* choice = menu.exec(ui->lstMissions->mapToGlobal(pos));

	int res_branch{-1};
	if (choice == to) {
		res_branch = model->addCurMnBranchTo(&here);
	} else if (choice == from) {
		res_branch = model->addCurMnBranchTo(&here, true);
	} else if (choice == end) {
		res_branch = model->addCurMnBranchTo(end);
	} else if (choice && par) {
		res_branch = model->setCurBrCond(par->title(), *mnName, choice->text());
	} // else menu was dismissed

	if (res_branch != -1)
		restoreBranchOpen(res_branch);
}*/

} // namespace fso::fred::dialogs

#include "BriefingEditorDialog.h"
#include "ui_BriefingEditorDialog.h"

#include "mission/util.h"
#include "ui/Theme.h"
#include <gamesnd/eventmusic.h>
#include "ui/widgets/BriefingMapWidget.h"
#include "BriefingEditor/CameraCoordinatesDialog.h"
#include "BriefingEditor/IconFromShipDialog.h"
#include "BriefingEditor/IconCoordinatesDialog.h"
#include "mission/missionbriefcommon.h"
#include "mission/missiongrid.h"
#include "math/fvi.h"
#include "mod_table/mod_table.h"

#include <globalincs/globals.h>
#include <globalincs/linklist.h>
#include <ui/util/default_dir.h>
#include <ui/util/DialogUndo.h>
#include <ui/util/SignalBlockers.h>

#include <mission/commands/FredCommands.h>

#include <QCheckBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QVBoxLayout>

namespace fso::fred::dialogs {

namespace {
float movementSpeedScaleForIndex(int index) {
	switch (index) {
	case 0:
		return 4.0f;
	case 1:
		return 8.0f;
	case 2:
	default:
		return 16.0f;
	}
}

float rotationSpeedScaleForIndex(int index) {
	switch (index) {
	case 0:
		return 0.0625f;
	case 1:
		return 0.125f;
	case 2:
	default:
		return 0.25f;
	}
}

vec3d getNewIconPlacement()
{
	const vec3d camPos = brief_get_current_cam_pos();
	const matrix camOrient = brief_get_current_cam_orient();

	float distance = 500.0f;
	if (The_grid != nullptr) {
		vec3d gridHitPos;
		const auto rayDist = fvi_ray_plane(&gridHitPos, &The_grid->center, &The_grid->gmatrix.vec.uvec, &camPos, &camOrient.vec.fvec, 0.0f);
		if (rayDist >= 0.0f) {
			distance = rayDist;
		}
	}

	vec3d placement;
	vm_vec_scale_add(&placement, &camPos, &camOrient.vec.fvec, distance);
	return placement;
}
} // namespace

BriefingEditorDialog::BriefingEditorDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), SexpTreeEditorInterface(flagset<TreeFlags>()), ui(new Ui::BriefingEditorDialog()),
	  _model(new BriefingEditorDialogModel(this, viewport)), _viewport(viewport), _fredView(parent)
{
	_viewportLock.emplace(_viewport->acquireControlLock());

	_dialogStack = new QUndoStack(this);
	_fredView->undoGroup()->addStack(_dialogStack);

	this->setFocus();
	ui->setupUi(this);
	util::setupDialogUndo(this, _fredView->undoGroup(), _dialogStack, tr("Briefing"));

	ui->iconLabelLineEdit->setMaxLength(MAX_LABEL_LEN - 1);
	ui->iconCloseupLabelLineEdit->setMaxLength(MAX_LABEL_LEN - 1);
	ui->voiceFileLineEdit->setMaxLength(MAX_FILENAME_LEN - 1);

	// validate the icon id when the edit is committed (focus-out/Enter) rather than on every
	// keystroke, so a transient value while typing can't pop a collision warning
	ui->iconIdSpinBox->setKeyboardTracking(false);

	setupMapWidget();
	initializeUi();
	updateUi();

	resize(QDialog::sizeHint());
}

BriefingEditorDialog::~BriefingEditorDialog() {
	_viewportLock.reset();
}

void BriefingEditorDialog::accept()
{
	QByteArray stateBefore = _model->captureState();
	if (_model->apply()) {
		ui->defaultMusicWidget->stopPlayback();
		ui->musicPackWidget->stopPlayback();
		QByteArray stateAfter = _model->captureState();
		_model->setParent(nullptr);
		_fredView->mainUndoStack()->push(
			new ApplyDialogCommand(std::move(_model), stateBefore, stateAfter,
			                       _viewport->editor, tr("Edit Briefing")));
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::accept();
		_viewportLock.reset(); // unlock before restoring the grid so the viewport can process controls again
		create_default_grid(); // restore the grid back to the normal version
	}
}

void BriefingEditorDialog::reject()
{
	if (!_model) {
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
		_viewportLock.reset();
		create_default_grid();
		return;
	}
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		ui->defaultMusicWidget->stopPlayback();
		ui->musicPackWidget->stopPlayback();
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
		_viewportLock.reset(); // unlock before restoring the grid so the viewport can process controls again
		create_default_grid(); // restore the grid back to the normal version
	}
}

void BriefingEditorDialog::closeEvent(QCloseEvent* e)
{
	reject();
	// reject() hides the dialog when it actually closes. Let that close
	// proceed (so a dialog created with WA_DeleteOnClose is destroyed),
	// and only veto it when reject() decided to keep the dialog open (e.g.
	// the user cancelled the unsaved-changes prompt).
	if (isVisible()) {
		e->ignore();
	} else {
		e->accept();
	}
}

void BriefingEditorDialog::setupMapWidget()
{
	// Replace the mapView placeholder with the BriefingMapWidget
	_mapWidget = new fso::fred::BriefingMapWidget(this, _model.get(), _viewport);
	_mapWidget->setMinimumSize(ui->mapView->minimumSize());
	_mapWidget->setSizePolicy(ui->mapView->sizePolicy());

	// Insert the map widget in place of the placeholder in the left pane layout
	int idx = ui->leftPaneLayout->indexOf(ui->mapView);
	ui->leftPaneLayout->removeWidget(ui->mapView);
	ui->mapView->hide();
	ui->leftPaneLayout->insertWidget(idx, _mapWidget);
	_mapWidget->setFocus(Qt::OtherFocusReason);

	// Wire icon selection from the map widget to our UI update
	connect(_mapWidget, &fso::fred::BriefingMapWidget::iconSelected, this, [this](int index, bool toggleSelection) {
		auto selection = _model->getLineSelection();
		if (!toggleSelection) {
			selection.clear();
			selection.push_back(index);
		} else {
			const auto it = std::find(selection.begin(), selection.end(), index);
			if (it == selection.end()) {
				selection.push_back(index);
			} else if (selection.size() > 1) {
				selection.erase(it);
			}
		}
		_model->setLineSelection(selection);
		_model->setCurrentIconIndex(index);
		updateUi();
	});

	// Undo wiring for map interactions: icon drags are bracketed into one
	// snapshot per gesture; camera moves become merging pose commands (the
	// cameraChanged handler filters the widget's per-frame reports down to
	// real model changes via the pose cache).
	connect(_mapWidget, &fso::fred::BriefingMapWidget::iconDragStarted, this, [this](int) { onIconDragStarted(); });
	connect(_mapWidget, &fso::fred::BriefingMapWidget::iconDragFinished, this, [this](int) { onIconDragFinished(); });
	connect(_mapWidget, &fso::fred::BriefingMapWidget::cameraChanged, this, [this](vec3d, matrix) { onMapCameraChanged(); });

	// Set the initial stage
	if (_model->getTotalStages() > 0) {
		_mapWidget->setStage(_model->getCurrentStage());
		captureResetCameraForCurrentStage();
	}

	applyMapWidgetAspectRatio();
}

void BriefingEditorDialog::applyMapWidgetAspectRatio()
{
	if (_mapWidget == nullptr) {
		return;
	}

	if (Briefing_window_resolution[0] <= 0 || Briefing_window_resolution[1] <= 0) {
		return;
	}

	auto mapWidth = _mapWidget->width();
	if (mapWidth <= 0) {
		mapWidth = _mapWidget->minimumWidth();
	}
	if (mapWidth <= 0) {
		return;
	}

	const auto targetHeight = std::max(1,
		static_cast<int>(std::lround(static_cast<double>(mapWidth) *
									 static_cast<double>(Briefing_window_resolution[1]) /
									 static_cast<double>(Briefing_window_resolution[0]))));

	_mapWidget->setMinimumHeight(targetHeight);
	_mapWidget->setMaximumHeight(targetHeight);
	_mapWidget->resize(mapWidth, targetHeight);

	const auto oldDialogHeight = height();
	if (auto* dialogLayout = layout(); dialogLayout != nullptr) {
		dialogLayout->activate();
	}

	const auto requiredSize = sizeHint();
	if (requiredSize.height() > oldDialogHeight) {
		resize(width(), requiredSize.height());
	}
}

void BriefingEditorDialog::initializeUi()
{
	fso::fred::bindStandardIcon(ui->voiceFilePlayButton, QStyle::SP_MediaPlay);

	util::SignalBlockers blockers(this);
	ui->drawLinesCheckBox->setTristate(true);
	ui->highlightCheckBox->setTristate(true);
	ui->flipIconCheckBox->setTristate(true);
	ui->useWingIconCheckBox->setTristate(true);
	ui->useCargoIconCheckBox->setTristate(true);

	auto list = _model->getTeamList();

	ui->teamComboBox->clear();

	for (const auto& team : list) {
		ui->teamComboBox->addItem(QString::fromStdString(team.first), team.second);
	}

	auto icons = _model->getIconList();
	for (const auto& icon : icons) {
		ui->iconImageComboBox->addItem(icon.second.c_str(), icon.first);
	}

	auto ships = _model->getShipList();
	for (const auto& ship : ships) {
		ui->iconShipTypeComboBox->addItem(ship.second.c_str(), ship.first);
	}

	auto iffs = _model->getIffList();
	for (const auto& iff : iffs) {
		ui->iconTeamComboBox->addItem(iff.second.c_str(), iff.first);
	}

	// Initialize the formula tree editor
	ui->formulaTreeView->initializeEditor(_viewport->editor, this, _viewport, _fredView);
	_model->setTreeControl(ui->formulaTreeView);
	connect(ui->formulaTreeView, &sexp_tree_view::modified, this, &BriefingEditorDialog::onFormulaTreeModified);

	on_movementSpeedComboBox_currentIndexChanged(ui->movementSpeedComboBox->currentIndex());
	on_rotationSpeedComboBox_currentIndexChanged(ui->rotationSpeedComboBox->currentIndex());
}

void BriefingEditorDialog::updateUi()
{

	util::SignalBlockers blockers(this);

	ui->teamComboBox->setCurrentIndex(ui->teamComboBox->findData(_model->getCurrentTeam()));

	ui->cameraTransitionTimeSpinBox->setValue(_model->getCameraTransitionTime());
	ui->cutToNextStageCheckBox->setChecked(_model->getCutToNext());
	ui->cutToPrevStageCheckBox->setChecked(_model->getCutFromPrev());
	ui->disableGridCheckBox->setChecked(_model->getDisableGrid());
	ui->changeLocallyCheckBox->setChecked(_model->getChangeLocally());

	ui->stageTextPlainTextEdit->setPlainText(QString::fromStdString(_model->getStageText()));
	ui->voiceFileLineEdit->setText(QString::fromStdString(_model->getSpeechFilename()));
	// getBriefingMusicIndex() is 1-based (0 = None); widget uses Spooled_music index (-1 = None)
	ui->defaultMusicWidget->setCurrentMusicIndex(_model->getBriefingMusicIndex() - 1);

	// Substitute music is stored by name; find the corresponding Spooled_music index
	{
		const SCP_string& subName = _model->getSubstituteBriefingMusicName();
		int smIdx = -1;
		for (int i = 0; i < static_cast<int>(Spooled_music.size()); ++i) {
			if (stricmp(Spooled_music[i].name, subName.c_str()) == 0) { smIdx = i; break; }
		}
		ui->musicPackWidget->setCurrentMusicIndex(smIdx);
	}

	SCP_string stages = "No Stages";
	int total = _model->getTotalStages();
	int current = _model->getCurrentStage() + 1; // internal is 0 based, ui is 1 based
	if (total > 0) {
		stages = "Stage ";
		stages += std::to_string(current);
		stages += " of ";
		stages += std::to_string(total);
	}
	ui->currentStageLabel->setText(stages.c_str());

	// SEXP tree formula
	ui->formulaTreeView->load_tree(_model->getFormula());
	ui->formulaTreeView->expandAll();
	if (ui->formulaTreeView->select_sexp_node != -1) {
		ui->formulaTreeView->hilite_item(ui->formulaTreeView->select_sexp_node);
	}

	const bool stage_exists = _model->getTotalStages() > 0 && _model->getCurrentStage() >= 0;
	const auto& lineSelection = _model->getLineSelection();
	const bool icon_selected = stage_exists && !lineSelection.empty();
	const bool enoughForLines = stage_exists && lineSelection.size() >= 2;
	if (icon_selected) {
		ui->iconIdSpinBox->setValue(_model->getIconId());
		ui->iconLabelLineEdit->setText(QString::fromStdString(_model->getIconLabel()));
		ui->iconCloseupLabelLineEdit->setText(QString::fromStdString(_model->getIconCloseupLabel()));
		ui->iconImageComboBox->setCurrentIndex(_model->getIconTypeIndex());
		ui->iconShipTypeComboBox->setCurrentIndex(_model->getIconShipTypeIndex());
		ui->iconTeamComboBox->setCurrentIndex(_model->getIconTeamIndex());
		ui->scaleDoubleSpinBox->setValue(_model->getIconScaleFactor());
		const auto toQtCheckState = [](TriStateBool state) {
			switch (state) {
			case TriStateBool::TRUE_:
				return Qt::Checked;
			case TriStateBool::UNKNOWN_:
				return Qt::PartiallyChecked;
			case TriStateBool::FALSE_:
			default:
				return Qt::Unchecked;
			}
		};
		ui->highlightCheckBox->setCheckState(toQtCheckState(_model->getIconHighlightedState()));
		ui->flipIconCheckBox->setCheckState(toQtCheckState(_model->getIconFlippedState()));
		ui->useWingIconCheckBox->setCheckState(toQtCheckState(_model->getIconUseWingState()));
		ui->useCargoIconCheckBox->setCheckState(toQtCheckState(_model->getIconUseCargoState()));
	}
	if (!icon_selected) {
		ui->highlightCheckBox->setCheckState(Qt::Unchecked);
		ui->flipIconCheckBox->setCheckState(Qt::Unchecked);
		ui->useWingIconCheckBox->setCheckState(Qt::Unchecked);
		ui->useCargoIconCheckBox->setCheckState(Qt::Unchecked);
	}

	switch (_model->getDrawLinesState()) {
	case BriefingEditorDialogModel::DrawLinesState::All:
		ui->drawLinesCheckBox->setCheckState(Qt::Checked);
		break;
	case BriefingEditorDialogModel::DrawLinesState::Partial:
		ui->drawLinesCheckBox->setCheckState(Qt::PartiallyChecked);
		break;
	case BriefingEditorDialogModel::DrawLinesState::None:
	default:
		ui->drawLinesCheckBox->setCheckState(Qt::Unchecked);
		break;
	}
	ui->drawLinesCheckBox->setEnabled(enoughForLines);

	enableDisableControls();
}

void BriefingEditorDialog::enableDisableControls()
{
	int total_stages = _model->getTotalStages();
	int current = _model->getCurrentStage();
	const bool stage_exists = total_stages > 0 && current >= 0;

	ui->prevStageButton->setEnabled(stage_exists && current > 0);
	ui->nextStageButton->setEnabled(stage_exists && current < total_stages - 1);
	ui->addStageButton->setEnabled(total_stages < MAX_BRIEF_STAGES);
	ui->insertStageButton->setEnabled(stage_exists && total_stages < MAX_BRIEF_STAGES);
	ui->deleteStageButton->setEnabled(stage_exists);

	ui->resetCameraButton->setEnabled(stage_exists);
	ui->copyCameraButton->setEnabled(stage_exists);
	ui->pasteCameraButton->setEnabled(stage_exists);
	ui->cameraCoordinatesButton->setEnabled(stage_exists);
	ui->cameraTransitionTimeSpinBox->setEnabled(stage_exists);
	ui->movementSpeedComboBox->setEnabled(stage_exists);
	ui->rotationSpeedComboBox->setEnabled(stage_exists);
	ui->cutToNextStageCheckBox->setEnabled(stage_exists);
	ui->cutToPrevStageCheckBox->setEnabled(stage_exists);
	ui->disableGridCheckBox->setEnabled(stage_exists);

	ui->makeIconButton->setEnabled(stage_exists);
	ui->makeIconFromShipButton->setEnabled(stage_exists);

	ui->teamComboBox->setEnabled(_model->getMissionIsMultiTeam());
	ui->copyToOtherTeamsButton->setEnabled(_model->getMissionIsMultiTeam());

	ui->stageTextPlainTextEdit->setEnabled(stage_exists);
	ui->voiceFileLineEdit->setEnabled(stage_exists);
	ui->voiceFileBrowseButton->setEnabled(stage_exists);
	ui->voiceFilePlayButton->setEnabled(stage_exists && !_model->getSpeechFilename().empty());
	ui->formulaTreeView->setEnabled(stage_exists);
	
	const bool icon_selected = stage_exists && !_model->getLineSelection().empty();
	const bool single_icon_selected = stage_exists && _model->getLineSelection().size() == 1;
	ui->currentIconInfoGroupBox->setEnabled(icon_selected);
	ui->iconCoordinatesButton->setEnabled(single_icon_selected);
	ui->deleteIconButton->setEnabled(single_icon_selected);
	ui->propagateIconButton->setEnabled(single_icon_selected);

}

void BriefingEditorDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void BriefingEditorDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void BriefingEditorDialog::on_prevStageButton_clicked()
{
	_model->gotoPreviousStage();
	_model->setLineSelection({_model->getCurrentIconIndex()});
	_mapWidget->setStage(_model->getCurrentStage());
	captureResetCameraForCurrentStage();
	updateUi();
}

void BriefingEditorDialog::on_nextStageButton_clicked()
{
	_model->gotoNextStage();
	_model->setLineSelection({_model->getCurrentIconIndex()});
	_mapWidget->setStage(_model->getCurrentStage());
	captureResetCameraForCurrentStage();
	updateUi();
}

void BriefingEditorDialog::on_addStageButton_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	const bool creatingFirstStage = (_model->getTotalStages() == 0);
	_model->addStage();
	_model->setLineSelection({_model->getCurrentIconIndex()});

	if (creatingFirstStage) {
		const auto stageView = _model->getStageView();
		_mapWidget->applyCameraToCurrentStage(stageView.first, stageView.second);
	} else {
		_mapWidget->setStage(_model->getCurrentStage());
	}
	captureResetCameraForCurrentStage();
	pushWorkingStateSnapshot(before, tr("Add Stage"));

	updateUi();
}

void BriefingEditorDialog::on_insertStageButton_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_model->insertStage();
	_model->setLineSelection({_model->getCurrentIconIndex()});
	_mapWidget->setStage(_model->getCurrentStage());
	captureResetCameraForCurrentStage();
	pushWorkingStateSnapshot(before, tr("Insert Stage"));
	updateUi();
}

void BriefingEditorDialog::on_deleteStageButton_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_model->deleteStage();
	_model->setLineSelection({_model->getCurrentIconIndex()});
	_mapWidget->setStage(_model->getCurrentStage());
	captureResetCameraForCurrentStage();
	pushWorkingStateSnapshot(before, tr("Delete Stage"));
	updateUi();
}

void BriefingEditorDialog::on_cameraCoordinatesButton_clicked()
{
	CameraCoordinatesDialog dlg(this, _model.get(), _mapWidget);
	dlg.exec(); // modal
}

void BriefingEditorDialog::on_resetCameraButton_clicked()
{
	const int currentTeam = _model->getCurrentTeam();
	const int currentStage = _model->getCurrentStage();
	if (!_resetCameraValid || _resetCameraTeam != currentTeam || _resetCameraStage != currentStage) {
		captureResetCameraForCurrentStage();
	}

	if (_resetCameraValid) {
		// Push an explicit no-merge command and prime the cache so the
		// cameraChanged the apply triggers is a no-op.
		CameraPose before;
		before.pos    = _model->getCameraPosition();
		before.orient = _model->getCameraOrientation();
		CameraPose after;
		after.pos    = _resetCameraPos;
		after.orient = _resetCameraOrient;
		if (memcmp(&before.pos, &after.pos, sizeof(vec3d)) != 0 ||
			memcmp(&before.orient, &after.orient, sizeof(matrix)) != 0) {
			_camPoseCache      = after;
			_camPoseCacheValid = true;
			pushCameraPoseCommand(tr("Reset Camera"), before, after, false);
		}
		_mapWidget->applyCameraToCurrentStage(_resetCameraPos, _resetCameraOrient);
	}
}

void BriefingEditorDialog::on_copyCameraButton_clicked()
{
	_model->copyStageViewToClipboard();
}

void BriefingEditorDialog::on_pasteCameraButton_clicked()
{
	CameraPose before;
	before.pos    = _model->getCameraPosition();
	before.orient = _model->getCameraOrientation();

	_model->pasteClipboardViewToStage();
	const auto stageView = _model->getStageView();

	CameraPose after;
	after.pos    = stageView.first;
	after.orient = stageView.second;
	if (memcmp(&before.pos, &after.pos, sizeof(vec3d)) != 0 ||
		memcmp(&before.orient, &after.orient, sizeof(matrix)) != 0) {
		_camPoseCache      = after;
		_camPoseCacheValid = true;
		pushCameraPoseCommand(tr("Paste Camera View"), before, after, false);
	}

	_mapWidget->applyCameraToCurrentStage(stageView.first, stageView.second);
}

void BriefingEditorDialog::on_copyToOtherTeamsButton_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_model->copyToOtherTeams();
	pushWorkingStateSnapshot(before, tr("Copy To Other Teams"));
}

void BriefingEditorDialog::on_teamComboBox_currentIndexChanged(int index)
{
	const int before = _model->getCurrentTeam();
	const int after  = ui->teamComboBox->itemData(index).toInt();
	if (before == after)
		return;

	const auto applyTeam = [this](int team) {
		_model->setCurrentTeam(team);
		_model->setLineSelection({_model->getCurrentIconIndex()});
		_mapWidget->setStage(_model->getCurrentStage());
		captureResetCameraForCurrentStage();
		updateUi();
	};
	applyTeam(after);

	auto* cmd = new FieldEditCommand<int>(FieldId::Brief_CurrentTeam, nullptr, tr("Change Team"), true);
	cmd->addEntry(before, after, applyTeam);
	_dialogStack->push(cmd);
}

void BriefingEditorDialog::captureResetCameraForCurrentStage()
{
	const auto stageView = _model->getStageView();
	if (_model->getTotalStages() <= 0 || _model->getCurrentStage() < 0) {
		_resetCameraValid = false;
		_resetCameraTeam = -1;
		_resetCameraStage = -1;
		return;
	}

	_resetCameraPos = stageView.first;
	_resetCameraOrient = stageView.second;
	_resetCameraTeam = _model->getCurrentTeam();
	_resetCameraStage = _model->getCurrentStage();
	_resetCameraValid = true;

	// Every editing-context switch (stage/team navigation, stage ops,
	// snapshot restores) comes through here, so it doubles as the camera
	// pose cache reset point.
	resetCameraPoseCache();
}

void BriefingEditorDialog::resetCameraPoseCache()
{
	if (_model->getTotalStages() <= 0 || _model->getCurrentStage() < 0) {
		_camPoseCacheValid = false;
		return;
	}
	_camPoseCache.pos    = _model->getCameraPosition();
	_camPoseCache.orient = _model->getCameraOrientation();
	_camPoseCacheValid   = true;
}

void BriefingEditorDialog::onMapCameraChanged()
{
	// The widget reports the RENDER camera every frame, including stage
	// transition animation, which never touches the model. Only a model pose
	// that differs from the cache is a real user edit.
	if (_model->getTotalStages() <= 0 || _model->getCurrentStage() < 0)
		return;
	if (!_camPoseCacheValid) {
		resetCameraPoseCache();
		return;
	}

	CameraPose current;
	current.pos    = _model->getCameraPosition();
	current.orient = _model->getCameraOrientation();
	if (memcmp(&current.pos, &_camPoseCache.pos, sizeof(vec3d)) == 0 &&
		memcmp(&current.orient, &_camPoseCache.orient, sizeof(matrix)) == 0) {
		return;
	}

	const CameraPose before = _camPoseCache;
	_camPoseCache = current;
	pushCameraPoseCommand(tr("Move Briefing Camera"), before, current, true);
}

void BriefingEditorDialog::pushCameraPoseCommand(const QString& label, const CameraPose& before, const CameraPose& after, bool allowMerge)
{
	const int team  = _model->getCurrentTeam();
	const int stage = _model->getCurrentStage();

	auto* cmd = new FieldEditCommand<CameraPose>(
	    FieldId::Brief_StageCamera + team * MAX_BRIEF_STAGES + stage,
	    nullptr, label, true);
	if (!allowMerge)
		cmd->setNoMerge();
	cmd->addEntry(before, after, [this, team, stage](const CameraPose& v) {
		// Update the cache first so the cameraChanged this triggers is a no-op.
		_camPoseCache      = v;
		_camPoseCacheValid = true;
		_model->setStageCameraAt(team, stage, v.pos, v.orient);
		if (team == _model->getCurrentTeam() && stage == _model->getCurrentStage())
			_mapWidget->applyCameraToCurrentStage(v.pos, v.orient);
	});
	_dialogStack->push(cmd);
}

void BriefingEditorDialog::onFormulaTreeModified()
{
	// Tree edits only reach the working copy via commitCurrentFormula(), so
	// at this point the WIP still holds the pre-edit formula: capture the
	// before-state first, then commit the widget's tree into the WIP.
	const QByteArray before = _model->captureWorkingState();
	_model->commitCurrentFormula();
	_model->setModified();
	pushWorkingStateSnapshot(before, tr("Edit Stage Formula"));
}

void BriefingEditorDialog::pushWorkingStateSnapshot(const QByteArray& before, const QString& label, int mergeId)
{
	const QByteArray after = _model->captureWorkingState();
	if (after == before)
		return;

	_dialogStack->push(new DialogSnapshotCommand(before, after,
		[this](const QByteArray& blob) {
			_model->restoreWorkingState(blob);
			_model->setLineSelection({_model->getCurrentIconIndex()});
			_mapWidget->setStage(_model->getCurrentStage());
			captureResetCameraForCurrentStage();
			_mapWidget->notifyIconVisualsChanged();
			updateUi();
		},
		label, true, mergeId));
}

int BriefingEditorDialog::iconSnapshotMergeId(int base) const
{
	const auto& selection = _model->getLineSelection();
	const int firstIcon = selection.empty() ? 0 : std::max(0, selection.front());
	return base + (_model->getCurrentTeam() * MAX_BRIEF_STAGES + _model->getCurrentStage()) + firstIcon * 80;
}

void BriefingEditorDialog::onIconDragStarted()
{
	_iconDragBefore = _model->captureWorkingState();
}

void BriefingEditorDialog::onIconDragFinished()
{
	if (_iconDragBefore.isEmpty())
		return;
	pushWorkingStateSnapshot(_iconDragBefore, tr("Move Icon"));
	_iconDragBefore.clear();
}

void BriefingEditorDialog::on_cameraTransitionTimeSpinBox_valueChanged(int arg1)
{
	const int team  = _model->getCurrentTeam();
	const int stage = _model->getCurrentStage();
	const int before = _model->getCameraTransitionTime();
	if (before == arg1)
		return;

	_model->setCameraTransitionTime(arg1);

	auto* cmd = new FieldEditCommand<int>(
	    FieldId::Brief_CameraTime + team * MAX_BRIEF_STAGES + stage,
	    nullptr, tr("Change Camera Transition Time"), true);
	cmd->addEntry(before, arg1, [this, team, stage](const int& v) {
		_model->setCameraTransitionTimeAt(team, stage, v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void BriefingEditorDialog::on_movementSpeedComboBox_currentIndexChanged(int index)
{
	if (_mapWidget != nullptr) {
		_mapWidget->setMovementSpeedScale(movementSpeedScaleForIndex(index));
	}
}

void BriefingEditorDialog::on_rotationSpeedComboBox_currentIndexChanged(int index)
{
	if (_mapWidget != nullptr) {
		_mapWidget->setRotationSpeedScale(rotationSpeedScaleForIndex(index));
	}
}

void BriefingEditorDialog::on_cutToNextStageCheckBox_toggled(bool checked)
{
	const int team  = _model->getCurrentTeam();
	const int stage = _model->getCurrentStage();
	const bool before = _model->getCutToNext();
	if (before == checked)
		return;

	_model->setCutToNext(checked);

	auto* cmd = new FieldEditCommand<bool>(
	    FieldId::Brief_CutToNext + team * MAX_BRIEF_STAGES + stage,
	    nullptr, tr("Toggle Cut To Next Stage"), true);
	cmd->addEntry(before, checked, [this, team, stage](const bool& v) {
		_model->setCutToNextAt(team, stage, v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void BriefingEditorDialog::on_cutToPrevStageCheckBox_toggled(bool checked)
{
	const int team  = _model->getCurrentTeam();
	const int stage = _model->getCurrentStage();
	const bool before = _model->getCutFromPrev();
	if (before == checked)
		return;

	_model->setCutFromPrev(checked);

	auto* cmd = new FieldEditCommand<bool>(
	    FieldId::Brief_CutToPrev + team * MAX_BRIEF_STAGES + stage,
	    nullptr, tr("Toggle Cut From Previous Stage"), true);
	cmd->addEntry(before, checked, [this, team, stage](const bool& v) {
		_model->setCutFromPrevAt(team, stage, v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void BriefingEditorDialog::on_disableGridCheckBox_toggled(bool checked)
{
	const int team  = _model->getCurrentTeam();
	const int stage = _model->getCurrentStage();
	const bool before = _model->getDisableGrid();
	if (before == checked)
		return;

	_model->setDisableGrid(checked);

	auto* cmd = new FieldEditCommand<bool>(
	    FieldId::Brief_DisableGrid + team * MAX_BRIEF_STAGES + stage,
	    nullptr, tr("Toggle Disable Grid"), true);
	cmd->addEntry(before, checked, [this, team, stage](const bool& v) {
		_model->setDisableGridAt(team, stage, v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void BriefingEditorDialog::on_iconIdSpinBox_valueChanged(int arg1)
{
	const QByteArray before = _model->captureWorkingState();
	if (!_model->setIconId(arg1)) {
		// the id was rejected; reset the spin box to the model's current value
		ui->iconIdSpinBox->blockSignals(true);
		ui->iconIdSpinBox->setValue(_model->getIconId());
		ui->iconIdSpinBox->blockSignals(false);
		return;
	}
	pushWorkingStateSnapshot(before, tr("Change Icon ID"), iconSnapshotMergeId(FieldId::Brief_SnapIconId));
}

void BriefingEditorDialog::on_iconLabelLineEdit_textChanged(const QString& string)
{
	const QByteArray before = _model->captureWorkingState();
	_model->setIconLabel(string.toUtf8().constData());
	pushWorkingStateSnapshot(before, tr("Change Icon Label"), iconSnapshotMergeId(FieldId::Brief_SnapIconLabel));
}

void BriefingEditorDialog::on_iconCloseupLabelLineEdit_textChanged(const QString& string)
{
	const QByteArray before = _model->captureWorkingState();
	_model->setIconCloseupLabel(string.toUtf8().constData());
	pushWorkingStateSnapshot(before, tr("Change Icon Closeup Label"), iconSnapshotMergeId(FieldId::Brief_SnapIconCloseup));
}

void BriefingEditorDialog::on_iconImageComboBox_currentIndexChanged(int index)
{
	const QByteArray before = _model->captureWorkingState();
	_model->setIconTypeIndex(index);
	pushWorkingStateSnapshot(before, tr("Change Icon Image"), iconSnapshotMergeId(FieldId::Brief_SnapIconImage));
}

void BriefingEditorDialog::on_iconShipTypeComboBox_currentIndexChanged(int index)
{
	const QByteArray before = _model->captureWorkingState();
	_model->setIconShipTypeIndex(index);
	pushWorkingStateSnapshot(before, tr("Change Icon Ship Type"), iconSnapshotMergeId(FieldId::Brief_SnapIconShip));
}

void BriefingEditorDialog::on_iconTeamComboBox_currentIndexChanged(int index)
{
	const QByteArray before = _model->captureWorkingState();
	_model->setIconTeamIndex(index);
	pushWorkingStateSnapshot(before, tr("Change Icon Team"), iconSnapshotMergeId(FieldId::Brief_SnapIconTeam));
}

void BriefingEditorDialog::on_scaleDoubleSpinBox_valueChanged(double arg1)
{
	const QByteArray before = _model->captureWorkingState();
	_model->setIconScaleFactor(static_cast<float>(arg1));
	pushWorkingStateSnapshot(before, tr("Change Icon Scale"), iconSnapshotMergeId(FieldId::Brief_SnapIconScale));
}

void BriefingEditorDialog::on_drawLinesCheckBox_stateChanged(int state)
{
	const QByteArray before = _model->captureWorkingState();
	if (static_cast<Qt::CheckState>(state) == Qt::PartiallyChecked) {
		_model->applyDrawLines(true);
		pushWorkingStateSnapshot(before, tr("Change Icon Lines"));
		ui->drawLinesCheckBox->setCheckState(Qt::Checked);
		return;
	}
	_model->applyDrawLines(state == Qt::Checked);
	pushWorkingStateSnapshot(before, tr("Change Icon Lines"));
}

void BriefingEditorDialog::on_changeLocallyCheckBox_toggled(bool checked)
{
	// Edit-mode flag, not mission data: intentionally not undoable.
	_model->setChangeLocally(checked);
	_mapWidget->notifyIconVisualsChanged();
}

void BriefingEditorDialog::on_flipIconCheckBox_toggled(bool checked)
{
	const QByteArray before = _model->captureWorkingState();
	_model->setIconFlipped(checked);
	_mapWidget->notifyIconVisualsChanged();
	pushWorkingStateSnapshot(before, tr("Toggle Icon Flip"));
}

void BriefingEditorDialog::on_highlightCheckBox_toggled(bool checked)
{
	const QByteArray before = _model->captureWorkingState();
	_model->setIconHighlighted(checked);
	_mapWidget->notifyIconVisualsChanged();
	pushWorkingStateSnapshot(before, tr("Toggle Icon Highlight"));
}

void BriefingEditorDialog::on_useWingIconCheckBox_toggled(bool checked)
{
	const QByteArray before = _model->captureWorkingState();
	_model->setIconUseWing(checked);
	_mapWidget->notifyIconVisualsChanged();
	pushWorkingStateSnapshot(before, tr("Toggle Wing Icon"));
}

void BriefingEditorDialog::on_useCargoIconCheckBox_toggled(bool checked)
{
	const QByteArray before = _model->captureWorkingState();
	_model->setIconUseCargo(checked);
	_mapWidget->notifyIconVisualsChanged();
	pushWorkingStateSnapshot(before, tr("Toggle Cargo Icon"));
}

void BriefingEditorDialog::on_makeIconButton_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_model->makeIcon("New Icon", 0, 0, 0);
	_model->setLineSelection({_model->getCurrentIconIndex()});
	_model->setIconPosition(getNewIconPlacement());
	pushWorkingStateSnapshot(before, tr("Make Icon"));
	updateUi();
}

void BriefingEditorDialog::on_makeIconFromShipButton_clicked()
{
	IconFromShipDialog dlg(this, _model.get());
	if (dlg.exec() == QDialog::Accepted) {
		const QByteArray before = _model->captureWorkingState();
		if (dlg.selectedKind() == IconFromShipDialog::SelectionKind::Ship && dlg.selectedShipIndex() >= 0) {
			_model->makeIconFromShip(dlg.selectedShipIndex());
		} else if (dlg.selectedKind() == IconFromShipDialog::SelectionKind::Wing && dlg.selectedWingIndex() >= 0) {
			_model->makeIconFromWing(dlg.selectedWingIndex());
		} else {
			return;
		}
		_model->setLineSelection({_model->getCurrentIconIndex()});
		_model->setIconPosition(getNewIconPlacement());
		pushWorkingStateSnapshot(before, tr("Make Icon From Ship"));
		updateUi();
	}
}

void BriefingEditorDialog::on_iconCoordinatesButton_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	IconCoordinatesDialog dlg(this, _model.get());
	dlg.exec(); // modal; a canceled dialog leaves the state unchanged and the equality check skips the push
	pushWorkingStateSnapshot(before, tr("Change Icon Coordinates"));
	updateUi();
}

void BriefingEditorDialog::on_deleteIconButton_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_model->deleteCurrentIcon();
	_model->setLineSelection({_model->getCurrentIconIndex()});
	pushWorkingStateSnapshot(before, tr("Delete Icon"));
	updateUi();
}

void BriefingEditorDialog::on_propagateIconButton_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_model->propagateCurrentIconForward();
	pushWorkingStateSnapshot(before, tr("Propagate Icon"));
}

void BriefingEditorDialog::on_stageTextPlainTextEdit_textChanged()
{
	const int team  = _model->getCurrentTeam();
	const int stage = _model->getCurrentStage();
	const SCP_string before = _model->getStageText();
	const SCP_string after  = ui->stageTextPlainTextEdit->toPlainText().toUtf8().constData();
	if (before == after)
		return;

	_model->setStageText(after);

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::Brief_StageText + team * MAX_BRIEF_STAGES + stage,
	    nullptr, tr("Change Stage Text"), true);
	cmd->addEntry(before, after, [this, team, stage](const SCP_string& v) {
		_model->setStageTextAt(team, stage, v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void BriefingEditorDialog::changeVoiceFilename(const SCP_string& newFilename)
{
	const int team  = _model->getCurrentTeam();
	const int stage = _model->getCurrentStage();
	const SCP_string before = _model->getSpeechFilename();
	if (before == newFilename)
		return;

	_model->setSpeechFilename(newFilename);

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::Brief_VoiceFile + team * MAX_BRIEF_STAGES + stage,
	    nullptr, tr("Change Voice File"), true);
	cmd->addEntry(before, newFilename, [this, team, stage](const SCP_string& v) {
		_model->setSpeechFilenameAt(team, stage, v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void BriefingEditorDialog::on_voiceFileLineEdit_textChanged(const QString& string)
{
	changeVoiceFilename(string.toUtf8().constData());
}

void BriefingEditorDialog::on_voiceFileBrowseButton_clicked()
{
	const QString lastDir = util::getLastDir("briefing/voiceFile", CF_TYPE_VOICE_DEBRIEFINGS);

	QFileDialog dlg(this, "Select Voice File", lastDir, "Voice Files (*.ogg *.wav)");
	if (dlg.exec() == QDialog::Accepted) {
		QStringList files = dlg.selectedFiles();
		if (!files.isEmpty()) {
			const QFileInfo fileInfo(files.first());
			util::saveLastDir("briefing/voiceFile", files.first());
			changeVoiceFilename(fileInfo.fileName().toUtf8().constData());
			updateUi();
		}
	}
}

void BriefingEditorDialog::on_voiceFilePlayButton_clicked()
{
	_model->testSpeech();
}

void BriefingEditorDialog::on_defaultMusicWidget_currentIndexChanged(int spooledMusicIdx)
{
	// Model uses 1-based index (0 = None); widget uses Spooled_music index (-1 = None)
	const int before = _model->getBriefingMusicIndex();
	const int after  = spooledMusicIdx + 1;
	if (before == after)
		return;

	_model->setBriefingMusicIndex(after);

	auto* cmd = new FieldEditCommand<int>(FieldId::Brief_Music, nullptr, tr("Change Briefing Music"), true);
	cmd->addEntry(before, after, [this](const int& v) {
		_model->setBriefingMusicIndex(v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void BriefingEditorDialog::on_musicPackWidget_currentIndexChanged(int spooledMusicIdx)
{
	SCP_string name = (spooledMusicIdx >= 0 && spooledMusicIdx < static_cast<int>(Spooled_music.size()))
	                      ? Spooled_music[spooledMusicIdx].name
	                      : "";
	const SCP_string before = _model->getSubstituteBriefingMusicName();
	if (before == name)
		return;

	_model->setSubstituteBriefingMusicName(name);

	auto* cmd = new FieldEditCommand<SCP_string>(FieldId::Brief_SubMusic, nullptr, tr("Change Substitute Music"), true);
	cmd->addEntry(before, name, [this](const SCP_string& v) {
		_model->setSubstituteBriefingMusicName(v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void BriefingEditorDialog::on_defaultMusicWidget_playbackStarted()
{
	ui->musicPackWidget->stopPlayback();
}

void BriefingEditorDialog::on_musicPackWidget_playbackStarted()
{
	ui->defaultMusicWidget->stopPlayback();
}

} // namespace fso::fred::dialogs

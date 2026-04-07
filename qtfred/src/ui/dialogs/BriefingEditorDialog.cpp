#include "BriefingEditorDialog.h"
#include "ui_BriefingEditorDialog.h"

#include "mission/util.h"
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
#include <mission/missionbriefcommon.h>
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>
#include <QCheckBox>
#include <QFileDialog>
#include <QVBoxLayout>

namespace fso::fred::dialogs {

int BriefingEditorDialog::_openDialogCount = 0;

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
	  _model(new BriefingEditorDialogModel(this, viewport)), _viewport(viewport)
{
	++_openDialogCount;

	this->setFocus();
	ui->setupUi(this);
	connect(ui->changeLocallyCheckBox,
		&QCheckBox::toggled,
		this,
		&BriefingEditorDialog::on_changeLocallyCheckBox_toggled,
		Qt::UniqueConnection);

	ui->iconLabelLineEdit->setMaxLength(MAX_LABEL_LEN - 1);
	ui->iconCloseupLabelLineEdit->setMaxLength(MAX_LABEL_LEN - 1);
	ui->voiceFileLineEdit->setMaxLength(MAX_FILENAME_LEN - 1);

	setupMapWidget();
	initializeUi();
	updateUi();

	resize(QDialog::sizeHint());
}

BriefingEditorDialog::~BriefingEditorDialog() {
	_openDialogCount = std::max(0, _openDialogCount - 1);
}

bool BriefingEditorDialog::isAnyDialogOpen() {
	return _openDialogCount > 0;
}

void BriefingEditorDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		_viewport->editor->autosave("briefing editor");
		QDialog::accept();
		create_default_grid(); // restore the grid back to the normal version
	}
	// else: validation failed, don't close
}

void BriefingEditorDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
		create_default_grid(); // restore the grid back to the normal version
	}
	// else: do nothing, don't close
}

void BriefingEditorDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
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

	auto musicList = _model->getMusicList();
	for (const auto& m : musicList) {
		ui->defaultMusicComboBox->addItem(m.c_str());
		ui->musicPackComboBox->addItem(m.c_str());
	}

	// Initialize the formula tree editor
	ui->formulaTreeView->initializeEditor(_viewport->editor, this);

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
	ui->defaultMusicComboBox->setCurrentIndex(_model->getBriefingMusicIndex());
	ui->musicPackComboBox->setCurrentIndex(ui->musicPackComboBox->findText(_model->getSubstituteBriefingMusicName().c_str()));

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

	updateUi();
}

void BriefingEditorDialog::on_insertStageButton_clicked()
{
	_model->insertStage();
	_model->setLineSelection({_model->getCurrentIconIndex()});
	_mapWidget->setStage(_model->getCurrentStage());
	captureResetCameraForCurrentStage();
	updateUi();
}

void BriefingEditorDialog::on_deleteStageButton_clicked()
{
	_model->deleteStage();
	_model->setLineSelection({_model->getCurrentIconIndex()});
	_mapWidget->setStage(_model->getCurrentStage());
	captureResetCameraForCurrentStage();
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
		_mapWidget->applyCameraToCurrentStage(_resetCameraPos, _resetCameraOrient);
	}
}

void BriefingEditorDialog::on_copyCameraButton_clicked()
{
	_model->copyStageViewToClipboard();
}

void BriefingEditorDialog::on_pasteCameraButton_clicked()
{
	_model->pasteClipboardViewToStage();
	const auto stageView = _model->getStageView();
	_mapWidget->applyCameraToCurrentStage(stageView.first, stageView.second);
}

void BriefingEditorDialog::on_copyToOtherTeamsButton_clicked()
{
	_model->copyToOtherTeams();
}

void BriefingEditorDialog::on_teamComboBox_currentIndexChanged(int index)
{
	_model->setCurrentTeam(ui->teamComboBox->itemData(index).toInt());
	_model->setLineSelection({_model->getCurrentIconIndex()});
	_mapWidget->setStage(_model->getCurrentStage());
	captureResetCameraForCurrentStage();
	updateUi();
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
}

void BriefingEditorDialog::on_cameraTransitionTimeSpinBox_valueChanged(int arg1)
{
	_model->setCameraTransitionTime(arg1);
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
	_model->setCutToNext(checked);
}

void BriefingEditorDialog::on_cutToPrevStageCheckBox_toggled(bool checked)
{
	_model->setCutFromPrev(checked);
}

void BriefingEditorDialog::on_disableGridCheckBox_toggled(bool checked)
{
	_model->setDisableGrid(checked);
}

void BriefingEditorDialog::on_iconIdSpinBox_valueChanged(int arg1)
{
	_model->setIconId(arg1);
}

void BriefingEditorDialog::on_iconLabelLineEdit_textChanged(const QString& string)
{
	_model->setIconLabel(string.toUtf8().constData());
}

void BriefingEditorDialog::on_iconCloseupLabelLineEdit_textChanged(const QString& string)
{
	_model->setIconCloseupLabel(string.toUtf8().constData());
}

void BriefingEditorDialog::on_iconImageComboBox_currentIndexChanged(int index)
{
	_model->setIconTypeIndex(index);
}

void BriefingEditorDialog::on_iconShipTypeComboBox_currentIndexChanged(int index)
{
	_model->setIconShipTypeIndex(index);
}

void BriefingEditorDialog::on_iconTeamComboBox_currentIndexChanged(int index)
{
	_model->setIconTeamIndex(index);
}

void BriefingEditorDialog::on_scaleDoubleSpinBox_valueChanged(double arg1)
{
	_model->setIconScaleFactor(static_cast<float>(arg1));
}

void BriefingEditorDialog::on_drawLinesCheckBox_stateChanged(int state)
{
	if (static_cast<Qt::CheckState>(state) == Qt::PartiallyChecked) {
		_model->applyDrawLines(true);
		ui->drawLinesCheckBox->setCheckState(Qt::Checked);
		return;
	}
	_model->applyDrawLines(state == Qt::Checked);
}

void BriefingEditorDialog::on_changeLocallyCheckBox_toggled(bool checked)
{
	_model->setChangeLocally(checked);
	_mapWidget->notifyIconVisualsChanged();
}

void BriefingEditorDialog::on_flipIconCheckBox_toggled(bool checked)
{
	_model->setIconFlipped(checked);
	_mapWidget->notifyIconVisualsChanged();
}

void BriefingEditorDialog::on_highlightCheckBox_toggled(bool checked)
{
	_model->setIconHighlighted(checked);
	_mapWidget->notifyIconVisualsChanged();
}

void BriefingEditorDialog::on_useWingIconCheckBox_toggled(bool checked)
{
	_model->setIconUseWing(checked);
	_mapWidget->notifyIconVisualsChanged();
}

void BriefingEditorDialog::on_useCargoIconCheckBox_toggled(bool checked)
{
	_model->setIconUseCargo(checked);
	_mapWidget->notifyIconVisualsChanged();
}

void BriefingEditorDialog::on_makeIconButton_clicked()
{
	_model->makeIcon("New Icon", 0, 0, 0);
	_model->setLineSelection({_model->getCurrentIconIndex()});
	_model->setIconPosition(getNewIconPlacement());
	updateUi();
}

void BriefingEditorDialog::on_makeIconFromShipButton_clicked()
{
	IconFromShipDialog dlg(this, _model.get());
	if (dlg.exec() == QDialog::Accepted) {
		if (dlg.selectedKind() == IconFromShipDialog::SelectionKind::Ship && dlg.selectedShipIndex() >= 0) {
			_model->makeIconFromShip(dlg.selectedShipIndex());
		} else if (dlg.selectedKind() == IconFromShipDialog::SelectionKind::Wing && dlg.selectedWingIndex() >= 0) {
			_model->makeIconFromWing(dlg.selectedWingIndex());
		} else {
			return;
		}
		_model->setLineSelection({_model->getCurrentIconIndex()});
		_model->setIconPosition(getNewIconPlacement());
		updateUi();
	}
}

void BriefingEditorDialog::on_iconCoordinatesButton_clicked()
{
	IconCoordinatesDialog dlg(this, _model.get());
	dlg.exec(); // modal
}

void BriefingEditorDialog::on_deleteIconButton_clicked()
{
	_model->deleteCurrentIcon();
	_model->setLineSelection({_model->getCurrentIconIndex()});
	updateUi();
}

void BriefingEditorDialog::on_propagateIconButton_clicked()
{
	_model->propagateCurrentIconForward();
}

void BriefingEditorDialog::on_stageTextPlainTextEdit_textChanged()
{
	_model->setStageText(ui->stageTextPlainTextEdit->toPlainText().toUtf8().constData());
}

void BriefingEditorDialog::on_voiceFileLineEdit_textChanged(const QString& string)
{
	_model->setSpeechFilename(string.toUtf8().constData());
}

void BriefingEditorDialog::on_voiceFileBrowseButton_clicked()
{
	int dir_pushed = cfile_push_chdir(CF_TYPE_VOICE_DEBRIEFINGS);

	QFileDialog dlg(this, "Select Voice File", "", "Voice Files (*.ogg *.wav)");
	if (dlg.exec() == QDialog::Accepted) {
		QStringList files = dlg.selectedFiles();
		if (!files.isEmpty()) {
			QFileInfo fileInfo(files.first());
			_model->setSpeechFilename(fileInfo.fileName().toUtf8().constData());
			updateUi();
		}
	}

	if (dir_pushed) {
		cfile_pop_dir();
	}
}

void BriefingEditorDialog::on_voiceFilePlayButton_clicked()
{
	_model->testSpeech();
}

void BriefingEditorDialog::on_formulaTreeView_nodeChanged(int newTree)
{
	_model->setFormula(newTree);
}

void BriefingEditorDialog::on_defaultMusicComboBox_currentIndexChanged(int index)
{
	_model->setBriefingMusicIndex(index);
}

void BriefingEditorDialog::on_musicPackComboBox_currentIndexChanged(int /*index*/)
{
	_model->setSubstituteBriefingMusicName(ui->musicPackComboBox->currentText().toUtf8().constData());
}

} // namespace fso::fred::dialogs

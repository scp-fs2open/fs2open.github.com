#include "ui/dialogs/CoordinatePointEditorDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_CoordinatePointEditorDialog.h"

#include <coordinate_points/coordinate_shapes.h>
#include <globalincs/globals.h>
#include <mission/missionparse.h>
#include <mission/util.h>

namespace fso::fred::dialogs {

CoordinatePointEditorDialog::CoordinatePointEditorDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent),
	_viewport(viewport),
	ui(new Ui::CoordinatePointEditorDialog()),
	_model(new CoordinatePointEditorDialogModel(this, viewport))
{
	this->setFocus();
	ui->setupUi(this);

	ui->nameEdit->setMaxLength(NAME_LENGTH - 1);

	// -1 is the "mixed selection" sentinel; rendered blank via specialValueText.
	for (auto* sb : {ui->colorRSpinBox, ui->colorGSpinBox, ui->colorBSpinBox, ui->colorASpinBox}) {
		sb->setMinimum(-1);
		sb->setSpecialValueText(" ");
	}
	ui->escortPrioritySpinBox->setMinimum(-1);
	ui->escortPrioritySpinBox->setSpecialValueText(" ");

	// Block combo signals during initial populate. Without this, addItem() fires
	// currentIndexChanged on the auto-connected slot and clobbers the model's value
	// (e.g. a freshly-created Diamond point gets overwritten with Triangle, the first
	// enum entry) before updateUi() ever reads the real state.
	{
		util::SignalBlockers blockers(this);

		// Multiplayer team combo: "Any" + one entry per TVT team. Mixed-selection state
		// is shown by setCurrentIndex(-1) (blank); selecting any real entry commits.
		ui->multiTeamCombo->clear();
		ui->multiTeamCombo->addItem("Any team", -1);
		for (const auto& team : Mission_event_teams_tvt) {
			ui->multiTeamCombo->addItem(QString::fromStdString(team.first), team.second);
		}

		ui->sizeSpinBox->setMinimum(-0.01);  // negative is the "mixed" sentinel
		ui->sizeSpinBox->setMaximum(static_cast<double>(COORDINATE_POINT_SIZE_MAX));
		ui->sizeSpinBox->setSingleStep(0.1);
		ui->sizeSpinBox->setSpecialValueText(" ");

		// Populate the shape combo: NGon and Star as the two built-in primitives, then every
		// tabled shape in load order. UserRole carries the "shape id" used by the model:
		// NGon=-2, Star=-1, Tabled=table_index (>= 0).
		ui->shapeCombo->clear();
		ui->shapeCombo->addItem("NGon", -2);
		ui->shapeCombo->addItem("Star", -1);
		for (int i = 0; i < static_cast<int>(Coordinate_shapes.size()); ++i) {
			ui->shapeCombo->addItem(QString::fromStdString(Coordinate_shapes[i].name), i);
		}
	}

	initializeUi();
	updateUi();

	connect(_model.get(), &CoordinatePointEditorDialogModel::coordinatePointMarkingChanged, this, [this] {
		initializeUi();
		updateUi();
	});

	resize(QDialog::sizeHint());
}

CoordinatePointEditorDialog::~CoordinatePointEditorDialog() = default;

void CoordinatePointEditorDialog::initializeUi()
{
	util::SignalBlockers blockers(this);

	ui->layerCombo->clear();
	for (const auto& name : _viewport->getLayerNames()) {
		ui->layerCombo->addItem(QString::fromStdString(name), QString::fromStdString(name));
	}

	const bool enabled = _model->hasValidSelection();
	const bool hasAny = _model->hasAnyCoordinatePointsInMission();
	const bool multi = _model->hasMultipleSelection();

	ui->nameEdit->setEnabled(enabled && !multi);
	ui->groupEdit->setEnabled(enabled);
	ui->shapeCombo->setEnabled(enabled);
	// Sides/Points/InnerRadius are gated per-kind in updateUi(); only the always-on rows are
	// touched here, plus a default-disabled baseline so they grey out when no point is selected.
	ui->sidesSpinBox->setEnabled(false);
	ui->pointsSpinBox->setEnabled(false);
	ui->innerRadiusSpinBox->setEnabled(false);
	ui->angleSpinBox->setEnabled(enabled);
	ui->sizeSpinBox->setEnabled(enabled);
	ui->escortPrioritySpinBox->setEnabled(enabled);
	// Team selector is greyed out when the mission isn't a team mission.
	ui->multiTeamCombo->setEnabled(enabled && CoordinatePointEditorDialogModel::missionIsMultiTeam());
	ui->visibleInMissionCheck->setEnabled(enabled);
	ui->layerCombo->setEnabled(enabled);
	ui->colorRSpinBox->setEnabled(enabled);
	ui->colorGSpinBox->setEnabled(enabled);
	ui->colorBSpinBox->setEnabled(enabled);
	ui->colorASpinBox->setEnabled(enabled);
	ui->prevPointButton->setEnabled(hasAny);
	ui->nextPointButton->setEnabled(hasAny);

	if (multi) {
		setWindowTitle(QString("Edit %1 Coordinate Points").arg(_model->getSelectionCount()));
	} else {
		setWindowTitle("Coordinate Point Editor");
	}
}

void CoordinatePointEditorDialog::updateUi()
{
	util::SignalBlockers blockers(this);

	ui->nameEdit->setText(QString::fromStdString(_model->getCurrentName()));

	if (_model->isGroupMixed()) {
		ui->groupEdit->setPlaceholderText("<mixed>");
		ui->groupEdit->setText("");
	} else {
		ui->groupEdit->setPlaceholderText("");
		ui->groupEdit->setText(QString::fromStdString(_model->getGroup()));
	}

	// Shape: combo holds the kind+table_index encoded as the "shape id" (-2 = NGon, -1 = Star,
	// >= 0 = tabled). Look up by data, not by index, so the combo's order is independent of
	// the encoding.
	if (_model->isShapeKindMixed()) {
		ui->shapeCombo->setCurrentIndex(-1);
	} else {
		const int idx = ui->shapeCombo->findData(_model->getShapeId());
		ui->shapeCombo->setCurrentIndex(idx);
	}

	// Per-kind parameter rows stay in place; we just disable the ones that don't apply to the
	// resolved kind. When kind is mixed across a multi-selection, all three per-kind rows are
	// disabled until the user picks a kind. Angle is always editable. Rows are also disabled
	// up front when there's no valid selection (handled by initializeUi).
	const bool kindMixed = _model->isShapeKindMixed();
	const auto kind = _model->getShapeKind();
	const bool selectionEnabled = _model->hasValidSelection();
	const bool ngonEnabled = selectionEnabled && !kindMixed && (kind == CoordinatePointShapeKind::NGon);
	const bool starEnabled = selectionEnabled && !kindMixed && (kind == CoordinatePointShapeKind::Star);
	ui->sidesLabel->setEnabled(ngonEnabled);
	ui->sidesSpinBox->setEnabled(ngonEnabled);
	ui->pointsLabel->setEnabled(starEnabled);
	ui->pointsSpinBox->setEnabled(starEnabled);
	ui->innerRadiusLabel->setEnabled(starEnabled);
	ui->innerRadiusSpinBox->setEnabled(starEnabled);

	// Mixed sentinels: spinboxes whose minimum is set below the legal range render
	// specialValueText (" ") when that minimum value is set.
	ui->sidesSpinBox->setValue(_model->isSidesMixed() ? 2 : _model->getSides());
	ui->pointsSpinBox->setValue(_model->isPointsMixed() ? 2 : _model->getPoints());
	ui->innerRadiusSpinBox->setValue(_model->isInnerRadiusMixed() ? -0.01
		: static_cast<double>(_model->getInnerRadius()));
	// Angle is a free-rotation float; no sentinel needed -- on mixed we just show the first
	// selection's value and let the next edit write through to all.
	ui->angleSpinBox->setValue(static_cast<double>(_model->getAngle()));

	ui->sizeSpinBox->setValue(_model->isSizeMixed() ? -0.01 : static_cast<double>(_model->getSize()));
	ui->escortPrioritySpinBox->setValue(_model->isEscortPriorityMixed() ? -1 : _model->getEscortPriority());
	if (_model->isMultiTeamMixed()) {
		ui->multiTeamCombo->setCurrentIndex(-1);  // blank when values differ across selection
	} else {
		ui->multiTeamCombo->setCurrentIndex(ui->multiTeamCombo->findData(_model->getMultiTeam()));
	}

	const int visState = _model->getVisibleInMissionState();
	ui->visibleInMissionCheck->setTristate(visState == Qt::PartiallyChecked);
	ui->visibleInMissionCheck->setCheckState(static_cast<Qt::CheckState>(visState));

	ui->layerCombo->setCurrentIndex(ui->layerCombo->findData(QString::fromStdString(_model->getLayer())));

	ui->colorRSpinBox->setValue(_model->isColorRMixed() ? -1 : _model->getColorR());
	ui->colorGSpinBox->setValue(_model->isColorGMixed() ? -1 : _model->getColorG());
	ui->colorBSpinBox->setValue(_model->isColorBMixed() ? -1 : _model->getColorB());
	ui->colorASpinBox->setValue(_model->isColorAMixed() ? -1 : _model->getColorA());

	updateColorSwatch();
}

void CoordinatePointEditorDialog::updateColorSwatch()
{
	if (_model->hasAnyColorMixed()) {
		ui->colorSwatch->setText("?");
		ui->colorSwatch->setAlignment(Qt::AlignCenter);
		ui->colorSwatch->setStyleSheet("background: #888; color: white;"
		                               "border: 1px solid #444; border-radius: 3px;");
		return;
	}
	ui->colorSwatch->setText("");
	ui->colorSwatch->setStyleSheet(QString("background: rgba(%1,%2,%3,%4);"
	                                       "border: 1px solid #444; border-radius: 3px;")
	        .arg(_model->getColorR())
	        .arg(_model->getColorG())
	        .arg(_model->getColorB())
	        .arg(_model->getColorA() / 255.0));
}

void CoordinatePointEditorDialog::on_prevPointButton_clicked()
{
	_model->selectPreviousPoint();
}

void CoordinatePointEditorDialog::on_nextPointButton_clicked()
{
	_model->selectNextPoint();
}

void CoordinatePointEditorDialog::on_nameEdit_editingFinished()
{
	if (!_model->setCurrentName(ui->nameEdit->text().toUtf8().constData())) {
		util::SignalBlockers blockers(this);
		ui->nameEdit->setText(QString::fromStdString(_model->getCurrentName()));
	}
}

void CoordinatePointEditorDialog::on_groupEdit_editingFinished()
{
	_model->setGroup(ui->groupEdit->text().toUtf8().constData());
	updateUi();
}

void CoordinatePointEditorDialog::on_shapeCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;
	const int shape_id = ui->shapeCombo->itemData(index).toInt();
	_model->setShapeId(shape_id);
	// Toggle which per-kind parameter rows are visible.
	updateUi();
}

void CoordinatePointEditorDialog::on_sidesSpinBox_valueChanged(int value)
{
	_model->setSides(value);
}

void CoordinatePointEditorDialog::on_pointsSpinBox_valueChanged(int value)
{
	_model->setPoints(value);
}

void CoordinatePointEditorDialog::on_innerRadiusSpinBox_valueChanged(double value)
{
	_model->setInnerRadius(static_cast<float>(value));
}

void CoordinatePointEditorDialog::on_angleSpinBox_valueChanged(double value)
{
	_model->setAngle(static_cast<float>(value));
}

void CoordinatePointEditorDialog::on_sizeSpinBox_valueChanged(double value)
{
	_model->setSize(static_cast<float>(value));
	updateUi();
}

void CoordinatePointEditorDialog::on_escortPrioritySpinBox_valueChanged(int value)
{
	_model->setEscortPriority(value);
	updateUi();
}

void CoordinatePointEditorDialog::on_multiTeamCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;
	_model->setMultiTeam(ui->multiTeamCombo->itemData(index).toInt());
	updateUi();
}

void CoordinatePointEditorDialog::on_layerCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;
	_model->setLayer(ui->layerCombo->itemData(index).toString().toUtf8().constData());
}

void CoordinatePointEditorDialog::on_visibleInMissionCheck_clicked()
{
	_model->setVisibleInMission(ui->visibleInMissionCheck->isChecked());
	updateUi();
}

void CoordinatePointEditorDialog::on_colorRSpinBox_valueChanged(int value)
{
	_model->setColorR(value);
	updateColorSwatch();
}

void CoordinatePointEditorDialog::on_colorGSpinBox_valueChanged(int value)
{
	_model->setColorG(value);
	updateColorSwatch();
}

void CoordinatePointEditorDialog::on_colorBSpinBox_valueChanged(int value)
{
	_model->setColorB(value);
	updateColorSwatch();
}

void CoordinatePointEditorDialog::on_colorASpinBox_valueChanged(int value)
{
	_model->setColorA(value);
	updateColorSwatch();
}

} // namespace fso::fred::dialogs

#include "ui/dialogs/CoordinatePointEditorDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_CoordinatePointEditorDialog.h"

#include <QRadioButton>

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
	// Angle's real range is [-360, 360]; drop the floor one step lower so that sentinel value can
	// render blank (specialValueText) when the selection's angles disagree.
	ui->angleSpinBox->setMinimum(-361.0);
	ui->angleSpinBox->setSpecialValueText(" ");
	// Sides/Points/InnerRadius carry their sentinel minimum in the .ui, but a whitespace-only
	// specialValueText there gets trimmed to empty (which disables the blank), so set it in code
	// like the spinboxes above. Their minimums (from the .ui) are already correct.
	ui->sidesSpinBox->setSpecialValueText(" ");
	ui->pointsSpinBox->setSpecialValueText(" ");
	ui->innerRadiusSpinBox->setSpecialValueText(" ");

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

		// The shape kind (NGon / Star / Custom) is chosen by the radio buttons; this combo lists
		// only the tabled shapes and is active when Custom is selected. UserRole carries the
		// table index (the model's "shape id" for a tabled shape, >= 0).
		ui->shapeCombo->clear();
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
	// Shape kind radios + the tabled-shape combo are gated in updateUi(): NGon/Star follow the
	// selection, Custom additionally needs at least one tabled shape, and the combo is only live
	// for Custom. Baseline-disable here so they grey out when no point is selected.
	ui->shapeNGonRadio->setEnabled(false);
	ui->shapeStarRadio->setEnabled(false);
	ui->shapeCustomRadio->setEnabled(false);
	ui->shapeCombo->setEnabled(false);
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

	// Shape kind is chosen by the NGon / Star / Custom radios; the combo lists the tabled shapes
	// and is only live for Custom. When the kind is mixed across a multi-selection, no radio is
	// checked (which needs a brief drop of auto-exclusivity to reach the all-off state).
	const bool kindMixed = _model->isShapeKindMixed();
	const auto kind = _model->getShapeKind();
	const bool selectionEnabled = _model->hasValidSelection();
	const bool hasTabled = !Coordinate_shapes.empty();
	const bool ngonChecked   = !kindMixed && (kind == CoordinatePointShapeKind::NGon);
	const bool starChecked   = !kindMixed && (kind == CoordinatePointShapeKind::Star);
	const bool customChecked = !kindMixed && (kind == CoordinatePointShapeKind::Tabled);

	const std::pair<QRadioButton*, bool> radios[] = {
		{ui->shapeNGonRadio, ngonChecked},
		{ui->shapeStarRadio, starChecked},
		{ui->shapeCustomRadio, customChecked},
	};
	for (const auto& [rb, on] : radios) {
		rb->setAutoExclusive(false);
		rb->setChecked(on);
		rb->setAutoExclusive(true);
	}
	ui->shapeNGonRadio->setEnabled(selectionEnabled);
	ui->shapeStarRadio->setEnabled(selectionEnabled);
	ui->shapeCustomRadio->setEnabled(selectionEnabled && hasTabled);

	// Point the combo at the resolved tabled shape; it's only interactive for Custom.
	if (customChecked) {
		ui->shapeCombo->setCurrentIndex(ui->shapeCombo->findData(_model->getShapeTableIndex()));
	}
	ui->shapeCombo->setEnabled(selectionEnabled && customChecked);

	// Per-kind parameter rows stay in place; we just disable the ones that don't apply to the
	// resolved kind. When kind is mixed across a multi-selection, all three per-kind rows are
	// disabled until the user picks a kind. Angle is always editable. Rows are also disabled
	// up front when there's no valid selection (handled by initializeUi).
	const bool ngonEnabled = selectionEnabled && ngonChecked;
	const bool starEnabled = selectionEnabled && starChecked;
	ui->sidesLabel->setEnabled(ngonEnabled);
	ui->sidesSpinBox->setEnabled(ngonEnabled);
	ui->pointsLabel->setEnabled(starEnabled);
	ui->pointsSpinBox->setEnabled(starEnabled);
	ui->innerRadiusLabel->setEnabled(starEnabled);
	ui->innerRadiusSpinBox->setEnabled(starEnabled);

	// Every value spinbox uses the same scheme as Size/Escort Priority: a below-range sentinel
	// value renders blank via the spinbox's specialValueText, the model setter rejects it and
	// clamps real edits, and each valueChanged handler re-runs updateUi() so a single selection
	// snaps back to the real value. So here we only ever push the sentinel (mixed) or the value.
	// For the mixed/blank case, push each spinbox's OWN minimum() rather than a literal sentinel:
	// specialValueText renders only when value() == minimum() exactly, and a hardcoded double can
	// miss that equality at the box's rounding precision (the QDoubleSpinBox blank-fails otherwise).
	ui->sidesSpinBox->setValue(_model->isSidesMixed() ? ui->sidesSpinBox->minimum() : _model->getSides());
	ui->pointsSpinBox->setValue(_model->isPointsMixed() ? ui->pointsSpinBox->minimum() : _model->getPoints());
	ui->innerRadiusSpinBox->setValue(_model->isInnerRadiusMixed() ? ui->innerRadiusSpinBox->minimum()
		: static_cast<double>(_model->getInnerRadius()));
	ui->angleSpinBox->setValue(_model->isAngleMixed() ? ui->angleSpinBox->minimum()
		: static_cast<double>(_model->getAngle()));

	ui->sizeSpinBox->setValue(_model->isSizeMixed() ? ui->sizeSpinBox->minimum() : static_cast<double>(_model->getSize()));
	ui->escortPrioritySpinBox->setValue(_model->isEscortPriorityMixed() ? ui->escortPrioritySpinBox->minimum() : _model->getEscortPriority());
	if (_model->isMultiTeamMixed()) {
		ui->multiTeamCombo->setCurrentIndex(-1);  // blank when values differ across selection
	} else {
		ui->multiTeamCombo->setCurrentIndex(ui->multiTeamCombo->findData(_model->getMultiTeam()));
	}

	const int visState = _model->getVisibleInMissionState();
	ui->visibleInMissionCheck->setTristate(visState == Qt::PartiallyChecked);
	ui->visibleInMissionCheck->setCheckState(static_cast<Qt::CheckState>(visState));

	ui->layerCombo->setCurrentIndex(ui->layerCombo->findData(QString::fromStdString(_model->getLayer())));

	ui->colorRSpinBox->setValue(_model->isColorRMixed() ? ui->colorRSpinBox->minimum() : _model->getColorR());
	ui->colorGSpinBox->setValue(_model->isColorGMixed() ? ui->colorGSpinBox->minimum() : _model->getColorG());
	ui->colorBSpinBox->setValue(_model->isColorBMixed() ? ui->colorBSpinBox->minimum() : _model->getColorB());
	ui->colorASpinBox->setValue(_model->isColorAMixed() ? ui->colorASpinBox->minimum() : _model->getColorA());

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

void CoordinatePointEditorDialog::on_shapeNGonRadio_toggled(bool checked)
{
	if (!checked)
		return;
	_model->setShapeId(-2);
	updateUi();
}

void CoordinatePointEditorDialog::on_shapeStarRadio_toggled(bool checked)
{
	if (!checked)
		return;
	_model->setShapeId(-1);
	updateUi();
}

void CoordinatePointEditorDialog::on_shapeCustomRadio_toggled(bool checked)
{
	if (!checked)
		return;
	// Commit whichever tabled shape the combo currently shows. The combo lists the tabled shapes
	// and the Custom radio is only enabled when at least one exists, so there is a valid entry.
	int index = ui->shapeCombo->currentIndex();
	if (index < 0 && ui->shapeCombo->count() > 0)
		index = 0;
	if (index >= 0)
		_model->setShapeId(ui->shapeCombo->itemData(index).toInt());
	updateUi();
}

void CoordinatePointEditorDialog::on_shapeCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;
	// The combo only holds tabled shapes; its data is the table index (the model's shape id for
	// a Custom shape). It's only interactive while Custom is selected.
	const int shape_id = ui->shapeCombo->itemData(index).toInt();
	_model->setShapeId(shape_id);
	// Toggle which per-kind parameter rows are visible.
	updateUi();
}

void CoordinatePointEditorDialog::on_sidesSpinBox_valueChanged(int value)
{
	_model->setSides(value);
	updateUi();
}

void CoordinatePointEditorDialog::on_pointsSpinBox_valueChanged(int value)
{
	_model->setPoints(value);
	updateUi();
}

void CoordinatePointEditorDialog::on_innerRadiusSpinBox_valueChanged(double value)
{
	_model->setInnerRadius(static_cast<float>(value));
	updateUi();
}

void CoordinatePointEditorDialog::on_angleSpinBox_valueChanged(double value)
{
	_model->setAngle(static_cast<float>(value));
	updateUi();
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
	updateUi();
}

void CoordinatePointEditorDialog::on_colorGSpinBox_valueChanged(int value)
{
	_model->setColorG(value);
	updateUi();
}

void CoordinatePointEditorDialog::on_colorBSpinBox_valueChanged(int value)
{
	_model->setColorB(value);
	updateUi();
}

void CoordinatePointEditorDialog::on_colorASpinBox_valueChanged(int value)
{
	_model->setColorA(value);
	updateUi();
}

} // namespace fso::fred::dialogs

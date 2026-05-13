#include "ui/dialogs/CoordinatePointEditorDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_CoordinatePointEditorDialog.h"

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

	// Multiplayer team combo: "Any" + one entry per TVT team. Mixed-selection state is
	// shown by setCurrentIndex(-1) (blank); selecting any real entry commits the value.
	ui->multiTeamCombo->clear();
	ui->multiTeamCombo->addItem("Any team", -1);
	for (const auto& team : Mission_event_teams_tvt) {
		ui->multiTeamCombo->addItem(QString::fromStdString(team.first), team.second);
	}
	ui->sizeSpinBox->setMinimum(-0.01);  // negative is the "mixed" sentinel
	ui->sizeSpinBox->setMaximum(static_cast<double>(COORDINATE_POINT_SIZE_MAX));
	ui->sizeSpinBox->setSingleStep(0.1);
	ui->sizeSpinBox->setSpecialValueText(" ");

	// Populate the shape combo from the enum.
	ui->shapeCombo->clear();
	for (int i = 0; i < static_cast<int>(CoordinatePointShape::NUM_SHAPES); ++i) {
		ui->shapeCombo->addItem(coordinate_point_shape_to_string(static_cast<CoordinatePointShape>(i)), i);
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
	ui->categoryEdit->setEnabled(enabled);
	ui->shapeCombo->setEnabled(enabled);
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

	if (_model->isCategoryMixed()) {
		ui->categoryEdit->setPlaceholderText("<mixed>");
		ui->categoryEdit->setText("");
	} else {
		ui->categoryEdit->setPlaceholderText("");
		ui->categoryEdit->setText(QString::fromStdString(_model->getCategory()));
	}

	if (_model->isShapeMixed()) {
		ui->shapeCombo->setCurrentIndex(-1);
	} else {
		ui->shapeCombo->setCurrentIndex(static_cast<int>(_model->getShape()));
	}

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

void CoordinatePointEditorDialog::on_categoryEdit_editingFinished()
{
	_model->setCategory(ui->categoryEdit->text().toUtf8().constData());
	updateUi();
}

void CoordinatePointEditorDialog::on_shapeCombo_currentIndexChanged(int index)
{
	if (index < 0 || index >= static_cast<int>(CoordinatePointShape::NUM_SHAPES))
		return;
	_model->setShape(static_cast<CoordinatePointShape>(index));
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

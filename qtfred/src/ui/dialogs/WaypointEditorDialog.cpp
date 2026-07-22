#include <QtWidgets/QTextEdit>
#include "ui/dialogs/WaypointEditorDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_WaypointEditorDialog.h"

#include <globalincs/globals.h>
#include <mission/util.h>
#include <object/waypoint.h>
#include <ui/util/menu.h>

namespace fso::fred::dialogs {


WaypointEditorDialog::WaypointEditorDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent),
	_viewport(viewport),
	ui(new Ui::WaypointEditorDialog()),
	_model(new WaypointEditorDialogModel(this, viewport))
{
	this->setFocus();
	ui->setupUi(this);

	ui->nameEdit->setMaxLength(NAME_LENGTH - 1);

	// -1 is the "mixed selection" sentinel; shown as blank via specialValueText.
	for (auto* sb : {ui->colorRSpinBox, ui->colorGSpinBox, ui->colorBSpinBox}) {
		sb->setMinimum(-1);
		sb->setSpecialValueText(" ");
	}

	initializeUi();
	updateUi();

	connect(_model.get(), &WaypointEditorDialogModel::waypointPathMarkingChanged, this, [this] {
		initializeUi();
		updateUi();
	});

	// "Select Waypoint Path" menu: jump the editor to any path in the mission.
	auto* model = _model.get();
	util::installSelectMenu(
		this,
		[]() {
			std::vector<util::SelectMenuEntry> entries;
				entries.reserve(Waypoint_lists.size());
			for (int i = 0; i < static_cast<int>(Waypoint_lists.size()); i++) {
				entries.push_back({QString::fromUtf8(Waypoint_lists[i].get_name()), i});
			}
			return entries;
		},
		[model]() { return model->hasMultipleSelection() ? -1 : model->getSelectedPathIndex(); },
		[model](int idx) { model->selectWaypointPathByIndex(idx); },
		tr("&Select Waypoint Path"));

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

WaypointEditorDialog::~WaypointEditorDialog() = default;

void WaypointEditorDialog::initializeUi()
{
	util::SignalBlockers blockers(this);

	ui->layerCombo->clear();
	for (const auto& name : _viewport->getLayerNames()) {
		ui->layerCombo->addItem(QString::fromStdString(name), QString::fromStdString(name));
	}

	const bool enabled = _model->hasValidSelection();
	const bool hasAny = _model->hasAnyPathsInMission();
	const bool multiSelect = _model->hasMultipleSelection();

	ui->nameEdit->setEnabled(enabled && !multiSelect);
	ui->noDrawLinesCheck->setEnabled(enabled);
	ui->customColorCheck->setEnabled(enabled);
	ui->layerCombo->setEnabled(enabled);
	ui->prevPathButton->setEnabled(hasAny);
	ui->nextPathButton->setEnabled(hasAny);

	if (multiSelect) {
		setWindowTitle(QString("Edit %1 Waypoint Paths").arg(_model->getSelectionCount()));
	} else {
		setWindowTitle("Waypoint Path Editor");
	}
}

void WaypointEditorDialog::updateUi()
{
	util::SignalBlockers blockers(this);
	ui->nameEdit->setText(QString::fromStdString(_model->getCurrentName()));
	ui->layerCombo->setCurrentIndex(ui->layerCombo->findData(QString::fromStdString(_model->getLayer())));

	const int noDrawState = _model->getNoDrawLinesState();
	ui->noDrawLinesCheck->setTristate(noDrawState == Qt::PartiallyChecked);
	ui->noDrawLinesCheck->setCheckState(static_cast<Qt::CheckState>(noDrawState));

	const int customColorState = _model->getHasCustomColorState();
	ui->customColorCheck->setTristate(customColorState == Qt::PartiallyChecked);
	ui->customColorCheck->setCheckState(static_cast<Qt::CheckState>(customColorState));

	ui->colorRSpinBox->setValue(_model->isColorRMixed() ? -1 : _model->getColorR());
	ui->colorGSpinBox->setValue(_model->isColorGMixed() ? -1 : _model->getColorG());
	ui->colorBSpinBox->setValue(_model->isColorBMixed() ? -1 : _model->getColorB());

	const bool customResolved = customColorState == Qt::Checked;
	const bool colorEnabled = _model->hasValidSelection() && customResolved;
	ui->colorRSpinBox->setEnabled(colorEnabled);
	ui->colorGSpinBox->setEnabled(colorEnabled);
	ui->colorBSpinBox->setEnabled(colorEnabled);

	updateColorSwatch();
}

void WaypointEditorDialog::updateColorSwatch()
{
	if (_model->hasAnyColorMixed()) {
		ui->colorSwatch->setText("?");
		ui->colorSwatch->setAlignment(Qt::AlignCenter);
		ui->colorSwatch->setStyleSheet("background: #888; color: white;"
		                               "border: 1px solid #444; border-radius: 3px;");
		return;
	}
	ui->colorSwatch->setText("");
	ui->colorSwatch->setStyleSheet(QString("background: rgb(%1,%2,%3);"
	                                       "border: 1px solid #444; border-radius: 3px;")
	        .arg(_model->getColorR())
	        .arg(_model->getColorG())
	        .arg(_model->getColorB()));
}

void WaypointEditorDialog::on_prevPathButton_clicked()
{
	_model->selectPreviousPath();
}

void WaypointEditorDialog::on_nextPathButton_clicked()
{
	_model->selectNextPath();
}

void WaypointEditorDialog::on_nameEdit_editingFinished()
{
	if (!_model->setCurrentName(ui->nameEdit->text().toUtf8().constData())) {
		util::SignalBlockers blockers(this);
		ui->nameEdit->setText(QString::fromStdString(_model->getCurrentName()));
	}
}

void WaypointEditorDialog::on_noDrawLinesCheck_clicked()
{
	// clicked() (not toggled()) so a click on a tri-state PartiallyChecked box still routes here.
	_model->setNoDrawLines(ui->noDrawLinesCheck->isChecked());
	// User has resolved any partial state; refresh so tristate(true) is cleared.
	updateUi();
}

void WaypointEditorDialog::on_customColorCheck_clicked()
{
	_model->setHasCustomColor(ui->customColorCheck->isChecked());
	updateUi();
}

void WaypointEditorDialog::on_colorRSpinBox_valueChanged(int value)
{
	_model->setColorR(value);
	updateColorSwatch();
}

void WaypointEditorDialog::on_colorGSpinBox_valueChanged(int value)
{
	_model->setColorG(value);
	updateColorSwatch();
}

void WaypointEditorDialog::on_colorBSpinBox_valueChanged(int value)
{
	_model->setColorB(value);
	updateColorSwatch();
}

void WaypointEditorDialog::on_layerCombo_currentIndexChanged(int index)
{
	if (index < 0)
		return;
	_model->setLayer(ui->layerCombo->itemData(index).toString().toUtf8().constData());
}

} // namespace fso::fred::dialogs

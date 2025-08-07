#include "ui/dialogs/AsteroidEditorDialog.h"
#include "ui/dialogs/General/CheckBoxListDialog.h"
#include "ui/util/SignalBlockers.h"

#include <algorithm>

#include "ui_AsteroidEditorDialog.h"
#include <mission/util.h>

namespace fso::fred::dialogs {

AsteroidEditorDialog::AsteroidEditorDialog(FredView *parent, EditorViewport* viewport) :
	QDialog(parent),
	_viewport(viewport),
	_editor(viewport->editor),
	ui(new Ui::AsteroidEditorDialog()),
	_model(new AsteroidEditorDialogModel(this, viewport))
{
	this->setFocus();
	ui->setupUi(this);

	// set our internal values, update the UI
	initializeUi();
	updateUi();

	// setup validators for text input
	_box_validator.setNotation(QDoubleValidator::StandardNotation);
	_box_validator.setDecimals(1);

	ui->lineEdit_obox_minX->setValidator(&_box_validator);
	ui->lineEdit_obox_minY->setValidator(&_box_validator);
	ui->lineEdit_obox_minZ->setValidator(&_box_validator);
	ui->lineEdit_obox_maxX->setValidator(&_box_validator);
	ui->lineEdit_obox_maxY->setValidator(&_box_validator);
	ui->lineEdit_obox_maxZ->setValidator(&_box_validator);
	ui->lineEdit_ibox_minX->setValidator(&_box_validator);
	ui->lineEdit_ibox_minY->setValidator(&_box_validator);
	ui->lineEdit_ibox_minZ->setValidator(&_box_validator);
	ui->lineEdit_ibox_maxX->setValidator(&_box_validator);
	ui->lineEdit_ibox_maxY->setValidator(&_box_validator);
	ui->lineEdit_ibox_maxZ->setValidator(&_box_validator);

	ui->lineEditAvgSpeed->setValidator(&_speed_validator);
}

AsteroidEditorDialog::~AsteroidEditorDialog() = default;

void AsteroidEditorDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void AsteroidEditorDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void AsteroidEditorDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}

void AsteroidEditorDialog::initializeUi()
{
	util::SignalBlockers blockers(this); // block signals while we set up the UI
	
	// Checkboxes
	ui->enabled->setChecked(_model->getFieldEnabled());
	ui->innerBoxEnabled->setChecked(_model->getInnerBoxEnabled());
	ui->enhancedFieldEnabled->setChecked(_model->getEnhancedEnabled());

	// Radio buttons for field type
	ui->radioButtonActiveField->setChecked(_model->getFieldType() == FT_ACTIVE);
	ui->radioButtonPassiveField->setChecked(_model->getFieldType() == FT_PASSIVE);

	// Radio buttons for debris genre
	ui->radioButtonAsteroid->setChecked(_model->getDebrisGenre() == DG_ASTEROID);
	ui->radioButtonDebris->setChecked(_model->getDebrisGenre() == DG_DEBRIS);

	// Spin box
	ui->spinBoxNumber->setValue(_model->getNumAsteroids());

	// Average speed
	ui->lineEditAvgSpeed->setText(_model->getAvgSpeed());

	// Outer box
	ui->lineEdit_obox_minX->setText(_model->getBoxText(AsteroidEditorDialogModel::_box_line_edits::_O_MIN_X));
	ui->lineEdit_obox_minY->setText(_model->getBoxText(AsteroidEditorDialogModel::_box_line_edits::_O_MIN_Y));
	ui->lineEdit_obox_minZ->setText(_model->getBoxText(AsteroidEditorDialogModel::_box_line_edits::_O_MIN_Z));
	ui->lineEdit_obox_maxX->setText(_model->getBoxText(AsteroidEditorDialogModel::_box_line_edits::_O_MAX_X));
	ui->lineEdit_obox_maxY->setText(_model->getBoxText(AsteroidEditorDialogModel::_box_line_edits::_O_MAX_Y));
	ui->lineEdit_obox_maxZ->setText(_model->getBoxText(AsteroidEditorDialogModel::_box_line_edits::_O_MAX_Z));

	// Inner box
	ui->lineEdit_ibox_minX->setText(_model->getBoxText(AsteroidEditorDialogModel::_box_line_edits::_I_MIN_X));
	ui->lineEdit_ibox_minY->setText(_model->getBoxText(AsteroidEditorDialogModel::_box_line_edits::_I_MIN_Y));
	ui->lineEdit_ibox_minZ->setText(_model->getBoxText(AsteroidEditorDialogModel::_box_line_edits::_I_MIN_Z));
	ui->lineEdit_ibox_maxX->setText(_model->getBoxText(AsteroidEditorDialogModel::_box_line_edits::_I_MAX_X));
	ui->lineEdit_ibox_maxY->setText(_model->getBoxText(AsteroidEditorDialogModel::_box_line_edits::_I_MAX_Y));
	ui->lineEdit_ibox_maxZ->setText(_model->getBoxText(AsteroidEditorDialogModel::_box_line_edits::_I_MAX_Z));

	// Housekeeping
	ui->spinBoxNumber->setRange(1, MAX_ASTEROIDS);
}

void AsteroidEditorDialog::updateUi()
{
	util::SignalBlockers blockers(this); // block signals while we update the UI
	
	bool overall_enabled = _model->getFieldEnabled();
	bool asteroids_enabled = overall_enabled && _model->getDebrisGenre() == DG_ASTEROID;
	bool debris_enabled = overall_enabled && _model->getDebrisGenre() == DG_DEBRIS;
	bool inner_box_enabled = _model->getInnerBoxEnabled();
	bool field_is_active = (_model->getFieldType() == FT_ACTIVE);

	// Checkboxes
	ui->innerBoxEnabled->setEnabled(overall_enabled);
	ui->enhancedFieldEnabled->setEnabled(overall_enabled);

	// Radio buttons for field type
	ui->radioButtonActiveField->setEnabled(overall_enabled);
	ui->radioButtonPassiveField->setEnabled(overall_enabled);

	// Radio buttons for debris genre
	ui->radioButtonAsteroid->setEnabled(overall_enabled);
	ui->radioButtonDebris->setEnabled(overall_enabled && !field_is_active);

	// Spin box
	ui->spinBoxNumber->setEnabled(overall_enabled);

	// Average speed
	ui->lineEditAvgSpeed->setEnabled(overall_enabled);

	// Outer box
	ui->lineEdit_obox_minX->setEnabled(overall_enabled);
	ui->lineEdit_obox_minY->setEnabled(overall_enabled);
	ui->lineEdit_obox_minZ->setEnabled(overall_enabled);
	ui->lineEdit_obox_maxX->setEnabled(overall_enabled);
	ui->lineEdit_obox_maxY->setEnabled(overall_enabled);
	ui->lineEdit_obox_maxZ->setEnabled(overall_enabled);

	// Inner box
	ui->lineEdit_ibox_minX->setEnabled(overall_enabled && inner_box_enabled);
	ui->lineEdit_ibox_minY->setEnabled(overall_enabled && inner_box_enabled);
	ui->lineEdit_ibox_minZ->setEnabled(overall_enabled && inner_box_enabled);
	ui->lineEdit_ibox_maxX->setEnabled(overall_enabled && inner_box_enabled);
	ui->lineEdit_ibox_maxY->setEnabled(overall_enabled && inner_box_enabled);
	ui->lineEdit_ibox_maxZ->setEnabled(overall_enabled && inner_box_enabled);

	// Push buttons for object types
	ui->asteroidSelectButton->setEnabled(overall_enabled && asteroids_enabled);
	ui->debrisSelectButton->setEnabled(overall_enabled && debris_enabled && !field_is_active);

	// Push buttons for ship targets
	ui->shipSelectButton->setEnabled(overall_enabled && field_is_active);

	// Update the radio buttons as these do depend on the field type
	ui->radioButtonAsteroid->setChecked(_model->getDebrisGenre() == DG_ASTEROID);
	ui->radioButtonDebris->setChecked(_model->getDebrisGenre() == DG_DEBRIS);
}

void AsteroidEditorDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void AsteroidEditorDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void AsteroidEditorDialog::on_enabled_toggled(bool enabled)
{
	_model->setFieldEnabled(enabled);
	updateUi();
}

void AsteroidEditorDialog::on_innerBoxEnabled_toggled(bool enabled)
{
	_model->setInnerBoxEnabled(enabled);
	updateUi();
}

void AsteroidEditorDialog::on_enhancedFieldEnabled_toggled(bool enabled)
{
	_model->setEnhancedEnabled(enabled);
}

void AsteroidEditorDialog::on_radioButtonActiveField_toggled(bool checked)
{
	if (checked) {
		_model->setFieldType(FT_ACTIVE);
		_model->setDebrisGenre(DG_ASTEROID); // only allow asteroids in active fields
		updateUi();
	}
}

void AsteroidEditorDialog::on_radioButtonPassiveField_toggled(bool checked)
{
	if (checked) {
		_model->setFieldType(FT_PASSIVE);
		updateUi();
	}
}

void AsteroidEditorDialog::on_radioButtonAsteroid_toggled(bool checked)
{
	if (checked) {
		_model->setDebrisGenre(DG_ASTEROID);
		updateUi();
	}
}

void AsteroidEditorDialog::on_radioButtonDebris_toggled(bool checked)
{
	if (checked) {
		_model->setDebrisGenre(DG_DEBRIS);
		updateUi();
	}
}

void AsteroidEditorDialog::on_spinBoxNumber_valueChanged(int num_asteroids)
{
	_model->setNumAsteroids(num_asteroids);
}

void AsteroidEditorDialog::on_lineEditAvgSpeed_textEdited(const QString& text)
{
	_model->setAvgSpeed(text);
}

void AsteroidEditorDialog::on_lineEdit_obox_minX_textEdited(const QString& text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_O_MIN_X);
}

void AsteroidEditorDialog::on_lineEdit_obox_minY_textEdited(const QString& text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_O_MIN_Y);
}

void AsteroidEditorDialog::on_lineEdit_obox_minZ_textEdited(const QString& text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_O_MIN_Z);
}

void AsteroidEditorDialog::on_lineEdit_obox_maxX_textEdited(const QString& text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_O_MAX_X);
}

void AsteroidEditorDialog::on_lineEdit_obox_maxY_textEdited(const QString& text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_O_MAX_Y);
}

void AsteroidEditorDialog::on_lineEdit_obox_maxZ_textEdited(const QString& text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_O_MAX_Z);
}

void AsteroidEditorDialog::on_lineEdit_ibox_minX_textEdited(const QString& text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_I_MIN_X);
}

void AsteroidEditorDialog::on_lineEdit_ibox_minY_textEdited(const QString& text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_I_MIN_Y);
}

void AsteroidEditorDialog::on_lineEdit_ibox_minZ_textEdited(const QString& text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_I_MIN_Z);
}

void AsteroidEditorDialog::on_lineEdit_ibox_maxX_textEdited(const QString& text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_I_MAX_X);
}

void AsteroidEditorDialog::on_lineEdit_ibox_maxY_textEdited(const QString& text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_I_MAX_Y);
}

void AsteroidEditorDialog::on_lineEdit_ibox_maxZ_textEdited(const QString& text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_I_MAX_Z);
}

void AsteroidEditorDialog::on_asteroidSelectButton_clicked()
{
	CheckBoxListDialog dlg(this);
	dlg.setCaption("Select Asteroid Types");
	dlg.setOptions(_model->getAsteroidSelections());

	if (dlg.exec() == QDialog::Accepted) {
		_model->setAsteroidSelections(dlg.getCheckedStates());
	}
}

void AsteroidEditorDialog::on_debrisSelectButton_clicked()
{
	CheckBoxListDialog dlg(this);
	dlg.setCaption("Select Debris Types");
	dlg.setOptions(_model->getDebrisSelections());

	if (dlg.exec() == QDialog::Accepted) {
		_model->setDebrisSelections(dlg.getCheckedStates());
	}
}

void AsteroidEditorDialog::on_shipSelectButton_clicked()
{
	CheckBoxListDialog dlg(this);
	dlg.setCaption("Select Ship Debris Types");
	dlg.setOptions(_model->getShipSelections());
	if (dlg.exec() == QDialog::Accepted) {
		_model->setShipSelections(dlg.getCheckedStates());
	}
}

} // namespace fso::fred::dialogs

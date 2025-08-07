#include "ui/dialogs/AsteroidEditorDialog.h"
#include "ui/dialogs/General/CheckBoxListDialog.h"
#include "ui/util/SignalBlockers.h"

#include <algorithm>

#include "ui_AsteroidEditorDialog.h"
#include <mission/util.h>

namespace fso {
namespace fred {
namespace dialogs {

AsteroidEditorDialog::AsteroidEditorDialog(FredView *parent, EditorViewport* viewport) :
	QDialog(parent),
	_viewport(viewport),
	_editor(viewport->editor),
	ui(new Ui::AsteroidEditorDialog()),
	_model(new AsteroidEditorDialogModel(this, viewport))
{
	connect(this, &QDialog::accepted, _model.get(), &AsteroidEditorDialogModel::apply);
	connect(ui->okAndCancelButtons, &QDialogButtonBox::rejected, this, &AsteroidEditorDialog::rejectHandler);

	ui->setupUi(this);

	// set our internal values, update the UI
	initializeUi();
	updateUi();

	// Now connect all our signals

	// checkboxes
	connect(ui->enabled, &QCheckBox::toggled, this, &AsteroidEditorDialog::toggleEnabled);
	connect(ui->innerBoxEnabled, &QCheckBox::toggled, this, &AsteroidEditorDialog::toggleInnerBoxEnabled);
	connect(ui->enhancedFieldEnabled, &QCheckBox::toggled, this, &AsteroidEditorDialog::toggleEnhancedEnabled);

	// (come in) spinners
	ui->spinBoxNumber->setRange(1, MAX_ASTEROIDS);
	ui->spinBoxNumber->setValue(_model->getNumAsteroids());
	// only connect once we're done setting values or unwanted signal's will be sent
	connect(ui->spinBoxNumber, QOverload<int>::of(&QSpinBox::valueChanged), this, \
			&AsteroidEditorDialog::asteroidNumberChanged);


	// radio buttons
	connect(ui->radioButtonActiveField, &QRadioButton::toggled, this, &AsteroidEditorDialog::setFieldActive);
	connect(ui->radioButtonPassiveField, &QRadioButton::toggled, this, &AsteroidEditorDialog::setFieldPassive);
	connect(ui->radioButtonAsteroid, &QRadioButton::toggled, this, &AsteroidEditorDialog::setGenreAsteroid);
	connect(ui->radioButtonDebris, &QRadioButton::toggled, this, &AsteroidEditorDialog::setGenreDebris);

	// lineEdit signals/slots
	connect(ui->lineEdit_obox_minX, &QLineEdit::textEdited, this, \
			&AsteroidEditorDialog::changedBoxTextOMinX);
	connect(ui->lineEdit_obox_minY, &QLineEdit::textEdited, this, \
			&AsteroidEditorDialog::changedBoxTextOMinY);
	connect(ui->lineEdit_obox_minZ, &QLineEdit::textEdited, this, \
			&AsteroidEditorDialog::changedBoxTextOMinZ);
	connect(ui->lineEdit_obox_maxX, &QLineEdit::textEdited, this, \
			&AsteroidEditorDialog::changedBoxTextOMaxX);
	connect(ui->lineEdit_obox_maxY, &QLineEdit::textEdited, this, \
			&AsteroidEditorDialog::changedBoxTextOMaxY);
	connect(ui->lineEdit_obox_maxZ, &QLineEdit::textEdited, this, \
			&AsteroidEditorDialog::changedBoxTextOMaxZ);
	connect(ui->lineEdit_ibox_minX, &QLineEdit::textEdited, this, \
			&AsteroidEditorDialog::changedBoxTextIMinX);
	connect(ui->lineEdit_ibox_minY, &QLineEdit::textEdited, this, \
			&AsteroidEditorDialog::changedBoxTextIMinY);
	connect(ui->lineEdit_ibox_minZ, &QLineEdit::textEdited, this, \
			&AsteroidEditorDialog::changedBoxTextIMinZ);
	connect(ui->lineEdit_ibox_maxX, &QLineEdit::textEdited, this, \
			&AsteroidEditorDialog::changedBoxTextIMaxX);
	connect(ui->lineEdit_ibox_maxY, &QLineEdit::textEdited, this, \
			&AsteroidEditorDialog::changedBoxTextIMaxY);
	connect(ui->lineEdit_ibox_maxZ, &QLineEdit::textEdited, this, \
			&AsteroidEditorDialog::changedBoxTextIMaxZ);

	connect(ui->lineEditAvgSpeed, &QLineEdit::textEdited, this, \
	        &AsteroidEditorDialog::changedBoxAvgSpeed);

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

void AsteroidEditorDialog::closeEvent(QCloseEvent* e)
{
	if (!rejectOrCloseHandler(this, _model.get(), _viewport)) {
		e->ignore();
	};
}

void AsteroidEditorDialog::rejectHandler()
{
	this->close();
}

QString & AsteroidEditorDialog::getBoxText(AsteroidEditorDialogModel::_box_line_edits type)
{
	return _model->getBoxText(type);
}

void AsteroidEditorDialog::changedBoxTextIMinX(const QString &text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_I_MIN_X);
}

void AsteroidEditorDialog::changedBoxTextIMinY(const QString &text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_I_MIN_Y);
}

void AsteroidEditorDialog::changedBoxTextIMinZ(const QString &text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_I_MIN_Z);
}

void AsteroidEditorDialog::changedBoxTextIMaxX(const QString &text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_I_MAX_X);
}

void AsteroidEditorDialog::changedBoxTextIMaxY(const QString &text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_I_MAX_Y);
}

void AsteroidEditorDialog::changedBoxTextIMaxZ(const QString &text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_I_MAX_Z);
}

void AsteroidEditorDialog::changedBoxTextOMinX(const QString &text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_O_MIN_X);
}

void AsteroidEditorDialog::changedBoxTextOMinY(const QString &text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_O_MIN_Y);
}

void AsteroidEditorDialog::changedBoxTextOMinZ(const QString &text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_O_MIN_Z);
}

void AsteroidEditorDialog::changedBoxTextOMaxX(const QString &text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_O_MAX_X);
}

void AsteroidEditorDialog::changedBoxTextOMaxY(const QString &text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_O_MAX_Y);
}

void AsteroidEditorDialog::changedBoxTextOMaxZ(const QString &text)
{
	_model->setBoxText(text, AsteroidEditorDialogModel::_O_MAX_Z);
}

QString& AsteroidEditorDialog::getAvgSpeedText()
{
	return _model->getAvgSpeed();
}

void AsteroidEditorDialog::changedBoxAvgSpeed(const QString& text)
{
	_model->setAvgSpeed(text);
}

void AsteroidEditorDialog::setFieldActive()
{
	_model->setFieldType(FT_ACTIVE);
	setGenreAsteroid();  // only allow asteroids in active fields
	updateUi();
}

void AsteroidEditorDialog::setFieldPassive()
{
	_model->setFieldType(FT_PASSIVE);
	updateUi();
}

void AsteroidEditorDialog::setGenreAsteroid()
{
	_model->setDebrisGenre(DG_ASTEROID);
	updateUi();
}

void AsteroidEditorDialog::setGenreDebris()
{
	_model->setDebrisGenre(DG_DEBRIS);
	updateUi();
}

void AsteroidEditorDialog::toggleEnabled(bool enabled)
{
	_model->setFieldEnabled(enabled);
	updateUi();
}

void AsteroidEditorDialog::toggleInnerBoxEnabled(bool enabled)
{
	_model->setInnerBoxEnabled(enabled);
	updateUi();
}

void AsteroidEditorDialog::toggleEnhancedEnabled(bool enabled)
{
	_model->setEnhancedEnabled(enabled);
}

void AsteroidEditorDialog::asteroidNumberChanged(int num_asteroids)
{
	_model->setNumAsteroids(num_asteroids);
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

void AsteroidEditorDialog::initializeUi()
{
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
	ui->lineEdit_obox_minX->setText(getBoxText(AsteroidEditorDialogModel::_box_line_edits::_O_MIN_X));
	ui->lineEdit_obox_minY->setText(getBoxText(AsteroidEditorDialogModel::_box_line_edits::_O_MIN_Y));
	ui->lineEdit_obox_minZ->setText(getBoxText(AsteroidEditorDialogModel::_box_line_edits::_O_MIN_Z));
	ui->lineEdit_obox_maxX->setText(getBoxText(AsteroidEditorDialogModel::_box_line_edits::_O_MAX_X));
	ui->lineEdit_obox_maxY->setText(getBoxText(AsteroidEditorDialogModel::_box_line_edits::_O_MAX_Y));
	ui->lineEdit_obox_maxZ->setText(getBoxText(AsteroidEditorDialogModel::_box_line_edits::_O_MAX_Z));

	// Inner box
	ui->lineEdit_ibox_minX->setText(getBoxText(AsteroidEditorDialogModel::_box_line_edits::_I_MIN_X));
	ui->lineEdit_ibox_minY->setText(getBoxText(AsteroidEditorDialogModel::_box_line_edits::_I_MIN_Y));
	ui->lineEdit_ibox_minZ->setText(getBoxText(AsteroidEditorDialogModel::_box_line_edits::_I_MIN_Z));
	ui->lineEdit_ibox_maxX->setText(getBoxText(AsteroidEditorDialogModel::_box_line_edits::_I_MAX_X));
	ui->lineEdit_ibox_maxY->setText(getBoxText(AsteroidEditorDialogModel::_box_line_edits::_I_MAX_Y));
	ui->lineEdit_ibox_maxZ->setText(getBoxText(AsteroidEditorDialogModel::_box_line_edits::_I_MAX_Z));
}

void AsteroidEditorDialog::updateUi()
{
	util::SignalBlockers blockers(this);
	
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

	// Push buttons for object types
	ui->asteroidSelectButton->setEnabled(overall_enabled && asteroids_enabled);
	ui->debrisSelectButton->setEnabled(overall_enabled && debris_enabled && !field_is_active);

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

	// Push buttons for ship targets
	ui->shipSelectButton->setEnabled(overall_enabled && field_is_active);

	// Update the radio buttons as these do depend on the field type
	ui->radioButtonAsteroid->setChecked(_model->getDebrisGenre() == DG_ASTEROID);
	ui->radioButtonDebris->setChecked(_model->getDebrisGenre() == DG_DEBRIS);

}

} // namespace dialogs
} // namespace fred
} // namespace fso

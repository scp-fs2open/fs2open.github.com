#include "ui/dialogs/AsteroidEditorDialog.h"
#include "ui/util/SignalBlockers.h"

#include <algorithm>

#include "ui_AsteroidEditorDialog.h"

namespace fso {
namespace fred {
namespace dialogs {

static bool sort_qcombobox_by_name(const QComboBox *left, const QComboBox *right)
{
	Assertion(left != nullptr && right != nullptr, "Don't pass nullptr's to sort!\n");
	return left->objectName() < right->objectName();
}

AsteroidEditorDialog::AsteroidEditorDialog(FredView *parent, EditorViewport* viewport) :
	QDialog(parent),
	_viewport(viewport),
	_editor(viewport->editor),
	ui(new Ui::AsteroidEditorDialog()),
	_model(new AsteroidEditorDialogModel(this, viewport))
{
	ui->setupUi(this);
	_model->update_init();

	// checkboxes
	connect(ui->enabled, &QCheckBox::toggled, this, &AsteroidEditorDialog::toggleEnabled);
	connect(ui->innerBoxEnabled, &QCheckBox::toggled, this, &AsteroidEditorDialog::toggleInnerBoxEnabled);

	connect(ui->checkBoxBrown, &QCheckBox::toggled, this,
				[this](bool enabled) { \
				AsteroidEditorDialog::toggleAsteroid(AsteroidEditorDialogModel::_AST_BROWN, enabled); });
	connect(ui->checkBoxBlue, &QCheckBox::toggled, this,
				[this](bool enabled) { \
				AsteroidEditorDialog::toggleAsteroid(AsteroidEditorDialogModel::_AST_BLUE, enabled); });
	connect(ui->checkBoxOrange, &QCheckBox::toggled, this,
				[this](bool enabled) { \
				AsteroidEditorDialog::toggleAsteroid(AsteroidEditorDialogModel::_AST_ORANGE, enabled); });

	// (come in) spinners
	ui->spinBoxNumber->setRange(1, MAX_ASTEROIDS);
	ui->spinBoxNumber->setValue(_model->getNumAsteroids());
	// only connect once we're done setting values or unwanted signal's will be sent
	connect(ui->spinBoxNumber, QOverload<int>::of(&QSpinBox::valueChanged), this, \
			&AsteroidEditorDialog::asteroidNumberChanged);

	// setup values in ship debris combo boxes
	// MFC let you set comboxbox item indexes, Qt doesn't so we'll need a lookup
	static_assert(MAX_ACTIVE_DEBRIS_TYPES == 3,
			"qtFRED only provides three combo boxes for debris type input");
	debrisComboBoxes = ui->fieldProperties->findChildren<QComboBox *>(QString(), Qt::FindDirectChildrenOnly);
	std::sort(debrisComboBoxes.begin(), debrisComboBoxes.end(), sort_qcombobox_by_name);

	QString debris_size[NUM_ASTEROID_SIZES] = { "Small", "Medium", "Large" };
	QStringList debris_names("None");
	for (const auto& i : Species_info)  // each species
	{
		for (const auto& j : debris_size) // each size
		{
			debris_names += QString(i.species_name) + " " + j;
		}
	}

	for (auto i = 0; i < MAX_ACTIVE_DEBRIS_TYPES; ++i) {
		debrisComboBoxes.at(i)->addItems(debris_names);
		// update debris combobox data on index changes
		connect(debrisComboBoxes.at(i), QOverload<int>::of(&QComboBox::currentIndexChanged), this, \
				[this, i](int debris_type) { AsteroidEditorDialog::updateComboBox(i,debris_type); });
	}


	// radio buttons
	connect(ui->radioButtonActiveField, &QRadioButton::toggled, this, &AsteroidEditorDialog::setFieldActive);
	connect(ui->radioButtonPassiveField, &QRadioButton::toggled, this, &AsteroidEditorDialog::setFieldPassive);
	connect(ui->radioButtonAsteroid, &QRadioButton::toggled, this, &AsteroidEditorDialog::setGenreAsteroid);
	connect(ui->radioButtonShip, &QRadioButton::toggled, this, &AsteroidEditorDialog::setGenreDebris);

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

	updateUI();
}

AsteroidEditorDialog::~AsteroidEditorDialog() = default;

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

void AsteroidEditorDialog::setFieldActive()
{
	_model->setFieldType(FT_ACTIVE);
	setGenreAsteroid();  // only allow asteroids in active fields
	updateUI();
}

void AsteroidEditorDialog::setFieldPassive()
{
	_model->setFieldType(FT_PASSIVE);
	updateUI();
}

void AsteroidEditorDialog::setGenreAsteroid()
{
	_model->setDebrisGenre(DG_ASTEROID);
	updateUI();
}

void AsteroidEditorDialog::setGenreDebris()
{
	_model->setDebrisGenre(DG_DEBRIS);
	updateUI();
}

void AsteroidEditorDialog::toggleEnabled(bool enabled)
{
	_model->setEnabled(enabled);
	updateUI();
}

void AsteroidEditorDialog::toggleInnerBoxEnabled(bool enabled)
{
	_model->setInnerBoxEnabled(enabled);
	updateUI();
}

void AsteroidEditorDialog::toggleAsteroid(AsteroidEditorDialogModel::_roid_types colour, bool enabled)
{
	_model->setAsteroidEnabled(colour, enabled);
	updateUI();
}

void AsteroidEditorDialog::asteroidNumberChanged(int num_asteroids)
{
	_model->setNumAsteroids(num_asteroids);
}

void AsteroidEditorDialog::updateComboBox(int idx, int debris_type)
{
	_model->setFieldDebrisType(idx, debris_type);
}

void AsteroidEditorDialog::updateUI()
{
	util::SignalBlockers blockers(this);

	// various useful states
	bool asteroids_enabled = _model->getEnabled();
	bool inner_box_enabled = _model->getInnerBoxEnabled();
	bool field_is_active = (_model->getFieldType() == FT_ACTIVE);
	bool debris_is_asteroid = (_model->getDebrisGenre() == DG_ASTEROID);

	// checkboxes
	ui->enabled->setChecked(asteroids_enabled);
	ui->innerBoxEnabled->setChecked(inner_box_enabled);
	ui->checkBoxBrown->setChecked(_model->getAsteroidEnabled(AsteroidEditorDialogModel::_AST_BROWN));
	ui->checkBoxBlue->setChecked(_model->getAsteroidEnabled(AsteroidEditorDialogModel::_AST_BLUE));
	ui->checkBoxOrange->setChecked(_model->getAsteroidEnabled(AsteroidEditorDialogModel::_AST_ORANGE));

	// radio buttons (2x groups)
	ui->radioButtonActiveField->setChecked(field_is_active);
	ui->radioButtonPassiveField->setChecked(!field_is_active);
	ui->radioButtonAsteroid->setChecked(debris_is_asteroid);
	ui->radioButtonShip->setChecked(!debris_is_asteroid);
	if (field_is_active) {
		ui->radioButtonShip->setToolTip(QString("Ship Debris is only allowed in passive fields"));
	}
	else {
		ui->radioButtonShip->setToolTip(QString(""));
	}

	// enable/disable sections of interface
	ui->fieldProperties->setEnabled(asteroids_enabled);
	ui->outerBox->setEnabled(asteroids_enabled);
	ui->innerBox->setEnabled(asteroids_enabled);

	ui->innerBoxEnabled->setEnabled(asteroids_enabled && field_is_active);
	ui->lineEdit_ibox_maxX->setEnabled(inner_box_enabled && field_is_active);
	ui->lineEdit_ibox_maxY->setEnabled(inner_box_enabled && field_is_active);
	ui->lineEdit_ibox_maxZ->setEnabled(inner_box_enabled && field_is_active);
	ui->lineEdit_ibox_minX->setEnabled(inner_box_enabled && field_is_active);
	ui->lineEdit_ibox_minY->setEnabled(inner_box_enabled && field_is_active);
	ui->lineEdit_ibox_minZ->setEnabled(inner_box_enabled && field_is_active);
	ui->label_ibox_maxX->setEnabled(inner_box_enabled && field_is_active);
	ui->label_ibox_maxY->setEnabled(inner_box_enabled && field_is_active);
	ui->label_ibox_maxZ->setEnabled(inner_box_enabled && field_is_active);
	ui->label_ibox_minX->setEnabled(inner_box_enabled && field_is_active);
	ui->label_ibox_minY->setEnabled(inner_box_enabled && field_is_active);
	ui->label_ibox_minZ->setEnabled(inner_box_enabled && field_is_active);

	ui->radioButtonShip->setEnabled(!field_is_active);

	ui->checkBoxBrown->setEnabled(debris_is_asteroid);
	ui->checkBoxBlue->setEnabled(debris_is_asteroid);
	ui->checkBoxOrange->setEnabled(debris_is_asteroid);

	// speed text
	ui->lineEditAvgSpeed->setText(_model->AsteroidEditorDialogModel::getAvgSpeed());

	// ship debris comboboxes
	for (auto i = 0; i < debrisComboBoxes.size(); ++i) {
		debrisComboBoxes.at(i)->setCurrentIndex(_model->AsteroidEditorDialogModel::getFieldDebrisType(i));
		debrisComboBoxes.at(i)->setEnabled(!debris_is_asteroid);
	}

	// mix/max field bounding boxes text
	ui->lineEdit_obox_minX->setText(_model->AsteroidEditorDialogModel::getBoxText(AsteroidEditorDialogModel::_O_MIN_X));
	ui->lineEdit_obox_minY->setText(_model->AsteroidEditorDialogModel::getBoxText(AsteroidEditorDialogModel::_O_MIN_Y));
	ui->lineEdit_obox_minZ->setText(_model->AsteroidEditorDialogModel::getBoxText(AsteroidEditorDialogModel::_O_MIN_Z));
	ui->lineEdit_obox_maxX->setText(_model->AsteroidEditorDialogModel::getBoxText(AsteroidEditorDialogModel::_O_MAX_X));
	ui->lineEdit_obox_maxY->setText(_model->AsteroidEditorDialogModel::getBoxText(AsteroidEditorDialogModel::_O_MAX_Y));
	ui->lineEdit_obox_maxZ->setText(_model->AsteroidEditorDialogModel::getBoxText(AsteroidEditorDialogModel::_O_MAX_Z));
	ui->lineEdit_ibox_minX->setText(_model->AsteroidEditorDialogModel::getBoxText(AsteroidEditorDialogModel::_I_MIN_X));
	ui->lineEdit_ibox_minY->setText(_model->AsteroidEditorDialogModel::getBoxText(AsteroidEditorDialogModel::_I_MIN_Y));
	ui->lineEdit_ibox_minZ->setText(_model->AsteroidEditorDialogModel::getBoxText(AsteroidEditorDialogModel::_I_MIN_Z));
	ui->lineEdit_ibox_maxX->setText(_model->AsteroidEditorDialogModel::getBoxText(AsteroidEditorDialogModel::_I_MAX_X));
	ui->lineEdit_ibox_maxY->setText(_model->AsteroidEditorDialogModel::getBoxText(AsteroidEditorDialogModel::_I_MAX_Y));
	ui->lineEdit_ibox_maxZ->setText(_model->AsteroidEditorDialogModel::getBoxText(AsteroidEditorDialogModel::_I_MAX_Z));
}

void AsteroidEditorDialog::done(int r)
{
	if(QDialog::Accepted == r)  // ok was pressed
	{
		// TODO consider moving the validation to when values are entered
		// but just visually indicate that there's a problem at that time
		// hard fail (by checking status boolean?) here instead
		// i.e. let FREDers have temp bad values when they're changing stuff
		// if they know what they're doing
		if (_model->apply()) {
			// all ok
			QDialog::done(r);
			_model->unset_modified();
			return;
		}
		else {
			// leave dialog open
			return;
		}
	}
	else    // cancel, close or exc was pressed
	{
		if (_model->get_modified()) {
			// give FREDer a chance in case they cancelled by mistake
			// although I wonder if we're better off with always saving & don't prompt
			// ~philisophical~
			auto z = _viewport->dialogProvider->showButtonDialog(DialogType::Question,
				"Question",
				"You have unsaved changes, do you wish to discard them?",
				{ DialogButton::Ok, DialogButton::Cancel });
			if (z == DialogButton::Cancel) {
				return;
			}
		}
		QDialog::done(r);
		_model->unset_modified();
		return;
	}
}

} // namespace dialogs
} // namespace fred
} // namespace fso

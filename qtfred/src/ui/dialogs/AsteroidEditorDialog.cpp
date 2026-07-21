#include "ui/dialogs/AsteroidEditorDialog.h"
#include "ui/dialogs/General/CheckBoxListDialog.h"
#include "ui/util/SignalBlockers.h"

#include <algorithm>

#include "ui_AsteroidEditorDialog.h"
#include <mission/util.h>
#include <mission/commands/FredCommands.h>
#include <ui/util/DialogUndo.h>

#include <utility>

namespace fso::fred::dialogs {

AsteroidEditorDialog::AsteroidEditorDialog(FredView *parent, EditorViewport* viewport) :
	QDialog(parent),
	_viewport(viewport),
	_editor(viewport->editor),
	_fredView(parent),
	ui(new Ui::AsteroidEditorDialog()),
	_model(new AsteroidEditorDialogModel(this, viewport))
{
	this->setFocus();
	ui->setupUi(this);

	_dialogStack = new QUndoStack(this);
	_fredView->undoGroup()->addStack(_dialogStack);
	util::setupDialogUndo(this, _fredView->undoGroup(), _dialogStack, tr("Asteroid Field"));

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
	QByteArray stateBefore = _model->captureState();
	if (_model->apply()) {
		QByteArray stateAfter = _model->captureState();
		_model->setParent(nullptr);
		_fredView->mainUndoStack()->push(
			new ApplyDialogCommand(std::move(_model), stateBefore, stateAfter,
			                       _viewport->editor, tr("Edit Asteroid Field")));
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::accept();
	}
}

void AsteroidEditorDialog::reject()
{
	if (!_model) {
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
		return;
	}
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
	}
}

void AsteroidEditorDialog::closeEvent(QCloseEvent* e)
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
	const bool before = _model->getFieldEnabled();
	if (before == enabled) {
		return;
	}

	_model->setFieldEnabled(enabled);
	updateUi();

	auto* cmd = new FieldEditCommand<bool>(FieldId::Ast_FieldEnabled, nullptr, tr("Toggle Asteroid Field"), true);
	cmd->addEntry(before, enabled, [this](const bool& v) {
		_model->setFieldEnabled(v);
		QSignalBlocker blocker(ui->enabled);
		ui->enabled->setChecked(v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void AsteroidEditorDialog::on_innerBoxEnabled_toggled(bool enabled)
{
	const bool before = _model->getInnerBoxEnabled();
	if (before == enabled) {
		return;
	}

	_model->setInnerBoxEnabled(enabled);
	updateUi();

	auto* cmd = new FieldEditCommand<bool>(FieldId::Ast_InnerEnabled, nullptr, tr("Toggle Inner Box"), true);
	cmd->addEntry(before, enabled, [this](const bool& v) {
		_model->setInnerBoxEnabled(v);
		QSignalBlocker blocker(ui->innerBoxEnabled);
		ui->innerBoxEnabled->setChecked(v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void AsteroidEditorDialog::on_enhancedFieldEnabled_toggled(bool enabled)
{
	const bool before = _model->getEnhancedEnabled();
	if (before == enabled) {
		return;
	}

	_model->setEnhancedEnabled(enabled);

	auto* cmd = new FieldEditCommand<bool>(FieldId::Ast_Enhanced, nullptr, tr("Toggle Enhanced Field"), true);
	cmd->addEntry(before, enabled, [this](const bool& v) {
		_model->setEnhancedEnabled(v);
		QSignalBlocker blocker(ui->enhancedFieldEnabled);
		ui->enhancedFieldEnabled->setChecked(v);
	});
	_dialogStack->push(cmd);
}

// Field type and debris genre change together (an active field forces the
// asteroid genre), so the radio commands track the (type, genre) pair.
void AsteroidEditorDialog::changeFieldType(field_type_t type, debris_genre_t genre)
{
	const std::pair<int, int> before{_model->getFieldType(), _model->getDebrisGenre()};
	const std::pair<int, int> after{type, genre};
	if (before == after) {
		return;
	}

	_model->setFieldType(type);
	_model->setDebrisGenre(genre);
	updateUi();

	auto* cmd = new FieldEditCommand<std::pair<int, int>>(FieldId::Ast_FieldType, nullptr, tr("Change Field Type"), true);
	cmd->addEntry(before, after, [this](const std::pair<int, int>& v) {
		_model->setFieldType(static_cast<field_type_t>(v.first));
		_model->setDebrisGenre(static_cast<debris_genre_t>(v.second));
		QSignalBlocker blockActive(ui->radioButtonActiveField);
		QSignalBlocker blockPassive(ui->radioButtonPassiveField);
		ui->radioButtonActiveField->setChecked(v.first == FT_ACTIVE);
		ui->radioButtonPassiveField->setChecked(v.first == FT_PASSIVE);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void AsteroidEditorDialog::on_radioButtonActiveField_toggled(bool checked)
{
	if (checked) {
		changeFieldType(FT_ACTIVE, DG_ASTEROID); // only allow asteroids in active fields
	}
}

void AsteroidEditorDialog::on_radioButtonPassiveField_toggled(bool checked)
{
	if (checked) {
		changeFieldType(FT_PASSIVE, _model->getDebrisGenre());
	}
}

void AsteroidEditorDialog::on_radioButtonAsteroid_toggled(bool checked)
{
	if (checked && _model->getDebrisGenre() != DG_ASTEROID) {
		_model->setDebrisGenre(DG_ASTEROID);
		updateUi();

		auto* cmd = new FieldEditCommand<int>(FieldId::Ast_DebrisGenre, nullptr, tr("Change Debris Genre"), true);
		cmd->addEntry(DG_DEBRIS, DG_ASTEROID, [this](const int& v) {
			_model->setDebrisGenre(static_cast<debris_genre_t>(v));
			updateUi(); // re-checks the genre radios
		});
		_dialogStack->push(cmd);
	}
}

void AsteroidEditorDialog::on_radioButtonDebris_toggled(bool checked)
{
	if (checked && _model->getDebrisGenre() != DG_DEBRIS) {
		_model->setDebrisGenre(DG_DEBRIS);
		updateUi();

		auto* cmd = new FieldEditCommand<int>(FieldId::Ast_DebrisGenre, nullptr, tr("Change Debris Genre"), true);
		cmd->addEntry(DG_ASTEROID, DG_DEBRIS, [this](const int& v) {
			_model->setDebrisGenre(static_cast<debris_genre_t>(v));
			updateUi(); // re-checks the genre radios
		});
		_dialogStack->push(cmd);
	}
}

void AsteroidEditorDialog::on_spinBoxNumber_valueChanged(int num_asteroids)
{
	const int before = _model->getNumAsteroids();
	if (before == num_asteroids) {
		return;
	}

	_model->setNumAsteroids(num_asteroids);

	auto* cmd = new FieldEditCommand<int>(FieldId::Ast_NumAsteroids, nullptr, tr("Change Asteroid Count"), true);
	cmd->addEntry(before, num_asteroids, [this](const int& v) {
		_model->setNumAsteroids(v);
		QSignalBlocker blocker(ui->spinBoxNumber);
		ui->spinBoxNumber->setValue(v);
	});
	_dialogStack->push(cmd);
}

void AsteroidEditorDialog::on_lineEditAvgSpeed_textEdited(const QString& text)
{
	const QString before = _model->getAvgSpeed();
	if (before == text) {
		return;
	}

	_model->setAvgSpeed(text);

	auto* cmd = new FieldEditCommand<QString>(FieldId::Ast_AvgSpeed, nullptr, tr("Change Average Speed"), true);
	cmd->addEntry(before, text, [this](const QString& v) {
		_model->setAvgSpeed(v);
		// textEdited does not fire for programmatic setText
		ui->lineEditAvgSpeed->setText(v);
	});
	_dialogStack->push(cmd);
}

void AsteroidEditorDialog::changeBoxText(QLineEdit* edit, AsteroidEditorDialogModel::_box_line_edits type, const QString& text)
{
	const QString before = _model->getBoxText(type);
	if (before == text) {
		return;
	}

	_model->setBoxText(text, type);

	auto* cmd = new FieldEditCommand<QString>(FieldId::Ast_Box + type, nullptr, tr("Change Field Bounds"), true);
	cmd->addEntry(before, text, [this, edit, type](const QString& v) {
		_model->setBoxText(v, type);
		// textEdited does not fire for programmatic setText
		edit->setText(v);
	});
	_dialogStack->push(cmd);
}

void AsteroidEditorDialog::on_lineEdit_obox_minX_textEdited(const QString& text)
{
	changeBoxText(ui->lineEdit_obox_minX, AsteroidEditorDialogModel::_O_MIN_X, text);
}

void AsteroidEditorDialog::on_lineEdit_obox_minY_textEdited(const QString& text)
{
	changeBoxText(ui->lineEdit_obox_minY, AsteroidEditorDialogModel::_O_MIN_Y, text);
}

void AsteroidEditorDialog::on_lineEdit_obox_minZ_textEdited(const QString& text)
{
	changeBoxText(ui->lineEdit_obox_minZ, AsteroidEditorDialogModel::_O_MIN_Z, text);
}

void AsteroidEditorDialog::on_lineEdit_obox_maxX_textEdited(const QString& text)
{
	changeBoxText(ui->lineEdit_obox_maxX, AsteroidEditorDialogModel::_O_MAX_X, text);
}

void AsteroidEditorDialog::on_lineEdit_obox_maxY_textEdited(const QString& text)
{
	changeBoxText(ui->lineEdit_obox_maxY, AsteroidEditorDialogModel::_O_MAX_Y, text);
}

void AsteroidEditorDialog::on_lineEdit_obox_maxZ_textEdited(const QString& text)
{
	changeBoxText(ui->lineEdit_obox_maxZ, AsteroidEditorDialogModel::_O_MAX_Z, text);
}

void AsteroidEditorDialog::on_lineEdit_ibox_minX_textEdited(const QString& text)
{
	changeBoxText(ui->lineEdit_ibox_minX, AsteroidEditorDialogModel::_I_MIN_X, text);
}

void AsteroidEditorDialog::on_lineEdit_ibox_minY_textEdited(const QString& text)
{
	changeBoxText(ui->lineEdit_ibox_minY, AsteroidEditorDialogModel::_I_MIN_Y, text);
}

void AsteroidEditorDialog::on_lineEdit_ibox_minZ_textEdited(const QString& text)
{
	changeBoxText(ui->lineEdit_ibox_minZ, AsteroidEditorDialogModel::_I_MIN_Z, text);
}

void AsteroidEditorDialog::on_lineEdit_ibox_maxX_textEdited(const QString& text)
{
	changeBoxText(ui->lineEdit_ibox_maxX, AsteroidEditorDialogModel::_I_MAX_X, text);
}

void AsteroidEditorDialog::on_lineEdit_ibox_maxY_textEdited(const QString& text)
{
	changeBoxText(ui->lineEdit_ibox_maxY, AsteroidEditorDialogModel::_I_MAX_Y, text);
}

void AsteroidEditorDialog::on_lineEdit_ibox_maxZ_textEdited(const QString& text)
{
	changeBoxText(ui->lineEdit_ibox_maxZ, AsteroidEditorDialogModel::_I_MAX_Z, text);
}

static QVector<bool> checkedStates(const QVector<std::pair<QString, bool>>& options)
{
	QVector<bool> out;
	out.reserve(options.size());
	for (const auto& option : options) {
		out.push_back(option.second);
	}
	return out;
}

void AsteroidEditorDialog::on_asteroidSelectButton_clicked()
{
	CheckBoxListDialog dlg(this);
	dlg.setCaption("Select Asteroid Types");
	dlg.setOptions(_model->getAsteroidSelections());

	if (dlg.exec() != QDialog::Accepted) {
		return;
	}

	const QVector<bool> before = checkedStates(_model->getAsteroidSelections());
	const QVector<bool> after  = dlg.getCheckedStates();
	if (before == after) {
		return;
	}

	_model->setAsteroidSelections(after);

	auto* cmd = new FieldEditCommand<QVector<bool>>(FieldId::Ast_AsteroidSel, nullptr, tr("Select Asteroid Types"), true);
	cmd->setNoMerge(); // each subdialog visit is a discrete action
	cmd->addEntry(before, after, [this](const QVector<bool>& v) { _model->setAsteroidSelections(v); });
	_dialogStack->push(cmd);
}

void AsteroidEditorDialog::on_debrisSelectButton_clicked()
{
	CheckBoxListDialog dlg(this);
	dlg.setCaption("Select Debris Types");
	dlg.setOptions(_model->getDebrisSelections());

	if (dlg.exec() != QDialog::Accepted) {
		return;
	}

	const QVector<bool> before = checkedStates(_model->getDebrisSelections());
	const QVector<bool> after  = dlg.getCheckedStates();
	if (before == after) {
		return;
	}

	_model->setDebrisSelections(after);

	auto* cmd = new FieldEditCommand<QVector<bool>>(FieldId::Ast_DebrisSel, nullptr, tr("Select Debris Types"), true);
	cmd->setNoMerge();
	cmd->addEntry(before, after, [this](const QVector<bool>& v) { _model->setDebrisSelections(v); });
	_dialogStack->push(cmd);
}

void AsteroidEditorDialog::on_shipSelectButton_clicked()
{
	CheckBoxListDialog dlg(this);
	dlg.setCaption("Select Ship Debris Types");
	dlg.setOptions(_model->getShipSelections());
	if (dlg.exec() != QDialog::Accepted) {
		return;
	}

	// Undo state is the resolved target-name list, not the checkbox bitmap:
	// the bitmap only has meaning against the ship list captured when the
	// subdialog opened (setShipSelections consumes and clears that list).
	const SCP_vector<SCP_string> before = _model->getShipTargetNames();
	_model->setShipSelections(dlg.getCheckedStates());
	const SCP_vector<SCP_string> after = _model->getShipTargetNames();
	if (before == after) {
		return;
	}

	auto* cmd = new FieldEditCommand<SCP_vector<SCP_string>>(
		FieldId::Ast_ShipSel, nullptr, tr("Select Ship Debris Types"), true);
	cmd->setNoMerge();
	cmd->addEntry(before, after, [this](const SCP_vector<SCP_string>& v) { _model->setShipTargetNames(v); });
	_dialogStack->push(cmd);
}

} // namespace fso::fred::dialogs

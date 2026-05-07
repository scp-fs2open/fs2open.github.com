#include "ShipTextureReplacementDialog.h"

#include "ui_ShipTextureReplacementDialog.h"

#include <globalincs/globals.h>
#include <mission/util.h>
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>
namespace fso::fred::dialogs {

// Model for list view, Should really have its own file but its not big enught for me to bother.
MapModel::MapModel(ShipTextureReplacementDialogModel* data, QObject* parent) : QAbstractListModel(parent), _model(data)
{
}

int MapModel::rowCount(const QModelIndex& /*parent*/) const
{
	return static_cast<int>(_model->getSize());
}

QVariant MapModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::DisplayRole) {
		QString out = _model->getDefaultName(index.row()).c_str();
		return out;
	}
	if (role == Qt::SizeHintRole) {
		QString out = _model->getDefaultName(index.row()).c_str();
		if (out.isEmpty()) {
			return (QSize(0, 0));
		}
	}
	return {};
}

ShipTextureReplacementDialog::ShipTextureReplacementDialog(QDialog* parent, EditorViewport* viewport, bool multiEdit)
	: QDialog(parent), ui(new Ui::ShipTextureReplacementDialog()),
	  _model(new ShipTextureReplacementDialogModel(this, viewport, multiEdit)), _viewport(viewport)
{
	ui->setupUi(this);

	ui->newTextureLineEdit->setMaxLength(MAX_FILENAME_LEN - 1);
	ui->AmbiantTextureLineEdit->setMaxLength(MAX_FILENAME_LEN - 1);
	ui->MiscTextureLineEdit->setMaxLength(MAX_FILENAME_LEN - 1);
	ui->ShineTextureLineEdit->setMaxLength(MAX_FILENAME_LEN - 1);
	ui->HeightTextureLineEdit->setMaxLength(MAX_FILENAME_LEN - 1);
	ui->GlowTextureLineEdit->setMaxLength(MAX_FILENAME_LEN - 1);
	ui->NormalTextureLineEdit->setMaxLength(MAX_FILENAME_LEN - 1);
	ui->ReflectTextureLineEdit->setMaxLength(MAX_FILENAME_LEN - 1);

	_listModel = new MapModel(_model.get(), this);
	ui->TexturesList->setModel(_listModel);
	QItemSelectionModel* selectionModel = ui->TexturesList->selectionModel();
	connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &ShipTextureReplacementDialog::updateUiFull);
	QModelIndex index = _listModel->index(0);
	ui->TexturesList->setCurrentIndex(index);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &ShipTextureReplacementDialog::updateUi);

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}
ShipTextureReplacementDialog::~ShipTextureReplacementDialog() = default;

void ShipTextureReplacementDialog::accept()
{ // If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don't close
}

void ShipTextureReplacementDialog::reject()
{   // Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void ShipTextureReplacementDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}
void ShipTextureReplacementDialog::on_buttonBox_accepted()
{
	accept();
}
void ShipTextureReplacementDialog::on_buttonBox_rejected()
{
	reject();
}
void ShipTextureReplacementDialog::on_newTextureLineEdit_editingFinished()
{
	SCP_string newText;
	if (!ui->newTextureLineEdit->text().isEmpty()) {
		newText = ui->newTextureLineEdit->text().toUtf8().constData();
	}
	_model->setMap(_selectedRow, "main", newText);
}

void ShipTextureReplacementDialog::on_MiscTextureLineEdit_editingFinished()
{
	SCP_string newText;
	if (!ui->MiscTextureLineEdit->text().isEmpty()) {
		newText = ui->MiscTextureLineEdit->text().toUtf8().constData();
		_model->setMap(_selectedRow, "misc", newText);
	}
}

void ShipTextureReplacementDialog::on_GlowTextureLineEdit_editingFinished()
{
	SCP_string newText;
	if (!ui->GlowTextureLineEdit->text().isEmpty()) {
		newText = ui->GlowTextureLineEdit->text().toUtf8().constData();
		_model->setMap(_selectedRow, "glow", newText);
	}
}

void ShipTextureReplacementDialog::on_ShineTextureLineEdit_editingFinished()
{
	SCP_string newText;
	if (!ui->ShineTextureLineEdit->text().isEmpty()) {
		newText = ui->ShineTextureLineEdit->text().toUtf8().constData();
		_model->setMap(_selectedRow, "shine", newText);
	}
}

void ShipTextureReplacementDialog::on_NormalTextureLineEdit_editingFinished()
{
	SCP_string newText;
	if (!ui->NormalTextureLineEdit->text().isEmpty()) {
		newText = ui->NormalTextureLineEdit->text().toUtf8().constData();
		_model->setMap(_selectedRow, "normal", newText);
	}
}

void ShipTextureReplacementDialog::on_HeightTextureLineEdit_editingFinished()
{
	SCP_string newText;
	if (!ui->HeightTextureLineEdit->text().isEmpty()) {
		newText = ui->HeightTextureLineEdit->text().toUtf8().constData();
		_model->setMap(_selectedRow, "height", newText);
	}
}

void ShipTextureReplacementDialog::on_AmbiantTextureLineEdit_editingFinished()
{
	SCP_string newText;
	if (!ui->AmbiantTextureLineEdit->text().isEmpty()) {
		newText = ui->AmbiantTextureLineEdit->text().toUtf8().constData();
		_model->setMap(_selectedRow, "ao", newText);
	}
}

void ShipTextureReplacementDialog::on_ReflectTextureLineEdit_editingFinished()
{
	SCP_string newText;
	if (!ui->ReflectTextureLineEdit->text().isEmpty()) {
		newText = ui->ReflectTextureLineEdit->text().toUtf8().constData();
		_model->setMap(_selectedRow, "reflect", newText);
	}
}

void ShipTextureReplacementDialog::on_useMiscTexturecheckbox_toggled(bool state)
{
	_model->setReplace(_selectedRow, "misc", state);
}

void ShipTextureReplacementDialog::on_useGlowTexturecheckbox_toggled(bool state)
{
	_model->setReplace(_selectedRow, "glow", state);
}

void ShipTextureReplacementDialog::on_useShineTexturecheckbox_toggled(bool state)
{
	_model->setReplace(_selectedRow, "shine", state);
}

void ShipTextureReplacementDialog::on_useNormalTexturecheckbox_toggled(bool state)
{
	_model->setReplace(_selectedRow, "normal", state);
}

void ShipTextureReplacementDialog::on_useHeightTexturecheckbox_toggled(bool state)
{
	_model->setReplace(_selectedRow, "height", state);
}

void ShipTextureReplacementDialog::on_useAmbiantTexturecheckbox_toggled(bool state)
{
	_model->setReplace(_selectedRow, "ao", state);
}

void ShipTextureReplacementDialog::on_useReflectTexturecheckbox_toggled(bool state)
{
	_model->setReplace(_selectedRow, "reflect", state);
}

void ShipTextureReplacementDialog::on_inheritMiscTexturecheckbox_toggled(bool state)
{
	_model->setInherit(_selectedRow, "misc", state);
}

void ShipTextureReplacementDialog::on_inheritGlowTexturecheckbox_toggled(bool state)
{
	_model->setInherit(_selectedRow, "glow", state);
}

void ShipTextureReplacementDialog::on_inheritShineTexturecheckbox_toggled(bool state)
{
	_model->setInherit(_selectedRow, "shine", state);
}

void ShipTextureReplacementDialog::on_inheritNormalTexturecheckbox_toggled(bool state)
{
	_model->setInherit(_selectedRow, "normal", state);
}

void ShipTextureReplacementDialog::on_inheritHeightTexturecheckbox_toggled(bool state)
{
	_model->setInherit(_selectedRow, "height", state);
}

void ShipTextureReplacementDialog::on_inheritAmbiantTexturecheckbox_toggled(bool state)
{
	_model->setInherit(_selectedRow, "ao", state);
}

void ShipTextureReplacementDialog::on_inheritReflectTexturecheckbox_toggled(bool state)
{
	_model->setInherit(_selectedRow, "reflect", state);
}

void ShipTextureReplacementDialog::updateUi()
{
	util::SignalBlockers blockers(this);
	ui->newTextureLineEdit->setText(_model->getMap(_selectedRow, "main").c_str());
	bool replace;
	bool inherit;
	ui->useMiscTexturecheckbox->setChecked(replace = _model->getReplace(_selectedRow)["misc"]);
	if (replace) {
		ui->inheritMiscTextureCheckbox->setChecked(inherit = _model->getInherit(_selectedRow)["misc"]);
		ui->inheritMiscTextureCheckbox->setEnabled(replace);
		ui->MiscTextureLineEdit->setEnabled(!inherit);
		if (!inherit) {
			ui->MiscTextureLineEdit->setText(_model->getMap(_selectedRow, "misc").c_str());
		}
	} else {
		ui->inheritMiscTextureCheckbox->setDisabled(true);
		ui->MiscTextureLineEdit->setDisabled(true);
	}

	ui->useShineTexturecheckbox->setChecked(replace = _model->getReplace(_selectedRow)["shine"]);
	if (replace) {
		ui->inheritShineTextureCheckbox->setChecked(inherit = _model->getInherit(_selectedRow)["shine"]);
		ui->inheritShineTextureCheckbox->setEnabled(replace);

		ui->ShineTextureLineEdit->setEnabled(!inherit);
		if (!inherit) {
			ui->ShineTextureLineEdit->setText(_model->getMap(_selectedRow, "shine").c_str());
		}
	} else {
		ui->inheritShineTextureCheckbox->setDisabled(true);
		ui->ShineTextureLineEdit->setDisabled(true);
	}

	ui->useGlowTexturecheckbox->setChecked(replace = _model->getReplace(_selectedRow)["glow"]);
	if (replace) {
		ui->inheritGlowTextureCheckbox->setEnabled(replace);
		ui->inheritGlowTextureCheckbox->setChecked(inherit = _model->getInherit(_selectedRow)["glow"]);
		ui->GlowTextureLineEdit->setEnabled(!inherit);
		if (!inherit) {
			ui->GlowTextureLineEdit->setText(_model->getMap(_selectedRow, "glow").c_str());
		}
	} else {
		ui->inheritGlowTextureCheckbox->setDisabled(true);
		ui->GlowTextureLineEdit->setDisabled(true);
	}

	ui->useNormalTexturecheckbox->setChecked(replace = _model->getReplace(_selectedRow)["normal"]);
	if (replace) {
		ui->inheritNormalTextureCheckbox->setChecked(inherit = _model->getInherit(_selectedRow)["normal"]);
		ui->inheritNormalTextureCheckbox->setEnabled(replace);
		ui->NormalTextureLineEdit->setEnabled(!inherit);
		if (!inherit) {
			ui->NormalTextureLineEdit->setText(_model->getMap(_selectedRow, "normal").c_str());
		}
	} else {
		ui->inheritNormalTextureCheckbox->setDisabled(true);
		ui->NormalTextureLineEdit->setDisabled(true);
	}

	ui->useHeightTexturecheckbox->setChecked(replace = _model->getReplace(_selectedRow)["height"]);
	if (replace) {
		ui->inheritHeightTextureCheckbox->setChecked(inherit = _model->getInherit(_selectedRow)["height"]);
		ui->inheritHeightTextureCheckbox->setEnabled(replace);
		ui->HeightTextureLineEdit->setEnabled(!inherit);
		if (!inherit) {
			ui->HeightTextureLineEdit->setText(_model->getMap(_selectedRow, "height").c_str());
		}
	} else {
		ui->inheritHeightTextureCheckbox->setDisabled(true);
		ui->HeightTextureLineEdit->setDisabled(true);
	}

	ui->useAmbiantTexturecheckbox->setChecked(replace = _model->getReplace(_selectedRow)["ao"]);
	if (replace) {
		ui->inheritAmbiantTextureCheckbox->setChecked(inherit = _model->getInherit(_selectedRow)["ao"]);
		ui->AmbiantTextureLineEdit->setEnabled(!inherit);
		ui->inheritAmbiantTextureCheckbox->setEnabled(replace);
		if (!inherit) {
			ui->AmbiantTextureLineEdit->setText(_model->getMap(_selectedRow, "ao").c_str());
		}
	} else {
		ui->inheritAmbiantTextureCheckbox->setDisabled(true);
		ui->AmbiantTextureLineEdit->setDisabled(true);
	}

	ui->useReflectTexturecheckbox->setChecked(replace = _model->getReplace(_selectedRow)["reflect"]);
	if (replace) {
		ui->inheritReflectTextureCheckbox->setChecked(inherit = _model->getInherit(_selectedRow)["reflect"]);
		ui->ReflectTextureLineEdit->setEnabled(!inherit);
		ui->inheritReflectTextureCheckbox->setEnabled(replace);
		if (!inherit) {
			ui->ReflectTextureLineEdit->setText(_model->getMap(_selectedRow, "reflect").c_str());
		}
	} else {
		ui->inheritReflectTextureCheckbox->setDisabled(true);
		ui->ReflectTextureLineEdit->setDisabled(true);
	}
}
void ShipTextureReplacementDialog::updateUiFull()
{
	util::SignalBlockers blockers(this);
	const QModelIndex index = ui->TexturesList->selectionModel()->currentIndex();
	_selectedRow = index.row();
	SCP_map<SCP_string, bool> subtypes = _model->getSubtypesForMap(_selectedRow);
	bool hide = !(subtypes)["misc"];
	ui->inheritMiscTextureCheckbox->setHidden(hide);
	ui->MiscTextureLabel->setHidden(hide);
	ui->MiscTextureLineEdit->setHidden(hide);
	ui->useMiscTexturecheckbox->setHidden(hide);
	hide = !(subtypes)["shine"];
	ui->inheritShineTextureCheckbox->setHidden(hide);
	ui->ShineTextureLabel->setHidden(hide);
	ui->ShineTextureLineEdit->setHidden(hide);
	ui->useShineTexturecheckbox->setHidden(hide);
	hide = !(subtypes)["glow"];
	ui->inheritGlowTextureCheckbox->setHidden(hide);
	ui->GlowTextureLabel->setHidden(hide);
	ui->GlowTextureLineEdit->setHidden(hide);
	ui->useGlowTexturecheckbox->setHidden(hide);
	hide = !(subtypes)["normal"];
	ui->inheritNormalTextureCheckbox->setHidden(hide);
	ui->NormalTextureLabel->setHidden(hide);
	ui->NormalTextureLineEdit->setHidden(hide);
	ui->useNormalTexturecheckbox->setHidden(hide);
	hide = !(subtypes)["height"];
	ui->inheritHeightTextureCheckbox->setHidden(hide);
	ui->HeightTextureLabel->setHidden(hide);
	ui->HeightTextureLineEdit->setHidden(hide);
	ui->useHeightTexturecheckbox->setHidden(hide);
	hide = !(subtypes)["ao"];
	ui->inheritAmbiantTextureCheckbox->setHidden(hide);
	ui->AmbiantTextureLabel->setHidden(hide);
	ui->AmbiantTextureLineEdit->setHidden(hide);
	ui->useAmbiantTexturecheckbox->setHidden(hide);
	hide = !(subtypes)["reflect"];
	ui->inheritReflectTextureCheckbox->setHidden(hide);
	ui->ReflectTextureLabel->setHidden(hide);
	ui->ReflectTextureLineEdit->setHidden(hide);
	ui->useReflectTexturecheckbox->setHidden(hide);
	resize(QDialog::sizeHint());
	updateUi();
}
} // namespace fso::fred::dialogs

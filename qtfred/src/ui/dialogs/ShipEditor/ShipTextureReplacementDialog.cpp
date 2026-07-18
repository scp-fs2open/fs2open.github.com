#include "ShipTextureReplacementDialog.h"

#include "ui_ShipTextureReplacementDialog.h"

#include <globalincs/globals.h>
#include <mission/util.h>
#include <model/model.h>
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>
#include <QGridLayout>
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

	auto* subLayout = qobject_cast<QGridLayout*>(ui->subTextureBox->layout());
	int gridRow = 0;
	for (const auto& [tmType, suffix] : MODEL_TEXTURE_SUFFIXES) {
		SCP_string typeKey = suffix.substr(1);
		SCP_string displayStr = typeKey;
		if (!displayStr.empty())
			displayStr[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(displayStr[0])));

		auto* label      = new QLabel(displayStr.c_str(), this);
		auto* useBox     = new QCheckBox(tr("Replace"), this);
		auto* inheritBox = new QCheckBox(tr("Inherit"), this);
		auto* lineEdit   = new QLineEdit(this);
		lineEdit->setMaxLength(MAX_FILENAME_LEN - 1);

		subLayout->addWidget(label,      gridRow, 0);
		subLayout->addWidget(useBox,     gridRow, 1);
		subLayout->addWidget(inheritBox, gridRow, 2);
		subLayout->addWidget(lineEdit,   gridRow, 3);

		connect(lineEdit, &QLineEdit::editingFinished, this, [this, typeKey, lineEdit]() {
			SCP_string newText;
			if (!lineEdit->text().isEmpty())
				newText = lineEdit->text().toUtf8().constData();
			_model->setMap(_selectedRow, typeKey, newText);
		});
		connect(useBox, &QCheckBox::toggled, this, [this, typeKey](bool state) {
			_model->setReplace(_selectedRow, typeKey, state);
		});
		connect(inheritBox, &QCheckBox::toggled, this, [this, typeKey](bool state) {
			_model->setInherit(_selectedRow, typeKey, state);
		});

		_textureRows[typeKey] = { label, useBox, inheritBox, lineEdit };
		++gridRow;
	}

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

void ShipTextureReplacementDialog::updateUi()
{
	util::SignalBlockers blockers(this);
	ui->newTextureLineEdit->setText(_model->getMap(_selectedRow, "main").c_str());
	for (auto& [type, widgets] : _textureRows) {
		bool replace = _model->getReplace(_selectedRow)[type];
		widgets.useCheckbox->setChecked(replace);
		if (replace) {
			bool inherit = _model->getInherit(_selectedRow)[type];
			widgets.inheritCheckbox->setEnabled(true);
			widgets.inheritCheckbox->setChecked(inherit);
			widgets.lineEdit->setEnabled(!inherit);
			if (inherit) {
				const SCP_string mainName = _model->getMap(_selectedRow, "main");
				if (mainName.empty() || mainName == "invisible") {
					widgets.lineEdit->setText(mainName.c_str());
				} else {
					SCP_string derived = mainName;
					derived += '-';
					derived += type;
					widgets.lineEdit->setText(derived.c_str());
				}
			} else {
				widgets.lineEdit->setText(_model->getMap(_selectedRow, type).c_str());
			}
		} else {
			widgets.inheritCheckbox->setDisabled(true);
			widgets.lineEdit->setDisabled(true);
		}
	}
}
void ShipTextureReplacementDialog::updateUiFull()
{
	util::SignalBlockers blockers(this);
	const QModelIndex index = ui->TexturesList->selectionModel()->currentIndex();
	_selectedRow = index.row();
	SCP_map<SCP_string, bool> subtypes = _model->getSubtypesForMap(_selectedRow);
	for (auto& [type, widgets] : _textureRows) {
		bool hide = !subtypes[type];
		widgets.label->setHidden(hide);
		widgets.useCheckbox->setHidden(hide);
		widgets.inheritCheckbox->setHidden(hide);
		widgets.lineEdit->setHidden(hide);
	}
	resize(QDialog::sizeHint());
	updateUi();
}
} // namespace fso::fred::dialogs

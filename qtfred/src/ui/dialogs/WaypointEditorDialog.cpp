#include <object/waypoint.h>
#include <jumpnode/jumpnode.h>
#include <QtWidgets/QTextEdit>
#include "ui/dialogs/WaypointEditorDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_WaypointEditorDialog.h"

namespace fso {
namespace fred {
namespace dialogs {


WaypointEditorDialog::WaypointEditorDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent),
	_viewport(viewport),
	_editor(viewport->editor),
	ui(new Ui::WaypointEditorDialog()),
	_model(new WaypointEditorDialogModel(this, viewport)) {
	ui->setupUi(this);

	connect(this, &QDialog::accepted, _model.get(), &WaypointEditorDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &WaypointEditorDialogModel::reject);

	connect(parent, &FredView::viewWindowActivated, _model.get(), &WaypointEditorDialogModel::apply);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &WaypointEditorDialog::updateUI);

	connect(ui->pathSelection,
			static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this,
			&WaypointEditorDialog::pathSelectionChanged);

	connect(ui->nameEdit, &QLineEdit::textChanged, this, &WaypointEditorDialog::nameTextChanged);

	// Initial set up of the UI
	updateUI();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}
WaypointEditorDialog::~WaypointEditorDialog() {
}
void WaypointEditorDialog::pathSelectionChanged(int index) {
	auto itemId = ui->pathSelection->itemData(index).value<int>();
	_model->idSelected(itemId);
}
void WaypointEditorDialog::reject() {
	// This dialog never rejects
	accept();
}
void WaypointEditorDialog::updateComboBox() {
	// Remove all previous entries
	ui->pathSelection->clear();

	for (auto& el : _model->getElements()) {
		ui->pathSelection->addItem(QString::fromStdString(el.name), QVariant(el.id));
	}

	auto itemIndex = ui->pathSelection->findData(QVariant(_model->getCurrentElementId()));
	ui->pathSelection->setCurrentIndex(itemIndex); // This also works if the index is -1

	ui->pathSelection->setEnabled(ui->pathSelection->count() > 0);
}
void WaypointEditorDialog::updateUI() {
	util::SignalBlockers blockers(this);

	updateComboBox();

	ui->nameEdit->setText(QString::fromStdString(_model->getCurrentName()));
	ui->nameEdit->setEnabled(_model->isEnabled());
}
void WaypointEditorDialog::nameTextChanged(const QString& newText) {
	_model->setNameEditText(newText.toStdString());
}
bool WaypointEditorDialog::event(QEvent* event) {
	switch(event->type()) {
	case QEvent::WindowDeactivate:
		_model->apply();
		event->accept();
		return true;
	default:
		return QDialog::event(event);
	}
}

}
}
}

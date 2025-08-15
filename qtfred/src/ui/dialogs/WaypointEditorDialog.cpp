#include <QtWidgets/QTextEdit>
#include "ui/dialogs/WaypointEditorDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_WaypointEditorDialog.h"

#include <mission/util.h>

namespace fso::fred::dialogs {


WaypointEditorDialog::WaypointEditorDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent),
	_viewport(viewport),
	ui(new Ui::WaypointEditorDialog()),
	_model(new WaypointEditorDialogModel(this, viewport))
{
	this->setFocus();
	ui->setupUi(this);

	initializeUi();
	updateUi();

	connect(_model.get(), &WaypointEditorDialogModel::waypointPathMarkingChanged, this, [this] {
		initializeUi();
		updateUi();
	});

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

WaypointEditorDialog::~WaypointEditorDialog() = default;

void WaypointEditorDialog::initializeUi()
{
	util::SignalBlockers blockers(this);

	updateWaypointListComboBox();
	ui->nameEdit->setEnabled(_model->isEnabled());
}

void WaypointEditorDialog::updateWaypointListComboBox()
{
	ui->pathSelection->clear();

	for (auto& wp : _model->getWaypointPathList()) {
		ui->pathSelection->addItem(QString::fromStdString(wp.first), wp.second);
	}

	ui->pathSelection->setEnabled(!_model->getWaypointPathList().empty());
}

void WaypointEditorDialog::updateUi()
{
	util::SignalBlockers blockers(this);
	ui->nameEdit->setText(QString::fromStdString(_model->getCurrentName()));
	ui->pathSelection->setCurrentIndex(ui->pathSelection->findData(_model->getCurrentlySelectedPath()));
}

void WaypointEditorDialog::on_pathSelection_currentIndexChanged(int index)
{
	auto itemId = ui->pathSelection->itemData(index).value<int>();
	_model->setCurrentlySelectedPath(itemId);
}

// This will run any time an edit is finished which includes the entire window closing, losing focus,
// the user clicking elsewhere in the dialog, or pressing Enter in the edit box.
// This is ok here because this is literally the only field that can be edited but if this dialog
// ever expands then it would be wise to change the whole thing to an ok/cancel type dialog.
void WaypointEditorDialog::on_nameEdit_editingFinished()
{
	// Waypoint editor applies immediately when the name is changed
	// so save the current, try to apply, if fails, restore the current
	// and update the text in the edit box

	SCP_string current = _model->getCurrentName();

	SCP_string newText = ui->nameEdit->text().toUtf8().constData();
	_model->setCurrentName(newText);

	if (!_model->apply()) {
		util::SignalBlockers blockers(this);
		// If apply failed, restore the old name
		ui->nameEdit->setText(QString::fromStdString(current));
		_model->setCurrentName(current); // Restore the model's current name
	}
}

} // namespace fso::fred::dialogs

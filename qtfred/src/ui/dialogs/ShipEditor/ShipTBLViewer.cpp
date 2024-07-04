#include "ShipTBLViewer.h"

#include "ui_ShipTBLViewer.h"

#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso {
namespace fred {
namespace dialogs {
ShipTBLViewer::ShipTBLViewer(QWidget* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::ShipTBLViewer()), _viewport(viewport)
{
	parentDialog = dynamic_cast<ShipEditorDialog*>(parent);
	Assert(parentDialog);
	_model = std::unique_ptr<ShipTBLViewerModel>(new ShipTBLViewerModel(this, viewport, parentDialog->getShipClass()));

	ui->setupUi(this);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &ShipTBLViewer::updateUI);

	updateUI();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

ShipTBLViewer::~ShipTBLViewer() = default;
void ShipTBLViewer::closeEvent(QCloseEvent* event)
{
	QDialog::closeEvent(event);
}
void ShipTBLViewer::showEvent(QShowEvent* e) {
	_model->initializeData(parentDialog->getShipClass());

	QDialog::showEvent(e);
}
void ShipTBLViewer::updateUI()
{
	util::SignalBlockers blockers(this);
	ui->TBLData->setPlainText(_model->getText().c_str());
}
} // namespace dialogs
} // namespace fred
} // namespace fso
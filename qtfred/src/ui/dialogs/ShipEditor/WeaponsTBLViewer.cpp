#include "WeaponsTBLViewer.h"

#include "ui_ShipTBLViewer.h"

#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso::fred::dialogs {
WeaponsTBLViewer::WeaponsTBLViewer(QWidget* parent, EditorViewport* viewport, int wc)
	: QDialog(parent), ui(new Ui::ShipTBLViewer()), _model(new WeaponsTBLViewerModel(this, viewport, wc)),
	  _viewport(viewport)
{

	ui->setupUi(this);
	this->setWindowTitle("Weapon TBL Data");
	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &WeaponsTBLViewer::updateUI);

	updateUI();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

WeaponsTBLViewer::~WeaponsTBLViewer() = default;
void WeaponsTBLViewer::closeEvent(QCloseEvent* event)
{
	QDialog::closeEvent(event);
}
void WeaponsTBLViewer::updateUI()
{
	util::SignalBlockers blockers(this);
	ui->TBLData->setPlainText(_model->getText().c_str());
}
} // namespace fso::fred::dialogs
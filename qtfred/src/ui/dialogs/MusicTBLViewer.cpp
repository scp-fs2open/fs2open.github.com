#include "MusicTBLViewer.h"

#include "ui_ShipTBLViewer.h"

#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso::fred::dialogs {
MusicTBLViewer::MusicTBLViewer(QWidget* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::ShipTBLViewer()), _model(new MusicTBLViewerModel(this, viewport)),
	  _viewport(viewport)
{

	ui->setupUi(this);
	this->setWindowTitle("Weapon TBL Data");
	connect(_model.get(), &AbstractDialogModel::modelChanged, this, &MusicTBLViewer::updateUi);

	updateUi();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

MusicTBLViewer::~MusicTBLViewer() = default;
void MusicTBLViewer::closeEvent(QCloseEvent* event)
{
	QDialog::closeEvent(event);
}
void MusicTBLViewer::updateUi()
{
	util::SignalBlockers blockers(this);
	ui->TBLData->setPlainText(_model->getText().c_str());
}
} // namespace fso::fred::dialogs
#include "ShipAltShipClass.h"

#include "ui_ShipAltShipClass.h"

#include <mission/util.h>
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso::fred::dialogs {
ShipAltShipClass::ShipAltShipClass(QDialog* parent, EditorViewport* viewport, bool is_several_ships)
	: QDialog(parent), ui(new Ui::ShipAltShipClass()),
	  _model(new ShipAltShipClassModel(this, viewport, is_several_ships)), _viewport(viewport)
{
	this->setFocus();
	ui->setupUi(this);
	updateUI();
}

ShipAltShipClass::~ShipAltShipClass() = default;

void ShipAltShipClass::accept()
{ // If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void ShipAltShipClass::reject()
{ // Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void ShipAltShipClass::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}

void ShipAltShipClass::on_buttonBox_accepted()
{
	accept();
}
void ShipAltShipClass::on_buttonBox_rejected() {
	reject();
}

void ShipAltShipClass::updateUI() {
	util::SignalBlockers blockers(this); // block signals while we set up the UI
}
} // namespace fso::fred::dialogs
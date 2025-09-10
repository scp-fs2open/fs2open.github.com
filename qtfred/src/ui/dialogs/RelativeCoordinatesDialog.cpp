#include "ui/dialogs/RelativeCoordinatesDialog.h"
#include "ui_RelativeCoordinatesDialog.h"
#include "ui/util/SignalBlockers.h"

#include <QCloseEvent>

namespace fso::fred::dialogs {

RelativeCoordinatesDialog::RelativeCoordinatesDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), _viewport(viewport), ui(new Ui::RelativeCoordinatesDialog()),
	  _model(new RelativeCoordinatesDialogModel(this, viewport))
{
	ui->setupUi(this);

	initializeUi();
}

RelativeCoordinatesDialog::~RelativeCoordinatesDialog() = default;

void RelativeCoordinatesDialog::initializeUi()
{
	util::SignalBlockers blockers(this);
	
	ui->originListWidget->clear();
	ui->satelliteListWidget->clear();

	const auto objects = _model->getObjectsList();

	auto addWithData = [](QListWidget* list, const std::string& name, int objnum) {
		auto* item = new QListWidgetItem(QString::fromStdString(name));
		item->setData(Qt::UserRole, objnum);
		list->addItem(item);
	};

	for (const auto& [name, objnum] : objects) {
		addWithData(ui->originListWidget, name, objnum);
		addWithData(ui->satelliteListWidget, name, objnum);
	}

	updateUi();
}

void RelativeCoordinatesDialog::updateUi()
{
	util::SignalBlockers blockers(this);
	
	// Restore selections based on model's obj indices
	auto selectByObjnum = [](QListWidget* list, int wantObj) {
		if (wantObj < 0)
			return;
		for (int r = 0; r < list->count(); ++r) {
			if (list->item(r)->data(Qt::UserRole).toInt() == wantObj) {
				list->setCurrentRow(r);
				break;
			}
		}
	};
	selectByObjnum(ui->originListWidget, _model->getOrigin());
	selectByObjnum(ui->satelliteListWidget, _model->getSatellite());

	ui->distanceDoubleSpinBox->setValue(static_cast<double>(_model->getDistance()));
	ui->pitchDoubleSpinBox->setValue(static_cast<double>(_model->getPitch()));
	ui->bankDoubleSpinBox->setValue(static_cast<double>(_model->getBank()));
	ui->headingDoubleSpinBox->setValue(static_cast<double>(_model->getHeading()));
}

void RelativeCoordinatesDialog::on_originListWidget_currentRowChanged(int row)
{
	auto* item = ui->originListWidget->item(row);
	if (!item)
		return;
	const int objnum = item->data(Qt::UserRole).toInt();
	_model->setOrigin(objnum);
	updateUi();
}

void RelativeCoordinatesDialog::on_satelliteListWidget_currentRowChanged(int row)
{
	auto* item = ui->satelliteListWidget->item(row);
	if (!item)
		return;
	const int objnum = item->data(Qt::UserRole).toInt();
	_model->setSatellite(objnum);
	updateUi();
}

} // namespace fred::dialogs
#include "ShipPathsDialog.h"
#include "ui_ShipPathsDialog.h"
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>
#include <mission/util.h>
namespace fso {
namespace fred {
namespace dialogs {
ShipPathsDialog::ShipPathsDialog(QWidget* parent,
	EditorViewport* viewport,
	const int ship,
	const int target_class,
	const bool departure)
	: QDialog(parent), ui(new Ui::ShipPathsDialog()),
	  _model(new ShipPathsDialogModel(this, viewport, ship, target_class, departure)), _viewport(viewport)
{
	ui->setupUi(this);
	connect(ui->pathList, &QListWidget::itemChanged, this, &ShipPathsDialog::changed);
	connect(this, &QDialog::accepted, _model.get(), &ShipPathsDialogModel::apply);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ShipPathsDialog::rejectHandler);
	if (departure) {
		ui->instructLabel->setText("Restrict departure paths to the following:");
	} else {
		ui->instructLabel->setText("Restrict arrival paths to the following:");
	}
	updateUI();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}
ShipPathsDialog::~ShipPathsDialog() = default;
void ShipPathsDialog::closeEvent(QCloseEvent* e) {
	if (!rejectOrCloseHandler(this, _model.get(), _viewport)) {
		e->ignore();
	};
}
void ShipPathsDialog::rejectHandler()
{
	this->close();
}
void ShipPathsDialog::updateUI() {
	util::SignalBlockers blockers(this);
	for (size_t i = 0; i < _model->getPathList().size(); i++) {
		QString name = _model->getModel()->paths[_model->getModel()->ship_bay->path_indexes[i]].name;
		auto item = new QListWidgetItem(name, ui->pathList);
		item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		Qt::CheckState state; 
		if (_model->getPathList()[i] == true) {
			state = Qt::Checked;
		} else {
			state = Qt::Unchecked;
		}
		item->setCheckState(state);
	}
}
void ShipPathsDialog::changed(QListWidgetItem* changeditem) {
	bool checked = changeditem->checkState() == Qt::Checked;
	int index = ui->pathList->row(changeditem);
	_model->modify(index, checked);
}
} // namespace dialogs
} // namespace fred
} // namespace fso
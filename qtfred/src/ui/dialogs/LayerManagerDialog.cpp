#include "LayerManagerDialog.h"
#include "ui_LayerManagerDialog.h"

#include <QCheckBox>
#include <QInputDialog>
#include <QListWidget>
#include <QMessageBox>

namespace fso::fred::dialogs {

LayerManagerDialog::LayerManagerDialog(EditorViewport* viewport, QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::LayerManagerDialog())
	, _model(new LayerManagerDialogModel(this, viewport))
{
	ui->setupUi(this);

	initializeUi();
	updateUi();

	connect(_model.get(), &LayerManagerDialogModel::modelChanged, this, &LayerManagerDialog::updateUi);
}

LayerManagerDialog::~LayerManagerDialog() = default;

void LayerManagerDialog::initializeUi() {
	// Populate IFF team checkboxes dynamically; insert before the spacer at the end of iffLayout
	const int iffCount = _model->getIffCount();
	const int spacerIndex = ui->iffLayout->count() - 1;
	for (int i = 0; i < iffCount; ++i) {
		auto* check = new QCheckBox(QString::fromStdString(_model->getIffName(i)), ui->iffScrollContents);
		connect(check, &QCheckBox::toggled, this, [this, i](bool checked) {
			_model->setShowIff(i, checked);
		});
		ui->iffLayout->insertWidget(spacerIndex + i, check);
		_iffChecks.append(check);
	}
}

void LayerManagerDialog::updateUi() {
	_refreshing = true;

	// Sync layer list
	const int previousRow = ui->layerList->currentRow();
	ui->layerList->clear();
	for (const auto& name : _model->getLayerNames()) {
		auto* item = new QListWidgetItem(QString::fromStdString(name), ui->layerList);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		item->setCheckState(_model->getLayerVisibility(name) ? Qt::Checked : Qt::Unchecked);
		if (_model->isDefaultLayer(name)) {
			item->setToolTip(tr("The default layer always exists and cannot be deleted."));
		}
	}
	if (ui->layerList->count() > 0) {
		ui->layerList->setCurrentRow(qMax(0, qMin(previousRow, ui->layerList->count() - 1)));
	}

	// Sync object type filter checkboxes
	{
		QSignalBlocker b1(ui->showShipsCheck);
		QSignalBlocker b2(ui->showStartsCheck);
		QSignalBlocker b3(ui->showWaypointsCheck);
		ui->showShipsCheck->setChecked(_model->getShowShips());
		ui->showStartsCheck->setChecked(_model->getShowStarts());
		ui->showWaypointsCheck->setChecked(_model->getShowWaypoints());
	}

	// Sync IFF checkboxes
	for (int i = 0; i < _iffChecks.size(); ++i) {
		QSignalBlocker blocker(_iffChecks[i]);
		_iffChecks[i]->setChecked(_model->getShowIff(i));
	}

	_refreshing = false;

	// Update delete button based on selection
	ui->deleteLayerButton->setEnabled(ui->layerList->currentRow() > 0);
}

void LayerManagerDialog::on_addLayerButton_clicked() {
	bool ok = false;
	auto name = QInputDialog::getText(this, tr("Add Layer"), tr("Layer name:"), QLineEdit::Normal, QString(), &ok).trimmed();
	if (!ok || name.isEmpty()) {
		if (ok && name.isEmpty()) {
			QMessageBox::warning(this, tr("Layer Error"), tr("Layer name cannot be empty."));
		}
		return;
	}

	SCP_string error;
	if (!_model->addLayer(name.toUtf8().constData(), &error)) {
		QMessageBox::warning(this, tr("Layer Error"), QString::fromStdString(error));
		return;
	}

	// Select the newly added layer
	for (int i = 0; i < ui->layerList->count(); ++i) {
		if (ui->layerList->item(i)->text() == name) {
			ui->layerList->setCurrentRow(i);
			break;
		}
	}
}

void LayerManagerDialog::on_deleteLayerButton_clicked() {
	auto* item = ui->layerList->currentItem();
	if (item == nullptr) {
		return;
	}

	const SCP_string layerName = item->text().toUtf8().constData();
	if (_model->isDefaultLayer(layerName)) {
		QMessageBox::warning(this, tr("Layer Error"), tr("The default layer cannot be deleted."));
		return;
	}

	SCP_string error;
	if (!_model->deleteLayer(layerName, &error)) {
		QMessageBox::warning(this, tr("Layer Error"), QString::fromStdString(error));
	}
}

void LayerManagerDialog::on_layerList_currentRowChanged(int row) {
	ui->deleteLayerButton->setEnabled(row > 0);
}

void LayerManagerDialog::on_layerList_itemChanged(QListWidgetItem* item) {
	if (_refreshing || item == nullptr) {
		return;
	}

	const SCP_string layerName = item->text().toUtf8().constData();
	const bool visible = item->checkState() == Qt::Checked;

	SCP_string error;
	if (!_model->setLayerVisibility(layerName, visible, &error)) {
		QMessageBox::warning(this, tr("Layer Error"), QString::fromStdString(error));
	}
}

void LayerManagerDialog::on_showShipsCheck_toggled(bool checked)     { _model->setShowShips(checked); }
void LayerManagerDialog::on_showStartsCheck_toggled(bool checked)    { _model->setShowStarts(checked); }
void LayerManagerDialog::on_showWaypointsCheck_toggled(bool checked) { _model->setShowWaypoints(checked); }

} // namespace fso::fred::dialogs

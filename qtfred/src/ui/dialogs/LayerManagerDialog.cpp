#include "LayerManagerDialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <mission/EditorViewport.h>

namespace fso::fred::dialogs {

LayerManagerDialog::LayerManagerDialog(EditorViewport* viewport, QWidget* parent) : QDialog(parent), _viewport(viewport) {
	setWindowTitle(tr("View Layers"));
	resize(380, 320);

	auto* layout = new QVBoxLayout(this);
	_layerList = new QListWidget(this);
	_layerList->setSelectionMode(QAbstractItemView::SingleSelection);
	layout->addWidget(_layerList);

	auto* buttonsLayout = new QHBoxLayout();
	auto* addButton = new QPushButton(tr("Add Layer"), this);
	_deleteButton = new QPushButton(tr("Delete Layer"), this);
	buttonsLayout->addWidget(addButton);
	buttonsLayout->addWidget(_deleteButton);
	buttonsLayout->addStretch(1);
	layout->addLayout(buttonsLayout);

	auto* closeButtons = new QDialogButtonBox(QDialogButtonBox::Close, this);
	layout->addWidget(closeButtons);

	connect(_layerList, &QListWidget::itemChanged, this, [this](QListWidgetItem* item) { onLayerItemChanged(item); });
	connect(_layerList, &QListWidget::currentRowChanged, this, [this](int row) {
		_deleteButton->setEnabled(row > 0);
	});
	connect(addButton, &QPushButton::clicked, this, &LayerManagerDialog::onAddLayer);
	connect(_deleteButton, &QPushButton::clicked, this, &LayerManagerDialog::onDeleteLayer);
	connect(closeButtons, &QDialogButtonBox::rejected, this, &QDialog::accept);

	refreshLayers();
}

void LayerManagerDialog::refreshLayers() {
	_refreshing = true;
	_layerList->clear();

	const auto layerNames = _viewport->getLayerNames();
	for (const auto& layerName : layerNames) {
		bool isVisible = true;
		_viewport->getLayerVisibility(layerName, &isVisible);
		auto* item = new QListWidgetItem(QString::fromStdString(layerName), _layerList);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		item->setCheckState(isVisible ? Qt::Checked : Qt::Unchecked);
		if (layerName == EditorViewport::DefaultLayerName) {
			item->setToolTip(tr("The default layer always exists and cannot be deleted."));
		}
	}

	if (_layerList->count() > 0) {
		_layerList->setCurrentRow(0);
	}
	_refreshing = false;
}

void LayerManagerDialog::onLayerItemChanged(QListWidgetItem* item) {
	if (_refreshing) {
		return;
	}

	if (item == nullptr) {
		return;
	}

	const SCP_string layerName = item->text().toUtf8().constData();
	const auto visible = item->checkState() == Qt::Checked;
	SCP_string error;
	if (!_viewport->setLayerVisibility(layerName, visible, &error)) {
		QMessageBox::warning(this, tr("Layer Error"), QString::fromStdString(error));
		refreshLayers();
	}
}

void LayerManagerDialog::onAddLayer() {
	bool ok = false;
	auto name = QInputDialog::getText(this, tr("Add Layer"), tr("Layer name:"), QLineEdit::Normal, QString(), &ok).trimmed();
	if (!ok) {
		return;
	}

	if (name.isEmpty()) {
		QMessageBox::warning(this, tr("Layer Error"), tr("Layer name cannot be empty."));
		return;
	}

	SCP_string error;
	if (!_viewport->addLayer(name.toUtf8().constData(), &error)) {
		QMessageBox::warning(this, tr("Layer Error"), QString::fromStdString(error));
		return;
	}

	refreshLayers();
	for (int i = 0; i < _layerList->count(); ++i) {
		if (_layerList->item(i)->text() == name) {
			_layerList->setCurrentRow(i);
			break;
		}
	}
}

void LayerManagerDialog::onDeleteLayer() {
	auto* item = _layerList->currentItem();
	if (item == nullptr) {
		return;
	}

	const SCP_string layerName = item->text().toUtf8().constData();
	if (layerName == EditorViewport::DefaultLayerName) {
		QMessageBox::warning(this, tr("Layer Error"), tr("The default layer cannot be deleted."));
		return;
	}

	SCP_string error;
	if (!_viewport->deleteLayer(layerName, &error)) {
		QMessageBox::warning(this, tr("Layer Error"), QString::fromStdString(error));
		return;
	}

	refreshLayers();
}

} // namespace fso::fred::dialogs

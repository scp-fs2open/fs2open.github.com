#pragma once

#include <QDialog>
#include <QVector>

#include "mission/dialogs/LayerManagerDialogModel.h"

class QCheckBox;
class QListWidgetItem;

namespace fso::fred::dialogs {

namespace Ui {
class LayerManagerDialog;
}

class LayerManagerDialog final : public QDialog {
	Q_OBJECT

public:
	explicit LayerManagerDialog(EditorViewport* viewport, QWidget* parent = nullptr);
	~LayerManagerDialog() override;

private slots:
	void on_addLayerButton_clicked();
	void on_deleteLayerButton_clicked();
	void on_layerList_currentRowChanged(int row);
	void on_layerList_itemChanged(QListWidgetItem* item);
	void on_showShipsCheck_toggled(bool checked);
	void on_showStartsCheck_toggled(bool checked);
	void on_showWaypointsCheck_toggled(bool checked);

private: // NOLINT(readability-redundant-access-specifiers)
	void initializeUi();
	void updateUi();

	bool _refreshing = false;
	QVector<QCheckBox*> _iffChecks;
	std::unique_ptr<Ui::LayerManagerDialog> ui;
	std::unique_ptr<LayerManagerDialogModel> _model;
};

} // namespace fso::fred::dialogs

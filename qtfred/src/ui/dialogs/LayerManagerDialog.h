#pragma once

#include <QDialog>

class QListWidget;
class QListWidgetItem;
class QPushButton;

namespace fso::fred {
class EditorViewport;
}

namespace fso::fred::dialogs {

class LayerManagerDialog final : public QDialog {
	Q_OBJECT

 public:
	explicit LayerManagerDialog(EditorViewport* viewport, QWidget* parent = nullptr);

 private:
	void refreshLayers();
	void onLayerItemChanged(QListWidgetItem* item);
	void onAddLayer();
	void onDeleteLayer();

	EditorViewport* _viewport = nullptr;
	QListWidget* _layerList = nullptr;
	QPushButton* _deleteButton = nullptr;
	bool _refreshing = false;
};

} // namespace fso::fred::dialogs

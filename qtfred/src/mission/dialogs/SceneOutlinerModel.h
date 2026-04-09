#pragma once

#include "AbstractDialogModel.h"

#include <QSet>
#include <QString>
#include <QTimer>
#include <QVector>

namespace fso::fred::dialogs {

/**
 * @brief A single selectable object in the scene outliner tree
 *
 * Leaf nodes (ships, props, jump nodes, individual waypoints) have objNum >= 0.
 * Wing headers have wingIndex >= 0 and objNum == -1.
 * Waypoint path headers have waypointListIndex >= 0 and objNum == -1.
 */
struct OutlinerObject {
	QString displayName;
	int objNum = -1;
	int wingIndex = -1;
	int waypointListIndex = -1;
	bool isPlayerStart = false;
	QVector<OutlinerObject> children;
};

struct OutlinerCategory {
	QString name;
	QVector<OutlinerObject> items;
};

struct OutlinerLayer {
	QString name;
	bool visible = true;
	QVector<OutlinerCategory> categories;
};

class SceneOutlinerModel : public AbstractDialogModel {
	Q_OBJECT

public:
	SceneOutlinerModel(QObject* parent, EditorViewport* viewport);

	bool apply() override { return true; }
	void reject() override {}

	const QVector<OutlinerLayer>& getTree() const { return _tree; }
	QSet<int> getMarkedSet() const;
	QVector<QString> getLayerNames() const;

	void toggleLayerVisibility(const QString& layerName);
	void moveObjectToLayer(int objNum, const QString& layerName);
	void moveWingToLayer(int wingIndex, const QString& layerName);
	void moveWaypointPathToLayer(int waypointListIndex, const QString& layerName);

	void selectObjectFromOutliner(int objNum);
	void multiSelectFromOutliner(const QVector<int>& objNums);
	void selectWingFromOutliner(int wingIndex);

	void setNameFilter(const QString& filter);
	const QString& getNameFilter() const { return _nameFilter; }

	// IFF filtering
	void setFilterIff(int team, bool visible);
	bool getFilterIff(int team) const;
	int iffCount() const;
	QString getIffName(int team) const;

	// Bulk selection
	void selectAll();
	void clearSelection();
	void invertSelection();

	bool isUpdatingFromOutliner() const { return _updatingFromOutliner; }

signals:
	/**
	 * @brief Fired when the full tree structure needs to be rebuilt (objects added/removed/renamed).
	 * The panel should call rebuildTree() in response.
	 */
	void treeStructureChanged();

private:
	void buildTree();

	QVector<OutlinerLayer> _tree;
	QString _nameFilter;
	QVector<bool> _filterIff;
	bool _updatingFromOutliner = false;
	QTimer* _rebuildTimer = nullptr;

private slots:
	void onCurrentObjectChanged(int newObj);
	void onObjectMarkingChanged(int obj, bool marked);
	void onLayerVisibilityChanged();
	void onLayerStructureChanged();
	void onMissionLoaded();
	void onMissionChanged();
	void onRebuildTimer();
};

} // namespace fso::fred::dialogs

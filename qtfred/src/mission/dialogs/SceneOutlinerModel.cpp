//

#include "SceneOutlinerModel.h"

#include <globalincs/linklist.h>
#include <iff_defs/iff_defs.h>
#include <jumpnode/jumpnode.h>
#include <object/object.h>
#include <object/waypoint.h>
#include <prop/prop.h>
#include <ship/ship.h>

namespace fso::fred::dialogs {

SceneOutlinerModel::SceneOutlinerModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	_rebuildTimer = new QTimer(this);
	_rebuildTimer->setSingleShot(true);
	_rebuildTimer->setInterval(150);  // 150ms debounce for structural rebuilds
	connect(_rebuildTimer, &QTimer::timeout, this, &SceneOutlinerModel::onRebuildTimer);

	connect(_editor, &Editor::currentObjectChanged, this, &SceneOutlinerModel::onCurrentObjectChanged);
	connect(_editor, &Editor::objectMarkingChanged, this, &SceneOutlinerModel::onObjectMarkingChanged);
	connect(_editor, &Editor::layerVisibilityChanged, this, &SceneOutlinerModel::onLayerVisibilityChanged);
	connect(_editor, &Editor::layerStructureChanged, this, &SceneOutlinerModel::onLayerStructureChanged);
	connect(_editor, &Editor::missionLoaded, this, [this](const std::string&) { onMissionLoaded(); });
	connect(_editor, &Editor::missionChanged, this, &SceneOutlinerModel::onMissionChanged);
	// Do NOT call buildTree() here — mission data (obj_used_list, Ships, etc.) is not
	// initialized at construction time. The tree is populated when missionLoaded fires.
}

// ---------------------------------------------------------------------------
// Tree building
// ---------------------------------------------------------------------------

void SceneOutlinerModel::buildTree()
{
	_tree.clear();

	const auto layerNames = _viewport->getLayerNames();
	for (const auto& layerName : layerNames) {
		OutlinerLayer layer;
		layer.name = QString::fromStdString(layerName);
		bool layerVisible = true;
		_viewport->getLayerVisibility(layerName, &layerVisible);
		layer.visible = layerVisible;

		// --- Ships (wingless, including player starts) ---
		OutlinerCategory ships;
		ships.name = "Ships";
		for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
			if (ptr->type != OBJ_SHIP && ptr->type != OBJ_START) continue;
			if (Ships[ptr->instance].wingnum != -1) continue;  // in a wing — shown there
			if (_viewport->getObjectLayerName(OBJ_INDEX(ptr)) != layerName) continue;
			int team = Ships[ptr->instance].team;
			if (!_filterIff.isEmpty() && team >= 0 && team < _filterIff.size() && !_filterIff[team]) continue;

			OutlinerObject obj;
			obj.objNum = OBJ_INDEX(ptr);
			obj.isPlayerStart = (ptr->type == OBJ_START);
			obj.displayName = QString::fromUtf8(Ships[ptr->instance].ship_name);
			if (obj.isPlayerStart)
				obj.displayName += " *";
			ships.items.push_back(obj);
		}
		if (!ships.items.isEmpty()) layer.categories.push_back(ships);

		// --- Wings (use layer of first valid member ship) ---
		OutlinerCategory wings;
		wings.name = "Wings";
		for (int wi = 0; wi < MAX_WINGS; wi++) {
			if (!Wings[wi].wave_count) continue;

			// Find this wing's layer from its first valid member
			SCP_string wingLayer = "Default";
			for (int si = 0; si < Wings[wi].current_count; si++) {
				int shipIdx = Wings[wi].ship_index[si];
				if (shipIdx < 0) continue;
				int objNum = Ships[shipIdx].objnum;
				if (objNum >= 0 && Objects[objNum].type != OBJ_NONE) {
					wingLayer = _viewport->getObjectLayerName(objNum);
					break;
				}
			}
			if (wingLayer != layerName) continue;

			OutlinerObject wingObj;
			wingObj.wingIndex = wi;
			wingObj.objNum = -1;
			wingObj.displayName = QString::fromUtf8(Wings[wi].name);

			for (int si = 0; si < Wings[wi].current_count; si++) {
				int shipIdx = Wings[wi].ship_index[si];
				if (shipIdx < 0) continue;
				int objNum = Ships[shipIdx].objnum;
				if (objNum < 0 || Objects[objNum].type == OBJ_NONE) continue;
				int memberTeam = Ships[shipIdx].team;
				if (!_filterIff.isEmpty() && memberTeam >= 0 && memberTeam < _filterIff.size() && !_filterIff[memberTeam]) continue;

				OutlinerObject member;
				member.objNum = objNum;
				member.isPlayerStart = (Objects[objNum].type == OBJ_START);
				member.displayName = QString::fromUtf8(Ships[shipIdx].ship_name);
				if (member.isPlayerStart)
					member.displayName += " *";
				wingObj.children.push_back(member);
			}
			if (wingObj.children.isEmpty()) continue;  // all members filtered out
			wings.items.push_back(wingObj);
		}
		if (!wings.items.isEmpty()) layer.categories.push_back(wings);

		// --- Waypoint paths ---
		OutlinerCategory waypoints;
		waypoints.name = "Waypoints";
		for (int wli = 0; wli < (int)Waypoint_lists.size(); wli++) {
			const auto& wl = Waypoint_lists[wli];
			if (wl.get_fred_layer() != layerName) continue;

			OutlinerObject pathObj;
			pathObj.waypointListIndex = wli;
			pathObj.objNum = -1;
			pathObj.displayName = QString::fromUtf8(wl.get_name());

			for (const auto& wpt : wl.get_waypoints()) {
				OutlinerObject wptObj;
				wptObj.objNum = wpt.get_objnum();
				SCP_string wptName;
				waypoint_stuff_name(wptName, wpt);
				wptObj.displayName = QString::fromStdString(wptName);
				pathObj.children.push_back(wptObj);
			}
			waypoints.items.push_back(pathObj);
		}
		if (!waypoints.items.isEmpty()) layer.categories.push_back(waypoints);

		// --- Jump nodes ---
		OutlinerCategory jumpNodes;
		jumpNodes.name = "Jump Nodes";
		for (auto& jn : Jump_nodes) {
			if (jn.GetFredLayer() != layerName) continue;
			int objNum = jn.GetSCPObjectNumber();
			if (objNum < 0) continue;

			OutlinerObject obj;
			obj.objNum = objNum;
			obj.displayName = QString::fromUtf8(jn.GetName());
			jumpNodes.items.push_back(obj);
		}
		if (!jumpNodes.items.isEmpty()) layer.categories.push_back(jumpNodes);

		// --- Props ---
		OutlinerCategory props;
		props.name = "Props";
		for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
			if (ptr->type != OBJ_PROP) continue;
			if (_viewport->getObjectLayerName(OBJ_INDEX(ptr)) != layerName) continue;
			auto* prop = prop_id_lookup(ptr->instance);
			if (!prop) continue;

			OutlinerObject obj;
			obj.objNum = OBJ_INDEX(ptr);
			obj.displayName = QString::fromUtf8(prop->prop_name);
			props.items.push_back(obj);
		}
		if (!props.items.isEmpty()) layer.categories.push_back(props);

		_tree.push_back(layer);
	}
}

// ---------------------------------------------------------------------------
// Public accessors
// ---------------------------------------------------------------------------

QSet<int> SceneOutlinerModel::getMarkedSet() const
{
	QSet<int> marked;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->flags[Object::Object_Flags::Marked]) {
			marked.insert(OBJ_INDEX(ptr));
		}
	}
	return marked;
}

QVector<QString> SceneOutlinerModel::getLayerNames() const
{
	QVector<QString> names;
	for (const auto& ln : _viewport->getLayerNames())
		names.push_back(QString::fromStdString(ln));
	return names;
}

// ---------------------------------------------------------------------------
// Layer operations
// ---------------------------------------------------------------------------

void SceneOutlinerModel::toggleLayerVisibility(const QString& layerName)
{
	// Store the QByteArray — NOT just constData(), which would dangle after the statement.
	const QByteArray name = layerName.toUtf8();
	bool current = true;
	_viewport->getLayerVisibility(name.constData(), &current);
	_viewport->setLayerVisibility(name.constData(), !current);
	// setLayerVisibility calls editor->notifyLayerVisibilityChanged() → onLayerVisibilityChanged()
}

void SceneOutlinerModel::moveObjectToLayer(int objNum, const QString& layerName)
{
	// Single inline call: temporary QByteArray lives until end of full expression — safe.
	_viewport->moveObjectToLayer(objNum, layerName.toUtf8().constData());
	_editor->autosave("move object to layer");
	buildTree();
	treeStructureChanged();
}

void SceneOutlinerModel::moveWingToLayer(int wingIndex, const QString& layerName)
{
	const QByteArray layer = layerName.toUtf8();
	for (int si = 0; si < Wings[wingIndex].current_count; si++) {
		int shipIdx = Wings[wingIndex].ship_index[si];
		if (shipIdx < 0) continue;
		int objNum = Ships[shipIdx].objnum;
		if (objNum >= 0 && Objects[objNum].type != OBJ_NONE) {
			_viewport->moveObjectToLayer(objNum, layer.constData());
		}
	}
	_editor->autosave("move wing to layer");
	buildTree();
	treeStructureChanged();
}

void SceneOutlinerModel::moveWaypointPathToLayer(int waypointListIndex, const QString& layerName)
{
	auto& wl = Waypoint_lists[waypointListIndex];
	// Moving any one waypoint propagates to the whole path (EditorViewport::setObjectLayerByIndex).
	// Single inline call — temporary QByteArray lives until end of full expression — safe.
	if (!wl.get_waypoints().empty()) {
		_viewport->moveObjectToLayer(wl.get_waypoints().front().get_objnum(), layerName.toUtf8().constData());
	}
	_editor->autosave("move waypoint path to layer");
	buildTree();
	treeStructureChanged();
}

// ---------------------------------------------------------------------------
// Selection
// ---------------------------------------------------------------------------

void SceneOutlinerModel::selectObjectFromOutliner(int objNum)
{
	_updatingFromOutliner = true;
	_editor->selectObject(objNum);
	_updatingFromOutliner = false;
}

void SceneOutlinerModel::multiSelectFromOutliner(const QVector<int>& objNums)
{
	if (objNums.isEmpty()) return;
	_updatingFromOutliner = true;
	_editor->unmark_all();
	for (int id : objNums)
		_editor->markObject(id);
	// selectObject sets the current object index via the public API; markObject above is
	// already a no-op for the last element since it's already marked, so this just sets
	// the "current" and emits missionChanged (which onMissionChanged guards via _updatingFromOutliner).
	_editor->selectObject(objNums.last());
	_updatingFromOutliner = false;
}

void SceneOutlinerModel::selectWingFromOutliner(int wingIndex)
{
	QVector<int> objNums;
	for (int si = 0; si < Wings[wingIndex].current_count; si++) {
		int shipIdx = Wings[wingIndex].ship_index[si];
		if (shipIdx < 0) continue;
		int objNum = Ships[shipIdx].objnum;
		if (objNum >= 0 && Objects[objNum].type != OBJ_NONE)
			objNums.push_back(objNum);
	}
	if (!objNums.isEmpty())
		multiSelectFromOutliner(objNums);
}

// ---------------------------------------------------------------------------
// Filter
// ---------------------------------------------------------------------------

void SceneOutlinerModel::setNameFilter(const QString& filter)
{
	_nameFilter = filter;
	modelChanged();
}

// ---------------------------------------------------------------------------
// Signal handlers
// ---------------------------------------------------------------------------

void SceneOutlinerModel::onCurrentObjectChanged(int /*newObj*/)
{
	if (_updatingFromOutliner) return;
	modelChanged();
}

void SceneOutlinerModel::onObjectMarkingChanged(int /*obj*/, bool /*marked*/)
{
	if (_updatingFromOutliner) return;
	modelChanged();
}

void SceneOutlinerModel::onLayerVisibilityChanged()
{
	// Update layer visibility in existing tree entries without full rebuild
	const auto layerNames = _viewport->getLayerNames();
	for (auto& layer : _tree) {
		bool visible = true;
		_viewport->getLayerVisibility(layer.name.toUtf8().constData(), &visible);
		layer.visible = visible;
	}
	modelChanged();
}

void SceneOutlinerModel::onMissionLoaded()
{
	_filterIff = QVector<bool>(static_cast<int>(Iff_info.size()), true);
	_rebuildTimer->stop();
	buildTree();
	treeStructureChanged();
}

void SceneOutlinerModel::onMissionChanged()
{
	if (_updatingFromOutliner) return;
	// Debounce structural rebuilds — many rapid changes (e.g. selection) trigger
	// missionChanged, but we only need to rebuild the tree occasionally.
	_rebuildTimer->start();
}

void SceneOutlinerModel::onRebuildTimer()
{
	buildTree();
	treeStructureChanged();
}

void SceneOutlinerModel::onLayerStructureChanged()
{
	_rebuildTimer->stop();  // flush any pending debounce — do it immediately
	buildTree();
	treeStructureChanged();
}

// ---------------------------------------------------------------------------
// IFF filtering
// ---------------------------------------------------------------------------

void SceneOutlinerModel::setFilterIff(int team, bool visible)
{
	if (team < 0 || team >= _filterIff.size()) return;
	_filterIff[team] = visible;
	buildTree();
	treeStructureChanged();
}

bool SceneOutlinerModel::getFilterIff(int team) const
{
	if (team < 0 || team >= _filterIff.size()) return true;
	return _filterIff[team];
}

int SceneOutlinerModel::iffCount() const
{
	return static_cast<int>(Iff_info.size());
}

QString SceneOutlinerModel::getIffName(int team) const
{
	if (team < 0 || team >= static_cast<int>(Iff_info.size())) return {};
	return QString::fromUtf8(Iff_info[team].iff_name);
}

// ---------------------------------------------------------------------------
// Bulk selection
// ---------------------------------------------------------------------------

void SceneOutlinerModel::selectAll()
{
	_updatingFromOutliner = true;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START) {
			int team = Ships[ptr->instance].team;
			if (!_filterIff.isEmpty() && team >= 0 && team < _filterIff.size() && !_filterIff[team])
				continue;
		}
		_editor->markObject(OBJ_INDEX(ptr));
	}
	_updatingFromOutliner = false;
	modelChanged();
}

void SceneOutlinerModel::clearSelection()
{
	_updatingFromOutliner = true;
	_editor->unmark_all();
	_updatingFromOutliner = false;
	modelChanged();
}

void SceneOutlinerModel::invertSelection()
{
	_updatingFromOutliner = true;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START) {
			int team = Ships[ptr->instance].team;
			if (!_filterIff.isEmpty() && team >= 0 && team < _filterIff.size() && !_filterIff[team])
				continue;
		}
		int objNum = OBJ_INDEX(ptr);
		if (ptr->flags[Object::Object_Flags::Marked])
			_editor->unmarkObject(objNum);
		else
			_editor->markObject(objNum);
	}
	_updatingFromOutliner = false;
	modelChanged();
}

} // namespace fso::fred::dialogs

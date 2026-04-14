//

#include "SceneBrowserModel.h"

#include <globalincs/linklist.h>
#include <iff_defs/iff_defs.h>
#include <jumpnode/jumpnode.h>
#include <object/object.h>
#include <object/waypoint.h>
#include <prop/prop.h>
#include <ship/ship.h>

namespace fso::fred::dialogs {

SceneBrowserModel::SceneBrowserModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	_rebuildTimer = new QTimer(this);
	_rebuildTimer->setSingleShot(true);
	_rebuildTimer->setInterval(150);  // 150ms debounce for structural rebuilds
	connect(_rebuildTimer, &QTimer::timeout, this, &SceneBrowserModel::onRebuildTimer);

	connect(_editor, &Editor::currentObjectChanged, this, &SceneBrowserModel::onCurrentObjectChanged);
	connect(_editor, &Editor::objectMarkingChanged, this, &SceneBrowserModel::onObjectMarkingChanged);
	connect(_editor, &Editor::layerVisibilityChanged, this, &SceneBrowserModel::onLayerVisibilityChanged);
	connect(_editor, &Editor::layerStructureChanged, this, &SceneBrowserModel::onLayerStructureChanged);
	connect(_editor, &Editor::missionLoaded, this, [this](const std::string&) { onMissionLoaded(); });
	connect(_editor, &Editor::missionChanged, this, &SceneBrowserModel::onMissionChanged);
	// Do NOT call buildTree() here — mission data (obj_used_list, Ships, etc.) is not
	// initialized at construction time. The tree is populated when missionLoaded fires.
}

// ---------------------------------------------------------------------------
// Tree building
// ---------------------------------------------------------------------------

void SceneBrowserModel::buildTree()
{
	_tree.clear();

	const auto layerNames = _viewport->getLayerNames();
	for (const auto& layerName : layerNames) {
		BrowserLayer layer;
		layer.name = QString::fromStdString(layerName);
		bool layerVisible = true;
		_viewport->getLayerVisibility(layerName, &layerVisible);
		layer.visible = layerVisible;

		// --- Ships (wingless, including player starts) ---
		BrowserCategory ships;
		ships.name = "Ships";
		for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
			if (ptr->type != OBJ_SHIP && ptr->type != OBJ_START) continue;
			if (Ships[ptr->instance].wingnum != -1) continue;  // in a wing — shown there
			if (_viewport->getObjectLayerName(OBJ_INDEX(ptr)) != layerName) continue;
			int team = Ships[ptr->instance].team;
			if (!_filterIff.isEmpty() && team >= 0 && team < _filterIff.size() && !_filterIff[team]) continue;

			BrowserObject obj;
			obj.objNum = OBJ_INDEX(ptr);
			obj.isPlayerStart = (ptr->type == OBJ_START);
			obj.displayName = QString::fromUtf8(Ships[ptr->instance].ship_name);
			if (obj.isPlayerStart)
				obj.displayName += " *";
			ships.items.push_back(obj);
		}
		if (!ships.items.isEmpty()) layer.categories.push_back(ships);

		// --- Wings (group members by their own layer; a wing may appear in multiple layers) ---
		BrowserCategory wings;
		wings.name = "Wings";
		for (int wi = 0; wi < MAX_WINGS; wi++) {
			if (!Wings[wi].wave_count) continue;

			BrowserObject wingObj;
			wingObj.wingIndex = wi;
			wingObj.objNum = -1;
			wingObj.displayName = QString::fromUtf8(Wings[wi].name);

			for (int si = 0; si < Wings[wi].wave_count; si++) {
				int shipIdx = Wings[wi].ship_index[si];
				if (shipIdx < 0) continue;
				int objNum = Ships[shipIdx].objnum;
				if (objNum < 0 || Objects[objNum].type == OBJ_NONE) continue;
				if (_viewport->getObjectLayerName(objNum) != layerName) continue;
				int memberTeam = Ships[shipIdx].team;
				if (!_filterIff.isEmpty() && memberTeam >= 0 && memberTeam < _filterIff.size() && !_filterIff[memberTeam]) continue;

				BrowserObject member;
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
		BrowserCategory waypoints;
		waypoints.name = "Waypoints";
		for (int wli = 0; wli < (int)Waypoint_lists.size(); wli++) {
			const auto& wl = Waypoint_lists[wli];
			if (wl.get_fred_layer() != layerName) continue;

			BrowserObject pathObj;
			pathObj.waypointListIndex = wli;
			pathObj.objNum = -1;
			pathObj.displayName = QString::fromUtf8(wl.get_name());

			for (const auto& wpt : wl.get_waypoints()) {
				BrowserObject wptObj;
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
		BrowserCategory jumpNodes;
		jumpNodes.name = "Jump Nodes";
		for (auto& jn : Jump_nodes) {
			if (jn.GetFredLayer() != layerName) continue;
			int objNum = jn.GetSCPObjectNumber();
			if (objNum < 0) continue;

			BrowserObject obj;
			obj.objNum = objNum;
			obj.displayName = QString::fromUtf8(jn.GetName());
			jumpNodes.items.push_back(obj);
		}
		if (!jumpNodes.items.isEmpty()) layer.categories.push_back(jumpNodes);

		// --- Props ---
		BrowserCategory props;
		props.name = "Props";
		for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
			if (ptr->type != OBJ_PROP) continue;
			if (_viewport->getObjectLayerName(OBJ_INDEX(ptr)) != layerName) continue;
			auto* prop = prop_id_lookup(ptr->instance);
			if (!prop) continue;

			BrowserObject obj;
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

QSet<int> SceneBrowserModel::getMarkedSet()
{
	QSet<int> marked;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->flags[Object::Object_Flags::Marked]) {
			marked.insert(OBJ_INDEX(ptr));
		}
	}
	return marked;
}

QVector<QString> SceneBrowserModel::getLayerNames() const
{
	QVector<QString> names;
	for (const auto& ln : _viewport->getLayerNames())
		names.push_back(QString::fromStdString(ln));
	return names;
}

// ---------------------------------------------------------------------------
// Layer operations
// ---------------------------------------------------------------------------

void SceneBrowserModel::toggleLayerVisibility(const QString& layerName)
{
	// Store the QByteArray — NOT just constData(), which would dangle after the statement.
	const QByteArray name = layerName.toUtf8();
	bool current = true;
	_viewport->getLayerVisibility(name.constData(), &current);
	_viewport->setLayerVisibility(name.constData(), !current);
	// setLayerVisibility calls editor->notifyLayerVisibilityChanged() → onLayerVisibilityChanged()
}

void SceneBrowserModel::moveObjectToLayer(int objNum, const QString& layerName)
{
	// Single inline call: temporary QByteArray lives until end of full expression — safe.
	_viewport->moveObjectToLayer(objNum, layerName.toUtf8().constData());
	_editor->autosave("move object to layer");
	buildTree();
	treeStructureChanged();
}

void SceneBrowserModel::moveWingToLayer(int wingIndex, const QString& layerName)
{
	if (wingIndex < 0 || wingIndex >= MAX_WINGS) return;
	const QByteArray layer = layerName.toUtf8();
	for (int si = 0; si < Wings[wingIndex].wave_count; si++) {
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

void SceneBrowserModel::moveWaypointPathToLayer(int waypointListIndex, const QString& layerName)
{
	if (!SCP_vector_inbounds(Waypoint_lists, waypointListIndex)) return;
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

void SceneBrowserModel::selectObjectFromBrowser(int objNum)
{
	_updatingFromBrowser = true;
	_editor->selectObject(objNum);
	_updatingFromBrowser = false;
}

void SceneBrowserModel::multiSelectFromBrowser(const QVector<int>& objNums)
{
	if (objNums.isEmpty()) return;
	_updatingFromBrowser = true;
	_editor->unmark_all();
	for (int id : objNums)
		_editor->markObject(id);
	// selectObject sets the current object index via the public API; markObject above is
	// already a no-op for the last element since it's already marked, so this just sets
	// the "current" and emits missionChanged (which onMissionChanged guards via _updatingFromBrowser).
	_editor->selectObject(objNums.last());
	_updatingFromBrowser = false;
}

void SceneBrowserModel::selectWingFromBrowser(int wingIndex)
{
	const auto objNums = getWingMemberObjects(wingIndex);
	if (!objNums.isEmpty())
		multiSelectFromBrowser(objNums);
}

QVector<int> SceneBrowserModel::getWingMemberObjects(int wingIndex)
{
	QVector<int> objNums;
	if (wingIndex < 0 || wingIndex >= MAX_WINGS) {
		return objNums;
	}

	for (int si = 0; si < Wings[wingIndex].wave_count; si++) {
		int shipIdx = Wings[wingIndex].ship_index[si];
		if (shipIdx < 0) continue;
		int objNum = Ships[shipIdx].objnum;
		if (objNum >= 0 && Objects[objNum].type != OBJ_NONE)
			objNums.push_back(objNum);
	}
	return objNums;
}

// ---------------------------------------------------------------------------
// Filter
// ---------------------------------------------------------------------------

void SceneBrowserModel::setNameFilter(const QString& filter)
{
	_nameFilter = filter;
	modelChanged();
}

// ---------------------------------------------------------------------------
// Signal handlers
// ---------------------------------------------------------------------------

void SceneBrowserModel::onCurrentObjectChanged(int /*newObj*/)
{
	if (_updatingFromBrowser) return;
	modelChanged();
}

void SceneBrowserModel::onObjectMarkingChanged(int /*obj*/, bool /*marked*/)
{
	if (_updatingFromBrowser) return;
	modelChanged();
}

void SceneBrowserModel::onLayerVisibilityChanged()
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

void SceneBrowserModel::onMissionLoaded()
{
	_filterIff = QVector<bool>(static_cast<int>(Iff_info.size()), true);
	_rebuildTimer->stop();
	buildTree();
	treeStructureChanged();
}

void SceneBrowserModel::onMissionChanged()
{
	if (_updatingFromBrowser) return;
	// Debounce structural rebuilds — many rapid changes (e.g. selection) trigger
	// missionChanged, but we only need to rebuild the tree occasionally.
	_rebuildTimer->start();
}

void SceneBrowserModel::onRebuildTimer()
{
	buildTree();
	treeStructureChanged();
}

void SceneBrowserModel::onLayerStructureChanged()
{
	_rebuildTimer->stop();  // flush any pending debounce — do it immediately
	buildTree();
	treeStructureChanged();
}

// ---------------------------------------------------------------------------
// IFF filtering
// ---------------------------------------------------------------------------

void SceneBrowserModel::setFilterIff(int team, bool visible)
{
	if (team < 0 || team >= _filterIff.size()) return;
	_filterIff[team] = visible;
	buildTree();
	treeStructureChanged();
}

bool SceneBrowserModel::getFilterIff(int team) const
{
	if (team < 0 || team >= _filterIff.size()) return true;
	return _filterIff[team];
}

int SceneBrowserModel::iffCount()
{
	return static_cast<int>(Iff_info.size());
}

QString SceneBrowserModel::getIffName(int team)
{
	if (team < 0 || team >= static_cast<int>(Iff_info.size())) return {};
	return QString::fromUtf8(Iff_info[team].iff_name);
}

// ---------------------------------------------------------------------------
// Bulk selection
// ---------------------------------------------------------------------------

void SceneBrowserModel::selectAll()
{
	_updatingFromBrowser = true;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START) {
			int team = Ships[ptr->instance].team;
			if (!_filterIff.isEmpty() && team >= 0 && team < _filterIff.size() && !_filterIff[team])
				continue;
		}
		_editor->markObject(OBJ_INDEX(ptr));
	}
	_updatingFromBrowser = false;
	modelChanged();
}

void SceneBrowserModel::clearSelection()
{
	_updatingFromBrowser = true;
	_editor->unmark_all();
	_updatingFromBrowser = false;
	modelChanged();
}

void SceneBrowserModel::invertSelection()
{
	_updatingFromBrowser = true;
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
	_updatingFromBrowser = false;
	modelChanged();
}

} // namespace fso::fred::dialogs

//

#include "SceneBrowserPanel.h"
#include "ui_SceneBrowserPanel.h"

#include <mission/dialogs/SceneBrowserModel.h>
#include <ui/FredView.h>

#include <QLayout>
#include <QMenu>

namespace fso::fred {

SceneBrowserPanel::SceneBrowserPanel(FredView* fredView, EditorViewport* viewport)
	: QDockWidget(tr("Scene Browser"), fredView)
	, ui(new ::Ui::SceneBrowserPanel())
	, _fredView(fredView)
{
	setObjectName("SceneBrowserPanel");  // Required for saveState/restoreState

	// Model
	_model = new dialogs::SceneBrowserModel(this, viewport);
	connect(_model, &dialogs::SceneBrowserModel::modelChanged,
	        this, &SceneBrowserPanel::onModelChanged);
	connect(_model, &dialogs::SceneBrowserModel::treeStructureChanged,
	        this, &SceneBrowserPanel::onTreeStructureChanged);

	// Set up content widget from UI file
	auto* container = new QWidget(this);
	ui->setupUi(container);
	setWidget(container);

	_searchBar = ui->searchBar;
	_tree = ui->browserTree;
	_iffFilterWidget = ui->iffFilterWidget;
	_selectAllButton = ui->selectAllButton;
	_clearButton = ui->clearButton;
	_invertButton = ui->invertButton;

	// Connections
	connect(_searchBar, &QLineEdit::textChanged, this, &SceneBrowserPanel::onSearchTextChanged);
	connect(_selectAllButton, &QPushButton::clicked, _model, &dialogs::SceneBrowserModel::selectAll);
	connect(_clearButton, &QPushButton::clicked, _model, &dialogs::SceneBrowserModel::clearSelection);
	connect(_invertButton, &QPushButton::clicked, _model, &dialogs::SceneBrowserModel::invertSelection);
	connect(_tree, &QTreeWidget::itemChanged, this, &SceneBrowserPanel::onItemChanged);
	connect(_tree, &QTreeWidget::itemSelectionChanged, this, &SceneBrowserPanel::onItemSelectionChanged);
	connect(_tree, &QTreeWidget::customContextMenuRequested,
	        this, &SceneBrowserPanel::onCustomContextMenuRequested);
	connect(this, &QDockWidget::topLevelChanged, this, &SceneBrowserPanel::updateFloatingMargins);
	updateFloatingMargins(isFloating());
	// Do NOT call rebuildTree() here — mission data is not initialized at construction time.
	// The tree is populated when missionLoaded fires, propagating via treeStructureChanged.
}

SceneBrowserPanel::~SceneBrowserPanel() = default;

// ---------------------------------------------------------------------------
// Tree construction
// ---------------------------------------------------------------------------

void SceneBrowserPanel::updateFloatingMargins(bool floating)
{
	auto* content = widget();
	if (content == nullptr || content->layout() == nullptr) {
		return;
	}

	// Give a little breathing room around the panel contents when floating.
	// Keep docked layout tight.
	content->layout()->setContentsMargins(floating ? 8 : 0, floating ? 8 : 0, floating ? 8 : 0, floating ? 8 : 0);
}

void SceneBrowserPanel::rememberExpansionState()
{
	for (int li = 0; li < _tree->topLevelItemCount(); li++) {
		auto* layerItem = _tree->topLevelItem(li);
		const auto layerName = layerItem->data(0, LayerNameRole).toString();
		if (layerName.isEmpty()) continue;

		_expansionState[QString("L|%1").arg(layerName)] = layerItem->isExpanded();

		for (int ci = 0; ci < layerItem->childCount(); ci++) {
			auto* catItem = layerItem->child(ci);
			const auto catName = catItem->text(0);
			_expansionState[QString("C|%1|%2").arg(layerName, catName)] = catItem->isExpanded();

			for (int oi = 0; oi < catItem->childCount(); oi++) {
				auto* objItem = catItem->child(oi);
				const auto varWing = objItem->data(0, WingIndexRole);
				if (!varWing.isNull()) {
					_expansionState[QString("W|%1|%2").arg(layerName).arg(varWing.toInt())] = objItem->isExpanded();
					continue;
				}

				const auto varPath = objItem->data(0, WptListIndexRole);
				if (!varPath.isNull()) {
					_expansionState[QString("P|%1|%2").arg(layerName).arg(varPath.toInt())] = objItem->isExpanded();
				}
			}
		}
	}
}

bool SceneBrowserPanel::expandedStateOrDefault(const QString& key, bool defaultExpanded) const
{
	const auto it = _expansionState.constFind(key);
	return it == _expansionState.constEnd() ? defaultExpanded : it.value();
}

void SceneBrowserPanel::rebuildTree()
{
	QSignalBlocker treeBlocker(_tree);
	rememberExpansionState();
	_tree->clear();

	const auto& layers = _model->getTree();
	const auto marked = dialogs::SceneBrowserModel::getMarkedSet();

	for (const auto& layer : layers) {
		// Count total objects across all categories
		int totalObjects = 0;
		for (const auto& cat : layer.categories) {
			for (const auto& obj : cat.items) {
				if (obj.wingIndex >= 0) {
					totalObjects += obj.children.size();
				} else if (obj.waypointListIndex >= 0) {
					totalObjects += obj.children.size();
				} else {
					totalObjects += 1;
				}
			}
		}

		auto* layerItem = new QTreeWidgetItem(_tree);
		layerItem->setText(0, QString("%1 (%2)").arg(layer.name).arg(totalObjects));
		layerItem->setData(0, IsLayerItemRole, true);
		layerItem->setData(0, LayerNameRole, layer.name);
		layerItem->setFlags(layerItem->flags() | Qt::ItemIsUserCheckable);
		layerItem->setCheckState(0, layer.visible ? Qt::Checked : Qt::Unchecked);
		const auto layerKey = QString("L|%1").arg(layer.name);
		layerItem->setExpanded(expandedStateOrDefault(layerKey, true));

		for (const auto& cat : layer.categories) {
			auto* catItem = new QTreeWidgetItem(layerItem);
			catItem->setText(0, cat.name);
			// Category rows are not selectable — they're just headers
			catItem->setFlags(Qt::ItemIsEnabled);
			const auto catKey = QString("C|%1|%2").arg(layer.name, cat.name);
			catItem->setExpanded(expandedStateOrDefault(catKey, true));

			for (const auto& obj : cat.items) {
				if (obj.wingIndex >= 0) {
					// Wing header
					auto* wingItem = new QTreeWidgetItem(catItem);
					wingItem->setText(0, obj.displayName);
					wingItem->setData(0, WingIndexRole, obj.wingIndex);
					const auto wingKey = QString("W|%1|%2").arg(layer.name).arg(obj.wingIndex);
					wingItem->setExpanded(expandedStateOrDefault(wingKey, false));

					for (const auto& member : obj.children) {
						auto* memberItem = new QTreeWidgetItem(wingItem);
						memberItem->setText(0, member.displayName);
						memberItem->setData(0, ObjNumRole, member.objNum);
						memberItem->setSelected(marked.contains(member.objNum));
					}
				} else if (obj.waypointListIndex >= 0) {
					// Waypoint path header
					auto* pathItem = new QTreeWidgetItem(catItem);
					pathItem->setText(0, obj.displayName);
					pathItem->setData(0, WptListIndexRole, obj.waypointListIndex);
					const auto pathKey = QString("P|%1|%2").arg(layer.name).arg(obj.waypointListIndex);
					pathItem->setExpanded(expandedStateOrDefault(pathKey, false));

					for (const auto& wpt : obj.children) {
						auto* wptItem = new QTreeWidgetItem(pathItem);
						wptItem->setText(0, wpt.displayName);
						wptItem->setData(0, ObjNumRole, wpt.objNum);
						wptItem->setSelected(marked.contains(wpt.objNum));
					}
				} else {
					// Regular leaf (ship, prop, jump node)
					auto* leafItem = new QTreeWidgetItem(catItem);
					leafItem->setText(0, obj.displayName);
					leafItem->setData(0, ObjNumRole, obj.objNum);
					leafItem->setSelected(marked.contains(obj.objNum));
				}
			}
		}
	}

	applyFilter(_model->getNameFilter());
}

// ---------------------------------------------------------------------------
// Sync (viewport → browser, selection only — no rebuild)
// ---------------------------------------------------------------------------

void SceneBrowserPanel::syncSelection()
{
	QSignalBlocker treeBlocker(_tree);
	const auto marked = dialogs::SceneBrowserModel::getMarkedSet();

	QTreeWidgetItemIterator it(_tree);
	QTreeWidgetItem* firstSelected = nullptr;
	while (*it) {
		auto varObjNum = (*it)->data(0, ObjNumRole);
		if (!varObjNum.isNull()) {
			bool sel = marked.contains(varObjNum.toInt());
			(*it)->setSelected(sel);
			if (sel && !firstSelected)
				firstSelected = *it;
		} else {
			// Wing/path headers: selected if any child is selected
			auto varWing = (*it)->data(0, WingIndexRole);
			if (!varWing.isNull()) {
				bool anyChild = false;
				for (int c = 0; c < (*it)->childCount(); c++) {
					auto cv = (*it)->child(c)->data(0, ObjNumRole);
					if (!cv.isNull() && marked.contains(cv.toInt())) { anyChild = true; break; }
				}
				(*it)->setSelected(anyChild);
			}
			auto varPath = (*it)->data(0, WptListIndexRole);
			if (!varPath.isNull()) {
				bool anyChild = false;
				for (int c = 0; c < (*it)->childCount(); c++) {
					auto cv = (*it)->child(c)->data(0, ObjNumRole);
					if (!cv.isNull() && marked.contains(cv.toInt())) { anyChild = true; break; }
				}
				(*it)->setSelected(anyChild);
			}
		}
		++it;
	}

	if (firstSelected)
		_tree->scrollToItem(firstSelected, QAbstractItemView::EnsureVisible);
}

void SceneBrowserPanel::syncLayerVisibility()
{
	QSignalBlocker treeBlocker(_tree);
	const auto& layers = _model->getTree();

	for (int i = 0; i < _tree->topLevelItemCount(); i++) {
		auto* item = _tree->topLevelItem(i);
		auto layerName = item->data(0, LayerNameRole).toString();
		for (const auto& layer : layers) {
			if (layer.name == layerName) {
				item->setCheckState(0, layer.visible ? Qt::Checked : Qt::Unchecked);
				break;
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Search filter
// ---------------------------------------------------------------------------

void SceneBrowserPanel::applyFilter(const QString& filter)
{
	if (filter.isEmpty()) {
		showAllItems(_tree->invisibleRootItem());
		return;
	}

	// First show all, then hide non-matching leaves
	showAllItems(_tree->invisibleRootItem());

	QTreeWidgetItemIterator it(_tree, QTreeWidgetItemIterator::NoChildren);
	while (*it) {
		auto varObjNum = (*it)->data(0, ObjNumRole);
		if (!varObjNum.isNull()) {
			bool matches = (*it)->text(0).contains(filter, Qt::CaseInsensitive);
			(*it)->setHidden(!matches);
		}
		++it;
	}

	// Hide parent items that have all children hidden
	// Walk bottom-up: leaves are already handled, now handle wings, paths, categories, layers
	for (int li = 0; li < _tree->topLevelItemCount(); li++) {
		auto* layerItem = _tree->topLevelItem(li);
		bool anyLayerVisible = false;
		for (int ci = 0; ci < layerItem->childCount(); ci++) {
			auto* catItem = layerItem->child(ci);
			bool anyCatVisible = false;
			for (int oi = 0; oi < catItem->childCount(); oi++) {
				auto* objItem = catItem->child(oi);
				// Wing or path header: check its children
				bool hasVisibleChild = false;
				for (int mi = 0; mi < objItem->childCount(); mi++) {
					if (!objItem->child(mi)->isHidden()) { hasVisibleChild = true; break; }
				}
				if (objItem->childCount() > 0) {
					objItem->setHidden(!hasVisibleChild);
					if (hasVisibleChild) anyCatVisible = true;
				} else {
					if (!objItem->isHidden()) anyCatVisible = true;
				}
			}
			catItem->setHidden(!anyCatVisible);
			if (anyCatVisible) anyLayerVisible = true;
		}
		// Don't hide layer items — always show them even if empty
		Q_UNUSED(anyLayerVisible)
	}
}

void SceneBrowserPanel::showAllItems(QTreeWidgetItem* root)
{
	for (int i = 0; i < root->childCount(); i++) {
		auto* child = root->child(i);
		child->setHidden(false);
		showAllItems(child);
	}
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------

void SceneBrowserPanel::onModelChanged()
{
	// Selection or layer visibility changed — fast sync, no full rebuild
	syncSelection();
	syncLayerVisibility();
	applyFilter(_model->getNameFilter());
}

void SceneBrowserPanel::onTreeStructureChanged()
{
	// Create IFF checkboxes on first call after missionLoaded, when Iff_info is populated
	if (_iffCheckBoxes.isEmpty() && dialogs::SceneBrowserModel::iffCount() > 0) {
		delete _iffFilterWidget->layout();  // remove any existing layout before setting a new one
		auto* layout = new FlowLayout(_iffFilterWidget, /*hSpacing=*/4, /*vSpacing=*/2);
		for (int i = 0; i < dialogs::SceneBrowserModel::iffCount(); i++) {
			auto* cb = new QCheckBox(dialogs::SceneBrowserModel::getIffName(i), _iffFilterWidget);
			cb->setChecked(true);
			const int team = i;
			connect(cb, &QCheckBox::toggled, this, [this, team](bool checked) {
				_model->setFilterIff(team, checked);
			});
			layout->addWidget(cb);
			_iffCheckBoxes.push_back(cb);
		}
	}

	// Full rebuild needed (objects added/removed/renamed/moved between layers)
	rebuildTree();
}

void SceneBrowserPanel::onItemChanged(QTreeWidgetItem* item, int column)
{
	if (column != 0) return;
	auto varLayer = item->data(0, IsLayerItemRole);
	if (varLayer.isNull() || !varLayer.toBool()) return;

	auto layerName = item->data(0, LayerNameRole).toString();
	_model->toggleLayerVisibility(layerName);
}

void SceneBrowserPanel::onItemSelectionChanged()
{
	if (_model->isUpdatingFromBrowser()) return;

	QVector<int> selectedObjNums;
	QVector<int> selectedWings;

	for (auto* item : _tree->selectedItems()) {
		auto varObjNum = item->data(0, ObjNumRole);
		if (!varObjNum.isNull()) {
			selectedObjNums.push_back(varObjNum.toInt());
		} else {
			// Wing header selected
			auto varWing = item->data(0, WingIndexRole);
			if (!varWing.isNull()) {
				const auto wingIndex = varWing.toInt();
				if (!selectedWings.contains(wingIndex)) {
					selectedWings.push_back(wingIndex);
				}
			}
			// Waypoint path header: select all children
			auto varPath = item->data(0, WptListIndexRole);
			if (!varPath.isNull()) {
				for (int c = 0; c < item->childCount(); c++) {
					auto cv = item->child(c)->data(0, ObjNumRole);
					if (!cv.isNull()) selectedObjNums.push_back(cv.toInt());
				}
			}
		}
	}

	if (!selectedWings.isEmpty()) {
		for (auto wingIndex : selectedWings) {
			const auto wingMembers = dialogs::SceneBrowserModel::getWingMemberObjects(wingIndex);
			selectedObjNums += wingMembers;
		}
		_model->multiSelectFromBrowser(selectedObjNums);
	} else if (!selectedObjNums.isEmpty()) {
		_model->multiSelectFromBrowser(selectedObjNums);
	} else {
		_model->multiSelectFromBrowser({});
	}
}

void SceneBrowserPanel::onCustomContextMenuRequested(const QPoint& pos)
{
	auto* item = _tree->itemAt(pos);
	if (!item) return;

	// Don't show a context menu for layer header items or category items
	auto varLayer = item->data(0, IsLayerItemRole);
	if (!varLayer.isNull()) return;
	if (item->flags() == Qt::ItemIsEnabled) return;  // category item

	const auto globalPos = _tree->viewport()->mapToGlobal(pos);

	auto varObjNum = item->data(0, ObjNumRole);
	auto varWing = item->data(0, WingIndexRole);
	auto varPath = item->data(0, WptListIndexRole);

	if (!varWing.isNull()) {
		_fredView->showWingContextMenu(varWing.toInt(), globalPos);
	} else if (!varPath.isNull()) {
		_fredView->showWaypointPathContextMenu(varPath.toInt(), globalPos);
	} else if (!varObjNum.isNull()) {
		// Regular object: delegate to FredView's context menu (handles Edit + Move to Layer)
		_fredView->showContextMenu(varObjNum.toInt(), globalPos);
	}
}

void SceneBrowserPanel::onSearchTextChanged(const QString& text)
{
	_model->setNameFilter(text);
}

} // namespace fso::fred

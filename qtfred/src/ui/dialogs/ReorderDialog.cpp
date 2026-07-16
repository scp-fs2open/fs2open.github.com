#include "ui/dialogs/ReorderDialog.h"
#include "ui/Theme.h"
#include "ui_ReorderDialog.h"

#include <QListWidget>
#include <QPushButton>
#include <QSignalBlocker>

#include <algorithm>

namespace fso::fred::dialogs {

ReorderDialog::ReorderDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent),
	_viewport(viewport),
	ui(new Ui::ReorderDialog()),
	_model(new ReorderDialogModel(this, viewport))
{
	ui->setupUi(this);

	using Type = ReorderDialogModel::Type;
	_tabs = {
		{Type::Ships, ui->shipList, ui->shipMoveTopBtn, ui->shipMoveUpBtn, ui->shipMoveDownBtn, ui->shipMoveBottomBtn},
		{Type::Wings, ui->wingList, ui->wingMoveTopBtn, ui->wingMoveUpBtn, ui->wingMoveDownBtn, ui->wingMoveBottomBtn},
		{Type::Props, ui->propList, ui->propMoveTopBtn, ui->propMoveUpBtn, ui->propMoveDownBtn, ui->propMoveBottomBtn},
		{Type::WaypointLists, ui->waypointList, ui->waypointMoveTopBtn, ui->waypointMoveUpBtn, ui->waypointMoveDownBtn, ui->waypointMoveBottomBtn},
		{Type::JumpNodes, ui->jumpNodeList, ui->jumpNodeMoveTopBtn, ui->jumpNodeMoveUpBtn, ui->jumpNodeMoveDownBtn, ui->jumpNodeMoveBottomBtn},
	};

	for (const auto& tab : _tabs) {
		setupTab(tab);
	}
}

ReorderDialog::~ReorderDialog() = default;

void ReorderDialog::setupTab(const Tab& tab)
{
	fso::fred::bindCustomIcon(tab.top, CustomIcon::MoveToTop);
	fso::fred::bindStandardIcon(tab.up, QStyle::SP_ArrowUp);
	fso::fred::bindStandardIcon(tab.down, QStyle::SP_ArrowDown);
	fso::fred::bindCustomIcon(tab.bottom, CustomIcon::MoveToBottom);

	connect(tab.top, &QAbstractButton::clicked, this, [this, tab] { move(tab, true, true); });
	connect(tab.up, &QAbstractButton::clicked, this, [this, tab] { move(tab, true, false); });
	connect(tab.down, &QAbstractButton::clicked, this, [this, tab] { move(tab, false, false); });
	connect(tab.bottom, &QAbstractButton::clicked, this, [this, tab] { move(tab, false, true); });
	connect(tab.list, &QListWidget::currentRowChanged, this, [this, tab] { updateButtons(tab); });

	rebuildList(tab);
	// Select the first item so the move buttons start in a sensible state.
	if (tab.list->count() > 0)
		tab.list->setCurrentRow(0);
	updateButtons(tab);
}

void ReorderDialog::rebuildList(const Tab& tab)
{
	QSignalBlocker blocker(tab.list);

	const int prevRow = tab.list->currentRow();

	tab.list->clear();
	for (const auto& name : _model->getItemNames(tab.type)) {
		tab.list->addItem(QString::fromStdString(name));
	}

	// Keep the previous row selected where possible (clamped to the new count).
	const int count = tab.list->count();
	if (count > 0 && prevRow >= 0)
		tab.list->setCurrentRow(std::min(prevRow, count - 1));
}

void ReorderDialog::updateButtons(const Tab& tab)
{
	const int row = tab.list->currentRow();
	const int count = tab.list->count();

	const bool canUp = (row > 0);
	const bool canDown = (row >= 0 && row < count - 1);

	tab.top->setEnabled(canUp);
	tab.up->setEnabled(canUp);
	tab.down->setEnabled(canDown);
	tab.bottom->setEnabled(canDown);
}

void ReorderDialog::move(const Tab& tab, bool up, bool all_the_way)
{
	const int from = tab.list->currentRow();
	const int count = tab.list->count();
	if (from < 0 || from >= count)
		return;

	int to;
	if (up)
		to = all_the_way ? 0 : from - 1;
	else
		to = all_the_way ? count - 1 : from + 1;

	if (to < 0 || to >= count || to == from)
		return;

	_model->moveItem(tab.type, from, to);

	rebuildList(tab);
	tab.list->setCurrentRow(to);
	tab.list->scrollToItem(tab.list->currentItem());
	updateButtons(tab);
}

} // namespace fso::fred::dialogs

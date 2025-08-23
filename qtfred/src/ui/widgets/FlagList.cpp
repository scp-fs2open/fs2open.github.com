#include "ui/widgets/FlagList.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QListView>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QToolButton>
#include <QVBoxLayout>

namespace fso::fred {

FlagListWidget::FlagListWidget(QWidget* parent) : QWidget(parent)
{
	buildUi();
	connectSignals();

	setFilterVisible(true);
	setToolbarVisible(true);
}

FlagListWidget::~FlagListWidget() = default;

void FlagListWidget::buildUi()
{
	auto* outer = new QVBoxLayout(this);
	outer->setContentsMargins(0, 0, 0, 0);
	outer->setSpacing(6);

	// filter row
	auto* filterRow = new QHBoxLayout();
	filterRow->setContentsMargins(0, 0, 0, 0);
	filterRow->setSpacing(6);

	_filter = new QLineEdit(this);
	_filter->setPlaceholderText(tr("Filter flags..."));
	filterRow->addWidget(_filter, /*stretch*/ 1);

	// toolbar
	_btnAll = new QToolButton(this);
	_btnAll->setText(tr("All"));
	_btnAll->setToolTip(tr("Select all"));

	_btnNone = new QToolButton(this);
	_btnNone->setText(tr("None"));
	_btnNone->setToolTip(tr("Clear all"));

	filterRow->addWidget(_btnAll);
	filterRow->addWidget(_btnNone);

	outer->addLayout(filterRow);

	// list view and setup
	_model = new QStandardItemModel(this);

	_proxy = new QSortFilterProxyModel(this);
	_proxy->setSourceModel(_model);
	_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
	_proxy->setFilterRole(Qt::DisplayRole);
	_proxy->setSortRole(Qt::DisplayRole);
	_proxy->setDynamicSortFilter(true);

	_list = new QListView(this);
	_list->setModel(_proxy);
	_list->setUniformItemSizes(true);
	_list->setSelectionMode(QAbstractItemView::NoSelection);
	_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	outer->addWidget(_list, /*stretch*/ 1);

	setLayout(outer);
}

void FlagListWidget::connectSignals()
{
	// React to user toggles
	connect(_model, &QStandardItemModel::itemChanged, this, &FlagListWidget::onItemChanged);
	// Filter field
	connect(_filter, &QLineEdit::textChanged, this, &FlagListWidget::onFilterTextChanged);
	// Toolbar actions
	connect(_btnAll, &QToolButton::clicked, this, &FlagListWidget::onSelectAll);
	connect(_btnNone, &QToolButton::clicked, this, &FlagListWidget::onClearAll);
}

void FlagListWidget::setFlags(const QVector<std::pair<QString, int>>& flags)
{
	rebuildModel(flags);
}

void FlagListWidget::setFlagDescriptions(const QVector<std::pair<QString, QString>>& descriptions)
{
	_descByName.clear();
	_descByName.reserve(descriptions.size());
	for (const auto& p : descriptions) {
		_descByName.insert(p.first, p.second);
	}
	applyTooltipsToItems(); // apply immediately if items already exist
}

void FlagListWidget::rebuildModel(const QVector<std::pair<QString, int>>& flags)
{
	_updating = true;

	_model->clear();
	_model->setColumnCount(1);

	_model->setHorizontalHeaderLabels({tr("Flag")});

	_model->insertRows(0, flags.size());
	for (int i = 0; i < flags.size(); ++i) {
		const auto& name = flags[i].first;
		const auto checked = flags[i].second;

		auto* item = new QStandardItem(name);
		item->setCheckable(true);
		item->setCheckState(Qt::CheckState(checked));
		item->setData(name, KeyRole);

		// If we have a description for this flag, set it as tooltip
		const auto it = _descByName.constFind(name);
		if (it != _descByName.constEnd())
			item->setToolTip(*it);

		_model->setItem(i, 0, item);
	}

	// Reapply filter text so a rebuild respects current filter
	onFilterTextChanged(_filter->text());

	_updating = false;

	Q_EMIT flagsChanged(snapshot());
}

void FlagListWidget::applyTooltipsToItems()
{
	// Apply descriptions to existing items (used when descriptions are set after setFlags)
	for (int r = 0; r < _model->rowCount(); ++r) {
		if (auto* it = _model->item(r, 0)) {
			const auto key = it->data(KeyRole).toString();
			const auto dIt = _descByName.constFind(key);
			it->setToolTip(dIt != _descByName.constEnd() ? *dIt : QString());
		}
	}
}

QVector<std::pair<QString, int>> FlagListWidget::getFlags() const
{
	return snapshot();
}

void FlagListWidget::clear()
{
	_updating = true;
	_model->clear();
	_updating = false;
	Q_EMIT flagsChanged({});
}

void FlagListWidget::setFilterVisible(bool visible)
{
	_filterVisible = visible;
	if (_filter)
		_filter->setVisible(visible);
}

bool FlagListWidget::filterVisible() const
{
	return _filterVisible;
}

void FlagListWidget::setToolbarVisible(bool visible)
{
	_toolbarVisible = visible;
	if (_btnAll)
		_btnAll->setVisible(visible);
	if (_btnNone)
		_btnNone->setVisible(visible);
}

bool FlagListWidget::toolbarVisible() const
{
	return _toolbarVisible;
}

void FlagListWidget::onItemChanged(QStandardItem* item)
{
	if (_updating || !item)
		return;

	const auto name = item->data(KeyRole).toString();
	const auto checked = item->checkState();

	Q_EMIT flagToggled(name, checked);
	Q_EMIT flagsChanged(snapshot());
}

void FlagListWidget::onFilterTextChanged(const QString& text)
{
	// Use a simple contains filter
	QRegularExpression re(QRegularExpression::escape(text), QRegularExpression::CaseInsensitiveOption);
	// Convert to text to emulate substring
	const QString pattern = QStringLiteral(".*%1.*").arg(re.pattern());
	_proxy->setFilterRegularExpression(QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));
}

void FlagListWidget::onSelectAll()
{
	_updating = true;
	for (int r = 0; r < _model->rowCount(); ++r) {
		if (auto* it = _model->item(r, 0)) {
			it->setCheckState(Qt::Checked);
		}
	}
	_updating = false;
	Q_EMIT flagsChanged(snapshot());
}

void FlagListWidget::onClearAll()
{
	_updating = true;
	for (int r = 0; r < _model->rowCount(); ++r) {
		if (auto* it = _model->item(r, 0)) {
			it->setCheckState(Qt::Unchecked);
		}
	}
	_updating = false;
	Q_EMIT flagsChanged(snapshot());
}

QVector<std::pair<QString, int>> FlagListWidget::snapshot() const
{
	QVector<std::pair<QString, int>> out;
	out.reserve(_model->rowCount());
	for (int r = 0; r < _model->rowCount(); ++r) {
		if (auto* it = _model->item(r, 0)) {
			const auto key = it->data(KeyRole).toString();
			const Qt::CheckState checked = it->checkState();
			out.append({key, checked});
		}
	}
	return out;
}

} // namespace fso::fred
#include "SimpleListSelectDialog.h"

#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QRegularExpression>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QVBoxLayout>

SimpleListSelectDialog::SimpleListSelectDialog(const QStringList& items, QWidget* parent) : QDialog(parent)
{
	setWindowTitle(QStringLiteral("Select item"));
	resize(420, 520);

	// --- UI ---
	m_filter = new QLineEdit(this);
	m_filter->setPlaceholderText(QStringLiteral("Type to filter..."));

	m_sourceModel = new QStringListModel(items, this);

	m_proxy = new QSortFilterProxyModel(this);
	m_proxy->setSourceModel(m_sourceModel);
	m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
	m_proxy->setDynamicSortFilter(true);
	m_proxy->setFilterKeyColumn(0);

	m_list = new QListView(this);
	m_list->setModel(m_proxy);
	m_list->setSelectionMode(QAbstractItemView::SingleSelection);
	m_list->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_list->setUniformItemSizes(true);

	m_okBtn = new QPushButton(QStringLiteral("OK"), this);
	m_okBtn->setEnabled(false);
	m_cancelBtn = new QPushButton(QStringLiteral("Cancel"), this);

	auto* btns = new QHBoxLayout();
	btns->addStretch();
	btns->addWidget(m_cancelBtn);
	btns->addWidget(m_okBtn);

	auto* lay = new QVBoxLayout(this);
	lay->setContentsMargins(10, 10, 10, 10);
	lay->addWidget(m_filter);
	lay->addWidget(m_list, 1);
	lay->addLayout(btns);

	// --- Signals ---
	connect(m_filter, &QLineEdit::textChanged, this, &SimpleListSelectDialog::onFilterTextChanged);
	connect(m_filter, &QLineEdit::returnPressed, this, &SimpleListSelectDialog::onFilterReturnPressed);
	auto* sel = m_list->selectionModel();
	connect(sel, &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection&, const QItemSelection&) {
		onSelectionChanged();
	});
	connect(m_list, &QListView::activated, this, &SimpleListSelectDialog::onItemActivated); // double-click/Enter
	connect(m_okBtn, &QPushButton::clicked, this, &QDialog::accept);
	connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void SimpleListSelectDialog::setTitle(const QString& title)
{
	setWindowTitle(title);
}
void SimpleListSelectDialog::setPlaceholder(const QString& text)
{
	m_filter->setPlaceholderText(text);
}

QString SimpleListSelectDialog::selectedText() const
{
	// Be defensive: any of these can be null/invalid if the model/view changed
	if (!m_list || !m_proxy || !m_sourceModel)
		return {};

	auto* sel = m_list->selectionModel();
	if (!sel)
		return {};

	// Prefer currentIndex; fall back to first selected
	QModelIndex proxyIdx = m_list->currentIndex();
	if (!proxyIdx.isValid()) {
		const auto idxs = sel->selectedIndexes();
		if (idxs.isEmpty())
			return {};
		proxyIdx = idxs.first();
	}
	if (!proxyIdx.isValid())
		return {};

	const QModelIndex src = m_proxy->mapToSource(proxyIdx);
	if (!src.isValid())
		return {};

	const QVariant v = m_sourceModel->data(src, Qt::DisplayRole);
	return v.isValid() ? v.toString() : QString{};
}

int SimpleListSelectDialog::selectedRow() const
{
	if (!m_list || !m_proxy || !m_sourceModel)
		return -1;

	auto* sel = m_list->selectionModel();
	if (!sel)
		return -1;

	QModelIndex proxyIdx = m_list->currentIndex();
	if (!proxyIdx.isValid()) {
		const auto idxs = sel->selectedIndexes();
		if (idxs.isEmpty())
			return -1;
		proxyIdx = idxs.first();
	}
	if (!proxyIdx.isValid())
		return -1;

	const QModelIndex src = m_proxy->mapToSource(proxyIdx);
	return src.isValid() ? src.row() : -1;
}

void SimpleListSelectDialog::onFilterTextChanged(const QString& text)
{
	const auto escaped = QRegularExpression::escape(text);
	QRegularExpression rx(QStringLiteral(".*%1.*").arg(escaped), QRegularExpression::CaseInsensitiveOption);
	m_proxy->setFilterRegularExpression(rx);

	// If filtering cleared selection, disable OK
	onSelectionChanged();
}

void SimpleListSelectDialog::onSelectionChanged()
{
	if (!m_list) {
		m_okBtn->setEnabled(false);
		return;
	}
	auto* sel = m_list->selectionModel();
	m_okBtn->setEnabled(sel && sel->hasSelection());
}


void SimpleListSelectDialog::onItemActivated(const QModelIndex& /*proxyIndex*/)
{
	// Double-click/Enter ? accept if something is selected
	if (!m_list->selectionModel()->selectedIndexes().isEmpty()) {
		accept();
	}
}

void SimpleListSelectDialog::onFilterReturnPressed()
{
	// If exactly one row matches, select it and accept
	if (m_proxy->rowCount() == 1) {
		const QModelIndex only = m_proxy->index(0, 0);
		m_list->selectionModel()->select(only, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		accept();
	}
}

int SimpleListSelectDialog::proxyToSourceRow(const QModelIndex& proxyIndex) const
{
	if (!proxyIndex.isValid())
		return -1;
	const QModelIndex src = m_proxy->mapToSource(proxyIndex);
	return src.isValid() ? src.row() : -1;
}

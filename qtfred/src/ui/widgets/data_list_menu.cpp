#include "data_list_menu.h"

#include <QApplication>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QScreen>
#include <QShowEvent>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidgetAction>

namespace fso::fred {
namespace {

constexpr int kIdRole = Qt::UserRole + 1;

void dismissAndActivate(int id, const std::function<void(int)>& onActivate) {
	while (auto* p = QApplication::activePopupWidget()) {
		p->close();
	}
	// Defer the actual edit so it runs after the menu close cascade has finished
	// processing — avoids reentrancy issues if the edit triggers more UI changes.
	QTimer::singleShot(0, [onActivate, id]() { onActivate(id); });
}

class SearchableMenuWidget : public QWidget {
public:
	SearchableMenuWidget(const std::vector<util::SelectMenuEntry>& items,
		std::function<void(int)> onActivate,
		int currentId)
		: QWidget(nullptr)
		, _onActivate(std::move(onActivate))
	{
		auto* layout = new QVBoxLayout(this);
		layout->setContentsMargins(4, 4, 4, 4);
		layout->setSpacing(4);

		_filter = new QLineEdit(this);
		_filter->setPlaceholderText(tr("Type to filter..."));
		_filter->setClearButtonEnabled(true);
		layout->addWidget(_filter);

		_model = new QStandardItemModel(0, 1, this);
		for (const auto& it : items) {
			auto* row = new QStandardItem(it.name);
			row->setData(it.id, kIdRole);
			row->setEditable(false);
			if (currentId >= 0 && it.id == currentId) {
				// Highlight the current object with a bold font
				QFont font = row->font();
				font.setBold(true);
				row->setFont(font);
				_currentSourceRow = _model->rowCount();
			}
			_model->appendRow(row);
		}

		_proxy = new QSortFilterProxyModel(this);
		_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
		_proxy->setSourceModel(_model);

		_list = new QListView(this);
		_list->setModel(_proxy);
		_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
		_list->setUniformItemSizes(true);
		_list->setSelectionMode(QAbstractItemView::SingleSelection);
		_list->setFrameShape(QFrame::NoFrame);

		const QFontMetrics fm(_list->font());
		const int rowHeight = fm.height() + 6;
		int maxTextWidth = 0;
		for (const auto& it : items) {
			maxTextWidth = std::max(maxTextWidth, fm.horizontalAdvance(it.name));
		}
		_list->setFixedWidth(std::clamp(maxTextWidth + 32, 220, 480));
		_list->setMinimumHeight(rowHeight * 13);
		layout->addWidget(_list);

		QObject::connect(_filter, &QLineEdit::textChanged, this, [this](const QString& text) {
			_proxy->setFilterFixedString(text);
			if (_proxy->rowCount() > 0) {
				_list->setCurrentIndex(_proxy->index(0, 0));
			}
		});
		QObject::connect(_filter, &QLineEdit::returnPressed, this, [this]() { activateCurrent(); });
		QObject::connect(_list, &QListView::activated, this, [this](const QModelIndex&) { activateCurrent(); });
		QObject::connect(_list, &QListView::clicked, this, [this](const QModelIndex&) { activateCurrent(); });

		selectDefaultRow();
	}

	QLineEdit* filterEdit() { return _filter; }

protected:
	void showEvent(QShowEvent* event) override {
		QWidget::showEvent(event);
		// Reset from any previous opening, land on the current item, and make
		// sure the filter has keyboard focus rather than the menu itself.
		_filter->clear();
		selectDefaultRow();
		_filter->setFocus();
	}

	void keyPressEvent(QKeyEvent* event) override {
		switch (event->key()) {
		case Qt::Key_Down:
		case Qt::Key_Up:
		case Qt::Key_PageDown:
		case Qt::Key_PageUp:
			QApplication::sendEvent(_list, event);
			return;
		case Qt::Key_Escape:
			while (auto* p = QApplication::activePopupWidget()) {
				p->close();
			}
			return;
		default:
			break;
		}
		QWidget::keyPressEvent(event);
	}

private:
	void selectDefaultRow() {
		if (_currentSourceRow >= 0) {
			const QModelIndex idx = _proxy->mapFromSource(_model->index(_currentSourceRow, 0));
			if (idx.isValid()) {
				_list->setCurrentIndex(idx);
				_list->scrollTo(idx);
				return;
			}
		}
		if (_proxy->rowCount() > 0) {
			_list->setCurrentIndex(_proxy->index(0, 0));
		}
	}

	void activateCurrent() {
		const QModelIndex idx = _list->currentIndex();
		if (!idx.isValid()) {
			return;
		}
		dismissAndActivate(idx.data(kIdRole).toInt(), _onActivate);
	}

	std::function<void(int)> _onActivate;
	QLineEdit* _filter = nullptr;
	QListView* _list = nullptr;
	QStandardItemModel* _model = nullptr;
	QSortFilterProxyModel* _proxy = nullptr;
	int _currentSourceRow = -1;
};

void appendActions(QMenu* menu,
	const std::vector<util::SelectMenuEntry>& items,
	const std::function<void(int)>& onActivate,
	int currentId)
{
	QAction* currentAct = nullptr;
	for (const auto& item : items) {
		const int id = item.id;
		QAction* act = menu->addAction(item.name, menu, [onActivate, id]() { onActivate(id); });
		if (currentId >= 0 && id == currentId) {
			// Highlight the current object with a bold font
			QFont font = act->font();
			font.setBold(true);
			act->setFont(font);
			currentAct = act;
		}
	}
	// Open with the current item pre-highlighted.
	if (currentAct != nullptr) {
		menu->setActiveAction(currentAct);
	}
}

// Resolves Auto to a concrete style: the native column menu while the list fits
// comfortably, then the searchable popup once it would grow past half the screen.
// The menu is not shown yet, so height is estimated from the item count.
DataMenuStyle resolveAutoStyle(const QMenu* menu, int itemCount, int fixedRows) {
	const QFontMetrics fm(menu->font());
	const int rowHeight = fm.height() + 6;
	const int estimatedHeight = (itemCount + fixedRows) * rowHeight;

	const QScreen* screen = menu->screen();
	if (screen == nullptr) {
		screen = QGuiApplication::primaryScreen();
	}
	if (screen == nullptr) {
		return DataMenuStyle::Columns;
	}

	const int available = screen->availableGeometry().height();
	return estimatedHeight > available / 2 ? DataMenuStyle::Searchable
										   : DataMenuStyle::Columns;
}

} // namespace

void populateDataListMenu(QMenu* menu,
	const std::vector<util::SelectMenuEntry>& items,
	DataMenuStyle style,
	std::function<void(int)> onActivate,
	int fixedRows,
	int currentId)
{
	if (!menu || items.empty()) {
		return;
	}

	if (style == DataMenuStyle::Auto) {
		style = resolveAutoStyle(menu, static_cast<int>(items.size()), fixedRows);
	}

	switch (style) {
	case DataMenuStyle::Auto:
	case DataMenuStyle::Columns:
		// Native menu: the platform tiles a too-tall list into columns.
		appendActions(menu, items, onActivate, currentId);
		break;
	case DataMenuStyle::Searchable: {
		auto* widget = new SearchableMenuWidget(items, std::move(onActivate), currentId);
		auto* action = new QWidgetAction(menu);
		action->setDefaultWidget(widget);
		menu->addAction(action);
		// Focus the filter once the menu is fully shown. When we're already
		// inside aboutToShow (the Select menus rebuild there), this connect
		// fires too late for the current opening; the widget's showEvent
		// covers that path.
		QObject::connect(menu, &QMenu::aboutToShow, widget, [widget]() {
			widget->filterEdit()->setFocus();
		});
		break;
	}
	}
}

} // namespace fso::fred

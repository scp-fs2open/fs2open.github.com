#include "sexp_data_menu.h"

#include <QApplication>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QScreen>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidgetAction>

namespace fso::fred {
namespace {

constexpr int kDataIdxRole = Qt::UserRole + 1;

void dismissAndActivate(int dataIdx, const std::function<void(int)>& onActivate) {
	while (auto* p = QApplication::activePopupWidget()) {
		p->close();
	}
	// Defer the actual edit so it runs after the menu close cascade has finished
	// processing — avoids reentrancy issues if the edit triggers more UI changes.
	QTimer::singleShot(0, [onActivate, dataIdx]() { onActivate(dataIdx); });
}

class SearchableMenuWidget : public QWidget {
public:
	SearchableMenuWidget(const std::vector<SexpDataMenuItem>& items,
		std::function<void(int)> onActivate)
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
			auto* row = new QStandardItem(it.text);
			row->setData(it.dataIdx, kDataIdxRole);
			row->setEditable(false);
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
			maxTextWidth = std::max(maxTextWidth, fm.horizontalAdvance(it.text));
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

		if (_proxy->rowCount() > 0) {
			_list->setCurrentIndex(_proxy->index(0, 0));
		}
	}

	QLineEdit* filterEdit() { return _filter; }

protected:
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
	void activateCurrent() {
		const QModelIndex idx = _list->currentIndex();
		if (!idx.isValid()) {
			return;
		}
		dismissAndActivate(idx.data(kDataIdxRole).toInt(), _onActivate);
	}

	std::function<void(int)> _onActivate;
	QLineEdit* _filter = nullptr;
	QListView* _list = nullptr;
	QStandardItemModel* _model = nullptr;
	QSortFilterProxyModel* _proxy = nullptr;
};

void appendActions(QMenu* menu,
	const std::vector<SexpDataMenuItem>& items,
	const std::function<void(int)>& onActivate)
{
	for (const auto& item : items) {
		const int idx = item.dataIdx;
		menu->addAction(item.text, menu, [onActivate, idx]() { onActivate(idx); });
	}
}

// Resolves Auto to a concrete style: the native column menu while the list fits
// comfortably, then the searchable popup once it would grow past half the screen.
// The menu is not shown yet, so height is estimated from the item count.
SexpDataMenuStyle resolveAutoStyle(const QMenu* menu, int itemCount) {
	// Extra rows for the Number / String / separator entries already in the menu.
	constexpr int fixedRows = 3;
	const QFontMetrics fm(menu->font());
	const int rowHeight = fm.height() + 6;
	const int estimatedHeight = (itemCount + fixedRows) * rowHeight;

	const QScreen* screen = menu->screen();
	if (screen == nullptr) {
		screen = QGuiApplication::primaryScreen();
	}
	if (screen == nullptr) {
		return SexpDataMenuStyle::Columns;
	}

	const int available = screen->availableGeometry().height();
	return estimatedHeight > available / 2 ? SexpDataMenuStyle::Searchable
										   : SexpDataMenuStyle::Columns;
}

} // namespace

void populateSexpDataSubmenu(QMenu* menu,
	const std::vector<SexpDataMenuItem>& items,
	SexpDataMenuStyle style,
	std::function<void(int)> onActivate)
{
	if (!menu || items.empty()) {
		return;
	}

	if (style == SexpDataMenuStyle::Auto) {
		style = resolveAutoStyle(menu, static_cast<int>(items.size()));
	}

	switch (style) {
	case SexpDataMenuStyle::Auto:
	case SexpDataMenuStyle::Columns:
		// Native menu: the platform tiles a too-tall list into columns.
		appendActions(menu, items, onActivate);
		break;
	case SexpDataMenuStyle::Searchable: {
		auto* widget = new SearchableMenuWidget(items, std::move(onActivate));
		auto* action = new QWidgetAction(menu);
		action->setDefaultWidget(widget);
		menu->addAction(action);
		// Focus the filter once the menu is fully shown.
		QObject::connect(menu, &QMenu::aboutToShow, widget, [widget]() {
			widget->filterEdit()->setFocus();
		});
		break;
	}
	}
}

} // namespace fso::fred

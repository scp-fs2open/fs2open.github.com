#include "LineEditDelegate.h"

#include <QApplication>
#include <QIntValidator>
#include <QLineEdit>
#include <QStyle>
#include <limits>

// A custom role to store our boolean flag
const int IsStringRole = Qt::UserRole + 1;
const int MaxLength = Qt::UserRole + 2;

namespace fso::fred::dialogs {

// A custom QIntValidator that clamps out-of-bounds values to the nearest valid limit
class ClampingIntValidator : public QIntValidator {
  public:
	ClampingIntValidator(int bottom, int top, QObject* parent) : QIntValidator(bottom, top, parent) {}

	void fixup(QString& input) const override
	{
		qlonglong val = input.toLongLong();
		if (val > top()) {
			input = QString::number(top());
		} else if (val < bottom()) {
			input = QString::number(bottom());
		}
	}
};

LineEditDelegate::LineEditDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

QWidget*
LineEditDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const
{
	auto* editor = new QLineEdit(parent);

	QVariant isStringData = index.data(IsStringRole);
	bool is_string = isStringData.isValid() ? isStringData.toBool() : true; // Default to string type

	bool ok;
	int max_length = index.data(MaxLength).toInt(&ok);
	if (!ok) {
		max_length = 16777215; // Default if MaxLength role is not defined is Qt's int max default
	}

	if (is_string) {
		editor->setMaxLength(max_length);
	} else {
		auto* validator = new ClampingIntValidator(std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), editor);
		editor->setValidator(validator);
	}

	return editor;
}

void LineEditDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyleOptionViewItem opt = option;
	initStyleOption(&opt, index); // fill opt from the index

	if (opt.state & QStyle::State_Selected) {
		QVariant bgData = index.data(Qt::BackgroundRole);
		if (bgData.isValid()) {
			// Determine blue vs orange from the row's background tint, then apply a
			// proper saturated selection color for that type (theme-aware).
			const QColor type_color = bgData.value<QBrush>().color();
			const bool is_blue = type_color.blue() > type_color.red();
			const bool dark_mode = opt.palette.color(QPalette::Window).lightness() < 128;
			QColor sel;
			if (is_blue) {
				sel = dark_mode ? QColor(55, 95, 155) : QColor(70, 130, 200);
			} else {
				sel = dark_mode ? QColor(155, 90, 30) : QColor(200, 120, 40);
			}
			opt.palette.setColor(QPalette::Highlight, sel);
		}
	}

	// Draw with our modified option directly bypassing QStyledItemDelegate::paint() which
	// would call initStyleOption() again internally and overwrite the palette changes above.
	const QWidget* widget = option.widget;
	QStyle* style = widget ? widget->style() : QApplication::style();
	style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
}

} // namespace fso::fred::dialogs
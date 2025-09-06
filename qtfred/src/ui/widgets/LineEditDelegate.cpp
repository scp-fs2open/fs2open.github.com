#include "LineEditDelegate.h"

#include <QIntValidator>
#include <QLineEdit>
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

} // namespace fso::fred::dialogs
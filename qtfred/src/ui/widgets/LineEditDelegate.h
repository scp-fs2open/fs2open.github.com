#pragma once

#include <QStyledItemDelegate>

namespace fso::fred::dialogs {

class LineEditDelegate : public QStyledItemDelegate {
	Q_OBJECT
  public:
	explicit LineEditDelegate(QObject* parent = nullptr);

	QWidget*
	createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const override;
};

} // namespace fso::fred::dialogs
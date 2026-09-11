//

#include "FlowLayout.h"

#include <QWidget>

namespace fso::fred {

FlowLayout::FlowLayout(QWidget* parent, int hSpacing, int vSpacing)
	: QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing) {}

FlowLayout::~FlowLayout()
{
	QLayoutItem* item;
	while ((item = takeAt(0)))
		delete item;
}

void FlowLayout::addItem(QLayoutItem* item)
{
	m_items.append(item);
}

QLayoutItem* FlowLayout::itemAt(int index) const
{
	return m_items.value(index);
}

QLayoutItem* FlowLayout::takeAt(int index)
{
	if (index >= 0 && index < m_items.size())
		return m_items.takeAt(index);
	return nullptr;
}

int FlowLayout::heightForWidth(int width) const
{
	return doLayout(QRect(0, 0, width, 0), true);
}

void FlowLayout::setGeometry(const QRect& rect)
{
	QLayout::setGeometry(rect);
	doLayout(rect, false);
}

QSize FlowLayout::sizeHint() const
{
	return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
	QSize size;
	for (const QLayoutItem* item : m_items)
		size = size.expandedTo(item->minimumSize());
	const QMargins m = contentsMargins();
	return size + QSize(m.left() + m.right(), m.top() + m.bottom());
}

int FlowLayout::doLayout(const QRect& rect, bool testOnly) const
{
	const QMargins m = contentsMargins();
	const QRect effective = rect.adjusted(m.left(), m.top(), -m.right(), -m.bottom());
	int x = effective.x();
	int y = effective.y();
	int lineHeight = 0;

	for (QLayoutItem* item : m_items) {
		const QSize sz = item->sizeHint();
		const int nextX = x + sz.width() + m_hSpace;
		if (nextX - m_hSpace > effective.right() && lineHeight > 0) {
			x = effective.x();
			y += lineHeight + m_vSpace;
			lineHeight = 0;
		}
		if (!testOnly)
			item->setGeometry(QRect(QPoint(x, y), sz));
		x += sz.width() + m_hSpace;
		lineHeight = qMax(lineHeight, sz.height());
	}
	return y + lineHeight - rect.y() + m.bottom();
}

} // namespace fso::fred

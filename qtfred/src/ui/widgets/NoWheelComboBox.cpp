#include "NoWheelComboBox.h"

#include <QAbstractItemView>
#include <QWheelEvent>

namespace fso::fred {

NoWheelComboBox::NoWheelComboBox(QWidget* parent) : QComboBox(parent)
{
	setFocusPolicy(Qt::ClickFocus); // don’t grab focus on scroll
	setSizeAdjustPolicy(QComboBox::AdjustToContents);
}

// Override the wheelEvent to ignore mouse scrolling when the popup is not open
void NoWheelComboBox::wheelEvent(QWheelEvent* e)
{
	// If the popup is open, let normal wheel navigation work.
	if (view() && view()->isVisible()) {
		QComboBox::wheelEvent(e);
		return;
	}
	// Otherwise, ignore so the table can scroll instead of changing selection.
	e->ignore();
}

} // namespace fso::fred

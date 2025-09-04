#include "NoWheelComboBox.h"

#include <QAbstractItemView>
#include <QWheelEvent>

namespace fso::fred {

NoWheelComboBox::NoWheelComboBox(QWidget* parent) : QComboBox(parent)
{
	
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

void NoWheelComboBox::mousePressEvent(QMouseEvent* event)
{
	// Allow the combobox to process the event first, which lets it gain focus.
	QComboBox::mousePressEvent(event);

	// Now, accept the event to stop it from propagating to the parent
	event->accept();
}

} // namespace fso::fred

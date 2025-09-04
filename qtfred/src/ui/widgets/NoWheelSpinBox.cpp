#include "NoWheelSpinBox.h"

#include <QWheelEvent>

namespace fso::fred {

NoWheelSpinBox::NoWheelSpinBox(QWidget* parent) : QSpinBox(parent)
{
	
}

// Override the wheelEvent to ignore mouse scrolling
void NoWheelSpinBox::wheelEvent(QWheelEvent* event)
{
	// By calling ignore(), we prevent the spin box from handling the event
	// and allow it to propagate to the parent widget (the table view).
	event->ignore();
}

void NoWheelSpinBox::mousePressEvent(QMouseEvent* event)
{
	// Allow the spinbox to process the event first, which lets it gain focus.
	QSpinBox::mousePressEvent(event);

	// Now, accept the event to stop it from propagating to the parent
	event->accept();
}

} // namespace fso::fred

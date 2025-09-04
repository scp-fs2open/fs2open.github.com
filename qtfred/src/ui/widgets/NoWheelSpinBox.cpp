#include "NoWheelSpinBox.h"

#include <QWheelEvent>

namespace fso::fred {

NoWheelSpinBox::NoWheelSpinBox(QWidget* parent) : QSpinBox(parent)
{
	// You can set other properties here if needed
	setFocusPolicy(Qt::StrongFocus);
}

// Override the wheelEvent to ignore mouse scrolling
void NoWheelSpinBox::wheelEvent(QWheelEvent* event)
{
	// By calling ignore(), we prevent the spin box from handling the event
	// and allow it to propagate to the parent widget (the table view).
	event->ignore();
}

} // namespace fso::fred

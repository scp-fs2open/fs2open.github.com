#include "IntegerSnapDoubleSpinBox.h"

#include <QtCore/qglobal.h>
#include <cmath>

#include "math/floating.h"

namespace fso::fred
{

IntegerSnapDoubleSpinBox::IntegerSnapDoubleSpinBox(QWidget* parent) : QDoubleSpinBox(parent)
{}

void IntegerSnapDoubleSpinBox::stepBy(int steps)
{
	if (steps == 0)
		return;
	double old_val = value();

	double new_val;
	if (steps < 0)
	{
		new_val = std::floor(old_val);
		if (fl_equal(new_val, old_val))
			new_val -= singleStep();

		//go min->max
		if (new_val < minimum())
		{
			if (wrapping())
				new_val = std::floor(maximum());
			else
				new_val = std::ceil(minimum());
		}

		++steps;
	}
	else
	{
		new_val = std::ceil(old_val);
		if (fl_equal(new_val, old_val))
			new_val += singleStep();

		//go max->min
		if (new_val > maximum())
		{
			if (wrapping())
				new_val = std::ceil(minimum());
			else
				new_val = std::floor(maximum());
		}

		--steps;
	}
	setValue(new_val);

	// we might have more steps
	if (steps != 0)
		QDoubleSpinBox::stepBy(steps);
}

} // namespace fso::fred


#include "MixedValueSpinBox.h"

#include <algorithm>
#include <limits>

namespace fso::fred {

MixedValueSpinBox::MixedValueSpinBox(QWidget* parent) : QSpinBox(parent) {}

void MixedValueSpinBox::setMixed()
{
	if (!_mixed) {
		_realMinimum = minimum();
		_mixed = true;
	}
	setSpecialValueText(QStringLiteral("-"));
	setMinimum(std::numeric_limits<int>::min());
	setValue(minimum());
}

void MixedValueSpinBox::setSingleValue(int value)
{
	leaveMixedState();
	setValue(value);
}

bool MixedValueSpinBox::isBlank() const
{
	return _mixed && value() == minimum();
}

void MixedValueSpinBox::leaveMixedState()
{
	if (!_mixed)
		return;

	_mixed = false;
	setSpecialValueText(QString());
	// Qt moves a value below the new minimum up to the minimum.
	setMinimum(_realMinimum);
}

void MixedValueSpinBox::stepBy(int steps)
{
	// From "-", the first step shows the lowest real value.
	const bool wasBlank = isBlank();
	leaveMixedState();
	if (!wasBlank)
		QSpinBox::stepBy(steps);
}

int MixedValueSpinBox::valueFromText(const QString& text) const
{
	// Enter on an untouched "-" field keeps the mixed state. The clamp below must not make it a real value.
	if (_mixed && text == specialValueText())
		return minimum();

	const int value = QSpinBox::valueFromText(text);
	// In the mixed state, the minimum is below the real range. A typed value must stay in the real range.
	return _mixed ? std::clamp(value, _realMinimum, maximum()) : value;
}

} // namespace fso::fred

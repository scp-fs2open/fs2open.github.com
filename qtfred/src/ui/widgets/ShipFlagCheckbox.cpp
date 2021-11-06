//
//

#include "ShipFlagCheckbox.h"

namespace fso {
namespace fred {
ShipFlagCheckbox::ShipFlagCheckbox(QWidget* parent) : QCheckBox(parent) {}
void ShipFlagCheckbox::nextCheckState()
{
	if (checkState() == Qt::Checked) {
		// return Qt::Unchecked;
		setCheckState(Qt::Unchecked);
	} else {
		setCheckState(Qt::Checked);
	}

} // namespace fred
} // namespace fred
} // namespace fso
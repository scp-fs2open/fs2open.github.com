#pragma once
#include <QDoubleSpinBox>

namespace fso::fred
{

class IntegerSnapDoubleSpinBox : public QDoubleSpinBox
{
	Q_OBJECT

  public:
	explicit IntegerSnapDoubleSpinBox(QWidget* parent = nullptr);

  protected:
	void stepBy(int steps) override;
};

} // namespace fso::fred

#pragma once
#include <QSpinBox>

namespace fso::fred {

class NoWheelSpinBox : public QSpinBox {
	Q_OBJECT

  public:
	explicit NoWheelSpinBox(QWidget* parent = nullptr);

  protected:
	void wheelEvent(QWheelEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
};

} // namespace fso::fred

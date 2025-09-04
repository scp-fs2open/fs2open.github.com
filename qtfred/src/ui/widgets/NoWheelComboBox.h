#pragma once
#include <QComboBox>

namespace fso::fred {

class NoWheelComboBox : public QComboBox {
	Q_OBJECT
  public:
	explicit NoWheelComboBox(QWidget* parent = nullptr);

  protected:
	void wheelEvent(QWheelEvent* e) override;
	void mousePressEvent(QMouseEvent* event) override;
};

} // namespace fso::fred

#pragma once

#include <QSpinBox>

namespace fso::fred {

/**
 * @brief A spin box that edits one field of several objects at the same time.
 *
 * When the objects have different values, setMixed() shows "-". A spin box shows its special value text only
 * at its minimum, so the mixed state moves the minimum below each real value. Thus each value that the user
 * enters, also the real minimum, is a change and gives valueChanged(). The values that the user enters always
 * stay in the real range.
 */
class MixedValueSpinBox : public QSpinBox {
	Q_OBJECT

  public:
	explicit MixedValueSpinBox(QWidget* parent = nullptr);

	// Shows "-" until the user enters a value.
	void setMixed();

	// Shows one value for all objects. This ends the mixed state.
	void setSingleValue(int value);

  protected:
	void stepBy(int steps) override;
	int valueFromText(const QString& text) const override;

  private:
	void leaveMixedState();

	bool _mixed = false;
	int _realMinimum = 0;
};

} // namespace fso::fred

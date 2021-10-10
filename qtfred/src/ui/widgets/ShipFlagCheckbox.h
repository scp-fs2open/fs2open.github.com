#include <QtWidgets/QCheckbox>
#include <QtGui/QStandardItemModel>

namespace fso {
namespace fred {
class ShipFlagCheckbox : public QCheckBox {
	Q_OBJECT
  public:
	ShipFlagCheckbox(QWidget* parent);

  protected:
	void nextCheckState() override;

};
}
} // namespace fso
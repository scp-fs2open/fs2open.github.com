#pragma once

#include <QtWidgets/QComboBox>
#include <QtGui/QStandardItemModel>
#include <FredApplication.h>
namespace fso::fred {
class PersonaColorComboBox : public QComboBox {
	Q_OBJECT
  public:
	PersonaColorComboBox(QWidget* parent);

  private:
	static QStandardItemModel* getPersonaModel();
};
} // namespace fso::fred
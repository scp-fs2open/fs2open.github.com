#ifndef SHIPWEAPONSDIALOG_H
#define SHIPWEAPONSDIALOG_H

#include "ui/dialogs/ShipEditor/BankModel.h"
#include "ui/widgets/weaponList.h"

#include <mission/dialogs/ShipEditor/ShipWeaponsDialogModel.h>
#include <ui/FredView.h>

#include <QtCore/QItemSelection>
#include <QtWidgets/QDialog>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class ShipWeaponsDialog;
}
/**
 * @brief QTFred's Weapons Editor
 */
class ShipWeaponsDialog : public QDialog {
	Q_OBJECT

  public:
	/**
	 * @brief QTFred's Weapons Editor Constructer.
	 * @param [in/out]	parent		The dialogs parent.
	 * @param [in/out]	viewport	Editor viewport.
	 * @param [in]		isMultiEdit If editing multiple ships.
	 */
	explicit ShipWeaponsDialog(QDialog* parent, EditorViewport* viewport, bool isMultiEdit);
	~ShipWeaponsDialog() override;

  protected:
	void closeEvent(QCloseEvent*) override;

  private slots:
	void on_AIButton_clicked();
	void on_setAllButton_clicked();
	void on_TBLButton_clicked();

  private:
	std::unique_ptr<Ui::ShipWeaponsDialog> ui;
	std::unique_ptr<ShipWeaponsDialogModel> _model;
	/**
	 * @brief Changes current weapon type.
	 * @param [in]	enabled		Always True
	 * @param [in]	mode	The mode to change to. 0 = Primary, 1 = Secondary
	 */
	void modeChanged(const bool enabled, const int mode);
	EditorViewport* _viewport;
	void updateUI();
	BankTreeModel* bankModel;
	int dialogMode;
	WeaponModel* weapons;
	int m_currentAI = 0;
	void aiClassChanged(const int index);
};
} // namespace dialogs
} // namespace fred
} // namespace fso
#endif
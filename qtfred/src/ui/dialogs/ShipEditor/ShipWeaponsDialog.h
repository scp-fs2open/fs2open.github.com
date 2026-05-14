#pragma once

#include "ui/widgets/bankTree.h"

#include <mission/dialogs/ShipEditor/ShipWeaponsDialogModel.h>
#include <QStandardItemModel>

#include <QtCore/QItemSelection>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QListView>
#include <QtWidgets/QPushButton>

namespace fso::fred::dialogs {

namespace Ui {
class ShipWeaponsDialog;
}

class ShipWeaponsDialog : public QDialog {
	Q_OBJECT

  public:
	explicit ShipWeaponsDialog(QDialog* parent, EditorViewport* viewport, bool isMultiEdit);
	~ShipWeaponsDialog() override;

	void accept() override;
	void reject() override;

  protected:
	void closeEvent(QCloseEvent*) override;

  private slots:
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

  private: // NOLINT(readability-redundant-access-specifiers)
	enum Mode { Primary = 0, Secondary = 1, Tertiary = 2 };

	struct TabState {
		Mode mode = Primary;
		bankTree* tree = nullptr;
		QListView* list = nullptr;
		QPushButton* setAllButton = nullptr;
		QPushButton* tblButton = nullptr;
		QPushButton* aiButton = nullptr;
		QComboBox* aiCombo = nullptr;
		QWidget* aiGroup = nullptr;
		QStandardItemModel* bankModel = nullptr;
		QStandardItemModel* weapons = nullptr;
		// Set while the dialog itself is writing into bankModel, so itemChanged handlers can ignore the resulting signals.
		bool internalUpdate = false;
	};

	void initTab(TabState& tab, Mode mode);
	void loadBankModel(TabState& tab);
	void loadWeaponList(TabState& tab);
	void updateTabUI(TabState& tab);
	void updateUI();

	void onSetAllClicked(TabState& tab);
	void onAiButtonClicked(TabState& tab);
	void onTblButtonClicked(TabState& tab);
	void onBankItemChanged(TabState& tab, QStandardItem* item);
	void onWeaponDroppedFromList(TabState& tab, const QModelIndex& target, int weaponId);
	void onWeaponMoved(TabState& tab, const QModelIndex& target, int weaponId, int sourceBanksId,
		int sourceBankId, bool isCopy);

	static QModelIndex indexForBank(const TabState& tab, int banksId, int bankId);

	Bank* bankForIndex(const TabState& tab, const QModelIndex& idx) const;
	Banks* banksForIndex(const TabState& tab, const QModelIndex& idx) const;
	void refreshBankItem(TabState& tab, const QModelIndex& idx);
	SCP_vector<Banks*> banksForMode(Mode mode) const;
	SCP_string banksLabel(const Banks* banks) const;

	std::unique_ptr<Ui::ShipWeaponsDialog> ui;
	std::unique_ptr<ShipWeaponsDialogModel> _model;
	EditorViewport* _viewport;

	TabState _primary;
	TabState _secondary;
};
} // namespace fso::fred::dialogs

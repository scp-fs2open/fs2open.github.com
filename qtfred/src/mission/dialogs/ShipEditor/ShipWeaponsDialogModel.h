#pragma once

#include "../AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {
namespace WeaponsDialog {
struct Bank {
  public:
	Bank(const int weaponId, const int bankId, const int ammoMax, const int ammo);

	int getWeaponId() const;
	int getAmmo() const;

	void setWeapon(const int id);
	void setAmmo(const int ammo);

  private:
	int weaponId;
	int bankId;
	int ammo;
	int ammoMax;
};
struct Banks {
	Banks(const SCP_string &name, int aiIndex, ship_subsys* subsys = nullptr);

	SCP_string name;
	ship_subsys* subsys;
	int aiClass;

	void add(Bank*);
	Bank* getByBankId(const int id);
	SCP_vector<Bank*> banks;
};
class ShipWeaponsDialogModel : public AbstractDialogModel {
  public:
	ShipWeaponsDialogModel(QObject* parent, EditorViewport* viewport, bool multi);

	void initPrimary(const int inst, bool first);

	void initSecondary(int inst, bool first);

	// void initTertiary(int inst, bool first);

	bool apply() override;
	void reject() override;
	SCP_vector<Banks*> getPrimaryBanks() const;
	SCP_vector<Banks*> getSecondaryBanks() const;
	// SCP_vector<Banks*> getTertiaryBanks() const;

  private:
	void initializeData(bool multi);
	int m_isMultiEdit;
	int m_ship;
	int big = 1;
	SCP_vector<Banks*> PrimaryBanks;
	SCP_vector<Banks*> SecondaryBanks;
	// SCP_vector<Banks*> TertiaryBanks;
};
} // namespace WeaponsDialog
} // namespace dialogs
} // namespace fred
} // namespace fso
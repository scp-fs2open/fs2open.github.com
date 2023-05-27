#pragma once

#include "AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {
struct Bank {
  public:
	Bank(const int weaponId, int bankId, int ammo);

	int getWeaponId() const;
	int getAmmo() const;

	void setWeapon(const int id);
	void setAmmo(const int ammo);

  private:
	int weaponId;
	int bankId;
	int ammo;
};
struct Banks {
	Banks(SCP_string name, ship_subsys* subsys = nullptr);

	SCP_string name;
	ship_subsys* subsys;
	int aiClass;

	void add(Bank*);
	Bank* getByBankId(int id);
	SCP_vector<Bank*> banks;

};
class ShipWeaponsDialogModel : public AbstractDialogModel {
  public:
	ShipWeaponsDialogModel(QObject* parent, EditorViewport* viewport, bool multi);
	void initializeData(bool multi);

	void initPrimary(const int inst, bool first);

	void initSecondary(int inst, bool first);

	void initTertiary(int inst, bool first);

	bool apply() override;
	void reject() override;
	SCP_vector<Banks*> getPrimaryBanks() const;
	SCP_vector<Banks*> getSecondaryBanks() const;
	SCP_vector<Banks*> getTertiaryBanks() const;

  private:
	int m_multi;
	int m_ship;
	int big = 1;
	SCP_vector<Banks*> PrimaryBanks;
	SCP_vector<Banks*> SecondaryBanks;
	SCP_vector<Banks*> TertiaryBanks;

};
} // namespace dialogs
} // namespace fred
} // namespace fso
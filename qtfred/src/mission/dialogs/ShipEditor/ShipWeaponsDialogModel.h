#pragma once

#include "../AbstractDialogModel.h"

#include <weapon/weapon.h>

namespace fso::fred {
struct Bank;
struct Banks {
	Banks(SCP_string name, int aiIndex, int ship, int multiedit, ship_subsys* subsys = nullptr);

  public:
	void add(Bank*);
	Bank* getByBankId(const int id);
	SCP_string getName() const;
	int getShip() const;
	ship_subsys* getSubsys() const;
	bool empty() const;
	SCP_vector<Bank*> getBanks() const;
	int getAiClass() const;
	void setAiClass(int);
	bool m_isMultiEdit;
	int getInitalAI() const;

  private:
	SCP_string name;
	ship_subsys* subsys;
	int aiClass;
	int initalAI;
	SCP_vector<Bank*> banks;
	int ship;
};
struct Bank {
  public:
	Bank(const int weaponId, const int bankId, const int ammoMax, const int ammo, Banks* parent);

	int getWeaponId() const;
	int getAmmo() const;
	int getBankId() const;
	int getMaxAmmo() const;

	void setWeapon(const int id);
	void setAmmo(const int ammo);

  private:
	int weaponId;
	int bankId;
	int ammo;
	int ammoMax;
	Banks* parent;
};
namespace dialogs {
/**
 * @brief QTFred's Weapons Editor Model
 */
class ShipWeaponsDialogModel : public AbstractDialogModel {
  public:
	/**
	 * @brief QTFred's Weapons Editor Model Constructer.
	 * @param [in/out]	parent		The dialogs parent.
	 * @param [in/out]	viewport	Editor viewport.
	 * @param [in]		multi If editing multiple ships.
	 */
	ShipWeaponsDialogModel(QObject* parent, EditorViewport* viewport, bool multi);

	// void initTertiary(int inst, bool first);

	bool apply() override;
	void reject() override;
	SCP_vector<Banks*> getPrimaryBanks() const;
	SCP_vector<Banks*> getSecondaryBanks() const;
	// SCP_vector<Banks*> getTertiaryBanks() const;

  private:
	void saveShip(int inst);
	void initPrimary(const int inst, bool first);

	void initSecondary(int inst, bool first);
	void initializeData(bool multi);
	bool m_isMultiEdit;
	int m_ship;
	bool big = true;
	SCP_vector<Banks*> PrimaryBanks;
	SCP_vector<Banks*> SecondaryBanks;
	// SCP_vector<Banks*> TertiaryBanks;
};
} // namespace dialogs
} // namespace fso::fred
#pragma once

#include "../AbstractDialogModel.h"

#include <weapon/weapon.h>

namespace fso::fred {
struct WeaponItem {
	int id;
	SCP_string name;
	bool allowed;
};

enum class WeaponListType { Primary, Secondary /*, Tertiary */ };

// Bank/Banks are buffered representations of a ship's weapon banks. Mutations through
// their setters (Bank::setWeapon, Bank::setAmmo, Banks::setAiClass) update only this
// in-memory state. They do not mark the model dirty or emit modelChanged(). Callers
// must call ShipWeaponsDialogModel::notifyChanged() after a batch of edits.
struct Bank;
struct Banks {
	Banks(SCP_string name, int aiIndex, int ship, int _id, ship_subsys* subsys = nullptr);
	~Banks();

  public:
	int getId() const;
	void add(std::unique_ptr<Bank>);
	SCP_string getName() const;
	int getShip() const;
	ship_subsys* getSubsys() const;
	bool empty() const;
	SCP_vector<Bank*> getBanks() const;
	// Returns the cached AI class for this bank-set; -1 if multi-edit and ships disagree.
	int getAiClass() const;
	void setAiClass(int);
	// Called per-additional-ship during multi-edit init: marks currentAi mixed (-1) if otherAi
	// differs from the cached value. Does not mark the bank dirty.
	void reconcileAiClass(int otherAi);
	bool isAiClassDirty() const;

  private:
	SCP_string name;
	ship_subsys* subsys;
	int currentAi;
	bool aiClassDirty = false;
	SCP_vector<std::unique_ptr<Bank>> banks;
	int ship;
	int id;
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
class ShipWeaponsDialogModel : public AbstractDialogModel {
  public:
	ShipWeaponsDialogModel(QObject* parent, EditorViewport* viewport, bool multi);
	~ShipWeaponsDialogModel() override;

	// True iff all currently-marked ships share the same ship_info_index. Used to gate multi-edit.
	static bool selectedShipsShareClass();

	bool apply() override;
	void reject() override;
	SCP_vector<Banks*> getPrimaryBanks() const;
	SCP_vector<Banks*> getSecondaryBanks() const;
	SCP_vector<WeaponItem> getAvailableWeapons(WeaponListType type) const;
	// "None" for -1, "CONFLICT" for -2, otherwise the weapon table name.
	static SCP_string getWeaponName(int weaponId);
	static SCP_vector<SCP_string> getAiClassNames();
	static SCP_string getAiClassName(int aiClass);
	int getShipClass() const;
	bool isBigShip() const;
	void notifyChanged();

  private:
	void saveShip(int inst);
	void initPrimary(int inst, bool first);
	void initSecondary(int inst, bool first);
	void initializeData(bool isMultiEdit);
	bool m_isMultiEdit;
	int m_ship;
	bool big = true;
	SCP_vector<std::unique_ptr<Banks>> PrimaryBanks;
	SCP_vector<std::unique_ptr<Banks>> SecondaryBanks;
};
} // namespace dialogs
} // namespace fso::fred

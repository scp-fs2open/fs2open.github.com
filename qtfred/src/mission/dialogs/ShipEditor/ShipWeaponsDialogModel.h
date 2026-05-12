#pragma once

#include "../AbstractDialogModel.h"

#include <weapon/weapon.h>

#include <QAbstractListModel>
#include <QMimeData>

namespace fso::fred {
struct WeaponItem {
	WeaponItem(int id, QString name, bool allowed);
	const QString name;
	const int id;
	const bool allowed;
};
class WeaponModel : public QAbstractListModel {
	Q_OBJECT
  public:
	WeaponModel(int type, int shipClass, bool bigShip);
	~WeaponModel() override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	QStringList mimeTypes() const override;
	QMimeData* mimeData(const QModelIndexList& indexes) const override;
	QVector<WeaponItem*> weapons;
};

struct Bank;
struct Banks {
	Banks(SCP_string name, int aiIndex, int ship, int multiedit, int _id, ship_subsys* subsys = nullptr);

  public:
	int getId() const;
	void add(Bank*);
	SCP_string getName() const;
	int getShip() const;
	ship_subsys* getSubsys() const;
	bool empty() const;
	SCP_vector<Bank*> getBanks() const;
	// Returns the single consistent AI class for this bank-set; -1 if multi-edit and ships disagree.
	int getAiClass() const;
	void setAiClass(int);
	bool isAiClassDirty() const;
	bool m_isMultiEdit;
	int getInitialAI() const;

  private:
	SCP_string name;
	ship_subsys* subsys;
	int aiClass;
	int initialAI;
	bool aiClassDirty = false;
	SCP_vector<Bank*> banks;
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

	// True iff all currently-marked ships share the same ship_info_index. Used to gate multi-edit.
	static bool selectedShipsShareClass();

	bool apply() override;
	void reject() override;
	SCP_vector<Banks*> getPrimaryBanks() const;
	SCP_vector<Banks*> getSecondaryBanks() const;
	int getShipClass() const;
	bool isBigShip() const;
	void notifyChanged();

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
};
} // namespace dialogs
} // namespace fso::fred
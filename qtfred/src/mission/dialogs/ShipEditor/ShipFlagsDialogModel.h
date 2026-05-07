#pragma once

#include "../AbstractDialogModel.h"

#include <mission/management.h>

namespace fso::fred::dialogs {

class ShipFlagsDialogModel : public AbstractDialogModel {
	Q_OBJECT
  public:
	ShipFlagsDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	const SCP_vector<std::pair<SCP_string, int>>& getFlagsList();
	static SCP_vector<std::pair<SCP_string, SCP_string>> getShipFlagDescriptions();
	std::pair<SCP_string, int>* getFlag(const SCP_string& flagName);
	void setFlag(const SCP_string& flagName, int state);

	void setDestroyTime(int destroyTime);
	int getDestroyTime() const;
	void setEscortPriority(int priority);
	int getEscortPriority() const;
	void setKamikazeDamage(int damage);
	int getKamikazeDamage() const;

  private: // NOLINT(readability-redundant-access-specifiers)
	void initializeData();
	static int tristateSet(int val, int curState);
	void updateShip(int shipNum);

	SCP_vector<std::pair<SCP_string, int>> _flags;
	int _destroyTime;
	int _escortPriority;
	int _kamikazeDamage;
};
} // namespace fso::fred::dialogs

#pragma once

#include "../AbstractDialogModel.h"

#include <mission/management.h>

namespace fso::fred::dialogs {

class ShipFlagsDialogModel : public AbstractDialogModel {
  private:
	static int tristate_set(const int val, const int cur_state);
	void update_ship(const int);
	SCP_vector<std::pair<SCP_string, int>> flags;
	int destroytime;
	int escortp;
	int kamikazed;

  public:
	ShipFlagsDialogModel(QObject* parent, EditorViewport* viewport);
	void initializeData();

	bool apply() override;
	void reject() override;

	const SCP_vector<std::pair<SCP_string, int>>& getFlagsList();
	std::pair<SCP_string, int>* getFlag(const SCP_string& flag_name);
	void setFlag(const SCP_string& flag_name, int);

	void setDestroyTime(int);
	int getDestroyTime() const;
	void setEscortPriority(int);
	int getEscortPriority() const;
	void setKamikazeDamage(int);
	int getKamikazeDamage() const;
};
} // namespace fso::fred::dialogs
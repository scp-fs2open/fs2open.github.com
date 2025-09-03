#pragma once

#include "iff_defs/iff_defs.h"
#include "mission/dialogs/AbstractDialogModel.h"

namespace fso::fred::dialogs {

class ShieldSystemDialogModel: public AbstractDialogModel {
 Q_OBJECT

 public:
	ShieldSystemDialogModel(QObject* parent, EditorViewport* viewport);
	~ShieldSystemDialogModel() override = default;

	bool apply() override;
	void reject() override;

	int getCurrentTeam() const;
	int getCurrentShipType() const;
	void setCurrentTeam(int team);
	void setCurrentShipType(int type);

	GlobalShieldStatus getCurrentTeamShieldSys() const;
	GlobalShieldStatus getCurrentTypeShieldSys() const;
	void setCurrentTeamShieldSys(bool value);
	void setCurrentTypeShieldSys(bool value);

	const SCP_vector<SCP_string>& getShipTypeOptions() const;
	const SCP_vector<SCP_string>& getTeamOptions() const;

	bool query_modified() const;
 private:
	void initializeData();


	SCP_vector<SCP_string> _shipTypeOptions;
	SCP_vector<SCP_string> _teamOptions;
	SCP_vector<GlobalShieldStatus> _teams;
	SCP_vector<GlobalShieldStatus> _types;
	int	_currTeam;
	int	_currType;
};

} // namespace fso::fred::dialogs

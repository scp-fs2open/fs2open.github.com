#pragma once

#include "iff_defs/iff_defs.h"
#include "mission/dialogs/AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

class ShieldSystemDialogModel: public AbstractDialogModel {
 Q_OBJECT

 public:
	ShieldSystemDialogModel(QObject* parent, EditorViewport* viewport);
	~ShieldSystemDialogModel() override = default;

	bool apply() override;
	void reject() override;

	int getCurrentTeam() const { return _currTeam; }
	int getCurrentShipType() const { return _currType; }
	void setCurrentTeam(int team) { Assert(team >= 0 && team < Iff_info.size());  modify<int>(_currTeam, team); }
	void setCurrentShipType(int type) { Assert(type >= 0 && type < MAX_SHIP_CLASSES); modify<int>(_currType, type); }

	int getCurrentTeamShieldSys() const { return _teams[_currTeam]; }
	int getCurrentTypeShieldSys() const { return _types[_currType]; }
	void setCurrentTeamShieldSys(const int value) { Assert(value == 0 || value == 1); modify<int>(_teams[_currTeam], value); }
	void setCurrentTypeShieldSys(const int value) { Assert(value == 0 || value == 1); modify<int>(_types[_currType], value); }

	const std::vector<SCP_string>& getShipTypeOptions() const { return _shipTypeOptions; }
	const std::vector<SCP_string>& getTeamOptions() const { return _teamOptions; }

	bool query_modified() const;
 private:
	void initializeData();

	template<typename T>
	void modify(T &a, const T &b);

	std::vector<SCP_string> _shipTypeOptions;
	std::vector<SCP_string> _teamOptions;
	std::vector<int> _teams;
	std::vector<int> _types;
	int		_currTeam;
	int		_currType;
};

template<typename T>
inline void ShieldSystemDialogModel::modify(T &a, const T &b) {
	if (a != b) {
		a = b;
		modelChanged();
	}
}

}
}
}

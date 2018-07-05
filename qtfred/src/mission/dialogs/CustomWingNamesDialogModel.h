#pragma once

#include "AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

class CustomWingNamesDialogModel : public AbstractDialogModel {
public:
	CustomWingNamesDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	void setStartingWing(SCP_string, int);
	void setSquadronWing(SCP_string, int);
	void setTvTWing(SCP_string, int);
	SCP_string getStartingWing(int);
	SCP_string getSquadronWing(int);
	SCP_string getTvTWing(int);

	bool query_modified();
private:
	void initializeData();

	template<typename T>
	void modify(T &a, T &b);

	SCP_string _m_starting[3];
	SCP_string _m_squadron[5];
	SCP_string _m_tvt[2];
};

template<typename T>
inline void CustomWingNamesDialogModel::modify(T & a, T & b) {
	if (a != b) {
		a = b;
		modelChanged();
	}
}

}
}
}

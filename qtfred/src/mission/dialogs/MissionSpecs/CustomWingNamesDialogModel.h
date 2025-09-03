#pragma once

#include "../AbstractDialogModel.h"

namespace fso::fred::dialogs {

class CustomWingNamesDialogModel : public AbstractDialogModel {
public:
	CustomWingNamesDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	void setInitialStartingWings(const std::array<SCP_string, MAX_STARTING_WINGS>& startingWings);
	void setInitialSquadronWings(const std::array<SCP_string, MAX_SQUADRON_WINGS>& squadronWings);
	void setInitialTvTWings(const std::array<SCP_string, MAX_TVT_WINGS>& tvtWings);

	const std::array<SCP_string, MAX_STARTING_WINGS>& getStartingWings() const;
	const std::array<SCP_string, MAX_SQUADRON_WINGS>& getSquadronWings() const;
	const std::array<SCP_string, MAX_TVT_WINGS>& getTvTWings() const;

	void setStartingWing(const SCP_string&, int);
	void setSquadronWing(const SCP_string&, int);
	void setTvTWing(const SCP_string&, int);
	const SCP_string& getStartingWing(int);
	const SCP_string& getSquadronWing(int);
	const SCP_string& getTvTWing(int);
private:

	std::array<SCP_string, MAX_STARTING_WINGS> _m_starting;
	std::array<SCP_string, MAX_SQUADRON_WINGS> _m_squadron;
	std::array<SCP_string, MAX_TVT_WINGS> _m_tvt;
};

} // namespace fso::fred::dialogs

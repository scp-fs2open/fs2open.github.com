#pragma once

#include "mission/dialogs/AbstractDialogModel.h"
#include "mission/missionparse.h"

namespace fso::fred::dialogs {

class PropEditorDialogModel : public AbstractDialogModel {
	Q_OBJECT

 public:
	PropEditorDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	bool hasValidSelection() const;
	bool hasMultipleSelection() const;
	bool hasAnyPropsInMission() const;
	const SCP_string& getPropName() const;
	void setPropName(const SCP_string& name);

	const SCP_vector<std::pair<SCP_string, size_t>>& getFlagLabels() const;
	const SCP_vector<int>& getFlagState() const;
	void setFlagState(size_t index, int state);

	void selectNextProp();
	void selectPreviousProp();

 signals:
	void modelDataChanged();

 private slots:
	void onSelectedObjectChanged(int);
	void onSelectedObjectMarkingChanged(int, bool);
	void onMissionChanged();

 private: // NOLINT(readability-redundant-access-specifiers)
	void initializeData();
	bool validateData();
	void showErrorDialogNoCancel(const SCP_string& message);
	void selectPropFromObjectList(object* start, bool forward);
	void selectFirstPropInMission();
	SCP_vector<int> getSelectedPropObjects() const;
	bool getFlagValueForObject(const object& obj, size_t flag_index) const;
	int tristate_set(bool value, int current_state) const;

	SCP_string _propName;
	SCP_vector<std::pair<SCP_string, size_t>> _flagLabels;
	SCP_vector<int> _flagState;
	SCP_vector<int> _selectedPropObjects;
	bool _bypass_errors = false;
};

}

#pragma once
#include "AbstractDialogModel.h"

namespace fso::fred::dialogs {

class JumpNodeEditorDialogModel : public AbstractDialogModel {
	Q_OBJECT
public:
	explicit JumpNodeEditorDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	bool hasValidSelection() const;
	bool hasMultipleSelection() const;
	static bool hasAnyNodesInMission();
	int getSelectionCount() const;
	const SCP_vector<int>& getSelectedObjNums() const { return _selectedJumpNodes; }

	bool setName(const SCP_string& v);
	const SCP_string& getName() const;
	bool setDisplayName(const SCP_string& v); // "<none>" means use Name
	const SCP_string& getDisplayName() const;
	bool setModelFilename(const SCP_string& v);
	const SCP_string& getModelFilename() const;

	void setColorR(int v); // pass -1 for no-op (mixed sentinel)
	int getColorR() const;
	void setColorG(int v);
	int getColorG() const;
	void setColorB(int v);
	int getColorB() const;
	void setColorA(int v);
	int getColorA() const;
	bool isColorRMixed() const;
	bool isColorGMixed() const;
	bool isColorBMixed() const;
	bool isColorAMixed() const;
	bool hasAnyColorMixed() const;

	void setHidden(bool v);
	bool getHidden() const;
	int getHiddenState() const; // returns Qt::CheckState as int (PartiallyChecked when mixed)

	SCP_string getLayer() const;
	void setLayer(const SCP_string& v);

	void selectNextNode();
	void selectPreviousNode();

signals:
	void jumpNodeMarkingChanged();

private slots:
	void onSelectedObjectChanged(int);
	void onSelectedObjectMarkingChanged(int, bool);
	void onMissionChanged();

private: // NOLINT(readability-redundant-access-specifiers)
	void initializeData();
	void showErrorDialogNoCancel(const SCP_string& message);
	bool validateName(const SCP_string& name);
	void selectNodeFromObjectList(object* start, bool forward);
	void selectFirstNodeInMission();

	SCP_vector<int> _selectedJumpNodes; // objnums of selected jump nodes

	SCP_string _name;
	SCP_string _display;
	SCP_string _modelFilename;
	int _red = 0, _green = 0, _blue = 0, _alpha = 0;
	bool _redMixed = false, _greenMixed = false, _blueMixed = false, _alphaMixed = false;
	bool _hidden = false;
	bool _hiddenMixed = false;

	bool _bypass_errors = false;
};

} // namespace fso::fred::dialogs

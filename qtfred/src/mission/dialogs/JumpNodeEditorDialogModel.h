#pragma once
#include "AbstractDialogModel.h"

namespace fso::fred::dialogs {

class JumpNodeEditorDialogModel : public AbstractDialogModel {
	Q_OBJECT
  public:
	explicit JumpNodeEditorDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	const SCP_vector<std::pair<SCP_string, int>>& getJumpNodeList() const;
	void selectJumpNodeByListIndex(int idx);
	int getCurrentJumpNodeIndex() const;
	bool hasValidSelection() const;

	void setName(const SCP_string& v);
	const SCP_string& getName() const;
	void setDisplayName(const SCP_string& v); // "<none>" means use Name
	const SCP_string& getDisplayName() const;
	void setModelFilename(const SCP_string& v);
	const SCP_string& getModelFilename() const;

	void setColorR(int v);
	int getColorR() const;
	void setColorG(int v);
	int getColorG() const;
	void setColorB(int v);
	int getColorB() const;
	void setColorA(int v);
	int getColorA() const;

	void setHidden(bool v);
	bool getHidden() const;

  signals:
	void jumpNodeMarkingChanged();

  private slots:
	void onSelectedObjectChanged(int);
	void onSelectedObjectMarkingChanged(int, bool);
	void onMissionChanged();

  private: // NOLINT(readability-redundant-access-specifiers)
	void initializeData();
	void buildNodeList();
	bool validateData();
	void showErrorDialogNoCancel(const SCP_string& message);
	int getSelectedJumpNodeObjnum(int idx) const;

	int _currentlySelectedNodeIndex = -1;

	SCP_string _name;
	SCP_string _display;
	SCP_string _modelFilename;
	int _red = 0, _green = 0, _blue = 0, _alpha = 0;
	bool _hidden = false;

	SCP_vector<std::pair<SCP_string, int>> _nodes;

	bool _bypass_errors = false;
};

} // namespace fso::fred::dialogs

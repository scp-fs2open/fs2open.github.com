#pragma once

#include "AbstractDialogModel.h"

#include <mission/missionparse.h>

#include "ui/widgets/sexp_tree.h"

namespace fso::fred::dialogs {

class MissionCutscenesDialogModel: public AbstractDialogModel {
 public:
	MissionCutscenesDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	mission_cutscene& getCurrentCutscene();

	bool isCurrentCutsceneValid() const;

	void setCurrentCutscene(int index);
	int getSelectedCutsceneType() const;

	void initializeData();

	SCP_vector<mission_cutscene>& getCutscenes();

	bool isCutsceneVisible(const mission_cutscene& goal) const;

	void setCutsceneType(int type);

	int getCutsceneType() const;

	void deleteCutscene(int formula);

	void changeFormula(int old_form, int new_form);

	mission_cutscene& createNewCutscene();

	bool query_modified();

	void setCurrentCutsceneType(int type);
	void setCurrentCutsceneFilename(const char* filename);

	// TODO HACK: This does not belong here since it is a UI specific control. Once the model based SEXP tree is implemented
	// this should be replaced
	void setTreeControl(sexp_tree* tree);
 private:
	int cur_cutscene = -1;
	SCP_vector<int> m_sig;
	SCP_vector<mission_cutscene> m_cutscenes;
	bool modified = false;

	int m_display_cutscene_types = 0;

	sexp_tree* _sexp_tree = nullptr;
};

} // namespace fso::fred::dialogs

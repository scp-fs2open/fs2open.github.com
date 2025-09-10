#pragma once

#include "globalincs/pstypes.h"

#include "AbstractDialogModel.h"

#include "mission/missionbriefcommon.h"

namespace fso::fred::dialogs {

class BriefingEditorDialogModel : public AbstractDialogModel {
  public:
	BriefingEditorDialogModel(QObject* parent, EditorViewport* viewport);

	enum class DrawLinesState {
		None,    // no lines between any selected pairs
		Partial, // some pairs connected, some not
		All      // every pair connected
	};

	bool apply() override;
	void reject() override;

	int getCurrentTeam() const;
	void setCurrentTeam(int teamIn);
	int getCurrentStage() const;
	int getTotalStages();

	SCP_string getStageText();
	void setStageText(const SCP_string& text);
	SCP_string getSpeechFilename();
	void setSpeechFilename(const SCP_string& speechFilename);
	int getFormula() const;
	void setFormula(int formula);

	void gotoPreviousStage();
	void gotoNextStage();
	void addStage();
	void insertStage();
	void deleteStage();

	void saveStageView(const vec3d& pos, const matrix& orient);
	std::pair<vec3d, matrix> getStageView() const;
	void copyStageViewToClipboard();
	void pasteClipboardViewToStage();

	void testSpeech();
	void copyToOtherTeams();
	const SCP_vector<std::pair<SCP_string, int>>& getTeamList();
	static bool getMissionIsMultiTeam();

	int getCameraTransitionTime() const;
	void setCameraTransitionTime(int ms);

	vec3d getCameraPosition() const;
	matrix getCameraOrientation() const;
	void setCameraPosition(const vec3d& pos);
	void setCameraOrientation(const matrix& orient);

	bool getCutToNext() const;
	void setCutToNext(bool enabled);
	bool getCutFromPrev() const;
	void setCutFromPrev(bool enabled);
	bool getDisableGrid() const;
	void setDisableGrid(bool disabled);	

	int getCurrentIconIndex() const;
	void setCurrentIconIndex(int idx);
	vec3d getIconPosition() const;
	void setIconPosition(const vec3d& pos);
	int getIconId() const;
	void setIconId(int id);
	SCP_string getIconLabel() const;
	void setIconLabel(const SCP_string& text);
	SCP_string getIconCloseupLabel() const;
	void setIconCloseupLabel(const SCP_string& text);

	int getIconTypeIndex() const;
	void setIconTypeIndex(int idx);
	int getIconShipTypeIndex() const;
	void setIconShipTypeIndex(int idx);
	int getIconTeamIndex() const;
	void setIconTeamIndex(int idx);
	float getIconScaleFactor() const;
	void setIconScaleFactor(float factor);

	void setLineSelection(const SCP_vector<int>& indices);
	void clearLineSelection();
	DrawLinesState getDrawLinesState() const;
	void applyDrawLines(bool checked);

	bool getChangeLocally() const;
	void setChangeLocally(bool enabled);

	bool getIconHighlighted() const;
	void setIconHighlighted(bool enabled);
	bool getIconFlipped() const;
	void setIconFlipped(bool enabled);
	bool getIconUseWing() const;
	void setIconUseWing(bool enabled);
	bool getIconUseCargo() const;
	void setIconUseCargo(bool enabled);

	void makeIcon(const SCP_string& label, int typeIndex, int teamIndex, int shipClassIndex);
	void deleteCurrentIcon();
	void propagateCurrentIconForward();

	int getBriefingMusicIndex() const;
	void setBriefingMusicIndex(int idx);
	SCP_string getSubstituteBriefingMusicName() const;
	void setSubstituteBriefingMusicName(const SCP_string& name);

	static SCP_vector<SCP_string> getMusicList();
	static SCP_vector<std::pair<int, SCP_string>> getIconList();
	static SCP_vector<std::pair<int, SCP_string>> getShipList();
	static SCP_vector<std::pair<int, SCP_string>> getIffList();

  private:
	void initializeData();
	void stopSpeech();
	void initializeTeamList();
	static bool valid_icon_index(const brief_stage& s, int idx);
	static bool same_line_unordered(int a0, int a1, int b0, int b1);
	void applyToIconCurrentAndForward(const std::function<void(brief_icon&)>& mutator);

	briefing _wipBriefings[MAX_TVT_TEAMS];
	int _briefingMusicIndex;
	SCP_string _subBriefingMusic;

	int _currentTeam;
	int _currentStage;
	int _currentIcon;
	int _waveId;
	SCP_vector<std::pair<SCP_string, int>> _teamList;

	bool _viewClipboardSet = false;
	vec3d _viewClipboardPos = vmd_zero_vector;
	matrix _viewClipboardOri = vmd_identity_matrix;

	SCP_vector<int> _lineSelection;
	bool _changeLocally = false;
};

} // namespace fso::fred::dialogs

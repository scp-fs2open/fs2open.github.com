#include "AbstractDialogModel.h"
#include "globalincs/pstypes.h"
#include "missionui/missioncmdbrief.h"


namespace fso {
namespace fred {
namespace dialogs {


class CommandBriefingDialogModel: public AbstractDialogModel {
 public:

	CommandBriefingDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	bool briefingUpdateRequired();
	bool stageNumberUpdateRequired();
	bool soundTestUpdateRequired();

	SCP_string getBriefingText();
	SCP_string getAnimationFilename();
	SCP_string getSpeechFilename();
	ubyte getCurrentTeam();
	SCP_string getLowResolutionFilename();
	SCP_string getHighResolutionFilename();
	int getTotalStages();
	int getCurrentStage();
	int getSpeechInstanceNumber();

	void setBriefingText(const SCP_string& briefingText);
	void setAnimationFilename(const SCP_string& animationFilename);
	void setSpeechFilename(const SCP_string& speechFilename);
	void setCurrentTeam(const ubyte& teamIn); // not yet fully supported
	void setLowResolutionFilename(const SCP_string& lowResolutionFilename);
	void setHighResolutionFilename(const SCP_string& highResolutionFilename);

	// work-around function to keep Command Brief Dialog from crashing unexpected on init
	void requestInitialUpdate();

	void testSpeech();
	void stopSpeech();

	void gotoPreviousStage();
	void gotoNextStage();
	void addStage();
	void insertStage();
	void deleteStage();

	void update_init();

 private:
	void initializeData();
	void setWaveID();

	int _currentTeam;
	int _currentStage;
	int _currentlyPlayingSound;

	bool _briefingTextUpdateRequired;
	bool _stageNumberUpdateRequired;
	bool _soundTestUpdateRequired;

	cmd_brief _wipCommandBrief;
};


}
}
}

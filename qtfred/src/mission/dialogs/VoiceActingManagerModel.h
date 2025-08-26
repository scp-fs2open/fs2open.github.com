#pragma once
#include "AbstractDialogModel.h"

#include "mission/missionmessage.h"

namespace fso::fred::dialogs {

	enum class Suffix {
		WAV,
		OGG,

		numSuffixes
	};

	enum class ExportSelection {
		Everything,
		CommandBriefings,
		Briefings,
		Debriefings,
		Messages
	};

	struct AnyWingmanCheckResult {
		bool anyWingmanFound = false;
		int issueCount = 0;
		SCP_string report; // empty = all good
	};

class VoiceActingManagerModel : public AbstractDialogModel {
	Q_OBJECT
  public:
	explicit VoiceActingManagerModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	// Abbreviations
	SCP_string abbrevBriefing() const;
	SCP_string abbrevCampaign() const;
	SCP_string abbrevCommandBriefing() const;
	SCP_string abbrevDebriefing() const;
	SCP_string abbrevMessage() const;
	SCP_string abbrevMission() const;

	void setAbbrevBriefing(const SCP_string& v);
	void setAbbrevCampaign(const SCP_string& v);
	void setAbbrevCommandBriefing(const SCP_string& v);
	void setAbbrevDebriefing(const SCP_string& v);
	void setAbbrevMessage(const SCP_string& v);
	void setAbbrevMission(const SCP_string& v);

	void setAbbrevSelection(ExportSelection sel);

	// Filename settings
	bool includeSenderInFilename() const;
	void setIncludeSenderInFilename(bool v);

	bool noReplace() const;
	void setNoReplace(bool v);

	Suffix suffix() const;
	void setSuffix(Suffix s);

	// Script export
	SCP_string scriptEntryFormat() const;
	void setScriptEntryFormat(const SCP_string& v);

	ExportSelection exportSelection() const;
	void setExportSelection(ExportSelection sel);

	bool groupMessages() const;
	void setGroupMessages(bool v);

	// Persona sync dropdown index:
	//   0 = <Wingman Personas>, 1 = <Non-Wingman Personas>, 2+ = specific persona index
	int whichPersonaToSync() const;
	void setWhichPersonaToSync(int idx);

	// Populates "<Wingman>", "<Non-Wingman>", then all persona names
	static SCP_vector<SCP_string> personaChoices();
	static SCP_vector<SCP_string> fileChoices();

	// Builds example filename using current settings
	// prefers command->brief->debrief->message ordering
	SCP_string buildExampleFilename() const;

	// Returns number of filenames modified across command/brief/debrief/messages
	int generateFilenames();

	// Writes a script file. Path must be absolute.
	bool generateScript(const SCP_string& absoluteFilePath);

	// Copy personas in one direction, restricted by whichPersonaToSync selection
	// Returns number of modified items
	int copyMessagePersonasToShips();
	int copyShipPersonasToMessages();

	// Clear personas from ships that never send a message and returns count cleared
	int clearPersonasFromNonSenders();

	// Set message head ANIs by matching builtin messages with same persona and returns count modified
	int setHeadAnisUsingMessagesTbl();

	// Validate <any wingman> messages
	static AnyWingmanCheckResult checkAnyWingmanPersonas();

  signals:
	

  private slots:
	

  private: // NOLINT(readability-redundant-access-specifiers)
	SCP_string _abbrevBriefing;
	SCP_string _abbrevCampaign;
	SCP_string _abbrevCommandBriefing;
	SCP_string _abbrevDebriefing;
	SCP_string _abbrevMessage;
	SCP_string _abbrevMission;

	bool _includeSenderInFilename = false;
	bool _noReplace = false;
	Suffix _suffix = Suffix::WAV;
	ExportSelection _previewSelection = ExportSelection::CommandBriefings; // A little hacky to re-use this enum.. but it's convenient

	SCP_string _scriptEntryFormat;
	ExportSelection _exportSelection = ExportSelection::Everything;
	bool _groupMessages = false;

	int _whichPersonaToSync = 0;

	void initializeData();

	SCP_string getSuffixString() const; // ".wav" or ".ogg"
	static int calcDigits(int size);    // 2..5
	SCP_string pickExampleSection() const; // chooses which abbrev to demo
	SCP_string generateFilename(ExportSelection sel, int number, int digits, const MMessage* messageOrNull) const;

	// Classic helpers adapted
	static const char* getMessageSender(const MMessage* message);
	static void getValidSender(char* out, size_t outSize, const MMessage* message, int* outSenderShipnum = nullptr, bool* outIsCommand = nullptr);
	static void groupMessageIndexes(SCP_vector<int>& messageIndexes);
	static void groupMessageIndexesInTree(int node, SCP_vector<int>& sourceList, SCP_vector<int>& destList);
	bool checkPersonaFilter(int persona) const;

	static bool fout(void* cfilePtr, const char* fmt, ...); // cfilePtr is CFILE*
	
};

} // namespace fso::fred::dialogs

#ifndef CAMPAIGNEDITORDIALOGMODEL_H
#define CAMPAIGNEDITORDIALOGMODEL_H

#include "mission/dialogs/AbstractDialogModel.h"
#include "ui/dialogs/CampaignEditorDialog.h"

namespace fso {
namespace fred {
namespace dialogs {


class CampaignEditorDialog;

class CampaignEditorDialogModel : public AbstractDialogModel
{
	Q_OBJECT
public:

	CampaignEditorDialogModel(CampaignEditorDialog *parent, EditorViewport *viewport);
	~CampaignEditorDialogModel() override = default;
	bool apply() override;

	void reject() override;

	inline const QString& getCurrentFile() const { return _currentFile; }
	inline void setCurrentFile(const QString &file) {
		modify<QString>(_currentFile, file, false); }

	inline const SCP_string& getCampaignName() const { return _campaignName; }
	inline const SCP_string& getCampaignType() const { return _campaignType; }
	inline bool getCampaignTechReset() const { return _campaignTechReset; }
	inline const SCP_string& getCampaignDescr() const { return _campaignDescr; }

	inline const SCP_string& getCurMissionBriefingCutscene() const {
		return _it_missionData->briefingCutscene; }
	inline const SCP_string& getCurMissionMainhall() const {
		return _it_missionData->mainhall; }
	inline const SCP_string& getCurMissionDebriefingPersona() const {
		return _it_missionData->debriefingPersona; }

	inline bool getCurBrIsLoop() const {
		return _it_missionData->it_branches->isLoop; }

	inline const SCP_string& getCurLoopDescr() const {
		return _it_missionData->it_branches->loopData.descr; }
	inline const SCP_string& getCurLoopAnim() const {
		return _it_missionData->it_branches->loopData.anim; }
	inline const SCP_string& getCurLoopVoice() const {
		return _it_missionData->it_branches->loopData.voice; }

	inline void setCampaignName(const SCP_string &campaignName) {
		modify<SCP_string>(_campaignName, campaignName); }
	inline void setCampaignType(const SCP_string &campaignType) {
		modify<SCP_string>(_campaignType, campaignType); }
	inline void setCampaignTechReset(bool campaignTechReset) {
		modify<bool>(_campaignTechReset, campaignTechReset); }
	inline void setCampaignDescr(const SCP_string &campaignDescr) {
		modify<SCP_string>(_campaignDescr, campaignDescr); }

	inline void setCurMissionBriefingCutscene(const SCP_string &briefingCutscene) {
		modify<SCP_string>(_it_missionData->mainhall, briefingCutscene); }
	inline void setCurMissionMainhall(const SCP_string &mainhall) {
		modify<SCP_string>(_it_missionData->mainhall, mainhall); }
	inline void setCurMissionDebriefingPersona(const SCP_string &debriefingPersona) {
		modify<SCP_string>(_it_missionData->debriefingPersona, debriefingPersona); }

	inline void setCurBrIsLoop(bool isLoop) {
		modify<bool>(_it_missionData->it_branches->isLoop, isLoop);}

	inline void setCurLoopDescr(const SCP_string &descr) {
		modify<SCP_string>(_it_missionData->it_branches->loopData.descr, descr); }
	inline void setCurLoopAnim(const SCP_string &anim) {
		modify<SCP_string>(_it_missionData->it_branches->loopData.anim, anim); }
	inline void setCurLoopVoice(const SCP_string &voice) {
		modify<SCP_string>(_it_missionData->it_branches->loopData.voice, voice); }

	bool loadCurrentFile();
	bool saveTo(const QString &file);

	inline bool query_modified() const { return modified; }

private:
	bool _saveTo(const QString &file);

	template<typename T>
	inline void modify(T &a, const T &b, const bool dataModification = true) {
		if (a != b) {
			a = b;
			modelChanged();
			if (dataModification)
				modified = true;
		}
	}
	bool modified = false;

	struct CampaignLoopData	{

		SCP_string descr = "";
		SCP_string anim = "";
		SCP_string voice = "";
	};

	struct CampaignBranchData {

		bool isLoop = false;
		CampaignLoopData loopData{};
	};

	struct CampaignMissionData {
		explicit CampaignMissionData();

		SCP_string briefingCutscene = "";
		SCP_string mainhall = "";
		SCP_string debriefingPersona = "";
		SCP_vector<CampaignBranchData>::iterator it_branches;
		SCP_vector<CampaignBranchData> branches;
	};

	CampaignEditorDialog *const _parent;
	QString _currentFile;

	//TODO constants

	SCP_string _campaignDescr;
	SCP_string _campaignName;
	SCP_string _campaignType;
	bool _campaignTechReset;

	SCP_vector<CampaignMissionData>::iterator _it_missionData;
	SCP_vector<CampaignMissionData> _missionData;
};


}
}
}
#endif // CAMPAIGNEDITORDIALOGMODEL_H

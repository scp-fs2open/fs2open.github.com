#ifndef CAMPAIGNEDITORDIALOGMODEL_H
#define CAMPAIGNEDITORDIALOGMODEL_H

#include "mission/dialogs/AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

class CampaignEditorDialogModel : public AbstractDialogModel
{
	Q_OBJECT
public:

	explicit CampaignEditorDialogModel(QObject* parent, EditorViewport* viewport);
	~CampaignEditorDialogModel() override = default;
	bool apply() override;

	void reject() override;

	const SCP_string& getCampaignName() const { return _campaignName; }
	const SCP_string& getCampaignType() const { return _campaignType; }
	bool getCampaignTechReset() const { return _campaignTechReset; }
	const SCP_string& getCampaignDescr() const { return _campaignDescr; }

	const SCP_string& getCurMissionBriefingCutscene() const {
		return _it_missionData->briefingCutscene; }
	const SCP_string& getCurMissionMainhall() const {
		return _it_missionData->mainhall; }
	const SCP_string& getCurMissionDebriefingPersona() const {
		return _it_missionData->debriefingPersona; }

	bool getCurBrIsLoop() const {
		return _it_missionData->it_branches->isLoop; }

	const SCP_string& getCurLoopDescr() const {
		return _it_missionData->it_branches->loopData.descr; }
	const SCP_string& getCurLoopAnim() const {
		return _it_missionData->it_branches->loopData.anim; }
	const SCP_string& getCurLoopVoice() const {
		return _it_missionData->it_branches->loopData.voice; }

	void setCampaignName(const SCP_string& campaignName) {
		modify<SCP_string>(_campaignName, campaignName); }
	void setCampaignType(const SCP_string& campaignType) {
		modify<SCP_string>(_campaignType, campaignType); }
	void setCampaignTechReset(bool campaignTechReset) {
		modify<bool>(_campaignTechReset, campaignTechReset); }
	void setCampaignDescr(const SCP_string& campaignDescr) {
		modify<SCP_string>(_campaignDescr, campaignDescr); }

	void setCurMissionBriefingCutscene(const SCP_string& briefingCutscene) {
		modify<SCP_string>(_it_missionData->mainhall, briefingCutscene); }
	void setCurMissionMainhall(const SCP_string& mainhall) {
		modify<SCP_string>(_it_missionData->mainhall, mainhall); }
	void setCurMissionDebriefingPersona(const SCP_string& debriefingPersona) {
		modify<SCP_string>(_it_missionData->debriefingPersona, debriefingPersona); }

	void setCurBrIsLoop(bool isLoop) {
		modify<bool>(_it_missionData->it_branches->isLoop, isLoop);}

	void setCurLoopDescr(const SCP_string& descr) {
		modify<SCP_string>(_it_missionData->it_branches->loopData.descr, descr); }
	void setCurLoopAnim(const SCP_string& anim) {
		modify<SCP_string>(_it_missionData->it_branches->loopData.anim, anim); }
	void setCurLoopVoice(const SCP_string& voice) {
		modify<SCP_string>(_it_missionData->it_branches->loopData.voice, voice); }

	bool query_modified() const { return modified; }
private slots:
	void flagModified() { modified = true;}

private:
	void initializeData();

	template<typename T>
	void modify(T &a, const T &b);

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

	SCP_string _campaignDescr;
	SCP_string _campaignName;
	SCP_string _campaignType;
	bool _campaignTechReset;

	SCP_vector<CampaignMissionData>::iterator _it_missionData;
	SCP_vector<CampaignMissionData> _missionData;
};


template<typename T>
inline void CampaignEditorDialogModel::modify(T &a, const T &b) {
	if (a != b) {
		a = b;
		modelChanged();
	}
}

}
}
}
#endif // CAMPAIGNEDITORDIALOGMODEL_H

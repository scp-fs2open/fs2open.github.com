#ifndef CAMPAIGNEDITORDIALOGMODEL_H
#define CAMPAIGNEDITORDIALOGMODEL_H

#include "mission/dialogs/AbstractDialogModel.h"
#include "ui/dialogs/CampaignEditorDialog.h"
#include "CheckedDataListModel.h"

namespace fso {
namespace fred {
namespace dialogs {


class CampaignEditorDialog;

class CampaignEditorDialogModel : public AbstractDialogModel
{
	Q_OBJECT
public:

	CampaignEditorDialogModel(const QString &file, CampaignEditorDialog *parent, EditorViewport *viewport);
	~CampaignEditorDialogModel() override = default;
	bool apply() override;

	void reject() override;

	inline const QString& getCurrentFile() const { return _currentFile; }
	inline bool isFileLoaded() const { return ! _currentFile.isEmpty(); }

	inline const QString& getCampaignName() const { return _campaignName; }
	inline const QString& getCampaignType() const { return _campaignType; }
	inline bool getCampaignTechReset() const { return _campaignTechReset; }
	inline const QString& getCampaignDescr() const { return _campaignDescr; }

	inline const QString& getCurMissionBriefingCutscene() const {
		return _it_missionData->briefingCutscene; }
	inline const QString& getCurMissionMainhall() const {
		return _it_missionData->mainhall; }
	inline const QString& getCurMissionDebriefingPersona() const {
		return _it_missionData->debriefingPersona; }

	inline bool getCurBrIsLoop() const {
		return _it_missionData->it_branches->isLoop; }

	inline const QString& getCurLoopDescr() const {
		return _it_missionData->it_branches->loopData.descr; }
	inline const QString& getCurLoopAnim() const {
		return _it_missionData->it_branches->loopData.anim; }
	inline const QString& getCurLoopVoice() const {
		return _it_missionData->it_branches->loopData.voice; }

	inline void setCampaignName(const QString &campaignName) {
		modify<QString>(_campaignName, campaignName); }
	inline void setCampaignType(const QString &campaignType) {
		modify<QString>(_campaignType, campaignType); }
	inline void setCampaignTechReset(bool campaignTechReset) {
		modify<bool>(_campaignTechReset, campaignTechReset); }
	inline void setCampaignDescr(const QString &campaignDescr) {
		modify<QString>(_campaignDescr, campaignDescr); }

	inline void setCurMissionBriefingCutscene(const QString &briefingCutscene) {
		modify<QString>(_it_missionData->mainhall, briefingCutscene); }
	inline void setCurMissionMainhall(const QString &mainhall) {
		modify<QString>(_it_missionData->mainhall, mainhall); }
	inline void setCurMissionDebriefingPersona(const QString &debriefingPersona) {
		modify<QString>(_it_missionData->debriefingPersona, debriefingPersona); }

	inline void setCurBrIsLoop(bool isLoop) {
		modify<bool>(_it_missionData->it_branches->isLoop, isLoop);}

	inline void setCurLoopDescr(const QString &descr) {
		modify<QString>(_it_missionData->it_branches->loopData.descr, descr); }
	inline void setCurLoopAnim(const QString &anim) {
		modify<QString>(_it_missionData->it_branches->loopData.anim, anim); }
	inline void setCurLoopVoice(const QString &voice) {
		modify<QString>(_it_missionData->it_branches->loopData.voice, voice); }

	bool saveTo(const QString &file);

	inline bool query_modified() const { return modified; }

private slots:
	inline void flagModified() { modified = true; }

private:
	bool _saveTo(QString file);

	bool modified{false};
	template<typename T>
	inline void modify(T &a, const T &b, const bool dataModification = true) {
		if (a != b) {
			a = b;
			modelChanged();
			if (dataModification)
				flagModified();
		}
	}


	struct CampaignLoopData	{

		QString descr;
		QString anim;
		QString voice;
	};

	struct CampaignBranchData {  //noUIu

		bool isLoop{false};
		CampaignLoopData loopData;
	};

	struct CampaignMissionData {
		explicit CampaignMissionData();

		QString briefingCutscene;
		QString mainhall;
		QString debriefingPersona;
		SCP_vector<CampaignBranchData>::iterator it_branches;  //noUIu
		SCP_vector<CampaignBranchData> branches;
	};

	QString _campaignDescr;
	QString _campaignName;
	QString _campaignType{ campaignTypes[0] };
	int _numPlayers{-1}; //noUI
	bool _campaignTechReset{false};

	SCP_vector<CampaignMissionData>::iterator _it_missionData;  //noUIu
	SCP_vector<CampaignMissionData> _missionData;

	CampaignEditorDialog *const _parent;
	const QString _currentFile;

public:

	static const QStringList campaignTypes;
	CheckedDataListModel<int> initialShips;
	CheckedDataListModel<int> initialWeapons;
};


}
}
}
#endif // CAMPAIGNEDITORDIALOGMODEL_H

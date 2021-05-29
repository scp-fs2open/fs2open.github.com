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

	CampaignEditorDialogModel(CampaignEditorDialog *parent, EditorViewport *viewport, const QString &file = "", const QString& newCampaignType = "");
	~CampaignEditorDialogModel() override = default;
	bool apply() override;

	void reject() override;

	inline bool isFileLoaded() const { return ! campaignFile.isEmpty(); }

	inline const QString& getCampaignName() const { return _campaignName; }
	inline bool getCampaignTechReset() const { return _campaignTechReset; }
	inline const QString& getCampaignDescr() const { return _campaignDescr; }

	inline const QString& getCurMissionFilename() const {
		return (_it_missionData ? _it_missionData : &mdEmpty)->filename; }
	inline bool getCurMissionFredable() const {
		return (_it_missionData ? _it_missionData : &mdEmpty)->fredable; }
	inline const char* getCurMissionDescr() const {
		return (_it_missionData ? _it_missionData : &mdEmpty)->fsoMission.notes; }
	inline const QString& getCurMissionBriefingCutscene() const {
		return (_it_missionData ? _it_missionData : &mdEmpty)->briefingCutscene; }
	inline const QString& getCurMissionMainhall() const {
		return (_it_missionData ? _it_missionData : &mdEmpty)->mainhall; }
	inline const QString& getCurMissionDebriefingPersona() const {
		return (_it_missionData ? _it_missionData : &mdEmpty)->debriefingPersona; }

	inline bool getCurBrIsLoop() const {
		return (_it_missionData ? _it_missionData : &mdEmpty)->it_branches->isLoop; }

	inline const QString& getCurLoopDescr() const {
		return (_it_missionData ? _it_missionData : &mdEmpty)->it_branches->loopData.descr; }
	inline const QString& getCurLoopAnim() const {
		return (_it_missionData ? _it_missionData : &mdEmpty)->it_branches->loopData.anim; }
	inline const QString& getCurLoopVoice() const {
		return (_it_missionData ? _it_missionData : &mdEmpty)->it_branches->loopData.voice; }

	bool saveTo(const QString &file);

	inline bool query_modified() const { return modified; }
	inline bool missionDropped() const { return ! droppedMissions.isEmpty(); }

private slots:
	inline void flagModified() { modified = true; }
	void checkMissionDrop(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
public slots:

	inline void setCampaignName(const QString &campaignName) {
		modify<QString>(_campaignName, campaignName); }
	inline void setCampaignTechReset(bool campaignTechReset) {
		modify<bool>(_campaignTechReset, campaignTechReset); }
	inline void setCampaignDescr(const QString &campaignDescr) {
		modify<QString>(_campaignDescr, campaignDescr); }

	void missionSelectionChanged(const QModelIndex &changed);

	inline void setCurMissionBriefingCutscene(const QString &briefingCutscene) {
		if (! _it_missionData) return;
		modify<QString>(_it_missionData->mainhall, briefingCutscene); }
	inline void setCurMissionMainhall(const QString &mainhall) {
		if (! _it_missionData) return;
		modify<QString>(_it_missionData->mainhall, mainhall); }
	inline void setCurMissionDebriefingPersona(const QString &debriefingPersona) {
		if (! _it_missionData) return;
		modify<QString>(_it_missionData->debriefingPersona, debriefingPersona); }

	inline void setCurBrIsLoop(bool isLoop) {
		if (! _it_missionData) return;
		modify<bool>(_it_missionData->it_branches->isLoop, isLoop);}

	inline void setCurLoopDescr(const QString &descr) {
		if (! _it_missionData) return;
		modify<QString>(_it_missionData->it_branches->loopData.descr, descr); }
	inline void setCurLoopAnim(const QString &anim) {
		if (! _it_missionData) return;
		modify<QString>(_it_missionData->it_branches->loopData.anim, anim); }
	inline void setCurLoopVoice(const QString &voice) {
		if (! _it_missionData) return;
		modify<QString>(_it_missionData->it_branches->loopData.voice, voice); }

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

	QStringList droppedMissions{};

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
		CampaignMissionData() = delete;
		CampaignMissionData(const QString &file);

		const QString filename;

		mission fsoMission{};
		bool fredable{false};

		QString briefingCutscene;
		QString mainhall;
		QString debriefingPersona;
		SCP_vector<CampaignBranchData>::iterator it_branches;  //noUIu
		SCP_vector<CampaignBranchData> branches;
	};

	friend CheckedDataListModel<std::unique_ptr<CampaignMissionData>>::RowData initMissions(SCP_vector<SCP_string>::const_iterator &m_it);

	QString _campaignDescr;
	QString _campaignName;
	int _numPlayers{-1}; //noUI
	bool _campaignTechReset{false};

	const CampaignMissionData mdEmpty{""};
	CampaignMissionData* _it_missionData{nullptr};
	CampaignEditorDialog *const _parent;


public:
	const QString campaignFile;
	static const QStringList campaignTypes;
	const QString campaignType;
	CheckedDataListModel<std::ptrdiff_t> initialShips;
	CheckedDataListModel<std::ptrdiff_t> initialWeapons;
	CheckedDataListModel<std::unique_ptr<CampaignMissionData>> missionData;
};


}
}
}
#endif // CAMPAIGNEDITORDIALOGMODEL_H

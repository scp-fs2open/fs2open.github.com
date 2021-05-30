#ifndef CAMPAIGNEDITORDIALOGMODEL_H
#define CAMPAIGNEDITORDIALOGMODEL_H

#include "mission/dialogs/AbstractDialogModel.h"
#include "ui/dialogs/CampaignEditorDialog.h"
#include "CheckedDataListModel.h"
#include <mission/missioncampaign.h>

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
	inline int getCampaignNumPlayers() const {
		if (campaignType == campaignTypes[0])
			return 0;
		auto checked = missionData.getCheckedData();
		if (checked.empty())
			return 0;
		return checked.front()->get()->fsoMission.num_players;
	}

private:
	inline const CampaignMissionData& getCurMn() const { return mnData_it ? *mnData_it : mdEmpty; }

public:
	inline const QString& getCurMnFilename() const { return getCurMn().filename; }
	inline bool getCurMnIncluded() const {
		return mnData_idx.isValid() && mnData_idx.data(Qt::CheckStateRole) == Qt::Checked; }
	inline bool getCurMnFredable() const { return getCurMn().fredable; }
	inline const char* getCurMnDescr() const { return getCurMn().fsoMission.notes; }
	inline const QString& getCurMnBriefingCutscene() const { return getCurMn().briefingCutscene; }
	inline const QString& getCurMnMainhall() const { return getCurMn().mainhall; }
	inline const QString& getCurMnDebriefingPersona() const { return getCurMn().debriefingPersona; }

	inline bool getCurBrIsLoop() const {
		return (mnData_it ? mnData_it : &mdEmpty)->it_branches->isLoop; }

	inline const QString& getCurLoopDescr() const {
		return (mnData_it ? mnData_it : &mdEmpty)->it_branches->loopData.descr; }
	inline const QString& getCurLoopAnim() const {
		return (mnData_it ? mnData_it : &mdEmpty)->it_branches->loopData.anim; }
	inline const QString& getCurLoopVoice() const {
		return (mnData_it ? mnData_it : &mdEmpty)->it_branches->loopData.voice; }

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

	inline void setCurMnBriefingCutscene(const QString &briefingCutscene) {
		if (! mnData_it) return;
		modify<QString>(mnData_it->mainhall, briefingCutscene); }
	inline void setCurMnMainhall(const QString &mainhall) {
		if (! mnData_it) return;
		modify<QString>(mnData_it->mainhall, mainhall); }
	inline void setCurMnDebriefingPersona(const QString &debriefingPersona) {
		if (! mnData_it) return;
		modify<QString>(mnData_it->debriefingPersona, debriefingPersona); }

	inline void setCurBrIsLoop(bool isLoop) {
		if (! mnData_it) return;
		modify<bool>(mnData_it->it_branches->isLoop, isLoop);}

	inline void setCurLoopDescr(const QString &descr) {
		if (! mnData_it) return;
		modify<QString>(mnData_it->it_branches->loopData.descr, descr); }
	inline void setCurLoopAnim(const QString &anim) {
		if (! mnData_it) return;
		modify<QString>(mnData_it->it_branches->loopData.anim, anim); }
	inline void setCurLoopVoice(const QString &voice) {
		if (! mnData_it) return;
		modify<QString>(mnData_it->it_branches->loopData.voice, voice); }

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
		CampaignMissionData() = delete;
		CampaignMissionData(const QString &file, const cmission &cm);
		CampaignMissionData(const QString &file);

		static CheckedDataListModel<std::unique_ptr<CampaignMissionData>>::RowData
		initMissions(const SCP_vector<SCP_string>::const_iterator &m_it);

		const QString filename;

		mission fsoMission{};
		bool fredable{false};

		QString briefingCutscene;
		QString mainhall;
		QString debriefingPersona;

		SCP_vector<CampaignBranchData>::iterator it_branches;  //noUIu
		SCP_vector<CampaignBranchData> branches;
	};



	QStringList droppedMissions{};

	static const CampaignMissionData mdEmpty;
	CampaignMissionData* mnData_it{nullptr};
	QPersistentModelIndex mnData_idx{};
	CampaignEditorDialog *const _parent;


public:
	const QString campaignFile;
	static const QStringList campaignTypes;
	const QString campaignType;
	CheckedDataListModel<std::ptrdiff_t> initialShips;
	CheckedDataListModel<std::ptrdiff_t> initialWeapons;
	CheckedDataListModel<std::unique_ptr<CampaignMissionData>> missionData;

private:
	QString _campaignName;
	QString _campaignDescr;
	bool _campaignTechReset{false};

};


}
}
}
#endif // CAMPAIGNEDITORDIALOGMODEL_H

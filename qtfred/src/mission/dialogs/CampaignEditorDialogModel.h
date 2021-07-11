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
	struct CampaignMissionData;
	Q_OBJECT
public:
	struct CampaignLoopData	{
		CampaignLoopData() = default;
		CampaignLoopData(const cmission *loop);
		bool is{false};

		QString descr;
		QString anim;
		QString voice;
	};

	struct CampaignBranchData {
		explicit CampaignBranchData() = default;
		CampaignBranchData(const int &sexp_branch, const QString &from, const cmission *loop = nullptr);

		void connect(const SCP_unordered_set<const CampaignMissionData*>& missions);

		enum BranchType { INVALID, REPEAT, NEXT, NEXT_NOT_FOUND, END, };
		static const SCP_map<BranchType, QString> branchTexts;

		BranchType type{INVALID};
		int sexp{-1};

		QString next;

		CampaignLoopData loop;
	};

	CampaignEditorDialogModel(CampaignEditorDialog *parent, EditorViewport *viewport, const QString &file = "", const QString& newCampaignType = "");
	~CampaignEditorDialogModel() override = default;
	bool apply() override;

	void reject() override;

	inline bool isFileLoaded() const { return ! campaignFile.isEmpty(); }

	inline const QString& getCampaignName() const { return campaignName; }
	inline bool getCampaignTechReset() const { return campaignTechReset; }
	inline const QString& getCampaignDescr() const { return campaignDescr; }
	inline int getCampaignNumPlayers() const {
		if (campaignType == campaignTypes[0])
			return 0;
		auto checked = missionData.getCheckedDataConst();
		if (checked.empty())
			return 0;
		return (*checked.cbegin())->nPlayers;
	}

private:
	inline const CampaignMissionData& getCurMn() const { return mnData_it ? *mnData_it : mdEmpty; }

public:
	inline const QString& getCurMnFilename() const { return getCurMn().filename; }
	inline bool getCurMnIncluded() const {
		return mnData_idx.isValid() && mnData_idx.data(Qt::CheckStateRole) == Qt::Checked; }
	inline bool getCurMnFredable() const { return getCurMn().fredable; }
	inline const QString& getCurMnDescr() const { return getCurMn().notes; }
	inline const QString& getCurMnBriefingCutscene() const { return getCurMn().briefingCutscene; }
	inline const QString& getCurMnMainhall() const { return getCurMn().mainhall; }
	inline const QString& getCurMnDebriefingPersona() const { return getCurMn().debriefingPersona; }

	inline const SCP_vector<CampaignBranchData>& getCurMnBranches() const {	return getCurMn().branches;	}
	inline bool isCurBrSelected() const { return mnData_it && mnData_it->brData_it; };
private:
	inline const CampaignBranchData& getCurBr() const {
		return isCurBrSelected() ? *mnData_it->brData_it : CampaignMissionData::bdEmpty; }
	void connectBranches(bool uiUpdate = true, const campaign *cpgn = nullptr);

public:
	inline int getCurBrIdx() const { return isCurBrSelected() ? mnData_it->brData_idx : -1; }

	inline bool getCurBrIsLoop() const { return getCurBr().loop.is; }
	inline const QString& getCurBrNext() const { return getCurBr().next; }

	inline const QString& getCurLoopDescr() const {	return getCurBr().loop.descr; }
	inline const QString& getCurLoopAnim() const { return getCurBr().loop.anim; }
	inline const QString& getCurLoopVoice() const {	return getCurBr().loop.voice; }

	bool saveTo(const QString &file);

	inline bool query_modified() const { return modified; }
	inline bool missionDropped() const { return ! droppedMissions.isEmpty(); }

private slots:
	inline void flagModified() { modified = true; }
	void checkMissionDrop(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

public slots:
	inline void setCampaignName(const QString &name) {
		modify<QString>(campaignName, name); }
	inline void setCampaignTechReset(bool techReset) {
		modify<bool>(campaignTechReset, techReset); }
	inline void setCampaignDescr(const QString &descr) {
		modify<QString>(campaignDescr, descr); }

	void missionSelectionChanged(const QModelIndex &changed);
	/*inline const QList<QAction *>& missionLinkMenus(const QModelIndex &idx){
		QMenu men{};
		men.addActions();
	}*/
	inline const QString *missionName(const QModelIndex idx) const {
		const CampaignMissionData *mn = missionData.internalDataConst(idx);
		return mn ? &mn->filename : nullptr;
	}
	inline const QList<QAction*> *missionEvents(const QModelIndex &idx) const {
		const CampaignMissionData *mn = missionData.internalDataConst(idx);
		return mn ? &mn->events : nullptr;
	}
	inline const QList<QAction*> *missionGoals(const QModelIndex &idx) const {
		const CampaignMissionData *mn = missionData.internalDataConst(idx);
		return mn ? &mn->goals : nullptr;
	}

	inline void setCurMnBriefingCutscene(const QString &briefingCutscene) {
		if (! mnData_it) return;
		modify<QString>(mnData_it->mainhall, briefingCutscene); }
	inline void setCurMnMainhall(const QString &mainhall) {
		if (! mnData_it) return;
		modify<QString>(mnData_it->mainhall, mainhall); }
	inline void setCurMnDebriefingPersona(const QString &debriefingPersona) {
		if (! mnData_it) return;
		modify<QString>(mnData_it->debriefingPersona, debriefingPersona); }

	void selectCurBr(const CampaignBranchData *br);

	void setCurBrIsLoop(bool isLoop);
	inline void setCurLoopDescr(const QString &descr) {
		if (! (mnData_it && mnData_it->brData_it)) return;
		modify<QString>(mnData_it->brData_it->loop.descr, descr); }
	void setCurLoopAnim(const QString &anim);
	void setCurLoopVoice(const QString &voice);

private:
	bool _saveTo(QString file) const;

	bool modified{false};
	template<typename T>
	inline void modify(T &a, const T &b) {
		if (a != b) {
			a = b;
			flagModified();
		}
	}

	struct CampaignMissionData {
		CampaignMissionData() = delete;
		CampaignMissionData(QString file,
							bool loaded = false,
							const mission *fsoMn = nullptr,
							const cmission *cm = nullptr,
							QObject* parent = nullptr);

		static void initMissions(
				const SCP_vector<SCP_string>::const_iterator &m_it,
				CheckedDataListModel<CampaignEditorDialogModel::CampaignMissionData> &model);

		const QString filename;

		bool fredable{false};

		const int nPlayers;
		const QString notes;

		const QList<QAction *> events;
		const QList<QAction *> goals;

		QString briefingCutscene;
		QString mainhall;
		QString debriefingPersona;

		static const CampaignBranchData bdEmpty;
		CampaignBranchData *brData_it{nullptr};
		int brData_idx{-1};
		SCP_vector<CampaignBranchData> branches;

		void branchesFromFormula(int formula, const cmission *loop = nullptr);
	};

	QStringList droppedMissions{};

	static const CampaignMissionData mdEmpty;
	CampaignMissionData* mnData_it{nullptr};
	QPersistentModelIndex mnData_idx{};
	CampaignEditorDialog *const parent;


public:
	const QStringList cutscenes;
	const QStringList mainhalls;
	const QStringList debriefingPersonas;
	const QStringList loopAnims;
	const QStringList loopVoices;

	const QString campaignFile;
	static const QStringList campaignTypes;
	const QString campaignType;
	CheckedDataListModel<std::ptrdiff_t> initialShips;
	CheckedDataListModel<std::ptrdiff_t> initialWeapons;
	CheckedDataListModel<CampaignMissionData> missionData;

private:
	QString campaignName;
	QString campaignDescr;
	bool campaignTechReset{false};

};


}
}
}
#endif // CAMPAIGNEDITORDIALOGMODEL_H

#pragma once

#include "mission/dialogs/AbstractDialogModel.h"
#include "ui/dialogs/CampaignEditorDialog.h"
#include "AssociatedPlainTextDocument.h"
#include "CheckedDataListModel.h"
#include <mission/missioncampaign.h>
#include <ui/widgets/sexp_tree.h>

#include <QTextDocument>
#include <QPlainTextEdit>

namespace fso {
namespace fred {
namespace dialogs {

const QString qstrEmpty{};

class CampaignEditorDialog;

class CampaignEditorDialogModel : public AbstractDialogModel, public SexpTreeEditorInterface
{
	class CampaignMissionData;
	class CampaignBranchData;
	Q_OBJECT

public:
	CampaignEditorDialogModel(CampaignEditorDialog *parent, EditorViewport *viewport, const QString &file = "", const QString& newCampaignType = "");
	~CampaignEditorDialogModel() override = default;
	void supplySubModels(QListView &ships, QListView &weps, QListView &missions, QPlainTextEdit &descr);

// AbstractDialogModel
	bool apply() override {	return saveTo(campaignFile); }
	void reject() override {} // nothing to do if the dialog is created each time it's opened

// SexpTreeEditorInterface
	QStringList getMissionGoals(const QString& reference_name) override;
	bool hasDefaultGoal(int) override {return true;}

	QStringList getMissionEvents(const QString& reference_name) override;
	bool hasDefaultEvent(int) override {return true;}

	QStringList getMissionNames() override;
	bool hasDefaultMissionName() override {return true;}

	bool requireCampaignOperators() const override {return true;}

	QList<QAction *> getContextMenuExtras(QObject *parent = nullptr) override;

// Model state getters
// Here and in the following, "Mn/mn" are short for a mission (in the context of the campaign),
// "Br/br" for the branches of a mission (to another or itself)
// "Cur" refers to the currently selected mission or branch, if any
	inline bool isFileLoaded() const { return ! campaignFile.isEmpty(); }

	inline const QString& getCampaignName() const { return campaignName; }
	inline bool getCampaignTechReset() const { return campaignTechReset; }
	int getCampaignNumPlayers() const;

	inline bool isCurMnSelected() const { return mnData_it; }
	inline const QString& getCurMnFilename() const { return mnData_it ? mnData_it->filename : qstrEmpty; }
	inline bool getCurMnIncluded() const {
		return mnData_idx.isValid() && mnData_idx.data(Qt::CheckStateRole) == Qt::Checked; }
	inline bool getCurMnFirst() const {
		return mnData_it && mnData_it->filename == firstMission;
	}
	inline bool getCurMnFredable() const { return mnData_it && mnData_it->fredable; }
	inline const QString& getCurMnDescr() const { return mnData_it ? mnData_it->notes : qstrEmpty; }
	inline const QString& getCurMnBriefingCutscene() const { return mnData_it ? mnData_it->briefingCutscene : qstrEmpty; }
	inline const QString& getCurMnMainhall() const { return mnData_it ? mnData_it->mainhall : qstrEmpty; }
	inline const QString& getCurMnDebriefingPersona() const { return mnData_it ? mnData_it->debriefingPersona : qstrEmpty; }
	inline int getCurMnBrCnt() const {return mnData_it ? static_cast<int>(mnData_it->branches.size()) : -1; }

//branch helpers
private:
	inline const CampaignBranchData* getCurBr() const {
		return mnData_it ? mnData_it->brData_it : nullptr; }
	void connectBranches(bool uiUpdate = true, const campaign *cpgn = nullptr);

public:
	bool fillTree(sexp_tree& sxt) const;
// Model state getters -- branch
	inline int getCurBrIdx() const { return getCurBr() ? mnData_it->brData_idx : -1; }

	inline bool isCurBrLoop() const { return getCurBr() && getCurBr()->loop; }
	inline const QString& getCurBrNext() const { return getCurBr() ? getCurBr()->next : qstrEmpty; }

	void supplySubModelLoop(QPlainTextEdit &descr);
	inline const QString& getCurLoopAnim() const { return getCurBr() ? getCurBr()->loopAnim : qstrEmpty; }
	inline const QString& getCurLoopVoice() const {	return getCurBr() ? getCurBr()->loopVoice : qstrEmpty; }

// Model state getters -- branch creation data
	inline const QString* missionName(const QModelIndex &idx) const {
		const CampaignMissionData *mn = missionData.managedData(idx);
		return mn ? &mn->filename : nullptr;
	}
	inline const QStringList* missionEvents(const QModelIndex &idx) const {
		const CampaignMissionData *mn = missionData.managedData(idx);
		return mn ? &mn->events : nullptr;
	}
	inline const QStringList* missionGoals(const QModelIndex &idx) const {
		const CampaignMissionData *mn = missionData.managedData(idx);
		return mn ? &mn->goals : nullptr;
	}

//saving & change checking
	bool saveTo(const QString &file);

	inline bool query_modified() const { return modified; }
	inline bool missionDropped() const { return ! droppedMissions.isEmpty(); }

private slots:
	void trackMissionUncheck(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
	inline void flagModified() { modified = true; }

public: //clang
private:
	bool _saveTo(QString file) const;

	template<typename T>
	inline void modify(T &a, const T &b) {
		if (a != b) {
			a = b;
			flagModified();
		}
	}
	QStringList droppedMissions{};
	bool modified{false};

// Model state setters
public slots:
	inline void setCampaignName(const QString &name) {
		modify<QString>(campaignName, name); }
	inline void setCampaignTechReset(bool techReset) {
		modify<bool>(campaignTechReset, techReset); }

	void missionSelectionChanged(const QItemSelection &selected);

	void setCurMnFirst();
	inline void setCurMnBriefingCutscene(const QString &briefingCutscene) {
		if (! mnData_it) return;
		modify<QString>(mnData_it->mainhall, briefingCutscene); }
	inline void setCurMnMainhall(const QString &mainhall) {
		if (! mnData_it) return;
		modify<QString>(mnData_it->mainhall, mainhall); }
	inline void setCurMnDebriefingPersona(const QString &debriefingPersona) {
		if (! mnData_it) return;
		modify<QString>(mnData_it->debriefingPersona, debriefingPersona); }

	int addCurMnBranchTo(const QModelIndex *other = nullptr, bool flip = false);
	void delCurMnBranch(int node);

	void selectCurBr(const QTreeWidgetItem *selected);
	int setCurBrCond(const QString &sexp, const QString &mn, const QString &arg);
	bool setCurBrSexp(int sexp);
	void setCurBrIsLoop(bool isLoop);

	void moveCurBr(bool up);

	void setCurLoopAnim(const QString &anim);
	void setCurLoopVoice(const QString &voice);

//Model inner types
private:
	class CampaignMissionData {
	public:
		CampaignMissionData() = delete;
		CampaignMissionData(QString file,
							bool loaded = false,
							const mission *fsoMn = nullptr,
							const cmission *cm = nullptr);

		static void initMissions(
				const SCP_vector<SCP_string>::const_iterator &m_it,
				CheckedDataListModel<CampaignEditorDialogModel::CampaignMissionData> &model);
		void branchesFromFormula(CampaignEditorDialogModel *model, int formula, const cmission *loop = nullptr);

	// state
		const QString filename;

		bool fredable{false};

		const int nPlayers;
		const QString notes;

		const QStringList events;
		const QStringList goals;

		QString briefingCutscene;
		QString mainhall;
		QString debriefingPersona;

		SCP_vector<CampaignBranchData> branches;
	// state -- current branch
		CampaignBranchData *brData_it{nullptr};
		int brData_idx{-1};
	};

	class CampaignBranchData {
	public:
		explicit CampaignBranchData() = default;
		CampaignBranchData(CampaignEditorDialogModel *model, int sexp_branch, const QString &from, const cmission *loop = nullptr);
		CampaignBranchData(CampaignEditorDialogModel *model, const QString &from, QString to = "");

		void connect(const SCP_unordered_set<const CampaignMissionData*>& missions);

	// constants
		enum BranchType { INVALID, REPEAT, NEXT, NEXT_NOT_FOUND, END, };
		static const SCP_map<BranchType, QString> branchTexts;

	// state
		BranchType type{INVALID};
		int sexp;

		QString next;

		bool loop;

		AssociatedPlainTextDocument* loopDescr;
		QString loopAnim;
		QString loopVoice;
	};

// Model state -- current mission
	CampaignMissionData* mnData_it{nullptr};
	QPersistentModelIndex mnData_idx{};

	CampaignEditorDialog *const parent;

public:
// parsed table / cfile data
	static const QStringList& cutscenes();
	static const QStringList& mainhalls();
	static const QStringList& debriefingPersonas();
	static const QStringList& loopAnims();
	static const QStringList& loopVoices();

// Model state -- specs
	const QString campaignFile;
	static const QStringList campaignTypes;
	const QString campaignType;

private:
// submodels
	CheckedDataListModel<std::ptrdiff_t> initialShips;
	CheckedDataListModel<std::ptrdiff_t> initialWeapons;
	CheckedDataListModel<CampaignMissionData> missionData;
	QString firstMission{""};

// Model state -- specs
	QString campaignName;
	AssociatedPlainTextDocument campaignDescr{""};
	bool campaignTechReset{false};
};


} // namespace dialogs
} // namespace fred
} // namespace fso

#include "CampaignEditorDialog.h"
#include "ui_CampaignEditorDialog.h"

#include "ui/util/SignalBlockers.h"
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringListModel>
#include <ui/FredView.h>

namespace fso {
namespace fred {
namespace dialogs {

CampaignEditorUtil::WarningVec CampaignEditorDialog::warnings{};

CampaignEditorUtil::WarningMsg::WarningMsg(QString &&_title, QString &&_msg, QString &&_type) :
	title(_title),
	msg(_msg),
	type(_type)
{}

CampaignEditorDialog::CampaignEditorDialog(QWidget *_parent, EditorViewport *_viewport) :
	QDialog(_parent),
	ui(new Ui::CampaignEditorDialog),
	model(new CampaignEditorDialogModel(this, _viewport)),
	parent(_parent),
	viewport(_viewport)
{
	ui->setupUi(this);

	connect(&warnings, &CampaignEditorUtil::WarningVec::gotMsg, this, [&]() {
		for (auto warn_it = warnings.begin();
			 warn_it != warnings.end();
			 warn_it = warnings.erase(warn_it)) {
			if (warn_it->type.isEmpty()) {
				QMessageBox::warning(this, warn_it->title, warn_it->msg);
			}
		}
	});
	warnings.gotMsg();

	QPalette p = ui->lblMissionDescr1->palette();
	p.setColor(QPalette::WindowText, Qt::darkYellow);
	ui->lblMissionDescr1->setPalette(p);
	p = ui->lblMissionDescr2->palette();
	p.setColor(QPalette::WindowText, Qt::red);
	ui->lblMissionDescr2->setPalette(p);

	connect(ui->lstMissions, &QListView::clicked, this, &CampaignEditorDialog::lstMissionsClicked);

	setModel();

	connect(ui->lstMissions, &QListView::customContextMenuRequested, this, &CampaignEditorDialog::mnLinkMenu);

	connect(ui->btnErrorChecker, &QPushButton::clicked, [](){
		uiWarn(tr("Error Checker"), tr("Error checker not implemented. Try saving a copy."));
	});
	connect(ui->btnFredMission, &QPushButton::clicked, this, [&](){
		reject();
		if(result() == Rejected)
			qobject_cast<FredView*>(parent)->loadMissionFile(model->getCurMnFilename());
	});
	connect(ui->btnFirstMission, &QPushButton::clicked, this, [&](bool toggledOn){
		Assertion(toggledOn, "Should not be able to unset first mission");
		model->setCurMnFirst();
		updateUIMission(false);
	});
	connect(ui->btnExit, &QPushButton::clicked, this, &CampaignEditorDialog::reject);

	QMenu *menFile { new QMenu(this) };
	ui->btnMenu->setMenu(menFile);
	QMenu *menFileNew { menFile->addMenu(tr("&New")) };
	for (auto& campaignType: model->campaignTypes)
		menFileNew->addAction(campaignType, this, &CampaignEditorDialog::fileNew);
	menFile->addSeparator();

	menFile->addAction(tr("&Open..."), this, &CampaignEditorDialog::fileOpen, tr("Ctrl+O"));
	menFile->addAction(tr("&Save"), this, &CampaignEditorDialog::fileSave, tr("Ctrl+S"));
	menFile->addAction(tr("Save &as..."), this, &CampaignEditorDialog::fileSaveAs, tr("Ctrl+Shift+S"));
	menFile->addAction(tr("Save &Copy as..."), this, &CampaignEditorDialog::fileSaveCopyAs);
	menFile->addSeparator();
	menFile->addAction(tr("E&xit"), this, &QDialog::reject);

	updateUIAll();
}
CampaignEditorDialog::~CampaignEditorDialog() = default;

void CampaignEditorDialog::setModel(CampaignEditorDialogModel *new_model) {
	if (new_model)
		model = std::unique_ptr<CampaignEditorDialogModel>(new_model);

	ui->cmbBriefingCutscene->setModel(new QStringListModel{CampaignEditorDialogModel::cutscenes(), model.get()});
	ui->cmbMainhall->setModel(new QStringListModel{CampaignEditorDialogModel::mainhalls(), model.get()});
	ui->cmbDebriefingPersona->setModel(new QStringListModel{CampaignEditorDialogModel::debriefingPersonas(), model.get()});
	ui->cmbLoopAnim->setModel(new QStringListModel{CampaignEditorDialogModel::loopAnims(), model.get()});
	ui->cmbLoopVoice->setModel(new QStringListModel{CampaignEditorDialogModel::loopVoices(), model.get()});

	model->supplySubModels(*(ui->lstShips), *(ui->lstWeapons), *(ui->lstMissions), *(ui->txaDescr));

	connect(ui->lstMissions->selectionModel(), &QItemSelectionModel::selectionChanged, model.get(), &CampaignEditorDialogModel::missionSelectionChanged);

	connect(ui->txtName, &QLineEdit::textChanged, model.get(), &CampaignEditorDialogModel::setCampaignName);
	connect(ui->chkTechReset, &QCheckBox::stateChanged, model.get(), [&](int changed) {
		model->setCampaignTechReset(changed == Qt::Checked);
	});

	connect(ui->cmbBriefingCutscene, &QComboBox::currentTextChanged, model.get(), &CampaignEditorDialogModel::setCurMnBriefingCutscene);
	connect(ui->cmbMainhall, &QComboBox::currentTextChanged, model.get(), &CampaignEditorDialogModel::setCurMnMainhall);
	connect(ui->cmbDebriefingPersona, &QComboBox::currentTextChanged, model.get(), &CampaignEditorDialogModel::setCurMnDebriefingPersona);

	ui->sxtBranches->initializeEditor(nullptr, model.get());
	connect(ui->sxtBranches, &sexp_tree::rootNodeDeleted, model.get(), &CampaignEditorDialogModel::delCurMnBranch, Qt::QueuedConnection);
	connect(ui->sxtBranches, &QTreeWidget::currentItemChanged, model.get(), &CampaignEditorDialogModel::selectCurBr);
	connect(ui->sxtBranches, &sexp_tree::nodeChanged, model.get(), [&](int node) {
		bool success = model->setCurBrSexp(
					ui->sxtBranches->save_tree(
							ui->sxtBranches->get_root(node)));
		if (!success) {
			int old = model->getCurBrIdx();
			restoreBranchOpen(old);
		}
	}, Qt::QueuedConnection);

	connect(ui->cmbLoopAnim, &QComboBox::currentTextChanged, model.get(), &CampaignEditorDialogModel::setCurLoopAnim);
	connect(ui->cmbLoopVoice, &QComboBox::currentTextChanged, model.get(), &CampaignEditorDialogModel::setCurLoopVoice);
}

void CampaignEditorDialog::reject() {  //merely means onClose
	if (! questionSaveChanges())
		return;

	QDialog::reject();
	deleteLater();
}

void CampaignEditorDialog::updateUISpec() {
	util::SignalBlockers blockers(this);

	setWindowTitle(model->campaignFile.isEmpty() ? "Untitled" : model->campaignFile + ".fc2");

	ui->txtName->setText(model->getCampaignName());
	if (CampaignEditorDialogModel::campaignTypes.indexOf(model->campaignType))
		ui->txtType->setText(model->campaignType + ": " + QString::number(model->numPlayers));
	else
		ui->txtType->setText(model->campaignType);
	ui->chkTechReset->setChecked(model->getCampaignTechReset());
}

void CampaignEditorDialog::updateUIMission(bool updateBranch) {
	util::SignalBlockers blockers(this);

	ui->btnFirstMission->setEnabled(model->getCurMnIncluded() && ! model->isCurMnFirst());
	ui->btnFirstMission->setChecked(model->isCurMnFirst());
	ui->btnFredMission->setEnabled(model->getCurMnFredable());
	ui->txbMissionDescr->setText(model->getCurMnDescr());

	ui->cmbBriefingCutscene->setCurrentText(model->getCurMnBriefingCutscene());
	ui->cmbMainhall->setCurrentText(model->getCurMnMainhall());
	ui->cmbDebriefingPersona->setCurrentText(model->getCurMnDebriefingPersona());

	bool included{model->getCurMnIncluded()};
	ui->cmbBriefingCutscene->setEnabled(included);
	ui->cmbMainhall->setEnabled(included);
	ui->cmbDebriefingPersona->setEnabled(included);

	ui->sxtBranches->setEnabled(included);
	ui->sxtBranches->clear_tree("");

	if (included)
		model->fillTree(*ui->sxtBranches);

	if (updateBranch)
		updateUIBranch();
}

void CampaignEditorDialog::updateUIBranch(int selectedIdx) {
	util::SignalBlockers blockers(this);

	if (selectedIdx >= 0)
		model->selectCurBr(ui->sxtBranches->topLevelItem(selectedIdx));
	else
		selectedIdx = model->getCurBrIdx();

	bool sel = selectedIdx >= 0;

	if (sel) {
		ui->sxtBranches->selectionModel()->select(
					ui->sxtBranches->model()->index(selectedIdx, 0),
					QItemSelectionModel::Select);
	}

	bool loop = model->isCurBrLoop();

	model->supplySubModelLoop(*ui->txaLoopDescr);
	ui->txaLoopDescr->setEnabled(loop);

	ui->cmbLoopAnim->setCurrentText(model->getCurLoopAnim());
	ui->cmbLoopAnim->setEnabled(loop);

	ui->cmbLoopVoice->setCurrentText(model->getCurLoopVoice());
	ui->cmbLoopVoice->setEnabled(loop);
}

void fso::fred::dialogs::CampaignEditorDialog::restoreBranchOpen(int branch)
{
	updateUIMission(false);
	updateUIBranch(branch);
	ui->sxtBranches->expand_branch(ui->sxtBranches->topLevelItem(branch));
}

bool CampaignEditorDialog::questionSaveChanges() {
	QMessageBox::StandardButton resBtn = QMessageBox::Discard;
	if (model->query_modified()) {
		QString msg =
				tr("This campaign has been modified.\n")
				+ (model->missionDropped() ?
					   tr("Additionally, packaged/missing\nmission(s) have been removed,\nwhich cannot be added again.\n") : "")
				+ tr("\nSave changes?");
		resBtn = QMessageBox::question( this, tr("Unsaved changes"),
										msg,
										QMessageBox::Cancel | QMessageBox::Discard | QMessageBox::Save,
										QMessageBox::Save);
	}

	switch (resBtn) {
		case QMessageBox::Discard :
			model->reject();
			return true;
		case QMessageBox::Save :
			return fileSave();
		case QMessageBox::Cancel :
			return false;
		default:
			UNREACHABLE("An unhandled button was pressed. A coder must handle the buttons they provide.");
			return false;
	}
}

void CampaignEditorDialog::fileNew() {
	if (! questionSaveChanges())
		return;

	auto *act = qobject_cast<QAction*>(sender());
	if (! act)
		return;

	setModel(new CampaignEditorDialogModel(this,
										   viewport,
										   "",
										   act->text(),
										   act->text().contains("multi") ?
											   QInputDialog::getInt(this, "Campaign Player Number", "Enter campaign player number", 2, 2) :
											   0));
	updateUIAll();
}

void CampaignEditorDialog::fileOpen() {
	if (! questionSaveChanges())
		return;

	QString pathName = QFileDialog::getOpenFileName(this, tr("Load campaign"), model->campaignFile, tr("FS2 campaigns (*.fc2)"));

	if (pathName.isEmpty())
		return;

	auto newModel = new CampaignEditorDialogModel(this, viewport, pathName);
	if (newModel->isFileLoaded()) {
		setModel(newModel);
	} else {
		delete newModel;
		uiWarn(tr("Error opening file"), pathName);
		setModel(new CampaignEditorDialogModel(this, viewport, ""));
	}

	updateUIAll();
}

bool CampaignEditorDialog::fileSave() {
	if (model->campaignFile.isEmpty())
		return fileSaveAs();

	bool res = model->apply();

	updateUIAll();
	return res;
}

bool CampaignEditorDialog::fileSaveAs() {
	QString pathName = QFileDialog::getSaveFileName(this, tr("Save campaign as"), model->campaignFile, tr("FS2 campaigns (*.fc2)"));
	if (pathName.isEmpty())
		return false;

	bool res = model->saveTo(pathName);
	if (res)
		setModel(new CampaignEditorDialogModel(this, viewport, pathName));

	updateUIAll();
	return res;
}

void CampaignEditorDialog::fileSaveCopyAs() {
	QString pathName = QFileDialog::getSaveFileName(this, tr("Save copy as"), model->campaignFile, tr("FS2 campaigns (*.fc2)"));
	if (pathName.isEmpty())
		return;

	model->saveTo(pathName);
}

void CampaignEditorDialog::lstMissionsClicked(const QModelIndex &idx) {
	QItemSelectionModel &sel = *ui->lstMissions->selectionModel();
	bool same = sel.selectedIndexes().contains(idx);
	sel.clearSelection();
	if (!same)
		sel.select(idx, QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
}

void CampaignEditorDialog::mnLinkMenu(const QPoint &pos){
	QModelIndex here = ui->lstMissions->indexAt(pos);
	if (here.data(Qt::CheckStateRole) != Qt::Checked)
		return;

	const QString *mnName = model->missionName(here);
	if (! mnName)
		return;
	const QStringList *goals{ model->missionGoals(here) };
	if (! goals)
		return;
	const QStringList *evts{ model->missionEvents(here) };
	if (! evts)
		return;

	QMenu menu{ ui->lstMissions };

	QAction *to = menu.addAction(tr("Add branch to ") + mnName);
	QAction *from = menu.addAction(tr("Add branch from ") + mnName);
	QAction *end = menu.addAction(tr("Add campaign end"));
	bool mnSel{ model->isCurMnSelected() };
	to->setEnabled(mnSel);
	from->setEnabled(mnSel);
	end->setEnabled(mnSel);
	if (! mnSel) {
		menu.exec(ui->lstMissions->mapToGlobal(pos));
		return;
	}

	QMenu *par{ nullptr };

	if (model->getCurBrIdx() < 0) {
		menu.addSection(tr("Select a branch to choose conditions!"));
		menu.addAction("")->setEnabled(false);
	} else {
		menu.addSection(tr("Branch Conditions"));

		QMenu *gt = menu.addMenu("is-previous-goal-true");
		QMenu *gf = menu.addMenu("is-previous-goal-false");
		for (auto& g : *goals) {
			gt->addAction(g);
			gf->addAction(g);
		}
		connect(gt, &QMenu::aboutToShow, this, [&](){par = gt;});
		connect(gf, &QMenu::aboutToShow, this, [&](){par = gf;});

		QMenu *et = menu.addMenu("is-previous-event-true");
		QMenu *ef = menu.addMenu("is-previous-event-false");
		for (auto& e : *evts) {
			et->addAction(e);
			ef->addAction(e);
		}
		connect(et, &QMenu::aboutToShow, this, [&](){par = et;});
		connect(ef, &QMenu::aboutToShow, this, [&](){par = ef;});
	}

	const QAction *choice = menu.exec(ui->lstMissions->mapToGlobal(pos));

	int res_branch{-1};
	if (choice == to) {
		res_branch = model->addCurMnBranchTo(&here);
	} else if (choice == from) {
		res_branch = model->addCurMnBranchTo(&here, true);
	} else if (choice == end) {
		res_branch = model->addCurMnBranchTo(/*end*/);
	} else if (choice && par) {
		res_branch = model->setCurBrCond(par->title(), *mnName, choice->text());
	} //else menu was dismissed

	if (res_branch != -1)
		restoreBranchOpen(res_branch);
}

} // namespace dialogs
} // namespace fred
} // namespace fso

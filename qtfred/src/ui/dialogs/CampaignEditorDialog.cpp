#include "CampaignEditorDialog.h"
#include "ui_CampaignEditorDialog.h"

#include "ui/util/SignalBlockers.h"
#include <QFileDialog>
#include <QStringListModel>
#include <ui/FredView.h>

namespace fso {
namespace fred {
namespace dialogs {

CampaignEditorDialog::CampaignEditorDialog(QWidget *_parent, EditorViewport *_viewport) :
	QDialog(_parent),
	ui(new Ui::CampaignEditorDialog),
	model(new CampaignEditorDialogModel(this, _viewport)),
	parent(_parent),
	viewport(_viewport)
{
	ui->setupUi(this);

	QPalette p = ui->lblMissionDescr1->palette();
	p.setColor(QPalette::WindowText, Qt::darkYellow);
	ui->lblMissionDescr1->setPalette(p);
	p = ui->lblMissionDescr2->palette();
	p.setColor(QPalette::WindowText, Qt::red);
	ui->lblMissionDescr2->setPalette(p);

	connect(ui->lstMissions, &QListView::clicked, this, &CampaignEditorDialog::lstMissionsClicked);

	setModel();

	connect(ui->lstMissions, &QListView::customContextMenuRequested, this, &CampaignEditorDialog::mnLinkMenu);

	connect(ui->btnErrorChecker, &QPushButton::clicked, this, &CampaignEditorDialog::btnErrorCheckerClicked);
	connect(ui->btnFredMission, &QPushButton::clicked, this, [&](){
		reject();
		if(result() == Rejected)
			qobject_cast<FredView*>(parent)->loadMissionFile(model->getCurMnFilename());
	});
	connect(ui->btnExit, &QPushButton::clicked, this, &CampaignEditorDialog::reject);

	QMenu *menFile { new QMenu(this) };
	ui->btnMenu->setMenu(menFile);
	QMenu *menFileNew { menFile->addMenu(tr("&New")) };
	for (auto& t: model->campaignTypes)
		menFileNew->addAction(t, this, &CampaignEditorDialog::fileNew);
	menFile->addSeparator();

	menFile->addAction(tr("&Open..."), this, &CampaignEditorDialog::fileOpen, tr("Ctrl+O"));
	menFile->addAction(tr("&Save"), this, &CampaignEditorDialog::fileSave, tr("Ctrl+S"))->setEnabled(false);
	menFile->addAction(tr("Save &as..."), this, &CampaignEditorDialog::fileSaveAs, tr("Ctrl+Shift+S"))->setEnabled(false);
	menFile->addAction(tr("Save &Copy as..."), this, &CampaignEditorDialog::fileSaveCopyAs)->setEnabled(false);
	menFile->addSeparator();
	menFile->addAction(tr("E&xit"), this, &QDialog::reject);

	updateUIAll();

	QMessageBox::information(this, "WORK-IN-PROGRESS", "This editor dialog is a work in progress.\nSpecifically, saving changes is NOT SUPPORTED yet.");
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

	model->supplySubModels(*ui->lstShips, *ui->lstWeapons, *ui->lstMissions, *ui->txaDescr);

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
		model->setCurBrSexp(
					ui->sxtBranches->save_tree(
							ui->sxtBranches->get_root(node)));
	}, Qt::QueuedConnection);

	connect(ui->btnBranchLoop, &QPushButton::toggled, model.get(), &CampaignEditorDialogModel::setCurBrIsLoop);
	connect(ui->btnBranchUp, &QPushButton::clicked, model.get(), [&](){model->moveCurBr(true);});
	connect(ui->btnBranchDown, &QPushButton::clicked, model.get(),[&](){model->moveCurBr(false);});

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
	if (model->getCampaignNumPlayers())
		ui->txtType->setText(model->campaignType + ": " + QString::number(model->getCampaignNumPlayers()));
	else
		ui->txtType->setText(model->campaignType);
	ui->chkTechReset->setChecked(model->getCampaignTechReset());

	//ui->btnErrorChecker->setEnabled()
	//ui->btnRealign->setEnabled()

}

void CampaignEditorDialog::updateUIMission(bool updateBranch) {
	util::SignalBlockers blockers(this);

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
					ui->sxtBranches->model()->index(
								selectedIdx, 0),
					QItemSelectionModel::Select);
	}

	bool loop = model->getCurBrIsLoop();

	ui->btnBranchUp->setEnabled(selectedIdx > 0);
	ui->btnBranchDown->setEnabled(sel && selectedIdx + 1 < model->getCurMnBrCnt());

	ui->btnBranchLoop->setEnabled(sel);
	ui->btnBranchLoop->setChecked(loop);

	model->supplySubModelLoop(*ui->txaLoopDescr);
	ui->txaLoopDescr->setEnabled(loop);

	ui->cmbLoopAnim->setCurrentText(model->getCurLoopAnim());
	ui->cmbLoopAnim->setEnabled(loop);

	ui->cmbLoopVoice->setCurrentText(model->getCurLoopVoice());
	ui->cmbLoopVoice->setEnabled(loop);
}

bool CampaignEditorDialog::questionSaveChanges() {
	QMessageBox::StandardButton resBtn = QMessageBox::Discard;
	if (model->query_modified()) {
		resBtn = QMessageBox::question( this, tr("Unsaved changes"),
										tr("This campaign has been modified.\n")
										+ (model->missionDropped() ?
											   tr("Additionally, packaged/missing\nmission(s) have been removed,\nwhich cannot be added again.\n") : "")
										+ tr("\nSave changes?"),
										QMessageBox::Cancel | QMessageBox::Discard | QMessageBox::Save,
										QMessageBox::Save);
	}
	if (resBtn == QMessageBox::Discard)
		model->reject();
	if (resBtn == QMessageBox::Save && ! fileSave())
		return false;
	return resBtn != QMessageBox::Cancel;
}

void CampaignEditorDialog::fileNew() {
	if (! questionSaveChanges())
		return;

	auto *act = qobject_cast<QAction*>(sender());

	setModel(new CampaignEditorDialogModel(this, viewport, "", act ? act->text() : ""));
	updateUIAll();
}

void CampaignEditorDialog::fileOpen() {
	if (! questionSaveChanges())
		return;

	QString pathName = QFileDialog::getOpenFileName(this, tr("Load campaign"), model->campaignFile, tr("FS2 campaigns (*.fc2)"));

	if (pathName.isEmpty())
		return;

	auto newModel = new CampaignEditorDialogModel(this, viewport, pathName);
	if (newModel->isFileLoaded())
		setModel(newModel);
	else {
		delete newModel;
		QMessageBox::information(this, tr("Error opening file"), pathName);
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
	if (! mnName) return;
	const QStringList *goals{ model->missionGoals(here) };
	if (! goals) return;
	const QStringList *evts{ model->missionEvents(here) };
	if (! evts) return;

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

	if (res_branch != -1) {
		updateUIMission(false);
		auto h = ui->sxtBranches->topLevelItem(res_branch);
		model->selectCurBr(h);
		ui->sxtBranches->expand_branch(h);
	}
}

void CampaignEditorDialog::btnErrorCheckerClicked() {
	QMessageBox::information(this, tr("Error Checker"), "Error checker not implemented yet.");
}


}
}
}

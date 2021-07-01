#include "CampaignEditorDialog.h"
#include "ui_CampaignEditorDialog.h"

#include "ui/util/SignalBlockers.h"
#include <QFileDialog>
#include <QStringListModel>
#include <ui/FredView.h>


Q_DECLARE_METATYPE(const fso::fred::dialogs::CampaignEditorDialogModel::CampaignBranchData*)

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

	ui->sxtBranches->initializeEditor(nullptr, this);

	setModel();

	connect(ui->btnBranchUp, &QPushButton::clicked, this, &CampaignEditorDialog::btnBranchUpClicked);
	connect(ui->btnBranchDown, &QPushButton::clicked, this, &CampaignEditorDialog::btnBranchDownClicked);
	connect(ui->btnRealign, &QPushButton::clicked, this, &CampaignEditorDialog::btnRealignClicked);
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

	ui->cmbBriefingCutscene->setModel(new QStringListModel{model->cutscenes, model.get()});
	ui->cmbMainhall->setModel(new QStringListModel{model->mainhalls, model.get()});
	ui->cmbDebriefingPersona->setModel(new QStringListModel{model->debriefingPersonas, model.get()});
	ui->cmbLoopAnim->setModel(new QStringListModel{model->loopAnims, model.get()});
	ui->cmbLoopVoice->setModel(new QStringListModel{model->loopVoices, model.get()});

	ui->lstShips->setModel(&model->initialShips);
	ui->lstWeapons->setModel(&model->initialWeapons);

	ui->lstMissions->setModel(&model->missionData);
	connect(ui->lstMissions->selectionModel(), &QItemSelectionModel::currentRowChanged, model.get(), &CampaignEditorDialogModel::missionSelectionChanged);

	connect(ui->txtName, &QLineEdit::textChanged, model.get(), &CampaignEditorDialogModel::setCampaignName);
	connect(ui->chkTechReset, &QCheckBox::stateChanged, model.get(), [&](int changed) {
		model->setCampaignTechReset(changed == Qt::Checked);
	});
	connect(ui->txaDescr, &QPlainTextEdit::textChanged, model.get(), [&]() {
		model->setCampaignDescr(ui->txaDescr->toPlainText());
	});

	connect(ui->cmbBriefingCutscene, &QComboBox::currentTextChanged, model.get(), &CampaignEditorDialogModel::setCurMnBriefingCutscene);
	connect(ui->cmbMainhall, &QComboBox::currentTextChanged, model.get(), &CampaignEditorDialogModel::setCurMnMainhall);
	connect(ui->cmbDebriefingPersona, &QComboBox::currentTextChanged, model.get(), &CampaignEditorDialogModel::setCurMnDebriefingPersona);

	connect(ui->sxtBranches, &QTreeWidget::currentItemChanged, model.get(), [&](QTreeWidgetItem *selected) {
		QTreeWidgetItem *parent_node;
		while ((parent_node = selected->parent()))
			selected = parent_node;
		model->selectCurBr(selected->data(0, Qt::UserRole).value<const CampaignEditorDialogModel::CampaignBranchData*>());
	});

	connect(ui->btnBranchLoop, &QPushButton::toggled, model.get(), &CampaignEditorDialogModel::setCurBrIsLoop);
	connect(ui->txaLoopDescr, &QPlainTextEdit::textChanged, model.get(), [&]() {
		model->setCurLoopDescr(ui->txaLoopDescr->toPlainText());
	});
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

	ui->txaDescr->setPlainText(model->getCampaignDescr());

	//ui->btnErrorChecker->setEnabled()
	//ui->btnRealign->setEnabled()

}

void CampaignEditorDialog::updateUIMission() {
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

	sexp_tree &sxt = *ui->sxtBranches;
	sxt.setEnabled(included);
	sxt.clear_tree("");

	using Branch = CampaignEditorDialogModel::CampaignBranchData;
	if (included) {
		for (const Branch& br : model->getCurMnBranches()) {
			NodeImage img;
			if (br.type == Branch::NEXT_NOT_FOUND)
				img = NodeImage::ROOT_DIRECTIVE;
			else if (br.loop.is)
				img = NodeImage::ROOT;
			else
				img = NodeImage::BLACK_DOT;
			QTreeWidgetItem *h = sxt.insert(Branch::branchTexts.at(br.type) + br.next, img);
			h->setData(0, Qt::UserRole, QVariant::fromValue(&br));
			sxt.add_sub_tree(sxt.load_sub_tree(br.sexp, true, "do-nothing"), h);
		}
	}

	updateUIBranch();
}

void CampaignEditorDialog::updateUIBranch() {
	util::SignalBlockers blockers(this);

	bool sel = model->isCurBrSelected();
	bool loop = model->getCurBrIsLoop();

	if (sel)
		ui->sxtBranches->selectionModel()->select(
					ui->sxtBranches->model()->index(model->getCurBrIdx(),0),
					QItemSelectionModel::Select);

	ui->btnBranchUp->setEnabled(sel);
	ui->btnBranchDown->setEnabled(sel);

	ui->btnBranchLoop->setEnabled(sel);
	ui->btnBranchLoop->setChecked(loop);

	ui->txaLoopDescr->setPlainText(model->getCurLoopDescr());
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

	auto newModel = new CampaignEditorDialogModel(this, viewport, pathName);
	if (newModel->isFileLoaded())
		setModel(newModel);
	else {
		delete newModel;
		QMessageBox::information(this, tr("Error opening file"), pathName);
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

void CampaignEditorDialog::btnBranchUpClicked() {

}

void CampaignEditorDialog::btnBranchDownClicked() {

}

void CampaignEditorDialog::btnErrorCheckerClicked() {

}

void CampaignEditorDialog::btnRealignClicked() {

}



}
}
}

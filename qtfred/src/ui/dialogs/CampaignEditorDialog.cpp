#include "CampaignEditorDialog.h"
#include "ui_CampaignEditorDialog.h"

#include "ui/util/SignalBlockers.h"
#include <QFileDialog>

namespace fso {
namespace fred {
namespace dialogs {

CampaignEditorDialog::CampaignEditorDialog(QWidget *parent, EditorViewport *viewport) :
	QDialog(parent),
	ui(new Ui::CampaignEditorDialog),
	model(new CampaignEditorDialogModel(this, viewport)),
	_parent(parent),
	_viewport(viewport)
{
	ui->setupUi(this);

	QPalette p = ui->lblMissionDescr1->palette();
	p.setColor(QPalette::WindowText, Qt::darkYellow);
	ui->lblMissionDescr1->setPalette(p);
	p = ui->lblMissionDescr2->palette();
	p.setColor(QPalette::WindowText, Qt::red);
	ui->lblMissionDescr2->setPalette(p);

	connect(ui->txtName, &QLineEdit::textChanged, this, &CampaignEditorDialog::txtNameChanged);
	connect(ui->chkTechReset, &QCheckBox::stateChanged, this, &CampaignEditorDialog::chkTechResetChanged);
	connect(ui->txaDescr, &QPlainTextEdit::textChanged, this, &CampaignEditorDialog::txaDescrTextChanged);
	connect(ui->txtBriefingCutscene, &QLineEdit::textChanged, this, &CampaignEditorDialog::txtBriefingCutsceneChanged);
	connect(ui->txtMainhall, &QLineEdit::textChanged, this, &CampaignEditorDialog::txtMainhallChanged);
	connect(ui->txtDebriefingPersona, &QLineEdit::textChanged, this, &CampaignEditorDialog::txtDebriefingPersonaChanged);
	connect(ui->btnBranchUp, &QPushButton::clicked, this, &CampaignEditorDialog::btnBranchUpClicked);
	connect(ui->btnBranchDown, &QPushButton::clicked, this, &CampaignEditorDialog::btnBranchDownClicked);
	connect(ui->btnBranchLoop, &QPushButton::toggled, this, &CampaignEditorDialog::btnBranchLoopToggled);
	connect(ui->txaLoopDescr, &QPlainTextEdit::textChanged, this, &CampaignEditorDialog::txaLoopDescrChanged);
	connect(ui->txtLoopAnim, &QLineEdit::textChanged, this, &CampaignEditorDialog::txtLoopAnimChanged);
	connect(ui->btnBrLoopAnim, &QPushButton::clicked, this, &CampaignEditorDialog::btnBrLoopAnimClicked);
	connect(ui->txtLoopVoice, &QLineEdit::textChanged, this, &CampaignEditorDialog::txtLoopVoiceChanged);
	connect(ui->btnBrLoopVoice, &QPushButton::clicked, this, &CampaignEditorDialog::btnBrLoopVoiceClicked);
	connect(ui->btnRealign, &QPushButton::clicked, this, &CampaignEditorDialog::btnRealignClicked);
	connect(ui->btnErrorChecker, &QPushButton::clicked, this, &CampaignEditorDialog::btnErrorCheckerClicked);
	connect(ui->btnLoadMission, &QPushButton::clicked, this, &CampaignEditorDialog::btnLoadMissionClicked);
	connect(ui->btnExit, &QPushButton::clicked, this, &CampaignEditorDialog::reject);

	connect(model.get(), &AbstractDialogModel::modelChanged, this, &CampaignEditorDialog::updateUI);

	QMenu *menFile { new QMenu(this) };
	ui->btnMenu->setMenu(menFile);
	QMenu *menFileNew { menFile->addMenu(tr("&New")) };
	for (auto& t: model->campaignTypes)
		menFileNew->addAction(t, this, &CampaignEditorDialog::fileNew);
	menFile->addSeparator();

	menFile->addAction(tr("&Open..."), this, &CampaignEditorDialog::fileOpen, tr("Ctrl+O"));
	menFile->addAction(tr("&Save"), this, &CampaignEditorDialog::fileSave, tr("Ctrl+S"));
	menFile->addAction(tr("Save &as..."), this, &CampaignEditorDialog::fileSaveAs);
	menFile->addAction(tr("Save &Copy as..."), this, &CampaignEditorDialog::fileSaveCopyAs);
	menFile->addSeparator();
	menFile->addAction(tr("E&xit"), this, &QDialog::reject);

	updateUI();
}

CampaignEditorDialog::~CampaignEditorDialog()
{
}

void CampaignEditorDialog::reject() {  //merely means onClose
	if (! questionSaveChanges())
		return;

	QDialog::reject();
	deleteLater();
}

void CampaignEditorDialog::updateUI() {
	util::SignalBlockers blockers(this);

	QString file = model->campaignFile;
	this->setWindowTitle(file.isEmpty() ? "Untitled" : file + ".fc2");

	ui->txtName->setText(model->getCampaignName());
	ui->txtType->setText(model->campaignType);
	ui->chkTechReset->setChecked(model->getCampaignTechReset());

	ui->txaDescr->setPlainText(model->getCampaignDescr());

	ui->lstShips->setModel(&model->initialShips);
	ui->lstWeapons->setModel(&model->initialWeapons);

	ui->lstMissions->setModel(&model->missionData);

	ui->txtBriefingCutscene->setText(model->getCurMissionBriefingCutscene());
	ui->txtMainhall->setText(model->getCurMissionMainhall());
	ui->txtDebriefingPersona->setText(model->getCurMissionDebriefingPersona());

	//ui->btnBranchUp->setEnabled()
	//ui->btnBranchDown->setEnabled()
	//ui->btnBranchLoop->setEnabled()

	ui->txaLoopDescr->setPlainText(model->getCurLoopDescr());
	ui->txtLoopAnim->setText(model->getCurLoopAnim());
	//ui->btnBrLoopAnim->setEnabled()
	ui->txtLoopVoice->setText(model->getCurLoopVoice());
	//ui->btnBrLoopVoice->setEnabled()

	//ui->btnErrorChecker->setEnabled()
	//ui->btnRealign->setEnabled()
	//ui->btnLoadMission->setEnabled()


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

	QAction *act = qobject_cast<QAction*>(sender());

	model = std::unique_ptr<CampaignEditorDialogModel>(new CampaignEditorDialogModel(this, _viewport, "", act ? act->text() : ""));
	updateUI();
}

void CampaignEditorDialog::fileOpen() {
	if (! questionSaveChanges())
		return;


	QString pathName = QFileDialog::getOpenFileName(this, tr("Load campaign"), model->campaignFile, tr("FS2 campaigns (*.fc2)"));

	auto newModel = std::unique_ptr<CampaignEditorDialogModel>(new CampaignEditorDialogModel(this, _viewport, pathName));
	if (newModel->isFileLoaded())
		model = std::move(newModel);
	else
		QMessageBox::information(this, tr("Error opening file"), pathName);

	updateUI();
}

bool CampaignEditorDialog::fileSave() {
	if (model->campaignFile.isEmpty())
		return fileSaveAs();

	bool res = model->apply();

	updateUI();
	return res;
}

bool CampaignEditorDialog::fileSaveAs() {
	QString pathName = QFileDialog::getSaveFileName(this, tr("Save campaign as"), model->campaignFile, tr("FS2 campaigns (*.fc2)"));
	if (pathName.isEmpty())
		return false;

	bool res = model->saveTo(pathName);
	if (res)
		model = std::unique_ptr<CampaignEditorDialogModel>(new CampaignEditorDialogModel(this, _viewport, pathName));

	updateUI();
	return res;
}

void CampaignEditorDialog::fileSaveCopyAs() {
	QString pathName = QFileDialog::getSaveFileName(this, tr("Save copy as"), model->campaignFile, tr("FS2 campaigns (*.fc2)"));
	if (pathName.isEmpty())
		return;

	model->saveTo(pathName);
}

void CampaignEditorDialog::txtNameChanged(const QString changed) {
	model->setCampaignName(changed);
}

void CampaignEditorDialog::chkTechResetChanged(const int changed) {
	model->setCampaignTechReset(changed == Qt::Checked);
}

void CampaignEditorDialog::txaDescrTextChanged() {
	model->setCampaignDescr(ui->txaDescr->toPlainText());
}

void CampaignEditorDialog::txtBriefingCutsceneChanged(const QString changed) {
	model->setCurMissionBriefingCutscene(changed);
}

void CampaignEditorDialog::txtMainhallChanged(const QString changed) {
	model->setCurMissionMainhall(changed);
}

void CampaignEditorDialog::txtDebriefingPersonaChanged(const QString changed) {
	model->setCurMissionDebriefingPersona(changed);
}

void CampaignEditorDialog::btnBranchUpClicked() {

}

void CampaignEditorDialog::btnBranchDownClicked() {

}

void CampaignEditorDialog::btnBranchLoopToggled(bool checked) {
	model->setCurBrIsLoop(checked);
}

void CampaignEditorDialog::txaLoopDescrChanged() {
	model->setCurLoopDescr(ui->txaLoopDescr->toPlainText());
}

void CampaignEditorDialog::txtLoopAnimChanged(const QString changed) {
	model->setCurLoopAnim(changed);
}

void CampaignEditorDialog::btnBrLoopAnimClicked() {

}

void CampaignEditorDialog::txtLoopVoiceChanged(const QString changed) {
	model->setCurLoopVoice(changed);
}

void CampaignEditorDialog::btnBrLoopVoiceClicked() {

}

void CampaignEditorDialog::btnErrorCheckerClicked() {

}

void CampaignEditorDialog::btnRealignClicked() {

}

void CampaignEditorDialog::btnLoadMissionClicked() {

}


}
}
}

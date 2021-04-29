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
	menubar(new QMenuBar),
	_parent(parent),
	_viewport(viewport)
{
	ui->setupUi(this);

	//TODO populate with constants

	connect(ui->listMissions, &QListWidget::itemActivated, this, &CampaignEditorDialog::listedMissionActivated);
	connect(ui->txtName, &QLineEdit::textChanged, this, &CampaignEditorDialog::txtNameChanged);
	connect(ui->cmbType, &QComboBox::currentTextChanged, this, &CampaignEditorDialog::cmbTypeChanged);
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

	QMenu *menFile = menubar->addMenu(tr("&File"));
	QAction *actFileNew = menFile->addAction(tr("&New"));
	actFileNew->setShortcut(tr("Ctrl+N"));
	connect(actFileNew, &QAction::triggered, this, &CampaignEditorDialog::fileNew);
	QAction *actFileOpen = menFile->addAction(tr("&Open..."));
	actFileOpen->setShortcut(tr("Ctrl+O"));
	connect(actFileOpen, &QAction::triggered, this, &CampaignEditorDialog::fileOpen);
	QAction *actFileSave = menFile->addAction(tr("&Save"));
	actFileSave->setShortcut(tr("Ctrl+S"));
	connect(actFileSave, &QAction::triggered, this, &CampaignEditorDialog::fileSave);
	QAction *actFileSaveAs = menFile->addAction(tr("Save &as..."));
	connect(actFileSaveAs, &QAction::triggered, this, &CampaignEditorDialog::fileSaveAs);
	QAction *actFileSaveCopyAs = menFile->addAction(tr("Save &Copy as..."));
	connect(actFileSaveCopyAs, &QAction::triggered, this, &CampaignEditorDialog::fileSaveCopyAs);
	menFile->addSeparator();
	QAction *actFileExit = menFile->addAction(tr("E&xit"));
	connect(actFileExit, &QAction::triggered, this, &QDialog::reject);

	ui->windowLayout->insertWidget(0, menubar.get());

	updateUI();
}

CampaignEditorDialog::~CampaignEditorDialog()
{
}

void CampaignEditorDialog::reject() {  //merely means onClose
	if (! questionSaveChanges())
		return;

	for (auto men : menubar->children())
		dynamic_cast<QWidget*>(men)->setVisible(false);
	QDialog::reject();
	deleteLater();
}

void CampaignEditorDialog::updateUI() {
	util::SignalBlockers blockers(this);

	QString file = model->getCurrentFile();
	this->setWindowTitle(file.isEmpty() ? "Untitled" : file);

	ui->txtName->setText(QString::fromStdString(model->getCampaignName()));
	ui->cmbType->setCurrentText(QString::fromStdString(model->getCampaignType()));
	ui->chkTechReset->setChecked(model->getCampaignTechReset());

	ui->txaDescr->setPlainText(QString::fromStdString(model->getCampaignDescr()));

	ui->txtBriefingCutscene->setText(QString::fromStdString(model->getCurMissionBriefingCutscene()));
	ui->txtMainhall->setText(QString::fromStdString(model->getCurMissionMainhall()));
	ui->txtDebriefingPersona->setText(QString::fromStdString(model->getCurMissionDebriefingPersona()));

	//ui->btnBranchUp->setEnabled()
	//ui->btnBranchDown->setEnabled()
	//ui->btnBranchLoop->setEnabled()

	ui->txaLoopDescr->setPlainText(QString::fromStdString(model->getCurLoopDescr()));
	ui->txtLoopAnim->setText(QString::fromStdString(model->getCurLoopAnim()));
	//ui->btnBrLoopAnim->setEnabled()
	ui->txtLoopVoice->setText(QString::fromStdString(model->getCurLoopVoice()));
	//ui->btnBrLoopVoice->setEnabled()

	//ui->btnErrorChecker->setEnabled()
	//ui->btnRealign->setEnabled()
	//ui->btnLoadMission->setEnabled()


}

bool CampaignEditorDialog::questionSaveChanges() {
	QMessageBox::StandardButton resBtn = QMessageBox::Discard;
	if (model->query_modified()) {
		resBtn = QMessageBox::question( this, tr("Unsaved changes"),
										tr("This campaign has been modified. Save changes?"),
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

	model = std::unique_ptr<CampaignEditorDialogModel>(new CampaignEditorDialogModel(this, _viewport));
	updateUI();
}

void CampaignEditorDialog::fileOpen() {
	if (! questionSaveChanges())
		return;
	QString oldFile = model->getCurrentFile();

	QString pathName = QFileDialog::getOpenFileName(this, tr("Load campaign"), oldFile, tr("FS2 campaigns (*.fc2)"));
	if (pathName.isEmpty())
		return;

	model->setCurrentFile(pathName);
	if (! model->loadCurrentFile())
		model->setCurrentFile(oldFile);
	updateUI();
}

bool CampaignEditorDialog::fileSave() {
	if (model->getCurrentFile().isEmpty())
		return fileSaveAs();

	bool res = model->apply();
	updateUI();
	return res;
}

bool CampaignEditorDialog::fileSaveAs() {
	QString oldFile = model->getCurrentFile();

	QString pathName = QFileDialog::getSaveFileName(this, tr("Save campaign as"), oldFile, tr("FS2 campaigns (*.fc2)"));
	if (pathName.isEmpty())
		return false;


	model->setCurrentFile(pathName);
	bool res = model->apply();
	if (! res)
		model->setCurrentFile(oldFile);
	updateUI();
	return res;
}

void CampaignEditorDialog::fileSaveCopyAs() {
	QString pathName = QFileDialog::getSaveFileName(this, tr("Save copy as"), model->getCurrentFile(), tr("FS2 campaigns (*.fc2)"));
	if (pathName.isEmpty())
		return;

	model->saveTo(pathName);
}

void CampaignEditorDialog::listedMissionActivated(const QListWidgetItem *item){
	QMessageBox::information(this, "", item->text());
	//TODO select current mission
}

void CampaignEditorDialog::txtNameChanged(const QString changed) {
	model->setCampaignName(changed.toStdString());
}

void CampaignEditorDialog::cmbTypeChanged(const QString changed) {
	model->setCampaignType(changed.toStdString());
}

void CampaignEditorDialog::chkTechResetChanged(const int changed) {
	model->setCampaignTechReset(changed == Qt::Checked);
}

void CampaignEditorDialog::txaDescrTextChanged() {
	model->setCampaignDescr(ui->txaDescr->toPlainText().toStdString());
}

void CampaignEditorDialog::txtBriefingCutsceneChanged(const QString changed) {
	model->setCurMissionBriefingCutscene(changed.toStdString());
}

void CampaignEditorDialog::txtMainhallChanged(const QString changed) {
	model->setCurMissionMainhall(changed.toStdString());
}

void CampaignEditorDialog::txtDebriefingPersonaChanged(const QString changed) {
	model->setCurMissionDebriefingPersona(changed.toStdString());
}

void CampaignEditorDialog::btnBranchUpClicked() {

}

void CampaignEditorDialog::btnBranchDownClicked() {

}

void CampaignEditorDialog::btnBranchLoopToggled(bool checked) {
	model->setCurBrIsLoop(checked);
}

void CampaignEditorDialog::txaLoopDescrChanged() {
	model->setCurLoopDescr(ui->txaLoopDescr->toPlainText().toStdString());
}

void CampaignEditorDialog::txtLoopAnimChanged(const QString changed) {
	model->setCurLoopAnim(changed.toStdString());
}

void CampaignEditorDialog::btnBrLoopAnimClicked() {

}

void CampaignEditorDialog::txtLoopVoiceChanged(const QString changed) {
	model->setCurLoopVoice(changed.toStdString());
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

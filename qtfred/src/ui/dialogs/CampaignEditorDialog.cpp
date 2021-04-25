#include "CampaignEditorDialog.h"
#include "ui_CampaignEditorDialog.h"

#include <QMessageBox>

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

	QMenu *menFile = menubar->addMenu("&File");
	QAction *actFileNew = menFile->addAction("&New");
	actFileNew->setShortcut(tr("Ctrl+N"));
	connect(actFileNew, &QAction::triggered, this, &CampaignEditorDialog::fileNew);
	QAction *actFileOpen = menFile->addAction("&Open...");
	actFileOpen->setShortcut(tr("Ctrl+O"));
	connect(actFileOpen, &QAction::triggered, this, &CampaignEditorDialog::fileOpen);
	QAction *actFileSave = menFile->addAction("&Save");
	actFileSave->setShortcut(tr("Ctrl+S"));
	connect(actFileSave, &QAction::triggered, this, &CampaignEditorDialog::fileSave);
	QAction *actFileSaveas = menFile->addAction("Save &As...");
	connect(actFileSaveas, &QAction::triggered, this, &CampaignEditorDialog::fileSaveAs);
	menFile->addSeparator();
	QAction *actFileExit = menFile->addAction("E&xit");
	connect(actFileExit, &QAction::triggered, this, &QDialog::reject);

	ui->windowLayout->insertWidget(0, menubar.get());
}

CampaignEditorDialog::~CampaignEditorDialog()
{
}

void CampaignEditorDialog::reject() {  //merely means onClose
	attemptClose();
}

bool CampaignEditorDialog::attemptClose() {
	QMessageBox::StandardButton resBtn = QMessageBox::No;
	if (model->query_modified()) {
		resBtn = QMessageBox::question( this, "",
										"This campaign has been modified. Save changes?",
										QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
										QMessageBox::Yes);
	}
	if (resBtn == QMessageBox::No)
		model->reject();
	if (resBtn == QMessageBox::Yes) {
		bool success = model->apply();
		QMessageBox::information(nullptr, "", success ? "Successfully saved" : "Error saving");
		if (!success)
			return false;
	}
	if (resBtn == QMessageBox::Yes || resBtn == QMessageBox::No) {
		for (auto men : menubar->children())
			dynamic_cast<QWidget*>(men)->setVisible(false);
		QDialog::reject();
		deleteLater();
		return true;
	}
	return false;
}

void CampaignEditorDialog::fileNew(){
	if (! attemptClose()) return;
	auto editorCampaign = new CampaignEditorDialog(_parent, _viewport);
	editorCampaign->show();
}

void CampaignEditorDialog::fileOpen(){

}

void CampaignEditorDialog::fileSave(){

}

void CampaignEditorDialog::fileSaveAs(){

}

void CampaignEditorDialog::listedMissionActivated(const QListWidgetItem *item){
	QMessageBox::information(this, "", item->text());
	model->modelChanged();
}

void CampaignEditorDialog::txtNameChanged(const QString changed){
	model->setCampaignName(changed.toStdString());
}

void CampaignEditorDialog::cmbTypeChanged(const QString changed){
	model->setCampaignType(changed.toStdString());
}

void CampaignEditorDialog::chkTechResetChanged(const int changed){
	model->setCampaignTechReset(changed == Qt::Checked);
}

void CampaignEditorDialog::txaDescrTextChanged(){
	model->setCampaignDescr(ui->txaDescr->toPlainText().toStdString());
}

void CampaignEditorDialog::txtBriefingCutsceneChanged(const QString changed){
	model->setCurMissionBriefingCutscene(changed.toStdString());
}

void CampaignEditorDialog::txtMainhallChanged(const QString changed){
	model->setCurMissionMainhall(changed.toStdString());
}

void CampaignEditorDialog::txtDebriefingPersonaChanged(const QString changed){
	model->setCurMissionDebriefingPersona(changed.toStdString());
}

void CampaignEditorDialog::btnBranchUpClicked(){

}

void CampaignEditorDialog::btnBranchDownClicked(){

}

void CampaignEditorDialog::btnBranchLoopToggled(bool checked){
	model->setCurBrIsLoop(checked);
}

void CampaignEditorDialog::txaLoopDescrChanged(){
	model->setCurLoopDescr(ui->txaLoopDescr->toPlainText().toStdString());
}

void CampaignEditorDialog::txtLoopAnimChanged(const QString changed){
	model->setCurLoopAnim(changed.toStdString());
}

void CampaignEditorDialog::btnBrLoopAnimClicked(){

}

void CampaignEditorDialog::txtLoopVoiceChanged(const QString changed){
	model->setCurLoopVoice(changed.toStdString());
}

void CampaignEditorDialog::btnBrLoopVoiceClicked(){

}

void CampaignEditorDialog::btnErrorCheckerClicked(){

}

void CampaignEditorDialog::btnRealignClicked(){

}

void CampaignEditorDialog::btnLoadMissionClicked(){

}


}
}
}

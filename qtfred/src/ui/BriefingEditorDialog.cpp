#include "BriefingEditorDialog.h"
#include "ui_briefingeditordialog.h"
#include <QtWidgets/QMenuBar>

namespace fso {
namespace fred {
namespace dialogs {

BriefingEditorDialog::BriefingEditorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BriefingEditorDialog)
{
    ui->setupUi(this);

    QMenuBar *menubar = new QMenuBar();
    QMenu *teamMenu = new QMenu("Select Team");
    teamMenu->addAction("Team 1");
    teamMenu->addAction("Team 2");
    QMenu *optionsMenu = new QMenu("Options");
    optionsMenu->addAction("Balance Teams");
    menubar->addMenu(teamMenu);
    menubar->addMenu(optionsMenu);

    ui->mainLayout->insertWidget(0,menubar);
}

BriefingEditorDialog::~BriefingEditorDialog()
{
    delete ui;
}

}
}
}

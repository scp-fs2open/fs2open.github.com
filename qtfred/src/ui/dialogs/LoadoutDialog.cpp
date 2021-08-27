#include "TeamLoadoutDialog.h"
#include "ui_TeamLoadoutDialog.h"

#include <QtWidgets/QMenuBar>

namespace fso {
namespace fred {
namespace dialogs {

TeamLoadoutDialog::TeamLoadoutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TeamLoadoutDialog)
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

    /*
     * Note: For the weapon and ship loadout listWidgest, will need to do something similar to the following during loading to
     * add each ship and weapon option as a checkbox
    QStringList  itemLabels= getLabels();

    QStringListIterator it(itemLabels);
    while (it.hasNext())
    {
          QListWidgetItem *listItem = new QListWidgetItem(it.next(),listWidget);
          listItem->setCheckState(Qt::Unchecked);
          ui->listWidget->addItem(listItem);
    }
    */
}

TeamLoadoutDialog::~TeamLoadoutDialog()
{
    delete ui;
}

}
}
}

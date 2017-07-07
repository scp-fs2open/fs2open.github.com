#include "MissionGoalsDialog.h"
#include "ui_MissionGoalsDialog.h"

namespace fso {
namespace fred {
namespace dialogs {

MissionGoalsDialog::MissionGoalsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MissionGoalsDialog)
{
    ui->setupUi(this);
}

MissionGoalsDialog::~MissionGoalsDialog()
{
    delete ui;
}

}
}
}

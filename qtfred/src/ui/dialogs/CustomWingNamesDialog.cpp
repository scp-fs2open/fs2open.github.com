#include "CustomWingNamesDialog.h"
#include "ui_CustomWingNamesDialog.h"

namespace fso {
namespace fred {
namespace dialogs {

CustomWingNamesDialog::CustomWingNamesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CustomWingNamesDialog)
{
    ui->setupUi(this);
}

CustomWingNamesDialog::~CustomWingNamesDialog()
{
    delete ui;
}

}
}
}

#ifndef CUSTOMWINGNAMESDIALOG_H
#define CUSTOMWINGNAMESDIALOG_H

#include <QDialog>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class CustomWingNamesDialog;
}

class CustomWingNamesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CustomWingNamesDialog(QWidget *parent = 0);
    ~CustomWingNamesDialog();

private:
    Ui::CustomWingNamesDialog *ui;
};

}
}
}

#endif // CUSTOMWINGNAMESDIALOG_H

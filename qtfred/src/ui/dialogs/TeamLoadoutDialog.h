#ifndef TEAMLOADOUTDIALOG_H
#define TEAMLOADOUTDIALOG_H

#include <QDialog>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class TeamLoadoutDialog;
}

class TeamLoadoutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TeamLoadoutDialog(QWidget *parent = 0);
    ~TeamLoadoutDialog();

private:
    Ui::TeamLoadoutDialog *ui;
};

}
}
}

#endif // TEAMLOADOUTDIALOG_H

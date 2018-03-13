#ifndef MISSIONGOALSDIALOG_H
#define MISSIONGOALSDIALOG_H

#include <QDialog>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class MissionGoalsDialog;
}

class MissionGoalsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MissionGoalsDialog(QWidget *parent = 0);
    ~MissionGoalsDialog();

private:
    Ui::MissionGoalsDialog *ui;
};

}
}
}

#endif // MISSIONGOALSDIALOG_H

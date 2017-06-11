#ifndef BRIEFINGEDITORDIALOG_H
#define BRIEFINGEDITORDIALOG_H

#include <QDialog>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class BriefingEditorDialog;
}

class BriefingEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BriefingEditorDialog(QWidget *parent = 0);
    ~BriefingEditorDialog();

private:
    Ui::BriefingEditorDialog *ui;
};

}
}
}

#endif // BRIEFINGEDITORDIALOG_H

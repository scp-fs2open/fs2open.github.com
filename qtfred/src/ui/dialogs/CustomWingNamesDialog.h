#ifndef CUSTOMWINGNAMESDIALOG_H
#define CUSTOMWINGNAMESDIALOG_H

#include <QDialog>
#include <QCloseEvent>

#include <ui/FredView.h>

#include "mission/dialogs/CustomWingNamesDialogModel.h"

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
    explicit CustomWingNamesDialog(QWidget* parent, EditorViewport* viewport);
    ~CustomWingNamesDialog();

protected:
	void closeEvent(QCloseEvent*) override;

private:
    std::unique_ptr<Ui::CustomWingNamesDialog> ui;
	std::unique_ptr<CustomWingNamesDialogModel> _model;
	EditorViewport* _viewport;

	void updateUI();

	void startingWingChanged(const QString &, int);
	void squadronWingChanged(const QString &, int);
	void dogfightWingChanged(const QString &, int);
};

}
}
}

#endif // CUSTOMWINGNAMESDIALOG_H

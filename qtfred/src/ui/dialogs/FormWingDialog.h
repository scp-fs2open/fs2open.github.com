#pragma once

#include <QDialog>
#include <QCloseEvent>

#include "ui/FredView.h"

#include "mission/dialogs/FormWingDialogModel.h"
#include "mission/IDialogProvider.h"

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class FormWingDialog;
}

class FormWingDialog: public QDialog, public IDialog<FormWingDialogModel> {
 Q_OBJECT

 public:
	explicit FormWingDialog(QWidget* parent, EditorViewport* viewport);
	~FormWingDialog();

 private:
	FormWingDialogModel* getModel() override;

 private:
	std::unique_ptr<Ui::FormWingDialog> ui;
	std::unique_ptr<FormWingDialogModel> _model;

	void updateUI();

	void nameTextChanged(const QString& newText);
};

}
}
}

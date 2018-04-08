#pragma once

#include <QtWidgets/QDialog>

#include <ui/FredView.h>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class ShieldSystemDialog;
}

class ShieldSystemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShieldSystemDialog(FredView* parent, EditorViewport* viewport);
	// TODO shouldn't all QDialog subclasses have a virtual destructor?
    virtual ~ShieldSystemDialog();

private:
    std::unique_ptr<Ui::ShieldSystemDialog> ui;
	//std::unique_ptr<ShieldSystemDialogModel> _model;
	EditorViewport* _viewport;	
};

}
}
}

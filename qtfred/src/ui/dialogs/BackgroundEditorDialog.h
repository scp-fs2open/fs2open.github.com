#pragma once

#include <QtWidgets/QDialog>

#include <ui/FredView.h>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class BackgroundEditor;
}

class BackgroundEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BackgroundEditorDialog(FredView* parent, EditorViewport* viewport);
	// TODO shouldn't all QDialog subclasses have a virtual destructor?
    virtual ~BackgroundEditorDialog();

private:
    std::unique_ptr<Ui::BackgroundEditor> ui;
	//std::unique_ptr<BackgroundEditorDialogModel> _model;
	EditorViewport* _viewport;	
};

}
}
}

#pragma once

#include <QtWidgets/QDialog>

#include <ui/FredView.h>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class VoiceActingManager;
}

class VoiceActingManager : public QDialog
{
    Q_OBJECT

public:
    explicit VoiceActingManager(FredView* parent, EditorViewport* viewport);
	// TODO shouldn't all QDialog subclasses have a virtual destructor?
    virtual ~VoiceActingManager();

private:
    std::unique_ptr<Ui::VoiceActingManager> ui;
	//std::unique_ptr<VoiceActingManagerModel> _model;
	EditorViewport* _viewport;	
};

}
}
}

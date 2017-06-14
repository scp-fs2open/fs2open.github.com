#pragma once

#include <QtCore/QObject>

#include <mission/Editor.h>
#include <mission/EditorViewport.h>

namespace fso {
namespace fred {
namespace dialogs {

class AbstractDialogModel : public QObject {
	Q_OBJECT

 protected:
	Editor* _editor = nullptr;
	EditorViewport* _viewport = nullptr;

 public:
	AbstractDialogModel(QObject* parent, EditorViewport* viewport);
};

}
}
}


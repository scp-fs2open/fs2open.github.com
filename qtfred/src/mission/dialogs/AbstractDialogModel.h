#pragma once

#include <QtCore/QObject>

#include <mission/Editor.h>

namespace fso {
namespace fred {
namespace dialogs {

class AbstractDialogModel : public QObject {
	Q_OBJECT

 protected:
	Editor* _editor = nullptr;
	FredRenderer* _renderer = nullptr;

 public:
	AbstractDialogModel(QObject* parent, Editor* editor, FredRenderer* renderer);
};

}
}
}


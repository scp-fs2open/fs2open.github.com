#pragma once

#include <QtCore/QObject>

#include <mission/Editor.h>
#include <mission/EditorViewport.h>

namespace fso {
namespace fred {
namespace dialogs {

class AbstractDialogModel: public QObject {
 Q_OBJECT

 protected:
	Editor* _editor = nullptr;
	EditorViewport* _viewport = nullptr;

 public:
	AbstractDialogModel(QObject* parent, EditorViewport* viewport);

	virtual ~AbstractDialogModel() {
	}

	virtual bool apply() = 0;
	virtual void reject() = 0;

 signals:
	/**
	 * @brief Signal emitted when the model has changed caused by an update operation
	 *
	 * This should be used for updating the view
	 */
	void modelChanged();
};

}
}
}


#pragma once

#include <QtCore/QObject>

#include <mission/Editor.h>
#include <mission/EditorViewport.h>

namespace fso {
namespace fred {
namespace dialogs {

/**
 * @brief Abstract class for a generic dialog model.
 *
 * Each editor dialog should have a subclass of this class which should be responsible for handling the editor logic.
 *
 * @c modelChanged() can be used for updating the UI when a model action caused a data change in the model.
 *
 * @c apply and @c reject should be connected to the appropriate signals from the @c QDialog class.
 */
class AbstractDialogModel: public QObject {
 Q_OBJECT

 protected:
	Editor* _editor = nullptr;
	EditorViewport* _viewport = nullptr;

 public:
	AbstractDialogModel(QObject* parent, EditorViewport* viewport);

	~AbstractDialogModel() override {
	}

	/**
	 * @brief Apply the changes currently stored in this model
	 * @return @c true if the operation was successful and the dialog can be closed.
	 */
	virtual bool apply() = 0;

	/**
	 * @brief Rejects the current changes and resets the mission to previous values.
	 */
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


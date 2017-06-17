//
//

#include "AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {


AbstractDialogModel::AbstractDialogModel(QObject* parent,
										 Editor* editor,
										 EditorViewport* viewport) :
	QObject(parent), _editor(editor), _viewport(viewport) {
}

}
}
}

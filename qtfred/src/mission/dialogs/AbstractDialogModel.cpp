//
//

#include "AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {


AbstractDialogModel::AbstractDialogModel(QObject* parent,
										 EditorViewport* viewport) :
	QObject(parent), _editor(viewport->editor), _viewport(viewport) {
}

}
}
}

//
//

#include "AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {


AbstractDialogModel::AbstractDialogModel(QObject* parent,
										 fso::fred::Editor* editor,
										 fso::fred::FredRenderer* renderer) :
	QObject(parent), _editor(editor), _renderer(renderer) {
}

}
}
}

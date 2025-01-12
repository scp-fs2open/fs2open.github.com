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

bool AbstractDialogModel::query_modified() const
{
	return _modified;
}

void AbstractDialogModel::set_modified()
{
	if (!_modified) {
		_modified = true;
	}
}

}
}
}

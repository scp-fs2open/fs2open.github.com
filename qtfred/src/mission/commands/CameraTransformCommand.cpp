#include "CameraTransformCommand.h"

#include <mission/CameraController.h>

namespace fso::fred {


CameraTransformCommand::CameraTransformCommand(CameraController* camera,
                                               const vec3d& posBefore, const matrix& orientBefore,
                                               const vec3d& posAfter,  const matrix& orientAfter,
                                               QUndoCommand* parent)
    : QUndoCommand(QObject::tr("View Change"), parent),
      _camera(camera),
      _posBefore(posBefore), _orientBefore(orientBefore),
      _posAfter(posAfter),   _orientAfter(orientAfter)
{
}

void CameraTransformCommand::undo()
{
	_camera->view_pos    = _posBefore;
	_camera->view_orient = _orientBefore;
}

void CameraTransformCommand::redo()
{
	_camera->view_pos    = _posAfter;
	_camera->view_orient = _orientAfter;
}

int CameraTransformCommand::id() const
{
	return -1;  // -1 disables merging; the idle timer already produces one command per gesture
}

bool CameraTransformCommand::mergeWith(const QUndoCommand*)
{
	return false;
}

} // namespace fso::fred

#pragma once

#include <QUndoCommand>
#include <globalincs/pstypes.h>
#include <math/vecmat.h>

namespace fso::fred {

class CameraController;

class CameraTransformCommand : public QUndoCommand {
	CameraController* _camera;
	vec3d  _posBefore;
	matrix _orientBefore;
	vec3d  _posAfter;
	matrix _orientAfter;

public:
	CameraTransformCommand(CameraController* camera,
	                       const vec3d& posBefore, const matrix& orientBefore,
	                       const vec3d& posAfter,  const matrix& orientAfter,
	                       QUndoCommand* parent = nullptr);

	void undo() override;
	void redo() override;
	bool mergeWith(const QUndoCommand* other) override;
	int  id() const override;
};

} // namespace fso::fred

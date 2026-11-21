// captureState() and restoreState() for ObjectOrientEditorDialogModel.
// Snapshots the position and orientation of every currently-marked object
// so that undo/redo can restore them without going through apply().

#include <mission/dialogs/ObjectOrientEditorDialogModel.h>

#include <globalincs/linklist.h>
#include <mission/object.h>
#include <object/object.h>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

namespace fso::fred::dialogs {

QByteArray ObjectOrientEditorDialogModel::captureState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	// Walk the live object list and snapshot every marked object by signature
	// (stable across undo/redo pool changes).
	SCP_vector<int> markedNums;
	for (auto ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->flags[Object::Object_Flags::Marked])
			markedNums.push_back(OBJ_INDEX(ptr));
	}

	ds << static_cast<qint32>(static_cast<int>(markedNums.size()));
	for (int objNum : markedNums) {
		const object& obj = Objects[objNum];
		ds << static_cast<qint32>(obj.signature);
		ds << obj.pos.xyz.x << obj.pos.xyz.y << obj.pos.xyz.z;
		ds << obj.orient.vec.rvec.xyz.x << obj.orient.vec.rvec.xyz.y << obj.orient.vec.rvec.xyz.z;
		ds << obj.orient.vec.uvec.xyz.x << obj.orient.vec.uvec.xyz.y << obj.orient.vec.uvec.xyz.z;
		ds << obj.orient.vec.fvec.xyz.x << obj.orient.vec.fvec.xyz.y << obj.orient.vec.fvec.xyz.z;
	}

	return data;
}

void ObjectOrientEditorDialogModel::restoreState(const QByteArray& state)
{
	QDataStream ds(state);

	qint32 count;
	ds >> count;

	for (int i = 0; i < count; i++) {
		qint32 sig;
		float px, py, pz;
		float r0, r1, r2, u0, u1, u2, f0, f1, f2;

		ds >> sig;
		ds >> px >> py >> pz;
		ds >> r0 >> r1 >> r2;
		ds >> u0 >> u1 >> u2;
		ds >> f0 >> f1 >> f2;

		const int objNum = obj_get_by_signature(static_cast<int>(sig));
		if (objNum < 0) continue;

		object& obj = Objects[objNum];
		vm_vec_make(&obj.pos, px, py, pz);
		vm_vec_make(&obj.orient.vec.rvec, r0, r1, r2);
		vm_vec_make(&obj.orient.vec.uvec, u0, u1, u2);
		vm_vec_make(&obj.orient.vec.fvec, f0, f1, f2);
		object_moved(&obj);
	}
}

// ---------------------------------------------------------------------------
// Working-state capture/restore for the in-dialog undo stack: the edit
// fields, modes, and the relative-mode rebase baseline.
// ---------------------------------------------------------------------------

QByteArray ObjectOrientEditorDialogModel::captureWorkingState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	ds << _position.xyz.x << _position.xyz.y << _position.xyz.z;
	ds << _orientationDeg.xyz.x << _orientationDeg.xyz.y << _orientationDeg.xyz.z;
	ds << _location.xyz.x << _location.xyz.y << _location.xyz.z;
	ds << _rebaseRefPos.xyz.x << _rebaseRefPos.xyz.y << _rebaseRefPos.xyz.z;
	ds << _rebaseRefAnglesDeg.xyz.x << _rebaseRefAnglesDeg.xyz.y << _rebaseRefAnglesDeg.xyz.z;
	ds << static_cast<qint32>(_setMode);
	ds << static_cast<qint32>(_transformMode);
	ds << static_cast<qint32>(_pointMode);
	ds << static_cast<quint8>(_pointTo ? 1 : 0);
	ds << static_cast<qint32>(_selectedPointToObjectIndex);

	return data;
}

void ObjectOrientEditorDialogModel::restoreWorkingState(const QByteArray& state)
{
	QDataStream ds(state);

	qint32 setMode, transformMode, pointMode, pointObject;
	quint8 pointTo;
	ds >> _position.xyz.x >> _position.xyz.y >> _position.xyz.z;
	ds >> _orientationDeg.xyz.x >> _orientationDeg.xyz.y >> _orientationDeg.xyz.z;
	ds >> _location.xyz.x >> _location.xyz.y >> _location.xyz.z;
	ds >> _rebaseRefPos.xyz.x >> _rebaseRefPos.xyz.y >> _rebaseRefPos.xyz.z;
	ds >> _rebaseRefAnglesDeg.xyz.x >> _rebaseRefAnglesDeg.xyz.y >> _rebaseRefAnglesDeg.xyz.z;
	ds >> setMode >> transformMode >> pointMode >> pointTo >> pointObject;

	_setMode                    = static_cast<SetMode>(setMode);
	_transformMode              = static_cast<TransformMode>(transformMode);
	_pointMode                  = static_cast<PointToMode>(pointMode);
	_pointTo                    = (pointTo != 0);
	_selectedPointToObjectIndex = static_cast<int>(pointObject);

	set_modified();
}

} // namespace fso::fred::dialogs

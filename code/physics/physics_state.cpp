#include "physics/physics_state.h"

// There are additional functions in object.cpp

void physics_interpolate_snapshots(physics_snapshot& result, const physics_snapshot& before, const physics_snapshot& after, const float percent) 
{
	// just interpolate each item in turn.
	vm_vec_linear_interpolate(&result.position, &before.position, &after.position, percent);
	vm_vec_linear_interpolate(&result.velocity, &before.velocity, &after.velocity, percent);
	vm_vec_linear_interpolate(&result.desired_velocity, &before.desired_velocity, &after.desired_velocity, percent);
	vm_vec_linear_interpolate(&result.rotational_velocity, &before.rotational_velocity, &after.rotational_velocity, percent);
	vm_vec_linear_interpolate(&result.desired_rotational_velocity, &before.desired_rotational_velocity, &after.desired_rotational_velocity, percent);
	vm_interpolate_matrices(&result.orientation, &before.orientation, &after.orientation, percent);
}

void physics_apply_snapshot_manual(vec3d& position, matrix& orient, vec3d& velocity, vec3d& desired_velocity, const physics_snapshot& source)
{
	position = source.position;
	orient = source.orientation;
	velocity = source.velocity;
	desired_velocity = source.desired_velocity;
}

void physics_apply_snapshot_manual(vec3d& position, matrix& orient, vec3d& velocity, vec3d& desired_velocity, 
	vec3d& rotational_velocity, vec3d& desired_rotational_velocity, const physics_snapshot& source)
{
	position = source.position;
	orient = source.orientation;
	velocity = source.velocity;
	desired_velocity = source.desired_velocity;
	rotational_velocity = source.rotational_velocity;
	desired_rotational_velocity = source.desired_rotational_velocity;
}

void physics_populate_snapshot_manual(physics_snapshot& destination, const vec3d& position, const matrix& orient, const vec3d& velocity, const vec3d& desired_velocity, 
	const vec3d& rotational_velocity, const vec3d& desired_rotational_velocity)
{
	destination.position = position;
	destination.orientation = orient;
	destination.velocity = velocity;
	destination.desired_velocity = desired_velocity;
	destination.rotational_velocity = rotational_velocity;
	destination.desired_rotational_velocity = desired_rotational_velocity;
}

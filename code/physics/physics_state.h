#pragma once

#include "globalincs/pstypes.h"
#include "math/vecmat.h"

// This is designed to be a small API that generalizes the FSO physics state of an object
// and allows interpolation between two states.  Timing is not handled here however.

struct physics_snapshot {
	vec3d position;						// the position at that moment.
	matrix orientation;					// the orientation at that moment.

	vec3d velocity;						// the velocity at that moment.
	vec3d desired_velocity;				// the desired velocity at that moment.

	vec3d rotational_velocity;			// the rotational velocity at that moment.
	vec3d desired_rotational_velocity;	// the desired rotational velocity at that moment.
};

/**
 * @brief Interpolate two physics snapshots and get the result
 *
 * @param[out] 	result Destination of the interpolated info
 * @param[in]	before The physics information that happened earlier in the simulation
 * @param[in]	after The physics information that happened later in the simulation
 * @param[in]	percent How much time has passed between the two states, in percent. 
 * 		0.00 means use only before, 1.00 means use only after, 0.50 would interpolate halfway between them.
 *
 * @details This takes two physics snapshots of objects and uses linear interpolation to figure out
 * 		where the objects would be given a specific time factor (percent "progress" towards after). 
 * 
 * @author J Fernandez
 */
void physics_interpolate_snapshots(physics_snapshot& result, const physics_snapshot& before, const physics_snapshot& after, const float percent);

/**
 * @brief Apply the contents of a physics state to manually specified variables
 *
 * @param[out] 	position Destination of the snapshot's position values
 * @param[out] 	orient Destination of the snapshot's orientation values
 * @param[out] 	velocity Destination of the snapshot's velocity values
 * @param[out] 	desired_velocity Destination of the snapshot's desired velocity
 * @param[in]	source Physics object that we are copying the values from
 * 
 * @details Apply a physics state to manually specified vectors and matrices.
 * 		This overload does not include rotational velocity or desired rotational velocity.
 * 
 * @author J Fernandez
 */
void physics_apply_snapshot_manual(vec3d& position, matrix& orient, vec3d& velocity, vec3d& desired_velocity, const physics_snapshot& source);

/**
 * @brief Apply the contents of a physics state to manually specified variables
 *
 * @param[out] 	position Destination of the snapshot's position values
 * @param[out] 	orient Destination of the snapshot's orientation values
 * @param[out] 	velocity Destination of the snapshot's velocity values
 * @param[out] 	desired_velocity Destination of the snapshot's desired velocity
 * @param[out] 	rotational_velocity Destination of the snapshot's rotational velocity
 * @param[out] 	desired_rotaional_velocity Destination of the snapshot's desired rotational velocity
 * @param[in]	source Physics object that we are copying the values from
 * 
 * @details Apply a physics state to manually specified vectors and matrices.
 * 
 * @author J Fernandez
 */
void physics_apply_snapshot_manual(vec3d& position, matrix& orient, vec3d& velocity, vec3d& desired_velocity, vec3d& rotational_velocity, vec3d& desired_rotational_velocity, const physics_snapshot& source);

/**
 * @brief Populate a physics snapshot by specifying each input vector and matrix
 *
 * @param[out]	destination Destination physics snapshot
 * @param[in]	position Position vector for populating the snapshot
 * @param[in]	orient Matrix orientation for populating the snapshot
 * @param[in]	velocity Velocity vector for populating the snapshot
 * @param[in]	desired_velocity The desired velocity vector for populating the snapshot
 * @param[in]	rotational_velocity The rotational velocity vector for populating the snapshot
 * @param[in]	desired_rotational_velocity The desired rotational velocity vector for populating the snapshot
 *
 * @author J Fernandez
 */
void physics_populate_snapshot_manual(physics_snapshot& destination, const vec3d& position, const matrix& orient, const vec3d& velocity, const vec3d& desired_velocity, const vec3d& rotational_velocity, const vec3d& desired_rotational_velocity);

#include "math/ik_solver.h"

ik_solver_fabrik::ik_solver_fabrik(unsigned int maxIterations, float targetDistance, float minProgress)
	: m_maxIterations(maxIterations), m_targetDistance(targetDistance), m_minProgress(minProgress) { }

void ik_solver_fabrik::solve(const vec3d& targetPos, const matrix* targetOrient) {
	//Implementation of the paper "FABRIK: A fast iterative solver for the Inverse Kinematics problem"
	//http://andreasaristidou.com/publications/papers/FABRIK.pdf
	
	Assertion(m_nodes.size() > 1, "Node-list for IK does not contain at least two nodes.");
	
	//Calculate distances and set up nodes
	for(size_t i = 1; i < m_nodes.size(); ++i){
		//This assumes that m_nodes[n] is _always_ a direct child of m_nodes[n-1]
		m_nodes[i - 1].distance = vm_vec_mag(&m_nodes[i].submodel->offset);
		m_nodes[i].calculatedPos = m_nodes[i - 1].calculatedPos + m_nodes[i].submodel->offset;
	}
	
	//Abort condition counter
	unsigned int iterationCounter = 0;
	float endEffectorDistance = FLT_MAX;
	float endEffectorDistanceDelta = FLT_MAX;
	
	while(iterationCounter++ < m_maxIterations && endEffectorDistance > m_targetDistance && endEffectorDistanceDelta > m_minProgress) {
		endEffectorDistanceDelta = endEffectorDistance;

		for(auto& node : m_nodes)
			node.lastPos = node.calculatedPos;
		
		m_nodes.back().calculatedPos = targetPos;
		
		//Backwards pass
		for(size_t i = m_nodes.size() - 2; i != static_cast<size_t>(-1); --i) {
			auto& child = m_nodes[i + 1];
			auto& parent = m_nodes[i];
			
			// Find new position for parent
			float localDistance = vm_vec_dist(&parent.calculatedPos, &child.calculatedPos);
			float lambda = parent.distance / localDistance;

			parent.calculatedPos *= lambda;
			parent.calculatedPos += child.calculatedPos * (1.0f - lambda);

			// Find the parent's global rotation based on parent-child combination
			parent.calculateGlobalRotation(child);
			
			if(i == m_nodes.size() - 2){
				//Now that we know where the first parent sits, we can update the end effector rotation.
				//If we have a target rotation, point to it, otherwise take its parent's rotation
				child.calculatedRot = targetOrient == nullptr ? parent.calculatedRot : *targetOrient;
			}
			
			// Find local rotation of child for constraint
			matrix localRot;
			vm_copy_transpose(&localRot, &parent.calculatedRot);
			vm_matrix_x_matrix(&localRot, &child.calculatedRot, &localRot);

			// Check if constraint requires change of local rotation
			if(child.constraint->constrain(localRot, true)){
				// Reposition parent to fulfill child local rotation constraint by changing parent global rotation
				
				// Convert new local rotation of child and global rotation of child to new parent global rotation
				vm_transpose(&localRot);
				vm_matrix_x_matrix(&parent.calculatedRot, &localRot, &child.calculatedRot);

				// Find new parent position from new parent global rotation, child offset and child position
				vm_vec_unrotate(&parent.calculatedPos, &child.submodel->offset, &parent.calculatedRot);
				parent.calculatedPos *= -1;
				parent.calculatedPos += child.calculatedPos;
			}
		}
		
		//Technically the base node would now need to be rotation constrained, but since the base position is reset anyways for the forwards pass with a fixed base, this has no effect
		
		m_nodes.front().calculatedPos = ZERO_VECTOR;
		m_nodes.front().calculatedRot = IDENTITY_MATRIX;

		//Forwards pass
		for(size_t i = 0; i < m_nodes.size() - 1; ++i) {
			auto& child = m_nodes[i + 1];
			auto& parent = m_nodes[i];
			
			// Find new position for child
			float localDistance = vm_vec_dist(&parent.calculatedPos, &child.calculatedPos);
			float lambda = parent.distance / localDistance;

			child.calculatedPos *= lambda;
			child.calculatedPos += parent.calculatedPos * (1.0f - lambda);
			
			// Find the parent's global rotation based on parent-child combination
			parent.calculateGlobalRotation(child);
			
			// Find local rotation of parent for constraint
			const matrix& parentRot = i == 0 ? vmd_identity_matrix : m_nodes[i - 1].calculatedRot;
			matrix localRot;
			vm_copy_transpose(&localRot, &parentRot);
			vm_matrix_x_matrix(&localRot, &parent.calculatedRot, &localRot);
			
			// Check if constraint requires change of local rotation
			if(parent.constraint->constrain(localRot)){
				// Reposition child to fulfill parent local rotation constraint by changing parent global rotation
				
				// Convert new local rotation and global rotation of tha parent's parent to new parent global rotation
				vm_matrix_x_matrix(&parent.calculatedRot, &localRot, &parentRot);
				
				// Find new child position from new parent global rotation, child offset and parent position
				vm_vec_unrotate(&child.calculatedPos, &child.submodel->offset, &parent.calculatedRot);
				child.calculatedPos += parent.calculatedPos;
			}
		}
		
		//After positioning the end effector, it now needs to be oriented correctly. If we have a target orientation use it, otherwise assume its parents rotation. Then constrain.
		{
			const matrix& parentRot = m_nodes[m_nodes.size() - 2].calculatedRot;
			// Find end effector target
			m_nodes.back().calculatedRot = targetOrient == nullptr ? parentRot : *targetOrient;
			
			// Calculate end effector local rotation
			matrix localRot;
			vm_copy_transpose(&localRot, &parentRot);
			vm_matrix_x_matrix(&localRot, &m_nodes.back().calculatedRot, &localRot);
			
			// Constrain
			if(m_nodes.back().constraint->constrain(localRot)) {
				// Calculate new global end effector rotation
				vm_matrix_x_matrix(&localRot, &localRot, &parentRot);
				m_nodes.back().calculatedRot = localRot;
			}
		}
		
		// Calculate abort conditions
		endEffectorDistance = vm_vec_dist(&targetPos, &m_nodes.back().calculatedPos);
		endEffectorDistanceDelta -= endEffectorDistance;
	}
	
	if (endEffectorDistanceDelta < 0.0f) {
		// Solution got worse, likely a singularity from hinges or something. Restore last good solution
		for(auto& node : m_nodes)
			node.calculatedPos = node.lastPos;

		for(size_t i = 0; i < m_nodes.size() - 1; ++i) {
			m_nodes[i].calculateGlobalRotation(m_nodes[i + 1]);
		}
	}
	
	//Localize rotations
	for(size_t i = m_nodes.size() - 1; i > 0; --i) {
		matrix parent;
		vm_copy_transpose(&parent, &m_nodes[i - 1].calculatedRot);
		vm_matrix_x_matrix(&m_nodes[i].calculatedRot, &parent, &m_nodes[i].calculatedRot);
	}
}

void ik_node::calculateGlobalRotation(const ik_node& child) {
	vec3d rotax;
	vec3d n_off;
	vec3d n_diff = child.calculatedPos - calculatedPos;
	vm_vec_copy_normalize(&n_off, &child.submodel->offset);
	vm_vec_normalize(&n_diff);
	
	vec3d checkSafeTransform = n_diff - n_off;
	if(vm_vec_mag(&checkSafeTransform) <= 0.001){
		calculatedRot = IDENTITY_MATRIX;
		return;
	}
	
	vm_vec_cross(&rotax, &n_off, &n_diff);
	vm_vec_normalize(&rotax);

	vm_quaternion_rotate(&calculatedRot, acosf(vm_vec_dot(&n_diff, &n_off)), &rotax);
}

static constexpr float angles::*pbh[] = { &angles::p, &angles::b, &angles::h };

ik_constraint_hinge::ik_constraint_hinge(const vec3d& _axis) : axis(_axis) { }

bool ik_constraint_hinge::constrain(matrix& localRot, bool /*backwardsPass*/) const {
	float angle;
	
	float distance = vm_closest_angle_to_matrix(&localRot, &axis, &angle);
	
	// Tolerance for off-axis.
	if(distance < fl_radians(0.5f)){
		return false;
	}

	/*if(backwardsPass){
		 TODO: If Hinge joints cause singularities in FABRIK, this might need random perturbations on the backwards pass
	}*/
	
	vm_quaternion_rotate(&localRot, angle, &axis);
	
	return true;
}

ik_constraint_window::ik_constraint_window(const angles& _absLimit) : absLimit(_absLimit) { }

bool ik_constraint_window::constrain(matrix& localRot, bool /*backwardsPass*/) const {
	//Convert to angles
	angles currentAngles;
	vm_extract_angles_matrix_alternate(&currentAngles, &localRot);
	
	bool needsClamp = false;
	
	//Clamp absolute value of individual angles to window
	for (float angles::* i : pbh) {
		const float absAngle = abs(currentAngles.*i);
		if(absAngle > absLimit.*i){
			needsClamp = true;
			currentAngles.*i = copysignf(std::min(absAngle, absLimit.*i), currentAngles.*i);
		}
	}
	
	//If any axis needed clamping, recalc the local rotation
	if(needsClamp){
		vm_angles_2_matrix(&localRot, &currentAngles);
	}
	
	return needsClamp;
}

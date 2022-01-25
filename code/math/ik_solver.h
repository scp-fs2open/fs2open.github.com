#pragma once

#include "math/vecmat.h"
#include "model/model.h"

class ik_constraint;

struct ik_node{
	const bsp_info* submodel;
	const ik_constraint* constraint;
	
	vec3d calculatedPos = ZERO_VECTOR;
	vec3d lastPos = ZERO_VECTOR;
	matrix calculatedRot = IDENTITY_MATRIX;
	float distance = 0.0f;
	
	ik_node(const bsp_info* _submodel, const ik_constraint* _constraint) : submodel(_submodel), constraint(_constraint) { }

	void calculateGlobalRotation(const ik_node& child);
};

class ik_solver {
protected:
	std::vector<ik_node> m_nodes;

public:
	template <typename... Args>
	void addNode(Args&&... args) {
		m_nodes.emplace_back(std::forward<Args>(args)...);
	}

	virtual void solve(const vec3d& targetPos, const matrix* targetOrient = nullptr) = 0;

	//Allow const-iterating over created nodes
	inline decltype(m_nodes)::const_iterator begin() const { return m_nodes.cbegin(); }
	inline decltype(m_nodes)::const_iterator end() const { return m_nodes.cend(); }
	
	virtual ~ik_solver() = default;
};

class ik_solver_fabrik : public ik_solver {
	unsigned int m_maxIterations;
	float m_targetDistance;
	float m_minProgress;
	
public:
	ik_solver_fabrik(unsigned int maxIterations = 15, float targetDistance = 0.01f, float minProgress = 0.05f);
	
	void solve(const vec3d& targetPos, const matrix* targetOrient = nullptr) override;
};

class ik_constraint{
public:
	/* 
	 * Return true if the local rotation was modified by the constraint
	 * */
	virtual bool constrain(matrix& /*localRot*/, bool /*backwardsPass*/ = false) const {
		return false;
	};
	
};

class ik_constraint_hinge : public ik_constraint{
	vec3d axis;
public:
	ik_constraint_hinge(const vec3d& _axis);
	
	bool constrain(matrix& localRot, bool backwardsPass) const override;
};

class ik_constraint_window : public ik_constraint{
	angles absLimit;
public:
	ik_constraint_window(const angles& _absLimit);
	
	bool constrain(matrix& localRot, bool backwardsPass) const override;
};
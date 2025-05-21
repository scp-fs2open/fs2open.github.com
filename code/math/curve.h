#pragma once

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#include "math/vecmat.h"

enum class CurveInterpFunction {
	Constant,
	Linear,
	Polynomial,
	Circular,
	Curve,
};

struct curve_keyframe {
	vec2d pos;
	CurveInterpFunction interp_func;
	float param1; // degree for polynomials, integer index for curves
	float param2; // > 0 for "ease in", < 0 for "ease out" on polynomials and circular
};

class Curve {
public :
	SCP_string	name;
	SCP_vector<curve_keyframe>		keyframes;

public :
	// constructor
	Curve(SCP_string in_name);

	//Get
	float GetValue(float x_val) const;

	// Get
	float GetValueIntegrated(float x_val) const;

	//Set
	void ParseData();
};

extern SCP_vector<Curve> Curves;

extern int curve_get_by_name(const SCP_string& in_name);
extern int curve_parse(const char* err_msg);
extern void curves_init();


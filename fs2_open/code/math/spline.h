/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __FS2_SPLINE_HEADER_FILE
#define __FS2_SPLINE_HEADER_FILE

#include "math/vecmat.h"

// -------------------------------------------------------------------------------------------------
// SPLINE DEFINES/VARS
//

struct color;

// max bezier degree - note the # of points directly corresponds to the degree (degree == n_points - 1).
// more points means more expensive!
#define MAX_BEZ_PTS			3

// bezier class. whee
class bez_spline {
public :
	vec3d	pts[MAX_BEZ_PTS];
	int		num_pts;

public :
	// constructor
	bez_spline();
	bez_spline(int _num_pts, vec3d *_pts[MAX_BEZ_PTS]);

	// set the points
	void bez_set_points(int _num_pts, vec3d *_pts[MAX_BEZ_PTS]);

	// bezier blend function
	float BEZ(int k, int n, float u);
	
	// get a point on the bez curve. u goes from 0.0 to 1.0
	void bez_get_point(vec3d *out, float u);

	// render a bezier
	void bez_render(int divs, color *c);
};

// hermite splines. cool cubic stuff
#define MAX_HERM_PTS			3
class herm_spline {
public :
	vec3d	pts[MAX_HERM_PTS];			// control points
	vec3d	d_pts[MAX_HERM_PTS];			// derivative of control points (think of as velocity)
	int		num_pts;
public :
	// constructor
	herm_spline();
	herm_spline(int _num_pts, vec3d *_pts[MAX_HERM_PTS], vec3d *_d_pts[MAX_HERM_PTS]);

	// set the points
	void herm_set_points(int _num_pts, vec3d *_pts[MAX_HERM_PTS], vec3d *_d_pts[MAX_HERM_PTS]);	
	
	// get a point on the hermite curve.
	void herm_get_point(vec3d *out, float u, int k);

	// the derivative of a point on the hermite curve
	void herm_get_deriv(vec3d *deriv, float u, int k);

	// render a bezier
	void herm_render(int divs, color *c);
};


// -------------------------------------------------------------------------------------------------
// SPLINE FUNCTIONS
//


#endif

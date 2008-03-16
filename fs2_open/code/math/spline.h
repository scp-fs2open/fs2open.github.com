/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Math/spline.h $
 * $Revision: 2.4 $
 * $Date: 2005-07-13 03:15:50 $
 * $Author: Goober5000 $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2005/04/05 05:53:18  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.2  2004/08/11 05:06:27  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     7/08/99 10:53a Dave
 * New multiplayer interpolation scheme. Not 100% done yet, but still
 * better than the old way.
 * 
 * 2     7/06/99 4:24p Dave
 * Mid-level checkin. Starting on some potentially cool multiplayer
 * smoothness crap.
 *  
 *
 * $NoKeywords: $
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

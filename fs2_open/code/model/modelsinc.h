/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#ifndef _MODELSINC_H
#define _MODELSINC_H

#include "model.h"

#ifndef MODEL_LIB 
#error This should only be used internally by the model library.  See John if you think you need to include this elsewhere.
#endif

#define OP_EOF 			0
#define OP_DEFPOINTS 	1
#define OP_FLATPOLY		2
#define OP_TMAPPOLY		3
#define OP_SORTNORM		4
#define OP_BOUNDBOX		5

// change header for freespace2
//#define FREESPACE1_FORMAT
#define FREESPACE2_FORMAT
#if defined( FREESPACE1_FORMAT )
#elif defined ( FREESPACE2_FORMAT )
#else
	#error Neither FREESPACE1_FORMAT or FREESPACE2_FORMAT defined
#endif

// FREESPACE1 FORMAT
#if defined( FREESPACE1_FORMAT )
	#define ID_OHDR 'RDHO'	// POF file header
	#define ID_SOBJ 'JBOS'	// Subobject header
#else
	#define ID_OHDR '2RDH'	// POF file header
	#define ID_SOBJ '2JBO'	// Subobject header
#endif
#define ID_TXTR 'RTXT'	// Texture filename list
#define ID_INFO 'FNIP'	// POF file information, like command line, etc
#define ID_GRID 'DIRG'	// Grid information
#define ID_SPCL 'LCPS'	// Special object -- like a gun, missile, docking point, etc.
#define ID_PATH 'HTAP'	// A spline based path
#define ID_GPNT 'TNPG'	// gun points
#define ID_MPNT 'TNPM'  // missile points
#define ID_DOCK 'KCOD'	// docking points
#define ID_TGUN 'NUGT'  // turret gun points
#define ID_TMIS 'SIMT'	// turret missile points
#define ID_FUEL 'LEUF'	// thruster points
#define ID_SHLD 'DLHS'	// shield definition
#define ID_EYE  ' EYE'	// eye information
#define ID_INSG 'GSNI'	// insignia information
#define ID_ACEN 'NECA'	// autocentering information

#define uw(p)	(*((uint *) (p)))
#define w(p)	(*((int *) (p)))
#define wp(p)	((int *) (p))
#define vp(p)	((vector *) (p))
#define fl(p)	(*((float *) (p)))

extern int model_interp(matrix * orient, ubyte * data, polymodel * pm );

// Creates the octants for a given polygon model
void model_octant_create( polymodel * pm );

// frees the memory the octants use for a given polygon model
void model_octant_free( polymodel * pm );

void model_calc_bound_box( vector *box, vector *big_mn, vector *big_mx);

void interp_clear_instance();

#define MAX_POLYGON_VECS	1100		//6500 (7x)
#define MAX_POLYGON_NORMS	2800		//6500 (3x)

extern vector *Interp_verts[MAX_POLYGON_VECS];

#endif

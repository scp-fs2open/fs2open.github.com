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

struct polymodel;

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


#if !defined(BYTE_ORDER) || !defined(LITTLE_ENDIAN) || !defined(BIG_ENDIAN)
#error BYTE_ORDER is not defined
#endif

#if BYTE_ORDER == LITTLE_ENDIAN

// little-endian (Intel) IDs
#define POF_HEADER_ID  0x4f505350	// 'OPSP' (PSPO) POF file header
#if defined( FREESPACE1_FORMAT )
	// FREESPACE1 FORMAT
	#define ID_OHDR 0x5244484f			// RDHO (OHDR): POF file header
	#define ID_SOBJ 0x4a424f53			// JBOS (SOBJ): Subobject header
#else
	#define ID_OHDR 0x32524448			// 2RDH (HDR2): POF file header
	#define ID_SOBJ 0x324a424f			// 2JBO (OBJ2): Subobject header
#endif
#define ID_TXTR 0x52545854				// RTXT (TXTR): Texture filename list
#define ID_INFO 0x464e4950				// FNIP (PINF): POF file information, like command line, etc
#define ID_GRID 0x44495247				// DIRG (GRID): Grid information
#define ID_SPCL 0x4c435053				// LCPS (SPCL): Special object -- like a gun, missile, docking point, etc.
#define ID_PATH 0x48544150				// HTAP (PATH): A spline based path
#define ID_GPNT 0x544e5047				// TNPG (GPNT): gun points
#define ID_MPNT 0x544e504d				// TNPM (MPNT): missile points
#define ID_DOCK 0x4b434f44				// KCOD (DOCK): docking points
#define ID_TGUN 0x4e554754				// NUGT (TGUN): turret gun points
#define ID_TMIS 0x53494d54				// SIMT (TMIS): turret missile points
#define ID_FUEL 0x4c455546				// LEUF (FUEL): thruster points
#define ID_SHLD 0x444c4853				// DLHS (SHLD): shield definition
#define ID_EYE  0x20455945				//  EYE (EYE ): eye information
#define ID_INSG 0x47534e49				// GSNI (INSG): insignia information
#define ID_ACEN 0x4e454341				// NECA (ACEN): autocentering information
#define ID_GLOW 0x574f4c47				// WOLG (GLOW): glow points -Bobboau
#define ID_GLOX 0x584f4c47				// experimental glow points will be gone as soon as we get a proper pof editor -Bobboau

#elif BYTE_ORDER == BIG_ENDIAN

// big-endian (Motorola, Sparc, etc.) IDs
#define POF_HEADER_ID  0x5053504f	// PSPO
#if defined( FREESPACE1_FORMAT )
	// FREESPACE1 FORMAT
	#define ID_OHDR 0x4f484452      // OHDR: POF file header
	#define ID_SOBJ 0x534f424a      // SOBJ: Subobject header
#else
	#define ID_OHDR 0x48445232      // HDR2: POF file header
	#define ID_SOBJ 0x4f424a32      // OBJ2: Subobject header
#endif
#define ID_TXTR 0x54585452				// TXTR: Texture filename list
#define ID_INFO 0x50494e46				// PINF: POF file information, like command line
#define ID_GRID 0x47524944				// GRID: Grid information
#define ID_SPCL 0x5350434c				// SPCL: Special object -- like a gun, missile, 
#define ID_PATH 0x50415448				// PATH: A spline based path
#define ID_GPNT 0x47504e54				// GPNT: gun points
#define ID_MPNT 0x4d504e54				// MPNT: missile points
#define ID_DOCK 0x444f434b				// DOCK: docking points
#define ID_TGUN 0x5447554e				// TGUN: turret gun points
#define ID_TMIS 0x544d4953				// TMIS: turret missile points
#define ID_FUEL 0x4655454c				// FUEL: thruster points
#define ID_SHLD 0x53484c44				// SHLD: shield definition
#define ID_EYE  0x45594520				// EYE : eye information
#define ID_INSG 0x494e5347				// INSG: insignia information
#define ID_ACEN 0x4143454e				// ACEN: autocentering information
#define ID_GLOW 0x474c4f57				// GLOW: glow points -Bobboau

#else
// neither BIG_ENDIAN nor LITTLE_ENDIAN ??
#error unknown value for BYTE_ORDER

#endif


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

#define MAX_POLYGON_VECS	3500		//6500 (7x)
//be sure to update 2d.h
#define MAX_POLYGON_NORMS	5500		//6500 (3x)

extern vector *Interp_verts[MAX_POLYGON_VECS];

#endif

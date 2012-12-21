/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _NEBULA_H
#define _NEBULA_H

// mainly only needed by Fred
extern int Nebula_pitch;
extern int Nebula_bank;
extern int Nebula_heading;

struct angles;

// You shouldn't pass the extension for filename.
// PBH = Pitch, Bank, Heading.   Pass NULL for default orientation.
void nebula_init( char *filename, int pitch, int bank, int heading );
void nebula_init( char *filename, angles *pbh = /*NULL*/ 0 );
void nebula_close();
void nebula_render();

#endif	//_NEBULA_H

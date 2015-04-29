/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _PALMAN_H
#define _PALMAN_H 

#include "globalincs/pstypes.h"

// Calculate tables for this palette.
// Assumes gr_palette is filled in.
extern void palette_update(const char *name, int restrict_colors_to_upper_128 );

// Writes current tables to disk.
extern void palette_flush();

// Functions to query a palette
extern uint palette_compute_checksum( ubyte *pal );		// computes checksum of palette
extern ubyte *palette_get_blend_table(float alpha);

extern uint palette_find( int r, int g, int b );

// Data used to query a palette
extern ubyte gr_palette[256*3];
extern ubyte gr_fade_table[(256*34)*2];
extern uint gr_palette_checksum;

// Functions to deal with changing the palette.
// These just call gr_set_palette, which will in turn
// call palette_flush and palette_update.
extern void palette_load_table( const char * filename );
extern void palette_use_bm_palette(int n);
extern void palette_restore_palette( void );

// nondarkening texture pixel colors
#define MAX_NONDARK_COLORS					10

extern int Palman_num_nondarkening_default;
extern ubyte Palman_non_darkening_default[MAX_NONDARK_COLORS][3];

extern int Palman_num_nondarkening;
extern ubyte Palman_non_darkening[MAX_NONDARK_COLORS][3];

extern int palman_is_nondarkening(int r,int g, int b);
extern void palman_load_pixels();
extern void palman_set_nondarkening(ubyte colors[MAX_NONDARK_COLORS][3], int size);

#endif

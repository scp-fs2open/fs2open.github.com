/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "palman/palman.h"
#include "bmpman/bmpman.h"
#include "debugconsole/console.h"
#include "pcxutils/pcxutils.h"
#include "parse/parselo.h"
#include "graphics/grinternal.h"
#include "cfile/cfile.h"


#define	SQUARE(x) ((x)*(x))

#define NUM_BLEND_TABLES 3
float blend_table_factors[NUM_BLEND_TABLES] = { 0.5f, 1.0f, 1.2f };

ubyte palette_org[256*3];
ubyte gr_palette[256*3];
ubyte gr_fade_table[(256*34)*2];
static ubyte palette_blend_table[NUM_BLEND_TABLES*256*256];

int palette_blend_table_calculated = 0;
int palette_fade_table_calculated = 0;

uint gr_palette_checksum = 0;

uint palman_screen_signature = 0;

#define LOOKUP_SIZE (64*64*64)
ubyte palette_lookup[64*64*64];

static char palette_name[128] = { "none" };

static int Palman_restrict_colors = 0;

//extern ubyte palette_org[256*3];

int Palman_num_nondarkening_default = 0;
ubyte Palman_non_darkening_default[MAX_NONDARK_COLORS][3];

int Palman_num_nondarkening = 0;
ubyte Palman_non_darkening[MAX_NONDARK_COLORS][3];

int palman_is_nondarkening(int r,int g, int b)
{
	int i;	

	for (i=0; i<Palman_num_nondarkening; i++ )	{
		if ( (r==Palman_non_darkening[i][0]) && (g==Palman_non_darkening[i][1]) && (b==Palman_non_darkening[i][2]) )	{
			return 1;
		}
	}
	return 0;
}

void palman_load_pixels()
{
	int rval;
	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", "pixels.tbl", rval));
		return;
	}

	// open pixels.tbl
	read_file_text("pixels.tbl", CF_TYPE_TABLES);
	reset_parse();

	// parse pixels	
	while(!optional_string("#END")){
		// nondarkening pixel
		if(required_string("+ND")){
			stuff_ubyte(&Palman_non_darkening_default[Palman_num_nondarkening_default][0]);
			stuff_ubyte(&Palman_non_darkening_default[Palman_num_nondarkening_default][1]);
			stuff_ubyte(&Palman_non_darkening_default[Palman_num_nondarkening_default++][2]);
		}
	}

	// set this to be the active table
	palman_set_nondarkening(Palman_non_darkening_default, Palman_num_nondarkening_default);
}

void palman_set_nondarkening(ubyte colors[MAX_NONDARK_COLORS][3], int size)
{
	// if we're supposed to use the passed table
	memcpy(Palman_non_darkening, colors, MAX_NONDARK_COLORS * 3);
	Palman_num_nondarkening = size;
}

void palette_cache_clear()
{
	int i;

	for (i=0; i<LOOKUP_SIZE; i++ )	{
		palette_lookup[i] = 255;
	}
}

int palette_cache_find( int r, int g, int b )
{
	if ( !palman_is_nondarkening(r,g,b))	{
		int value = ((r/4)<<12)+((g/4)<<6)+(b/4);
		if ( palette_lookup[value] != 255 )	{
			return palette_lookup[value];
		}
	}
	return -1;
}

void palette_cache_add( int r, int g, int b, int index )
{
	int value = ((r/4)<<12)+((g/4)<<6)+(b/4);

	if ( !palman_is_nondarkening(r,g,b))	{
		palette_lookup[value] = (ubyte)index;
	}
}

char palette_base_filename[128] = { "default" };
int palette_base_loaded = 0;

void palette_load_table( const char * filename )
{
	int i;
	int w, h;
	int pcx_error;

	strcpy_s( palette_base_filename, filename );
	char * p = strchr(palette_base_filename,'.');
	if ( p )	{
		*p = 0;
	}

	pcx_error = pcx_read_header(palette_base_filename, NULL, &w, &h, NULL, palette_org );
	if ( pcx_error != PCX_ERROR_NONE )	{
		// Read the old .256 file
		CFILE *fp;
		int fsize;
		fp = cfopen( palette_base_filename, "rb" );
		if ( fp==NULL)
			Error( LOCATION, "Can't open palette file <%s>",palette_base_filename);

		fsize	= cfilelength( fp );
		Assert( fsize == 9472 );
		cfread( palette_org, 256*3, 1, fp );
		cfclose(fp);

		for (i=0; i<768; i++ )	{	
			palette_org[i] = ubyte((palette_org[i]*255)/63);
		}
	}

	palette_base_loaded = 1;

	gr_set_palette(palette_base_filename, palette_org);
}


DCF(palette,"Loads a new palette")
{
	char palette_file[MAX_FILENAME_LEN];

	if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: palette <filename>\nLoads the palette file.\n" );
	}

	dc_stuff_string_white(palette_file, MAX_FILENAME_LEN);
	palette_load_table(palette_file);
}

int Palman_allow_any_color = 0;


uint palette_find( int r, int g, int b )
{
	int i, j;
	int best_value, best_index, value;

	int	is_transparent = 0;
	if ( (r == 0) && (g==255) && (b==0) )	{
		is_transparent = 1;
	}

	int restrict = 0;
	if ( Palman_restrict_colors && (!Palman_allow_any_color) )	{
		restrict = 1;
	}
			
//	int	rgb = ((r/4)<<12)+((g/4)<<6)+(b/4);

	i = palette_cache_find(r,g,b);
	if ( i != -1 )	{
		if ( restrict )	{
			if ( i > 127 )	{
				return i;
			}
		} else {
			return i;
		}
	}

	best_value = 1000000000;
	best_index = -1;

	int bottom_color = 0;
	if ( restrict )	{
		bottom_color = 128;
	}

	j=3*bottom_color; 

	for (i=bottom_color; i<255; i++ )	{
		int pr, pg, pb;

		pr = gr_palette[j];
		pg = gr_palette[j+1];
		pb = gr_palette[j+2];

		value = SQUARE(r-pr) + SQUARE(g-pg) + SQUARE(b-pb);

		if ( (best_index==-1) || (value < best_value) )	{
			// Don't map anything to 0,255,0 (transparent) ever, except 0,255,0
			if (value==0) {
				palette_cache_add( r, g, b, i );
				return i;
			}
			// Not an exact match, so don't let anything map to a nondarkening color.
			if ( (!is_transparent) && (!palman_is_nondarkening( pr, pg, pb ))  )	{
				best_value = value;
				best_index = i;
			}
		}
		j += 3;
	}

	if ( best_index == -1 )	{
		best_index = bottom_color;
	}

	palette_cache_add( r, g, b, best_index );
	return best_index;
}

// version 0 - initial revision
// version 2 - changed 16-bpp fade table to include the 1.5 brightening factor
// version 3 - changed to put black in color 0 all the time
// version 4 - took out all truecolor lookup tables
// version 5 - changed palette to use 254 entries for nebula, made color mapping different
// version 6 - took out 1.5 brightness
// version 7 - added blending tables
// version 8 - made glows use additive blending
// version 9 - made 255,255,255 not fade
// version 10 - made fade table go to white again for anything l above 0.75.
// version 11 - made fade table go from 0-1 instead of 0-0.75
// version 12 - added variable gamma
// version 13 - Reduced blending tables from 5 to 3. Save 128KB RAM.
// version 14 - made palette never map anything to green.
// version 15 - made green blending with anything be black
// version 16 - added compression
// version 17 - added more nondarkening colors
// version 18 - fixed bug with blue nondarkening colors
// version 19 - fixed bug where only colors divisible by 4 got used.
// version 20 - added flag to only use lower 128 colors for palette.
#define PAL_ID 0x4c415056			// "LAPV" in file = VPAL (Volition Palette)
#define PAL_VERSION  20
#define PAL_LAST_COMPATIBLE_VERSION 20

void palette_write_cached1( char *name )
{
	CFILE *fp;
	char new_name[MAX_PATH_LEN];

	strcpy_s( new_name, name );
	strcat_s( new_name, ".clr" );
	
//	mprintf(( "Writing palette cache file '%s'\n", new_name ));

	fp = cfopen( new_name, "wb", CFILE_NORMAL, CF_TYPE_CACHE );
	if ( !fp ) return;
	
	cfwrite_uint( PAL_ID, fp );
	cfwrite_int(PAL_VERSION, fp );
	cfwrite( &gr_palette_checksum, 4, 1, fp );

	cfwrite_compressed( &gr_palette, 256*3, 1, fp );						// < 1 KB

	cfwrite_compressed( &palette_lookup, LOOKUP_SIZE, 1, fp );			// 256KB
	
	if ( palette_fade_table_calculated )	{
		cfwrite_int(1,fp);
		cfwrite_int(Gr_gamma_int,fp);
		cfwrite_compressed( &gr_fade_table,   256*34*2, 1, fp );		// 17KB
	} else {
		cfwrite_int(0,fp);
	}
	
	if ( palette_blend_table_calculated )	{
		cfwrite_int(NUM_BLEND_TABLES,fp);
		cfwrite_compressed( &palette_blend_table,  256*256, NUM_BLEND_TABLES, fp );	//64KB*
	} else {
		cfwrite_int(0,fp);
	}

	cfclose(fp);
//	mprintf(( "Done.\n" ));
}

// Returns TRUE if successful, else 0

int palette_read_cached( char *name )
{
	CFILE *fp;
	char new_name[MAX_PATH_LEN];
	int version;
	uint id, new_checksum;
	ubyte new_palette[768];

	strcpy_s( new_name, name );
	strcat_s( new_name, ".clr" );

//	mprintf(( "Reading palette '%s'\n", name ));
	
	fp = cfopen( new_name, "rb", CFILE_NORMAL, CF_TYPE_CACHE );

	// Couldn't find file
	if ( !fp ) {
		mprintf(( "No cached palette file\n" ));
		return 0;
	}

	id  = cfread_uint( fp );
	if ( id != PAL_ID )	{
		mprintf(( "Cached palette file has incorrect ID\n" ));
		cfclose(fp);
		return 0;
	}
	version = cfread_int( fp );
	if ( version < PAL_LAST_COMPATIBLE_VERSION ) {
		mprintf(( "Cached palette file is an older incompatible version\n" ));
		cfclose(fp);
		return 0;
	}
	
	cfread( &new_checksum, 4, 1, fp );
	if ( gr_palette_checksum != new_checksum )	{
		mprintf(( "Cached palette file is out of date (Checksum)\n" ));
		cfclose(fp);
		return 0;
	}

	cfread_compressed( &new_palette, 256*3, 1, fp );
	if ( memcmp( new_palette, gr_palette, 768 ) )	{
		mprintf(( "Cached palette file is out of date (Contents)\n" ));
		cfclose(fp);
		return 0;
	}

	cfread_compressed( &palette_lookup, LOOKUP_SIZE, 1, fp );			// 256KB

	int fade_table_saved = cfread_int(fp);
	
	if ( fade_table_saved )	{
		int new_gamma;
		cfread( &new_gamma, 4, 1, fp );
		cfread_compressed( &gr_fade_table,   256*34*2, 1, fp );		// 17KB
		if ( new_gamma == Gr_gamma_int )	{
			palette_fade_table_calculated = 1;
		} else {
			palette_fade_table_calculated = 0;
		}
	} else {
		palette_fade_table_calculated = 0;
	}
	
	int num_blend_tables_saved = cfread_int(fp);
	if ( (num_blend_tables_saved == NUM_BLEND_TABLES) && (num_blend_tables_saved>0))	{
		palette_blend_table_calculated = 1;
		cfread_compressed( &palette_blend_table,  256*256, NUM_BLEND_TABLES, fp );	//64KB*
	} else {
		palette_blend_table_calculated = 0;
	}

	cfclose(fp);

//	mprintf(( "Done.\n" ));

	return 1;
}

void palman_create_blend_table(float factor, ubyte *table)
{
	int i;

	// Make the blending table
	for (i=0; i<256; i++ )	{
		int j, r, g, b;
		float si, fr, fg, fb, br, bg, bb;
		float Sf, Df;

		fr = i2fl(gr_palette[i*3+0]);
		fg = i2fl(gr_palette[i*3+1]);
		fb = i2fl(gr_palette[i*3+2]);

		// Make everything blended with Xparent be black
		if ( i==255 )	{
			fr = fg = fb = 0.0f;
		}

		si = (( fr+fg+fb ) / (256.0f*3.0f)) * factor;

		if ( factor > 1.0f )	{
			if ( si > 1.0f )	{
				Sf = 1.0f;
				Df = 0.0f;
			} else	{
				Sf = 1.0f;
				Df = 1.0f - si;
			}
		} else {
			if ( si > 1.0f )	{
				Sf = 1.0f;
				Df = 0.0f;
			} else	{
				Sf = si;
				Df = 1.0f;
			}
			Sf = factor;
			Df = 1.0f;
		}
 
//		Sf = Df =1.0f;

		for (j=0; j<256; j++ )	{
			br = i2fl(gr_palette[j*3+0]);
			bg = i2fl(gr_palette[j*3+1]);
			bb = i2fl(gr_palette[j*3+2]);

			// Make all things on top of Xparent be black
			if ( j==255 )	{
				br = bg = bb = 0.0f;
			}

			r = fl2i( fr*Sf + br*Df );
			g = fl2i( fg*Sf + bg*Df );
			b = fl2i( fb*Sf + bb*Df );

			int max = r;
			if ( g > max ) max = g;
			if ( b > max ) max = b;
			if ( max > 255 )	{
				r = (255*r)/max;
				g = (255*g)/max;
				b = (255*b)/max;
			}
			if ( r > 255 ) r = 255; else if ( r < 0 ) r = 0;
			if ( g > 255 ) g = 255; else if ( g < 0 ) g = 0;
			if ( b > 255 ) b = 255; else if ( b < 0 ) b = 0;

			if ( i == 255 )
				table[i*256+j] = (unsigned char)j;
			else {
				// If background transparent, and color isn't bright, call it transparent.
				if ( j == 255 && ((r+g+b) < 110))	{
					table[i*256+j] = 255;
				} else {
					table[i*256+j] = (unsigned char)palette_find(r,g,b);
				}
			}
		}
	}
}

void palette_flush()
{
	// DB 2/3/99 - I think this was causing some wacky unhandled exceptions at game shutdown. Since we don't use palettes anymore.....
	/*
	if ( stricmp( palette_name, "none" ) )	{
		palette_write_cached1( palette_name );
	}
	*/
}


// When gr_set_palette is called, it fills in gr_palette and then calls this
// function, which should update all the tables.
// Pass NULL to flush current palette.
void palette_update(const char *name_with_extension, int restrict_font_to_128)
{
	uint tmp_checksum;
	char name[MAX_PATH_LEN];

	Palman_restrict_colors = restrict_font_to_128;
	
	strcpy_s( name, name_with_extension );
	char *p = strchr( name, '.' );
	if ( p ) *p = 0;

	strcpy_s( palette_name, name );

	tmp_checksum = palette_compute_checksum( gr_palette );
	if ( tmp_checksum == gr_palette_checksum ) return;

	gr_palette_checksum = tmp_checksum;

	// Clear the lookup cache, since the palette has changed
	palette_cache_clear();
	palette_blend_table_calculated = 0;
	palette_fade_table_calculated = 0;

	// For "none" palettes, don't calculate tables
	if ( !stricmp( name, "none" ) ) {
		return;
	}

	// Read in the cached info if there is any.
	if ( palette_read_cached( name ) )	{
		return;
	}
}

ubyte *palette_get_fade_table()
{
	int i,l;

	if ( palman_screen_signature != gr_screen.signature )	{
		palman_screen_signature = gr_screen.signature;
		palette_fade_table_calculated = 0;
	}


	if ( !palette_fade_table_calculated )	{
		//mprintf(( "Creating fading table..." ));	

		for (i=0; i<256; i++ )	{
			int r, g, b;
			int ur, ug, ub;
			r = gr_palette[i*3+0];
			g = gr_palette[i*3+1];
			b = gr_palette[i*3+2];

			if ( palman_is_nondarkening(r,g,b))		{
				// Make pure white not fade
				for (l=0; l<32; l++ )	{
					gr_fade_table[((l+1)*256)+i] = (unsigned char)i;
				}
			} else {
				for (l=0; l<32; l++ )	{

					if ( l < 24 )	{
						float f = (float)pow(i2fl(l)/23.0f, 1.0f/Gr_gamma);
						ur = fl2i(i2fl(r)*f); if ( ur > 255 ) ur = 255;
						ug = fl2i(i2fl(g)*f); if ( ug > 255 ) ug = 255;
						ub = fl2i(i2fl(b)*f); if ( ub > 255 ) ub = 255;
					} else {
						int x,y;
						int gi, gr, gg, gb;
			
						gi = (r+g+b)/3;

						#ifdef RGB_LIGHTING
							gr = r;
							gg = g;
							gb = gi*2;
						#else
							gr = r*2;
							gg = g*2;
							gb = b*2;
						#endif
				
						x = l-24;			// x goes from 0 to 7
						y = 31-l;			// y goes from 7 to 0

						ur = ((gr*x)+(r*y))/7; if ( ur > 255 ) ur = 255;
						ug = ((gg*x)+(g*y))/7; if ( ug > 255 ) ug = 255;
						ub = ((gb*x)+(b*y))/7; if ( ub > 255 ) ub = 255;
					}
					gr_fade_table[((l+1)*256)+i] = (unsigned char)palette_find( ur, ug, ub );

				}
			}
			gr_fade_table[ (0*256)+i ] = gr_fade_table[ (1*256)+i ];
			gr_fade_table[ (33*256)+i ] = gr_fade_table[ (32*256)+i ];
		}

		// Mirror the fade table
		for (i=0; i<34; i++ )	{
			for ( l = 0; l < 256; l++ )	{
				gr_fade_table[ ((67-i)*256)+l ] = gr_fade_table[ (i*256)+l ];
			}
		}

//		mprintf(( "done\n" ));	
		palette_fade_table_calculated = 1;
	}

	return &gr_fade_table[0];
}


ubyte *palette_get_blend_table(float alpha)
{
	int i;

	if ( !palette_blend_table_calculated )	{
//		mprintf(( "Creating blending table..." ));	
		for (i=0; i<NUM_BLEND_TABLES; i++ )	{
			palman_create_blend_table(blend_table_factors[i], &palette_blend_table[i*256*256] );
		}
//		mprintf(( "done\n" ));	
		palette_blend_table_calculated = 1;
	}
	
	for (i=0; i<NUM_BLEND_TABLES; i++ )	{
		if ( alpha <= blend_table_factors[i] )	
			break;
	} 
	if ( i<0 ) i = 0;
	if ( i>NUM_BLEND_TABLES-1 ) i = NUM_BLEND_TABLES-1;

	return &palette_blend_table[i*256*256];
}



// compute a simple checksum on the given palette.  Used by the bitmap manager
// to determine if we need to reload a new palette for a bitmap.  Code liberally
// stolen from descent networking checksum code
uint palette_compute_checksum( ubyte *pal )
{
	int i;
	uint sum1, sum2;

	sum1 = sum2 = 0;

	for (i = 0; i < 768; i++) {
		sum1 += (uint)pal[i];
		if ( sum1 >= 255 ) sum1 -= 255;
		sum2 += sum1;
	}
	sum2 %= 255;

	return ((sum1<<8)+sum2);
}

// this function takes a bitmap number and sets the game palette to the palette of this
// bitmap.
void palette_use_bm_palette(int n)
{
	ubyte tmp[768];
	char name[MAX_PATH_LEN];

	bm_get_palette(n, tmp, name);				// get the palette for this bitmap

	gr_set_palette(name, tmp);				// load the new palette.
}

void palette_restore_palette()
{
	ubyte tmp[768];
	memcpy(tmp, palette_org, 3*256);

	if ( palette_base_loaded )		{
		gr_set_palette(palette_base_filename, tmp);
	}
}

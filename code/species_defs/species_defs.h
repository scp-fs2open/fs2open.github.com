// Species_Defs.h
// Extended Species Support for FS2 Open
// Derek Meek
// 10-14-2003

/*
 * $Logfile: /Freespace2/code/species_defs/species_defs.h $
 * $Revision: 1.6 $
 * $Date: 2005-09-25 02:15:02 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2005/09/24 07:07:17  Goober5000
 * another species overhaul
 * --Goober5000
 *
 * Revision 1.4  2005/07/13 03:35:35  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 1.3  2005/01/31 10:34:39  taylor
 * merge with Linux/OSX tree - p0131
 *
 * Revision 1.2  2004/08/11 05:06:34  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 1.1  2003/10/15 22:03:27  Kazan
 * Da Species Update :D
 *
 *
 *
 *
 */

#if !defined(_SPECIES_DEFS_H_)
#define _SPECIES_DEFS_H_

#include "globalincs\pstypes.h"
#include "globalincs\globals.h"
#include "bmpman/bmpman.h"


typedef struct thruster_pair {
	generic_anim normal;
	generic_anim afterburn;
} thruster_pair;

typedef struct thruster_info {
	thruster_pair flames;
	thruster_pair glow;
} thruster_info;


typedef struct species_info {
	char species_name[NAME_LENGTH];
	union {
		struct {
			int r, g, b;
		} rgb;
		int a1d[3];
	} fred_color;

	char debris_texture_file[MAX_FILENAME_LEN];

	generic_anim shield_anim;
	thruster_info thruster_info;

	float awacs_multiplier;
} species_info;


extern int Num_species;
extern species_info Species_info[MAX_SPECIES];


// load up the species_defs.tbl into the correct data areas
// IMPORTANT: If NumSpecies != 3 icons.tbl has to be modified to compensate!
void Init_Species_Definitions();

#endif

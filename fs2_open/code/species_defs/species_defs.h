// Species_Defs.h
// Extended Species Support for FS2 Open
// Derek Meek
// 10-14-2003

/*
 * $Logfile: /Freespace2/code/species_defs/species_defs.h $
 * $Revision: 1.2 $
 * $Date: 2004-08-11 05:06:34 $
 * $Author: Kazan $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2003/10/15 22:03:27  Kazan
 * Da Species Update :D
 *
 *
 *
 *
 */

#include "PreProcDefines.h"
#if !defined(_species_defs_h_)
#define _species_defs_h_

extern int True_NumSpecies;

// load up the species_defs.tbl into the correct data areas
// IMPORTANT: If NumSpecies != 3 icons.tbl has to be modified to compensate!
void Init_Species_Definitions();

#endif
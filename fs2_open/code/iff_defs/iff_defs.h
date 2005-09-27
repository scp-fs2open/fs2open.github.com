/*
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */

/*
 * $Logfile: /Freespace2/code/iff_defs/iff_defs.h $
 * $Revision: 1.1 $
 * $Date: 2005-09-27 05:25:18 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 */


#ifndef _IFF_DEFS_H_
#define _IFF_DEFS_H_

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"


typedef struct iff_info {

	char iff_name[NAME_LENGTH];


	// constructor to initialize everything to 0
	iff_info()
	{
		memset(this, 0, sizeof(iff_info));
	}

} iff_info;


extern int Num_iffs;
extern iff_info Iff_info[MAX_IFFS];

extern int Iff_traitor;


// load the iff table
void iff_init();


#endif

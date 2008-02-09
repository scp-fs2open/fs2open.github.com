/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Starfield/Nebula.h $
 * $Revision: 2.1 $
 * $Date: 2004-03-05 09:02:07 $
 * $Author: Goober5000 $
 *
 * Include file for nebula stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 4     11/25/97 11:40a Hoffoss
 * Added support for nebula placement editing.
 * 
 * 3     11/24/97 12:04p John
 * 
 * 2     11/16/97 2:29p John
 * added versioning to nebulas; put nebula code into freespace.
 * 
 * 1     11/16/97 1:14p John
 *
 * $NoKeywords: $
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

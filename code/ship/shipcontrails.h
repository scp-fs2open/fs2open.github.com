/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ship/ShipContrails.h $
 * $Revision: 2.4 $
 * $Date: 2005-07-13 03:35:30 $
 * $Author: Goober5000 $
 *
 * all sorts of cool stuff about ships
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2004/08/11 05:06:34  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2003/11/11 02:15:41  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.1  2002/10/19 19:29:29  bobboau
 * inital commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam shield hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
 *
 * Revision 2.0  2002/06/03 04:02:28  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     11/14/98 5:33p Dave
 * Lots of nebula work. Put in ship contrails.
 * 
 * 1     11/14/98 3:40p Dave
 * 
 * 1     11/13/98 3:28p Dave
 * 
 * 
 * $NoKeywords: $
 */

#ifndef _FREESPACE2_SHIP_CONTRAIL_HEADER_FILE
#define _FREESPACE2_SHIP_CONTRAIL_HEADER_FILE

// ----------------------------------------------------------------------------------------------
// CONTRAIL DEFINES/VARS
//

// prototypes
struct ship;

// ----------------------------------------------------------------------------------------------
// CONTRAIL FUNCTIONS
//

// call during level initialization
void ct_level_init();

// call during level shutdown
void ct_level_close();

// call when a ship is created to initialize its contrail stuff
void ct_ship_create(ship *shipp);

// call when a ship is deleted to free up its contrail stuff
void ct_ship_delete(ship *shipp);

// call each frame for processing a ship's contrails
void ct_ship_process(ship *shipp);

// determine if the ship has AB trails
int ct_has_ABtrails(ship *shipp);

// update active ABtrails - moving existing ones, adding new ones where necessary
void ct_update_ABtrails(ship *shipp);

// create new ABtrails
void ct_create_ABtrails(ship *shipp);

#endif

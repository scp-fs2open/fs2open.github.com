/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/wing.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:32 $
 * $Author: Goober5000 $
 *
 * Wing management functions for dealing with wing related operations
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.2  2004/09/29 17:26:33  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.1.1.1  2002/07/15 03:11:03  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:02p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 7     8/16/97 4:51p Hoffoss
 * Fixed bugs with wing deletion and removing ships from a wing.
 * 
 * 6     8/12/97 1:55a Hoffoss
 * Made extensive changes to object reference checking and handling for
 * object deletion call.
 * 
 * 5     6/18/97 2:36p Hoffoss
 * Wing ship numbering starts at 1 instead of 0, and changed form wing to
 * allow reforming a wing.
 * 
 * 4     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 * 
 * 3     1/24/97 4:14p Hoffoss
 * Added error checking to Fred.
 * 
 * 2     11/26/96 2:40p Hoffoss
 * Wing toolbar buttons supported.
 * 
 * 1     11/05/96 9:14a Hoffoss
 * 
 * $NoKeywords: $
 */

#include "Management.h"

int	create_wing();
void	remove_wing(int wing_num);
void	remove_ship_from_wing(int ship, int min = 1);
void	mark_wing(int wing);
int	delete_wing(int wing = cur_wing, int bypass = 0);

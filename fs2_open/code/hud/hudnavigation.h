// HUDNavigation.h
// Derek Meek
// 4-30-2004

/*
 * $Logfile: /Freespace2/code/Hud/HUDNavigation.h $
 * $Revision: 1.3 $
 * $Date: 2004-08-11 05:06:25 $
 * $Author: Kazan $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2004/07/26 17:54:04  Kazan
 * Autopilot system completed -- i am dropping plans for GUI nav map
 * All builds should have ENABLE_AUTO_PILOT defined from now on (.dsp's i am committing reflect this) the system will only be noticed if the mission designer brings it online by defining a nav point
 * Fixed FPS counter during time compression
 *
 * Revision 1.1  2004/05/24 07:23:09  taylor
 * filename case change
 *
 * Revision 1.1  2004/05/07 23:50:14  Kazan
 * Sorry Guys!
 *
 *
 *
 *
 */

#include "PreProcDefines.h"
#if !defined(_HUD_NAV_)
#define _HUD_NAV_

// Draws the Navigation stuff on the HUD
void HUD_Draw_Navigation();


#endif
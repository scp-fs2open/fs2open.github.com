/*
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */ 

/*
 * $Logfile: /Freespace2/code/wxFRED2/wxFRED2.h $
 * $Revision: 1.2 $
 * $Date: 2005-03-30 01:27:49 $
 * $Author: Goober5000 $
 *
 * New cross-platform version of FRED2
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/03/30 01:23:32  Goober5000
 * added header back in, with a small tweak for the .cpp
 * --Goober5000
 *
 * Revision 1.0  2005/03/28 22:15:00  Goober5000
 * Addition to CVS repository
 *
 */

#ifndef _WXFRED2_H
#define _WXFRED2_H

class wxFRED2 : public wxApp
{
	public:
		virtual bool OnInit();
};

DECLARE_APP(wxFRED2)

#endif
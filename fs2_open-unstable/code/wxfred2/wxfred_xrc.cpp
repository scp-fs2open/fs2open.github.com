/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 

/*
 * $Logfile: /Freespace2/code/wxFRED2/wxFREDXRC.cpp $
 * $Revision: 1.4 $
 * $Date: 2006-04-20 06:32:30 $
 * $Author: Goober5000 $
 *
 * New cross-platform version of FRED2
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/04/25 12:14:21  Goober5000
 * -made the XRC file parsed on load instead of embedded
 * -migrated wxFRED to wxWidgets 2.6.0
 * --Goober5000
 *
 * Revision 1.2  2005/04/25 06:36:25  taylor
 * newline fix
 *
 * Revision 1.1  2005/04/24 14:42:27  Goober5000
 * wxFRED now uses XRC-based resources
 * --Goober5000
 *
 * Revision 1.0  2005/04/24 08:41:00  Goober5000
 * Addition to CVS repository
 *
 */

#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include "wxfred_xrc.h"

// Either autogenerate the XRC stuff or load it, depending on the compiler option.
// Note that if the XRC resources are not embedded, the XRC file must be present
// in the folder wxFRED2 is run from.

#ifdef EMBED_XRC_RESOURCES

	#include "wxfred_xrc.inl"

#else

	#include <wx/xrc/xmlres.h>

	void InitXmlResource()
	{
		wxXmlResource::Get()->Load(wxT("wxfred.xrc"));
	}

#endif

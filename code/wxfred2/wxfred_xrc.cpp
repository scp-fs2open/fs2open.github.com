/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
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

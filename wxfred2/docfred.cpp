/*
 * Created by "z64555" for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "docfred.h"

#include <cfile/cfile.h>
#include <mission/missionparse.h>
#include <object/waypoint.h>

#include <wx/docview.h>
#include <wx/wx.h>

// TODO: Initialize docFRED2
docFRED2::docFRED2( void )
	: wxDocument(NULL)/*, Mission()*/
{
}

bool docFRED2::OnSaveDocument( const wxString& filename )
{
	// Get the system time and stamp it into the mission file
	return FALSE;
}

bool docFRED2::OnOpenDocument( const wxString& filename )
{
	return FALSE;
}

bool docFRED2::OnNewDocument( void )
{
	return FALSE;
}

bool docFRED2::OnCloseDocument( void )
{
	return FALSE;
}


bool docFRED2::DoSaveDocument( const wxString& filename )
{
	return FALSE;
}

bool docFRED2::DoOpenDocument( const wxString& filename )
{
	return FALSE;
}

bool docFRED2::DoNewDocument( void )
{
	return FALSE;
}

bool docFRED2::DoCloseDocument( void )
{
	return FALSE;
}



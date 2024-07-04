/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// CampaignFilelistBox.cpp : implementation file
//

#include "stdafx.h"

#include "FRED.h"
#include "CampaignFilelistBox.h"
#include "CampaignTreeWnd.h"

#include "mission/missioncampaign.h"
#include "mission/missionparse.h"

#include <freespace.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// campaign_filelist_box

campaign_filelist_box::campaign_filelist_box()
{
}

campaign_filelist_box::~campaign_filelist_box()
{
}


BEGIN_MESSAGE_MAP(campaign_filelist_box, CListBox)
	//{{AFX_MSG_MAP(campaign_filelist_box)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// campaign_filelist_box message handlers

void campaign_filelist_box::initialize(const CString &path)
{
	int i, z;
	SCP_vector<SCP_string> mission_filenames;
	SCP_string filter;
	mission a_mission;

	ResetContent();

	// if there's a path, cf_get_file_list will look there; if not, it will use the file filter in the standard location
	if (!path.IsEmpty())
	{
		filter = (LPCTSTR)path;
		if (filter.back() != DIR_SEPARATOR_CHAR)
			filter += DIR_SEPARATOR_CHAR;
	}
	filter += "*.fs2";

	extern int Skip_packfile_search;
	Skip_packfile_search = 1;
	cf_get_file_list(mission_filenames, CF_TYPE_MISSIONS, filter.c_str());
	Skip_packfile_search = 0;


	for (i = 0; i < (int)mission_filenames.size(); i++)
	{
		// add the extension; the file list box wants to display it, and we need it for absolute paths anyway
		mission_filenames[i] += ".fs2";

		// make a call to get the mission info for this mission.  Passing a mission as the second
		// parameter will prevent The_mission from getting overwritten.
		if (path.IsEmpty())
			get_mission_info(mission_filenames[i].c_str(), &a_mission);
		else
		{
			auto file_to_load = mission_filenames[i];
			if (!path.IsEmpty())
				file_to_load = (LPCTSTR)path + file_to_load;
			get_mission_info(file_to_load.c_str(), &a_mission, true, true);
		}

		// only add missions of the appropriate type to the file listbox
		if ( (Campaign.type == CAMPAIGN_TYPE_SINGLE) && (a_mission.game_type & (MISSION_TYPE_SINGLE|MISSION_TYPE_TRAINING)) )
			AddString(mission_filenames[i].c_str());
		else if ( (Campaign.type == CAMPAIGN_TYPE_MULTI_COOP) && (a_mission.game_type & MISSION_TYPE_MULTI_COOP) )
			AddString(mission_filenames[i].c_str());
		else if ( (Campaign.type == CAMPAIGN_TYPE_MULTI_TEAMS) && (a_mission.game_type & MISSION_TYPE_MULTI_TEAMS) )
			AddString(mission_filenames[i].c_str());
	} 
	

	for (i=0; i<Campaign.num_missions; i++) {
		z = FindString(-1, Campaign.missions[i].name);
		if (z != LB_ERR) {
			DeleteString(z);  // take out all missions already in the campaign
			i--;  // recheck for name just in case there are two (should be impossible but can't be sure)
		}
	}
}

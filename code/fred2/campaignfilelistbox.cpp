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
#include "freespace2/freespace.h"
#include "CampaignFilelistBox.h"
#include "CampaignTreeWnd.h"
#include "mission/missioncampaign.h"
#include "mission/missionparse.h"

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

void campaign_filelist_box::initialize()
{
	int i, z, num_files;
	char *mission_filenames[2000];
	char mission_filenames_arr[2000][MAX_FILENAME_LEN];
	mission a_mission;

	ResetContent();

	extern int Skip_packfile_search;
	Skip_packfile_search = 1;
	num_files = cf_get_file_list_preallocated(2000, mission_filenames_arr, mission_filenames, CF_TYPE_MISSIONS, "*.fs2");
	Skip_packfile_search = 0;

	i=0;

	while (i < num_files)
	{
		// make a call to get the mission info for this mission.  Passing a misison as the second
		// parameter will prevent The_mission from getting overwritten.
		get_mission_info( mission_filenames[i] , &a_mission );
		strcat(mission_filenames[i],".fs2");

		// only add missions of the appropriate type to the file listbox
		if ( (Campaign.type == CAMPAIGN_TYPE_SINGLE) && (a_mission.game_type & (MISSION_TYPE_SINGLE|MISSION_TYPE_TRAINING)) )
			AddString(mission_filenames[i]);
		else if ( (Campaign.type == CAMPAIGN_TYPE_MULTI_COOP) && (a_mission.game_type & MISSION_TYPE_MULTI_COOP) )
			AddString(mission_filenames[i]);
		else if ( (Campaign.type == CAMPAIGN_TYPE_MULTI_TEAMS) && (a_mission.game_type & MISSION_TYPE_MULTI_TEAMS) )
			AddString(mission_filenames[i]);

		i++;
	} 
	

	for (i=0; i<Campaign.num_missions; i++) {
		z = FindString(-1, Campaign.missions[i].name);
		if (z != LB_ERR) {
			DeleteString(z);  // take out all missions already in the campaign
			i--;  // recheck for name just in case there are two (should be impossible but can't be sure)
		}
	}
	
}

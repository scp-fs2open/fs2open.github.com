/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/CampaignTreeWnd.cpp $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * Campaign display tree window code.  Works very closely with the Campaign editor dialog box.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2006/01/11 04:46:17  phreak
 * Some campaign editor fixes:
 *
 * Filebox is initialized upon opening the window
 * Campaign will save to directories other than /data/missions/ or /mod/data/missions
 *
 * Revision 1.4  2003/03/03 04:15:24  Goober5000
 * FRED portion of tech room fix, plus some neatening up of the dialogs
 * --Goober5000
 *
 * Revision 1.3  2003/01/14 04:00:15  Goober5000
 * allowed for up to 256 main halls
 * --Goober5000
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:10:53  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 3     1/07/99 1:52p Andsager
 * Initial check in of Sexp_variables
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 27    9/16/98 3:08p Dave
 * Upped  max sexpression nodes to 1800 (from 1600). Changed FRED to sort
 * the ship list box. Added code so that tracker stats are not stored with
 * only 1 player.
 * 
 * 26    8/19/98 9:46a Hoffoss
 * Added some 'save first?' type checks to the campaign editor.
 * 
 * 25    5/26/98 2:32p Hoffoss
 * Made campaign editor come up with a new campaign instead of loading
 * built-in one.
 * 
 * 24    5/22/98 1:06a Hoffoss
 * Made Fred not use OLE.
 * 
 * 23    4/14/98 11:55a Allender
 * add end-of-campaign sexpression to allow for mission replay at the end
 * of campaigns
 * 
 * 22    3/31/98 12:23a Allender
 * changed macro names of campaign types to be more descriptive.  Added
 * "team" to objectives dialog for team v. team missions.  Added two
 * distinct multiplayer campaign types
 * 
 * 21    3/18/98 10:38p Allender
 * added required "num players" for multiplayer missions.  Put in required
 * "num players" for multiplayer campaigns.  Added campaign editor support
 * to determine "num players"
 * 
 * 20    2/24/98 10:23a Johnson
 * Fixed up some build bugs caused by a multiplayer struct removal.
 * 
 * 19    12/19/97 10:29a Allender
 * made initial status windows appear on top of campaign editor window
 * instead of Fred main window
 * 
 * 18    12/18/97 5:11p Allender
 * initial support for ship/weapon persistence
 * 
 * 17    12/02/97 5:29p Johnson
 * Fixed bug in branch sexp error checking.  Was using link number instead
 * of mission number.
 * 
 * 16    11/22/97 3:05p Allender
 * support for new fields for multiplayer in campaign editor
 * 
 * 15    9/16/97 4:19p Jasen
 * Fixed a bug in Fred with Campaign mode.
 * 
 * 14    9/01/97 6:31p Hoffoss
 * Added remaining missing features to campaign editor in Fred.
 * 
 * 13    8/14/97 11:54p Hoffoss
 * Added more error checking to Campaign editor, and made exit from
 * Campaign editor reload last mission in Fred (unless specifically
 * loading another mission).
 * 
 * 12    8/13/97 5:49p Hoffoss
 * Fixed bugs, made additions.
 * 
 * 11    8/13/97 12:46p Hoffoss
 * Added campaign error checker, accelerator table, and mission goal data
 * listings to sexp tree right click menu.
 * 
 * 10    7/09/97 2:28p Hoffoss
 * Fixed bug with adding new links, and made campaign general info update.
 * 
 * 9     5/15/97 12:45p Hoffoss
 * Extensive changes to fix many little bugs.
 * 
 * 8     5/14/97 12:54p Hoffoss
 * Added sexp tree for campaign branches, branch hilighting, and branch
 * reordering.
 * 
 * 7     5/13/97 12:46p Hoffoss
 * Added close campaign editor functions, changed global pointer to have
 * capped first letter.
 * 
 * 6     5/13/97 11:13a Hoffoss
 * Added remaining file menu options to campaign editor.
 * 
 * 5     5/13/97 10:52a Hoffoss
 * Added campaign saving code.
 * 
 * 4     5/09/97 9:50a Hoffoss
 * Routine code check in.
 * 
 * 3     5/05/97 9:40a Hoffoss
 * Campaign editor begun.
 * 
 * 2     5/01/97 4:11p Hoffoss
 * Started on Campaign editor stuff, being sidetracked with fixing bugs
 * now, though, so checking it for now.
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "FRED.h"
#include "CampaignTreeWnd.h"
#include "CampaignEditorDlg.h"
#include "CampaignTreeView.h"
#include "Management.h"
#include "MainFrm.h"
#include "FREDView.h"
#include "MissionSave.h"
#include "InitialShips.h"
#include "mission/missionparse.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int Mission_filename_cb_format;
int Campaign_modified = 0;
int Bypass_clear_mission;
campaign_tree_wnd *Campaign_wnd = NULL;

IMPLEMENT_DYNCREATE(campaign_tree_wnd, CFrameWnd)

/////////////////////////////////////////////////////////////////////////////
// campaign_tree_wnd

campaign_tree_wnd::campaign_tree_wnd()
{
	Bypass_clear_mission = 0;
}

campaign_tree_wnd::~campaign_tree_wnd()
{
}

BEGIN_MESSAGE_MAP(campaign_tree_wnd, CFrameWnd)
	//{{AFX_MSG_MAP(campaign_tree_wnd)
	ON_UPDATE_COMMAND_UI(ID_CPGN_FILE_OPEN, OnUpdateCpgnFileOpen)
	ON_COMMAND(ID_CPGN_FILE_OPEN, OnCpgnFileOpen)
	ON_WM_DESTROY()
	ON_COMMAND(ID_CPGN_FILE_SAVE, OnCpgnFileSave)
	ON_COMMAND(ID_CPGN_FILE_SAVE_AS, OnCpgnFileSaveAs)
	ON_COMMAND(ID_CPGN_FILE_NEW, OnCpgnFileNew)
	ON_COMMAND(ID_CLOSE, OnClose2)
	ON_COMMAND(ID_ERROR_CHECKER, OnErrorChecker)
	ON_WM_CLOSE()
	ON_COMMAND(ID_INITIAL_SHIPS, OnInitialShips)
	ON_COMMAND(ID_INITIAL_WEAPONS, OnInitialWeapons)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// campaign_tree_wnd message handlers

BOOL campaign_tree_wnd::OnCreateClient(LPCREATESTRUCT, CCreateContext* pContext)
{
	CSize s;

	LoadAccelTable("IDR_ACC_CAMPAIGN");
	Mission_filename_cb_format = RegisterClipboardFormat("Mission Filename");
	Campaign_modified = 0;
	clear_mission();

	// create a splitter with 1 row, 2 columns
	if (!m_splitter.CreateStatic(this, 1, 2))
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		return FALSE;
	}

	// add the first splitter pane - the campaign input form in column 0
	if (!m_splitter.CreateView(0, 0, RUNTIME_CLASS(campaign_editor), CSize(0, 0), pContext))
	{
		TRACE0("Failed to create first pane\n");
		return FALSE;
	}

	// add the second splitter pane - the campaign tree view in column 1
	if (!m_splitter.CreateView(0, 1, RUNTIME_CLASS(campaign_tree_view), CSize(240, 100), pContext))
	{
		TRACE0("Failed to create second pane\n");
		return FALSE;
	}

	Campaign_tree_formp = (campaign_editor *) m_splitter.GetPane(0, 0);
	Campaign_tree_viewp = (campaign_tree_view *) m_splitter.GetPane(0, 1);
	s = Campaign_tree_formp->GetTotalSize();
	m_splitter.SetColumnInfo(0, s.cx, 0);
	m_splitter.SetColumnInfo(1, 0, 0);
	m_splitter.RecalcLayout();

	// activate the input view
	SetActiveView(Campaign_tree_formp);
	OnCpgnFileNew();
//	Campaign_tree_formp->load_campaign();
	Fred_main_wnd->EnableWindow(FALSE);
	return TRUE;
}

void campaign_tree_wnd::OnUpdateCpgnFileOpen(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable();
}

void campaign_tree_wnd::OnCpgnFileOpen() 
{
	CString name;

	if (Campaign_modified)
		if (save_modified())
			return;

	CFileDialog dlg(TRUE, "fc2", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "FreeSpace Campaign files (*.fc2)|*.fc2||", this);
	if (dlg.DoModal() == IDOK)
	{
		name = dlg.GetFileName();
		if (strlen(name) > MAX_FILENAME_LEN - 1) {
			MessageBox("Filename is too long", "Error");
			return;
		}

		if (!strlen(name))
			return;

		string_copy(Campaign.filename, name, MAX_FILENAME_LEN);
		Campaign_tree_formp->load_campaign();
	}
}

void campaign_tree_wnd::OnDestroy()
{
	CString str;

	OnCpgnFileNew();
	Fred_main_wnd->EnableWindow(TRUE);
//	if (!Bypass_clear_mission)
//		create_new_mission();
	str = FREDDoc_ptr->GetPathName();
	if (str.IsEmpty())
		create_new_mission();
	else
		FREDDoc_ptr->OnOpenDocument(str);

	CFrameWnd::OnDestroy();
	Campaign_wnd = NULL;
	Fred_main_wnd->SetFocus();
}

void campaign_tree_wnd::OnCpgnFileSave() 
{
	CFred_mission_save save;

	Campaign_tree_formp->update();
	if (!Campaign.filename[0]) {
		OnCpgnFileSaveAs();
		return;
	}

	// sanity checking for multiplayer	
	/*
	if ( Campaign.type == TYPE_MULTI_PLAYER ) {
	
		if ( (Campaign.mc_info.min_players < 0) || (Campaign.mc_info.min_players > Campaign.mc_info.max_players) ) {
			MessageBox("Min players must be > 0 and <= max players", "Error", MB_OK | MB_ICONEXCLAMATION);
			return;
		}
		if ( (Campaign.mc_info.max_players < 0) || (Campaign.mc_info.max_players > MAX_PLAYERS) ) {
			char buf[256];

			sprintf(buf, "Max players must be > 0 and <= %d", MAX_PLAYERS );
			MessageBox(buf, "Error", MB_OK | MB_ICONEXCLAMATION);
			return;
		}
		if ( Campaign.mc_info.max_players < Campaign.mc_info.min_players ) {
			MessageBox("Max Players must be greater than min players", "Error", MB_OK | MB_ICONEXCLAMATION);
			return;
		}
	}
	*/	

	if (save.save_campaign_file(Campaign.filename))
	{
		MessageBox("An error occured while saving!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	Campaign_modified = 0;
	return;
}

void campaign_tree_wnd::OnCpgnFileSaveAs() 
{
	char *old_name = NULL;
	char campaign_path[256];
	CString name;
	CFred_mission_save save;

	Campaign_tree_formp->update();
	if (Campaign.filename[0])
		old_name = Campaign.filename;

	CFileDialog dlg(FALSE, "fc2", old_name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "FreeSpace Campaign files (*.fc2)|*.fc2||", this);
	if (dlg.DoModal() == IDOK)
	{
		name = dlg.GetFileName();
		if (strlen(name) > MAX_FILENAME_LEN - 1) {
			MessageBox("Filename is too long", "Error");
			return;
		}

		if (!strlen(name)){
			return;		
		}

		string_copy(Campaign.filename, name, MAX_FILENAME_LEN);
		string_copy(campaign_path, dlg.GetPathName(), 256);
		if (save.save_campaign_file(campaign_path))
		{
			MessageBox("An error occured while saving!", "Error", MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		Campaign_modified = 0;
	}
}

void campaign_tree_wnd::OnCpgnFileNew()
{
	if (Campaign_modified)
		if (save_modified())
			return;

	Campaign.filename[0] = 0;
	Campaign.num_missions = 0;
	Campaign.num_players = 0;
	strcpy(Campaign.name, "Unnamed");
	Campaign.desc = NULL;
	Campaign_tree_viewp->free_links();
	Campaign_tree_formp->initialize();
	Campaign_modified = 0;
	Campaign.flags = CF_DEFAULT_VALUE;
	((CButton *) (Campaign_tree_formp->GetDlgItem(IDC_CUSTOM_TECH_DB)))->SetCheck(0);
}

void campaign_tree_wnd::OnClose() 
{
	if (Campaign_modified)
		if (save_modified())
			return;

	CFrameWnd::OnClose();
}

void campaign_tree_wnd::OnClose2() 
{
	if (Campaign_modified)
		if (save_modified())
			return;

	DestroyWindow();
}

// returns 0 for success and 1 for cancel
int campaign_tree_wnd::save_modified()
{
	int r;

	r = MessageBox("This campaign has been modified.  Save changes first?", "Campaign Modified",
		MB_YESNOCANCEL | MB_ICONQUESTION);

	if (r == IDCANCEL)
		return 1;

	if (r == IDYES) {
		OnCpgnFileSave();
		if (Campaign_modified)  // error occured in saving.
			return 1;
	}

	Campaign_modified = 0;
	return 0;
}

void campaign_tree_wnd::OnErrorChecker() 
{
	Campaign_tree_formp->save_tree(0);
	error_checker();
	if (!g_err)
		MessageBox("No errors detected in campaign", "Woohoo!");
}

int campaign_tree_wnd::error_checker()
{
	int i, j, z;
	int mcount[MAX_CAMPAIGN_MISSIONS], true_at[MAX_CAMPAIGN_MISSIONS];

	for (i=0; i<MAX_CAMPAIGN_MISSIONS; i++) {
		mcount[i] = 0;
		true_at[i] = -1;
	}

	g_err = 0;
	for (i=0; i<Total_links; i++) {
		if ( (Links[i].from < 0) || (Links[i].from >= Campaign.num_missions) )
			return internal_error("Branch #%d has illegal source mission", i);
		if ( (Links[i].to < -1) || (Links[i].to >= Campaign.num_missions) )
			return internal_error("Branch #%d has illegal target mission", i);
		Sexp_useful_number = Links[i].from;
		if (fred_check_sexp(Links[i].sexp, OPR_BOOL, "formula of branch #%d", i))
			return -1;

		z = Links[i].from;
		mcount[z]++;
		if (Links[i].sexp == Locked_sexp_false)
			if (error("Mission \"%s\" branch %d is always false", Campaign.missions[z].name, mcount[z]))
				return 1;

		if (Links[i].sexp == Locked_sexp_true) {
			if (true_at[z] >= 0)
				if (error("Mission \"%s\" branch %d is true but is not last branch", Campaign.missions[z].name, true_at[z]))
					return 1;

			true_at[z] = mcount[z];
		}
	}

	// check that all missions in a multiplayer game have the same number of players
	if ( Campaign.type != CAMPAIGN_TYPE_SINGLE ) {
		for (i = 0; i < Campaign.num_missions; i++ ) {
			mission a_mission;

			get_mission_info(Campaign.missions[i].name, &a_mission);
			if ( a_mission.num_players != Campaign.num_players ) {
				if ( error("Mission \"%s\" has %d players.  Multiplayer campaign allows %d", Campaign.missions[i].name, a_mission.num_players, Campaign.num_players) )
					return 1;
			}
		}
	}

	for (i=0; i<Campaign.num_missions; i++)
		if (mcount[i] && true_at[i] < mcount[i])
			if (error("Mission \"%s\" last branch isn't set to true", Campaign.missions[i].name))
				return 1;

	for (i=z=0; i<Campaign.num_missions; i++) {
		for (j=0; j<Campaign.num_missions; j++)
			if ((i != j) && !stricmp(Campaign.missions[i].name, Campaign.missions[j].name))
				return internal_error("Mission \"%s\" is listed twice in campaign", Campaign.missions[i].name);

		if (!Campaign.missions[i].level)
			z++;
	}

	if (!z)
		if (error("No top level mission present in tree"))
			return 1;

	if (z > 1)
		return internal_error("More than one top level mission present in tree");

	return 0;
}

int campaign_tree_wnd::error(char *msg, ...)
{
	char buf[2048];
	va_list args;

	g_err++;
	va_start(args, msg);
	vsprintf(buf, msg, args);
	va_end(args);

	if (MessageBox(buf, "Error", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
		return 0;

	return 1;
}

int campaign_tree_wnd::internal_error(char *msg, ...)
{
	char buf[2048], buf2[2048];
	va_list args;

	g_err++;
	va_start(args, msg);
	vsprintf(buf, msg, args);
	va_end(args);

	sprintf(buf2, "%s\n\nThis is an internal error.  Please let Hoffoss\n"
		"know about this so he can fix it.  Click cancel to debug.", buf);

	if (MessageBox(buf2, "Internal Error", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
		Int3();  // drop to debugger so the problem can be analyzed.

	return -1;
}

int campaign_tree_wnd::fred_check_sexp(int sexp, int type, char *msg, ...)
{
	char buf[512], buf2[2048], buf3[4096];
	int err = 0, z, faulty_node;
	va_list args;

	va_start(args, msg);
	vsprintf(buf, msg, args);
	va_end(args);

	if (sexp == -1)
		return 0;

	z = check_sexp_syntax(sexp, type, 1, &faulty_node, SEXP_MODE_CAMPAIGN);
	if (!z)
		return 0;

	convert_sexp_to_string(sexp, buf2, SEXP_ERROR_CHECK_MODE);
	sprintf(buf3, "Error in %s: %s\n\nIn sexpression: %s\n\n(Error appears to be: %s)",
		buf, sexp_error_message(z), buf2, Sexp_nodes[faulty_node].text);

	if (z < 0 && z > -100)
		err = 1;

	if (err)
		return internal_error(buf3);

	if (error(buf3))
		return 1;

	return 0;
}

// code to deal with the initial ships that a player can choose
void campaign_tree_wnd::OnInitialShips() 
{
	InitialShips isd(Campaign_tree_formp);

	isd.m_initial_items = INITIAL_SHIPS;
	isd.DoModal();
}

void campaign_tree_wnd::OnInitialWeapons() 
{
	InitialShips isd(Campaign_tree_formp);

	isd.m_initial_items = INITIAL_WEAPONS;
	isd.DoModal();
}

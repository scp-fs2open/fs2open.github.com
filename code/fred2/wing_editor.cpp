/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/fred2/wing_editor.cpp $
 * $Revision: 1.4.2.2 $
 * $Date: 2007-02-11 09:25:42 $
 * $Author: taylor $
 *
 * Wing editor dialog box handler code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.4.2.1  2006/06/04 01:03:13  Goober5000
 * add fighterbay restriction code
 * --Goober5000
 *
 * Revision 1.4  2006/05/30 05:37:29  Goober5000
 * add capability to restrict arrival/departure paths
 * (FRED only; FS2 implementation to follow)
 * --Goober5000
 *
 * Revision 1.3  2006/04/20 06:32:01  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 1.2  2006/02/04 07:05:03  Goober5000
 * fixed several IFF bugs in FRED (plus one or two other bugs)
 * --Goober5000
 *
 * Revision 1.1  2006/01/19 02:27:32  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.14  2006/01/14 23:49:01  Goober5000
 * second pass; all the errors are fixed now; one more thing to take care of
 * --Goober5000
 *
 * Revision 1.13  2005/10/29 23:02:33  Goober5000
 * partial FRED commit of changes
 * --Goober5000
 *
 * Revision 1.12  2005/09/26 07:10:20  Goober5000
 * fix a small bug with custom wing names
 * --Goober5000
 *
 * Revision 1.11  2005/08/23 07:26:49  Goober5000
 * Noooooo!  Not the double-equals bug!
 * --Goober5000
 *
 * Revision 1.10  2005/07/12 23:49:13  Goober5000
 * remove restriction on names starting with "player"
 * --Goober5000
 *
 * Revision 1.9  2005/06/29 19:00:25  taylor
 * some departure target fixes:
 *  - the ships editor was just plain wrong since it wouldn't properly add ships to the departure target list
 *  - the wing editor tried to set the departure target before having a list to set with
 *
 * Revision 1.8  2005/03/29 03:43:11  phreak
 * ai directory fixes as well as fixes for the new jump node code
 *
 * Revision 1.7  2005/03/06 22:43:15  wmcoolmon
 * Fixx0red some of the jump node errors
 *
 * Revision 1.6  2004/12/15 19:16:13  Goober5000
 * FRED code for custom wing names
 * --Goober5000
 *
 * Revision 1.5  2004/09/17 07:56:51  Goober5000
 * bunch of FRED tweaks and fixes
 * --Goober5000
 *
 * Revision 1.4  2004/03/15 12:14:46  randomtiger
 * Fixed a whole heap of problems with Fred introduced by changes to global vars.
 *
 * Revision 1.3  2003/01/06 20:49:15  Goober5000
 * FRED2 support for wing squad logos - look in the wing editor
 * --Goober5000
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:11:04  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 3     8/16/99 10:52p Andsager
 * Allow closer ship positioning for NEAR_SHIP ship and wing arrivals.
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
 * 83    9/14/98 3:31p Allender
 * don't allow alpha, beta, and gamma to get > 1 wave when editing a
 * multiplayer missions
 * 
 * 82    6/17/98 4:50p Hoffoss
 * Added error checking for arrival delays used on wing player is in.
 * 
 * 81    5/22/98 10:10a Hoffoss
 * Fixed bug with hide cue button not working correctly.
 * 
 * 80    3/24/98 12:30p Allender
 * arrival/departure target boxes were getting initialized incorrectly
 * 
 * 79    3/21/98 7:36p Lawrance
 * Move jump nodes to own lib.
 * 
 * 78    3/16/98 8:27p Allender
 * Fred support for two new AI flags -- kamikaze and no dynamic goals.
 * 
 * 77    3/10/98 6:11p Hoffoss
 * Added jump node renaming abilities to Fred.
 * 
 * 76    2/23/98 9:48p Allender
 * added no arrival/departure warps to wings
 * 
 * 75    12/31/97 3:56p Hoffoss
 * Forced alpha wing to always have a true arrival cue.
 * 
 * 74    12/29/97 4:55p Johnson
 * Added some fixes.
 * 
 * 73    12/18/97 10:40a Hoffoss
 * Fixed bug with a few of the checkboxes not initializing properly.
 * 
 * 72    11/25/97 10:03a Allender
 * added no arrival message checkbox to wing editor
 * 
 * 71    11/25/97 9:42a Hoffoss
 * Removed starting wing checkbox from wing editor.
 * 
 * 70    11/13/97 4:14p Allender
 * automatic assignment of hotkeys for starting wings.  Appripriate
 * warnings when they are incorrectly used.  hotkeys correctly assigned to
 * ships/wing arriving after mission start
 * 
 * 69    11/11/97 2:13p Allender
 * docking bay support for Fred and FreeSpace.  Added hook to ai code for
 * arrival/departure from dock bays.  Fred support now sufficient.
 * 
 * 68    11/10/97 10:13p Allender
 * added departure anchor to Fred and FreeSpace in preparation for using
 * docking bays.  Functional in Fred, not in FreeSpace.
 * 
 * 67    10/28/97 3:33p Hoffoss
 * Fixed bug where <1 num_waves was being allowed to be entered into Fred.
 * 
 * 66    10/14/97 5:33p Hoffoss
 * Added Fred support (and fsm support) for the no_arrival_music flags in
 * ships and wings.
 * 
 * 65    10/01/97 12:37p Hoffoss
 * Changed Fred (and FreeSpace) to utilize alpha, beta and gamma as player
 * starting wing names.
 * 
 * 64    9/04/97 5:35p Hoffoss
 * Fixed arrival distance stuff.
 * 
 * 63    9/04/97 5:04p Johnson
 * Fixed bug with arrival target distance checking.
 * 
 * 62    9/04/97 4:30p Hoffoss
 * Removed sexp tree info from grayed trees.
 * 
 * 61    8/30/97 9:52p Hoffoss
 * Implemented arrival location, distance, and anchor in Fred.
 * 
 * 60    8/22/97 4:16p Hoffoss
 * added support for arrival and departure info in ship editor using
 * wing's info if editing marked ships in a wing instead of using ship's.
 * 
 * 59    8/21/97 3:20p Duncan
 * Fixed bug in wing renaming when a player is in the wing.
 * 
 * 58    8/19/97 1:44p Hoffoss
 * Fixed bug with updating too quickly (i.e. via prev and next buttons).
 * 
 * 57    8/15/97 5:14p Hoffoss
 * Completely changed around how initial orders dialog worked.  It's
 * pretty awesome now.
 * 
 * 56    8/15/97 11:24a Hoffoss
 * Changed order of events.
 * 
 * 55    8/13/97 11:31p Hoffoss
 * Added bound checking on min/max wave delay.
 * 
 * 54    8/13/97 11:22p Hoffoss
 * Implemented wave delay min and max in Fred.
 * 
 * 53    8/12/97 7:17p Hoffoss
 * Added previous button to ship and wing editors.
 * 
 * 52    8/12/97 6:32p Hoffoss
 * Added code to allow hiding of arrival and departure cues in editors.
 * 
 * 51    8/12/97 3:33p Hoffoss
 * Fixed the "press cancel to go to reference" code to work properly.
 * 
 * 50    8/12/97 1:55a Hoffoss
 * Made extensive changes to object reference checking and handling for
 * object deletion call.
 * 
 * 49    8/10/97 4:22p Hoffoss
 * Made main display update when ships or wings are renamed.
 * 
 * 48    8/08/97 10:00a Hoffoss
 * Added protection from threshold being equal or higher than the number
 * of ships in a wing.
 * 
 * 47    8/01/97 2:45p Hoffoss
 * Fixed bug with no new item in MFC CTreeCtrl selection changed message.
 * Throught it shouldn't happen, but Whiteside made it happen.
 * 
 * 46    7/30/97 5:23p Hoffoss
 * Removed Sexp tree verification code, since it duplicates normal sexp
 * verification, and is just another set of code to keep maintained.
 * 
 * 45    7/30/97 12:31p Hoffoss
 * Made improvements to ship goals editor (initial orders) to disallow
 * illegal orders.
 * 
 * 44    7/28/97 2:28p Hoffoss
 * Added bypasses to MFC integer validation routines.
 * 
 * 43    7/25/97 2:40p Hoffoss
 * Fixed bug in sexp tree selection updating handling.
 * 
 * 42    7/24/97 4:44p Hoffoss
 * Added sexp help to more dialogs, and changed made some changes to make
 * it work correctly for ship and wing editors.
 * 
 * 41    7/18/97 2:05p Hoffoss
 * Fixed some bugs BoundsChecker turned up.
 * 
 * 40    7/16/97 6:30p Hoffoss
 * Added icons to sexp trees, mainly because I think they will be required
 * for drag n drop.
 * 
 * 39    7/09/97 2:38p Allender
 * organized ship/wing editor dialogs.  Added protect ship and ignore
 * count checkboxes to those dialogs.  Changed flag code for
 * parse_objects.  Added unprotect sexpressions
 * 
 * 38    7/08/97 10:15a Allender
 * making ships/wings reinforcements now do not set the arrival cue to
 * false.  A reinforcement may only be available after its arrival cue is
 * true
 * 
 * 37    7/02/97 3:30p Hoffoss
 * Put in code to validate and bash if necessary the wing wave threshold.
 * 
 * 36    6/18/97 3:07p Hoffoss
 * Wing ship names are 1 indexes instead of 0 indexed now.
 * 
 * 35    6/05/97 6:10p Hoffoss
 * Added features: Autosaving, object hiding.  Also fixed some minor bugs.
 * 
 * 34    5/30/97 11:33a Allender
 * more hotkey combo box stuff
 * 
 * 33    5/23/97 1:53p Hoffoss
 * Fixed problems with modeless dialog updating.  It won't get caught in
 * an infinate loop anymore, but still gives an error warning 3 times when
 * using cancel and trying to switch window focus to main window.  Don't
 * know if I can fix that, but it's not too critical right now.
 * 
 * 32    5/01/97 4:15p Hoffoss
 * Fixed bugs.
 * 
 * 31    4/28/97 2:37p Hoffoss
 * Added hotkey editing to Fred for ships and wings.
 * 
 * 30    4/23/97 11:55a Hoffoss
 * Fixed many bugs uncovered while trying to create Mission 6.
 * 
 * 29    4/07/97 1:53p Hoffoss
 * Fixed a few bugs, and added sexp chain duplicating for object
 * duplicating.
 * 
 * 28    4/03/97 11:35a Hoffoss
 * Fixed bugs: viewpoint didn't reset, initial orders not updated when
 * referenced ship is renamed or deleted.
 * 
 * 27    4/01/97 5:15p Hoffoss
 * Fixed errors in max length checks, renaming a wing now renames the
 * ships in the wing as well, as it should.
 * 
 * 26    3/20/97 3:55p Hoffoss
 * Major changes to how dialog boxes initialize (load) and update (save)
 * their internal data.  This should simplify things and create less
 * problems.
 * 
 * 25    3/17/97 4:29p Hoffoss
 * Automated player's wing flaging as a starting player wing.
 * 
 * 24    3/05/97 10:54a Hoffoss
 * removed special arrival/departure cue token usage in wing editor.
 * 
 * 23    2/28/97 11:31a Hoffoss
 * Implemented modeless dialog saving and restoring, and changed some
 * variables names.
 * 
 * 22    2/27/97 3:16p Allender
 * major wing structure enhancement.  simplified wing code.  All around
 * better wing support
 * 
 * 21    2/24/97 5:38p Hoffoss
 * Added dialog box to name a wing at creation, code to change ship names
 * to match wing name, and code to maintain these ship names.
 * 
 * 20    2/21/97 5:34p Hoffoss
 * Added extensive modification detection and fixed a bug in initial
 * orders editor.
 * 
 * 19    2/20/97 4:03p Hoffoss
 * Several ToDo items: new reinforcement clears arrival cue, reinforcement
 * control from ship and wing dialogs, show grid toggle.
 * 
 * 18    2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "MainFrm.h"
#include "FRED.h"
#include "FREDDoc.h"
#include "Management.h"
#include "wing.h"
#include "globalincs/linklist.h"
#include "ai/aigoals.h"
#include "FREDView.h"
#include "starfield/starfield.h"
#include "jumpnode/jumpnode.h"
#include "cfile/cfile.h"
#include "restrictpaths.h"

#define ID_WING_MENU 9000

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// wing_editor dialog

wing_editor::wing_editor(CWnd* pParent /*=NULL*/)
	: CDialog(wing_editor::IDD, pParent)
{
	//{{AFX_DATA_INIT(wing_editor)
	m_wing_name = _T("");
	m_wing_squad_filename = _T("");
	m_special_ship = -1;
	m_waves = 0;
	m_threshold = 0;
	m_arrival_location = -1;
	m_departure_location = -1;
	m_arrival_delay = 0;
	m_departure_delay = 0;
	m_reinforcement = FALSE;
	m_hotkey = -1;
	m_ignore_count = FALSE;
	m_arrival_delay_max = 0;
	m_arrival_delay_min = 0;
	m_arrival_dist = 0;
	m_arrival_target = -1;
	m_no_arrival_music = FALSE;
	m_departure_target = -1;
	m_no_arrival_message = FALSE;
	m_no_arrival_warp = FALSE;
	m_no_departure_warp = FALSE;
	m_no_dynamic = FALSE;
	//}}AFX_DATA_INIT
	modified = 0;
	select_sexp_node = -1;
	bypass_errors = 0;
}

void wing_editor::DoDataExchange(CDataExchange* pDX)
{
	CString str;

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(wing_editor)
	DDX_Control(pDX, IDC_DEPARTURE_DELAY_SPIN, m_departure_delay_spin);
	DDX_Control(pDX, IDC_ARRIVAL_DELAY_SPIN, m_arrival_delay_spin);
	DDX_Control(pDX, IDC_DEPARTURE_TREE, m_departure_tree);
	DDX_Control(pDX, IDC_ARRIVAL_TREE, m_arrival_tree);
	DDX_Control(pDX, IDC_SPIN_WAVE_THRESHOLD, m_threshold_spin);
	DDX_Control(pDX, IDC_SPIN_WAVES, m_waves_spin);
	DDX_Text(pDX, IDC_WING_NAME, m_wing_name);
	DDX_Text(pDX, IDC_WING_SQUAD_LOGO, m_wing_squad_filename);
	DDX_CBIndex(pDX, IDC_WING_SPECIAL_SHIP, m_special_ship);
	DDX_CBIndex(pDX, IDC_ARRIVAL_LOCATION, m_arrival_location);
	DDX_CBIndex(pDX, IDC_DEPARTURE_LOCATION, m_departure_location);
	DDX_Check(pDX, IDC_REINFORCEMENT, m_reinforcement);
	DDX_CBIndex(pDX, IDC_HOTKEY, m_hotkey);
	DDX_Check(pDX, IDC_IGNORE_COUNT, m_ignore_count);
	DDX_Text(pDX, IDC_ARRIVAL_DISTANCE, m_arrival_dist);
	DDX_CBIndex(pDX, IDC_ARRIVAL_TARGET, m_arrival_target);
	DDX_Check(pDX, IDC_NO_ARRIVAL_MUSIC, m_no_arrival_music);
	DDX_CBIndex(pDX, IDC_DEPARTURE_TARGET, m_departure_target);
	DDX_Check(pDX, IDC_NO_ARRIVAL_MESSAGE, m_no_arrival_message);
	DDX_Check(pDX, IDC_NO_ARRIVAL_WARP, m_no_arrival_warp);
	DDX_Check(pDX, IDC_NO_DEPARTURE_WARP, m_no_departure_warp);
	DDX_Check(pDX, IDC_NO_DYNAMIC, m_no_dynamic);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate) {  // get dialog control values
		GetDlgItem(IDC_ARRIVAL_DELAY)->GetWindowText(str);
		m_arrival_delay = atoi(str);
		if (m_arrival_delay < 0)
			m_arrival_delay = 0;

		GetDlgItem(IDC_DEPARTURE_DELAY)->GetWindowText(str);
		m_departure_delay = atoi(str);
		if (m_departure_delay < 0)
			m_departure_delay = 0;

		GetDlgItem(IDC_WING_WAVES)->GetWindowText(str);
		m_waves = atoi(str);
		if (m_waves < 0)
			m_waves = 0;

		GetDlgItem(IDC_WING_WAVE_THRESHOLD)->GetWindowText(str);
		m_threshold = atoi(str);
		if (m_threshold < 0)
			m_threshold = 0;

		GetDlgItem(IDC_ARRIVAL_DELAY_MIN)->GetWindowText(str);
		m_arrival_delay_min = atoi(str);
		if (m_arrival_delay_min < 0)
			m_arrival_delay_min = 0;

		GetDlgItem(IDC_ARRIVAL_DELAY_MAX)->GetWindowText(str);
		m_arrival_delay_max = atoi(str);
		if (m_arrival_delay_max < 0)
			m_arrival_delay_max = 0;

	} else {
		DDX_Text(pDX, IDC_ARRIVAL_DELAY, m_arrival_delay);
		DDX_Text(pDX, IDC_DEPARTURE_DELAY, m_departure_delay);
		DDX_Text(pDX, IDC_WING_WAVES, m_waves);
		DDX_Text(pDX, IDC_WING_WAVE_THRESHOLD, m_threshold);
		DDX_Text(pDX, IDC_ARRIVAL_DELAY_MIN, m_arrival_delay_min);
		DDX_Text(pDX, IDC_ARRIVAL_DELAY_MAX, m_arrival_delay_max);
	}
}

BEGIN_MESSAGE_MAP(wing_editor, CDialog)
	//{{AFX_MSG_MAP(wing_editor)
	ON_WM_INITMENU()
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_WAVES, OnDeltaposSpinWaves)
	ON_NOTIFY(NM_RCLICK, IDC_ARRIVAL_TREE, OnRclickArrivalTree)
	ON_NOTIFY(NM_RCLICK, IDC_DEPARTURE_TREE, OnRclickDepartureTree)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_ARRIVAL_TREE, OnBeginlabeleditArrivalTree)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_DEPARTURE_TREE, OnBeginlabeleditDepartureTree)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_ARRIVAL_TREE, OnEndlabeleditArrivalTree)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_DEPARTURE_TREE, OnEndlabeleditDepartureTree)
	ON_BN_CLICKED(IDC_DELETE_WING, OnDeleteWing)
	ON_BN_CLICKED(IDC_DISBAND_WING, OnDisbandWing)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_GOALS2, OnGoals2)
	ON_BN_CLICKED(IDC_REINFORCEMENT, OnReinforcement)
	ON_BN_CLICKED(IDC_NEXT, OnNext)
	ON_NOTIFY(TVN_SELCHANGED, IDC_ARRIVAL_TREE, OnSelchangedArrivalTree)
	ON_NOTIFY(TVN_SELCHANGED, IDC_DEPARTURE_TREE, OnSelchangedDepartureTree)
	ON_BN_CLICKED(IDC_HIDE_CUES, OnHideCues)
	ON_BN_CLICKED(IDC_PREV, OnPrev)
	ON_CBN_SELCHANGE(IDC_ARRIVAL_LOCATION, OnSelchangeArrivalLocation)
	ON_CBN_SELCHANGE(IDC_DEPARTURE_LOCATION, OnSelchangeDepartureLocation)
	ON_CBN_SELCHANGE(IDC_HOTKEY, OnSelchangeHotkey)
	ON_BN_CLICKED(IDC_WING_SQUAD_LOGO_BUTTON, OnSquadLogo)
	ON_BN_CLICKED(IDC_RESTRICT_ARRIVAL, OnRestrictArrival)
	ON_BN_CLICKED(IDC_RESTRICT_DEPARTURE, OnRestrictDeparture)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// wing_editor message handlers

BOOL wing_editor::Create()
{
	BOOL r;
	int i;
	CComboBox *box;

	r = CDialog::Create(IDD, Fred_main_wnd);
	box = (CComboBox *) GetDlgItem(IDC_ARRIVAL_LOCATION);
	box->ResetContent();
	for (i=0; i<MAX_ARRIVAL_NAMES; i++)
		box->AddString(Arrival_location_names[i]);

	box = (CComboBox *) GetDlgItem(IDC_DEPARTURE_LOCATION);
	box->ResetContent();
	for (i=0; i<MAX_DEPARTURE_NAMES; i++)
		box->AddString(Departure_location_names[i]);

	m_hotkey = 0;
	m_waves_spin.SetRange(1, 99);
	m_arrival_tree.link_modified(&modified);  // provide way to indicate trees are modified in dialog
	m_arrival_tree.setup((CEdit *) GetDlgItem(IDC_HELP_BOX));
	m_departure_tree.link_modified(&modified);
	m_departure_tree.setup();
	m_arrival_delay_spin.SetRange(0, 999);
	m_departure_delay_spin.SetRange(0, 999);

	initialize_data(1);
	return r;
}

void wing_editor::OnInitMenu(CMenu* pMenu)
{
	CMenu *m;

	m = pMenu->GetSubMenu(0);
	clear_menu(m);
	generate_wing_popup_menu(m, ID_WING_MENU, MF_ENABLED);
	if (cur_wing != -1)
		m->CheckMenuItem(ID_WING_MENU + cur_wing, MF_BYCOMMAND | MF_CHECKED);

	CDialog::OnInitMenu(pMenu);
}

void wing_editor::OnOK()
{
	HWND h;
	CWnd *w;

	w = GetFocus();
	if (w) {
		h = w->m_hWnd;
		GetDlgItem(IDC_ARRIVAL_TREE)->SetFocus();
		::SetFocus(h);
	}
}

void wing_editor::OnClose() 
{
	if (verify() && (!bypass_errors)) {
		SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		bypass_errors = 0;
		return;
	}

	if (update_data()) {
		SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		bypass_errors = 0;
		return;
	}

	SetWindowPos(Fred_main_wnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
	Fred_main_wnd->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

// initialize everything that update_data_safe() saves.
void wing_editor::initialize_data_safe(int full_update)
{
	int i, enable = TRUE, player_wing = 0, player_enabled = 1;
	CComboBox *arrival_box, *departure_box;

	nprintf(("Fred routing", "Wing dialog load safe\n"));
	if (!GetSafeHwnd())
		return;

	arrival_box = (CComboBox *) GetDlgItem(IDC_ARRIVAL_TARGET);
	departure_box = (CComboBox *)GetDlgItem(IDC_DEPARTURE_TARGET);

	m_ignore_count = 0;
	if (cur_wing < 0) {
		m_wing_squad_filename = _T("");
		m_special_ship = -1;
		m_arrival_location = -1;
		m_departure_location = -1;
		m_arrival_delay = 0;
		m_arrival_delay_min = 0;
		m_arrival_delay_max = 0;
		m_arrival_dist = 0;
		m_arrival_target = -1;
		m_departure_target = -1;
		m_departure_delay = 0;
		m_arrival_tree.clear_tree();
		m_departure_tree.clear_tree();
		m_arrival_tree.DeleteAllItems();
		m_departure_tree.DeleteAllItems();
		m_reinforcement = FALSE;
		m_hotkey = 0;
		m_ignore_count = 0;
		m_no_arrival_music = 0;
		m_no_arrival_message = 0;
		m_no_arrival_warp = 0;
		m_no_departure_warp = 0;
		m_no_dynamic = 0;
		player_enabled = enable = FALSE;

	} else {
		CComboBox *ptr;

		if (The_mission.game_type & MISSION_TYPE_MULTI)
		{
			if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS)
			{
				for (i=0; i<MAX_TVT_WINGS; i++)
				{
					if (cur_wing == TVT_wings[i])
						player_enabled = 0;
				}
			}
			else
			{
				for (i=0; i<MAX_STARTING_WINGS; i++)
				{
					if (cur_wing == Starting_wings[i])
						player_enabled = 0;
				}
			}
		}
		else
		{
			if (cur_wing == Ships[Player_start_shipnum].wingnum)
				player_enabled = 0;
		}

		if ((Player_start_shipnum >= 0) && (Player_start_shipnum < MAX_SHIPS) && (Ships[Player_start_shipnum].objnum >= 0))
			if (Ships[Player_start_shipnum].wingnum == cur_wing)
				player_wing = 1;

		m_wing_squad_filename = _T(Wings[cur_wing].wing_squad_filename);
		m_special_ship = Wings[cur_wing].special_ship;
		m_waves = Wings[cur_wing].num_waves;
		m_threshold = Wings[cur_wing].threshold;
		m_arrival_location = Wings[cur_wing].arrival_location;
		m_departure_location = Wings[cur_wing].departure_location;
		m_arrival_delay = Wings[cur_wing].arrival_delay;
		m_arrival_delay_min = Wings[cur_wing].wave_delay_min;
		m_arrival_delay_max = Wings[cur_wing].wave_delay_max;
		m_arrival_dist = Wings[cur_wing].arrival_distance;
		m_arrival_target = Wings[cur_wing].arrival_anchor;
		m_departure_target = Wings[cur_wing].departure_anchor;
		m_no_dynamic = (Wings[cur_wing].flags & WF_NO_DYNAMIC)?1:0;

		// Add the ships/special items to the combo box here before data is updated
		if ( m_arrival_location == ARRIVE_FROM_DOCK_BAY ) {
			management_add_ships_to_combo( arrival_box, SHIPS_2_COMBO_DOCKING_BAY_ONLY );
		} else {
			management_add_ships_to_combo( arrival_box, SHIPS_2_COMBO_SPECIAL | SHIPS_2_COMBO_ALL_SHIPS );
		}

		// Goober5000 - gah, this is ridiculous!  Prior to this point in the function (and only in this function),
		// m_arrival_target seems to refer to the arrival anchor.  The rest of the time, it refers to the index
		// of the drop-down list.
		if (m_arrival_target >= 0)
		{
			if (m_arrival_target & SPECIAL_ARRIVAL_ANCHOR_FLAG)
			{
				// figure out what the box represents this as
				char tmp[NAME_LENGTH + 15];
				stuff_special_arrival_anchor_name(tmp, m_arrival_target, 0);
	
				// find it in the box
				m_arrival_target = arrival_box->FindStringExact(-1, tmp);
			}
			else
			{
				// find it in the box
				m_arrival_target = arrival_box->FindStringExact(-1, Ships[m_arrival_target].ship_name);
			}
		}

		// add the ships to the departure target combo box
		if ( m_departure_location == DEPART_AT_DOCK_BAY ) {
			management_add_ships_to_combo( departure_box, SHIPS_2_COMBO_DOCKING_BAY_ONLY );
		} else {
			departure_box->ResetContent();
		}

		if ( m_departure_target >= 0 )
			m_departure_target = departure_box->FindStringExact(-1, Ships[m_departure_target].ship_name);

		m_departure_delay = Wings[cur_wing].departure_delay;
		if (player_wing)
			m_arrival_tree.load_tree(Locked_sexp_true);
		else
			m_arrival_tree.load_tree(Wings[cur_wing].arrival_cue);

		m_departure_tree.load_tree(Wings[cur_wing].departure_cue, "false");
		m_hotkey = Wings[cur_wing].hotkey+1;
		if (Wings[cur_wing].flags & WF_IGNORE_COUNT)
			m_ignore_count = 1;
		else
			m_ignore_count = 0;

		if (Wings[cur_wing].flags & WF_NO_ARRIVAL_MUSIC)
			m_no_arrival_music = 1;
		else
			m_no_arrival_music = 0;

		if ( Wings[cur_wing].flags & WF_NO_ARRIVAL_MESSAGE )
			m_no_arrival_message = 1;
		else
			m_no_arrival_message = 0;

		if ( Wings[cur_wing].flags & WF_NO_ARRIVAL_WARP )
			m_no_arrival_warp = 1;
		else
			m_no_arrival_warp = 0;

		if ( Wings[cur_wing].flags & WF_NO_DEPARTURE_WARP )
			m_no_departure_warp = 1;
		else
			m_no_departure_warp = 0;

		ptr = (CComboBox *) GetDlgItem(IDC_WING_SPECIAL_SHIP);
		ptr->ResetContent();
		for (i=0; i<Wings[cur_wing].wave_count; i++)
			ptr->AddString(Ships[Wings[cur_wing].ship_index[i]].ship_name);

		m_threshold_spin.SetRange(0, Wings[cur_wing].wave_count - 1);
		for (i=0; i<Num_reinforcements; i++)
			if (!stricmp(Reinforcements[i].name, Wings[cur_wing].name))
				break;

		if (i < Num_reinforcements)
			m_reinforcement = TRUE;
		else
			m_reinforcement = FALSE;
	}

	if (full_update)
		UpdateData(FALSE);

	GetDlgItem(IDC_WING_NAME)->EnableWindow(enable);
	GetDlgItem(IDC_WING_SQUAD_LOGO_BUTTON)->EnableWindow(enable);
	GetDlgItem(IDC_WING_SPECIAL_SHIP)->EnableWindow(enable);
	GetDlgItem(IDC_WING_WAVES)->EnableWindow(player_enabled);
	GetDlgItem(IDC_WING_WAVE_THRESHOLD)->EnableWindow(player_enabled);
	GetDlgItem(IDC_DISBAND_WING)->EnableWindow(enable);
	GetDlgItem(IDC_SPIN_WAVES)->EnableWindow(player_enabled);
	GetDlgItem(IDC_SPIN_WAVE_THRESHOLD)->EnableWindow(player_enabled);
	GetDlgItem(IDC_ARRIVAL_LOCATION)->EnableWindow(enable);

	GetDlgItem(IDC_ARRIVAL_DELAY)->EnableWindow(player_enabled);
	GetDlgItem(IDC_ARRIVAL_DELAY_MIN)->EnableWindow(player_enabled);
	GetDlgItem(IDC_ARRIVAL_DELAY_MAX)->EnableWindow(player_enabled);
	GetDlgItem(IDC_ARRIVAL_DELAY_SPIN)->EnableWindow(player_enabled);
	if (m_arrival_location) {
		GetDlgItem(IDC_ARRIVAL_DISTANCE)->EnableWindow(enable);
		GetDlgItem(IDC_ARRIVAL_TARGET)->EnableWindow(enable);
	} else {
		GetDlgItem(IDC_ARRIVAL_DISTANCE)->EnableWindow(FALSE);
		GetDlgItem(IDC_ARRIVAL_TARGET)->EnableWindow(FALSE);
	}
	if (m_arrival_location == ARRIVE_FROM_DOCK_BAY) {
		GetDlgItem(IDC_RESTRICT_ARRIVAL)->EnableWindow(enable);
	} else {
		GetDlgItem(IDC_RESTRICT_ARRIVAL)->EnableWindow(FALSE);
	}
	GetDlgItem(IDC_NO_DYNAMIC)->EnableWindow(enable);

	if (m_departure_location) {
		GetDlgItem(IDC_DEPARTURE_TARGET)->EnableWindow(enable);
	} else {
		GetDlgItem(IDC_DEPARTURE_TARGET)->EnableWindow(FALSE);
	}
	if (m_departure_location == DEPART_AT_DOCK_BAY) {
		GetDlgItem(IDC_RESTRICT_DEPARTURE)->EnableWindow(enable);
	} else {
		GetDlgItem(IDC_RESTRICT_DEPARTURE)->EnableWindow(FALSE);
	}

	if (player_wing)
		GetDlgItem(IDC_ARRIVAL_TREE)->EnableWindow(0);
	else
		GetDlgItem(IDC_ARRIVAL_TREE)->EnableWindow(enable);

	GetDlgItem(IDC_DEPARTURE_LOCATION)->EnableWindow(enable);
	GetDlgItem(IDC_DEPARTURE_DELAY)->EnableWindow(enable);
	GetDlgItem(IDC_DEPARTURE_DELAY_SPIN)->EnableWindow(enable);
	GetDlgItem(IDC_DEPARTURE_TREE)->EnableWindow(enable);
	GetDlgItem(IDC_GOALS2)->EnableWindow(enable);
	GetDlgItem(IDC_DELETE_WING)->EnableWindow(enable);
	GetDlgItem(IDC_REINFORCEMENT)->EnableWindow(enable);
	GetDlgItem(IDC_HOTKEY)->EnableWindow(enable);
	GetDlgItem(IDC_IGNORE_COUNT)->EnableWindow(enable);
	GetDlgItem(IDC_NO_ARRIVAL_MUSIC)->EnableWindow(enable);
	GetDlgItem(IDC_NO_ARRIVAL_MESSAGE)->EnableWindow(enable);
	GetDlgItem(IDC_NO_ARRIVAL_WARP)->EnableWindow(enable);
	GetDlgItem(IDC_NO_DEPARTURE_WARP)->EnableWindow(enable);

	if (cur_wing >= 0) {
		enable = TRUE;

		// check to see if the wing has a ship which is not a fighter/bomber type.  If so, then disable
		// the wing_waves and wing_threshold  stuff
		for (i = 0; i < Wings[cur_wing].wave_count; i++ ) {
			int sflag;

			sflag = Ship_info[Ships[Wings[cur_wing].ship_index[i]].ship_info_index].flags;
			if ( !(sflag & SIF_FIGHTER) && !(sflag & SIF_BOMBER) )
				enable = FALSE;
		}

	} else
		enable = FALSE;
}

void wing_editor::initialize_data(int full_update)
{
	int i;
	CWnd *w = NULL;

	nprintf(("Fred routing", "Wing dialog load\n"));
	if (!GetSafeHwnd())
		return;

	m_arrival_tree.select_sexp_node = m_departure_tree.select_sexp_node = select_sexp_node;
	select_sexp_node = -1;
	if (cur_wing == -1)
		m_wing_name = _T("");
	else
		m_wing_name = _T(Wings[cur_wing].name);

	initialize_data_safe(full_update);
	modified = 0;
	if (w)
		w -> SetFocus();

	i = m_arrival_tree.select_sexp_node;
	if (i != -1) {
		w = GetDlgItem(IDC_ARRIVAL_TREE);
		m_arrival_tree.hilite_item(i);

	} else {
		i = m_departure_tree.select_sexp_node;
		if (i != -1) {
			w = GetDlgItem(IDC_DEPARTURE_TREE);
			m_departure_tree.hilite_item(i);
		}
	}
}

// update wing structure(s) with dialog data.  The data is first checked for errors.  If
// no errors occur, returns 0.  If an error occurs, returns -1.  If the update is bypassed,
// returns 1.  Bypass is necessary to avoid an infinite loop, and it doesn't actually
// update the data.  Bypass only occurs if bypass mode is active and we still get an error.
// Once the error no longer occurs, bypass mode is cleared and data is updated.
int wing_editor::update_data(int redraw)
{
	char *str, old_name[255], buf[512];
	int i, z;
	object *ptr;

	nprintf(("Fred routing", "Wing dialog save\n"));
	if (!GetSafeHwnd())
		return 0;

	UpdateData(TRUE);
	UpdateData(TRUE);

	m_wing_name.Trim(); 

	if (cur_wing >= 0) {
		for (i=0; i<MAX_WINGS; i++)
			if (Wings[i].wave_count && !stricmp(Wings[i].name, m_wing_name) && (i != cur_wing)) {
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				z = MessageBox("This wing name is already being used by another wing\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_wing_name = _T(Wings[cur_wing].name);
				UpdateData(FALSE);
			}

		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (ptr->type == OBJ_SHIP) {
				if (!stricmp(m_wing_name, Ships[ptr->instance].ship_name)) {
					if (bypass_errors)
						return 1;

					bypass_errors = 1;
					z = MessageBox("This wing name is already being used by a ship\n"
						"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

					if (z == IDCANCEL)
						return -1;

					m_wing_name = _T(Wings[cur_wing].name);
					UpdateData(FALSE);
				}
			}

			ptr = GET_NEXT(ptr);
		}

		for (i=0; i<MAX_WAYPOINT_LISTS; i++)
			if (Waypoint_lists[i].count && !stricmp(Waypoint_lists[i].name, m_wing_name)) {
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				z = MessageBox("This wing name is already being used by a waypoint path\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_wing_name = _T(Wings[cur_wing].name);
				UpdateData(FALSE);
			}

			if(jumpnode_get_by_name(m_wing_name) != NULL)
			{
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				z = MessageBox("This wing name is already being used by a jump node\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_wing_name = _T(Wings[cur_wing].name);
				UpdateData(FALSE);
			}

		strcpy(old_name, Wings[cur_wing].name);
		string_copy(Wings[cur_wing].name, m_wing_name, NAME_LENGTH, 1);
		update_data_safe();

		update_custom_wing_indexes();

		bypass_errors = 0;
		modified = 0;
		str = Wings[cur_wing].name;
		if (stricmp(old_name, str)) {
			update_sexp_references(old_name, str);
			ai_update_goal_references(REF_TYPE_WING, old_name, str);
			update_texture_replacements(old_name, str);
			for (i=0; i<Num_reinforcements; i++)
				if (!stricmp(old_name, Reinforcements[i].name)) {
					Assert(strlen(str) < NAME_LENGTH);
					strcpy(Reinforcements[i].name, str);
				}

			for (i=0; i<Wings[cur_wing].wave_count; i++) {
				if ((Objects[wing_objects[cur_wing][i]].type == OBJ_SHIP) || (Objects[wing_objects[cur_wing][i]].type == OBJ_START)) {
					sprintf(buf, "%s %d", str, i + 1);
					rename_ship(Wings[cur_wing].ship_index[i], buf);
				}
			}

			Update_window = 1;
		}

		if (set_reinforcement(str, m_reinforcement) == 1) {
			free_sexp2(Wings[cur_wing].arrival_cue);
			Wings[cur_wing].arrival_cue = Locked_sexp_false;
		}
	}

	if (redraw)
		update_map_window();

	return 0;
}

// update parts of wing that can't fail.  This is useful if for when you need to change
// something in a wing that this updates elsewhere in Fred.  Normally when auto-update
// kicks in, the changes you make will be wiped out by the auto=update, so instead you
// would call this function to update the wing, make your changes, and then call the
// initialize_data_safe() function to show your changes in the dialog
void wing_editor::update_data_safe()
{
	char buf[512];
	int i, d, hotkey = -1;

	nprintf(("Fred routing", "Wing dialog save safe\n"));
	if (!GetSafeHwnd())
		return;

	UpdateData(TRUE);
	UpdateData(TRUE);

	if (cur_wing < 0)
		return;

	if (m_threshold >= Wings[cur_wing].wave_count) {
		m_threshold = Wings[cur_wing].wave_count - 1;
		if (!bypass_errors) {
			sprintf(buf, "Wave threshold is set too high.  Value has been lowered to %d", (int) m_threshold);
			MessageBox(buf);
		}
	}

	if (m_threshold + Wings[cur_wing].wave_count > MAX_SHIPS_PER_WING) {
		m_threshold = MAX_SHIPS_PER_WING - Wings[cur_wing].wave_count;
		if (!bypass_errors) {
			sprintf(buf, "Wave threshold is set too high.  Value has been lowered to %d", (int) m_threshold);
			MessageBox(buf);
		}
	}

	if (m_waves < 1) {
		m_waves = 1;
		if (!bypass_errors) {
			sprintf(buf, "Number of waves illegal.  Has been set to 1.", (int) m_waves);
			MessageBox(buf);
		}
	}

	MODIFY(Wings[cur_wing].special_ship, m_special_ship);
	MODIFY(Wings[cur_wing].num_waves, m_waves);
	MODIFY(Wings[cur_wing].threshold, m_threshold);
	MODIFY(Wings[cur_wing].arrival_location, m_arrival_location);
	MODIFY(Wings[cur_wing].departure_location, m_departure_location);
	MODIFY(Wings[cur_wing].arrival_delay, m_arrival_delay);
	if (m_arrival_delay_min > m_arrival_delay_max) {
		if (!bypass_errors) {
			sprintf(buf, "Arrival delay minimum greater than maximum.  Value lowered to %d", m_arrival_delay_max);
			MessageBox(buf);
		}

		m_arrival_delay_min = m_arrival_delay_max;
	}

	MODIFY(Wings[cur_wing].wave_delay_min, m_arrival_delay_min);
	MODIFY(Wings[cur_wing].wave_delay_max, m_arrival_delay_max);
	MODIFY(Wings[cur_wing].arrival_distance, m_arrival_dist);
	if (m_arrival_target >= 0) {
		i = ((CComboBox *) GetDlgItem(IDC_ARRIVAL_TARGET)) -> GetItemData(m_arrival_target);
		MODIFY(Wings[cur_wing].arrival_anchor, i);

		// when arriving near or in front of a ship, be sure that we are far enough away from it!!!
		if (((m_arrival_location != ARRIVE_AT_LOCATION) && (m_arrival_location != ARRIVE_FROM_DOCK_BAY)) && (i >= 0) && !(i & SPECIAL_ARRIVAL_ANCHOR_FLAG)) {
			d = int(min(500, 2.0f * Objects[Ships[i].objnum].radius));
			if ((Wings[cur_wing].arrival_distance < d) && (Wings[cur_wing].arrival_distance > -d)) {
				if (!bypass_errors) {
					sprintf(buf, "Ship must arrive at least %d meters away from target.\n"
						"Value has been reset to this.  Use with caution!\r\n"
						"Reccomended distance is %d meters.\r\n", d, (int)(2.0f * Objects[Ships[i].objnum].radius) );

					MessageBox(buf);
				}

				if (Wings[cur_wing].arrival_distance < 0)
					Wings[cur_wing].arrival_distance = -d;
				else
					Wings[cur_wing].arrival_distance = d;

				m_arrival_dist = Wings[cur_wing].arrival_distance;
			}
		}
	}

	i = ((CComboBox*)GetDlgItem(IDC_DEPARTURE_TARGET))->GetItemData(m_departure_target);
	MODIFY(Wings[cur_wing].departure_anchor,  i);

	MODIFY(Wings[cur_wing].departure_delay, m_departure_delay);
	hotkey = m_hotkey - 1;
	MODIFY(Wings[cur_wing].hotkey, hotkey);
	if ( m_ignore_count ) {
		if ( !(Wings[cur_wing].flags & WF_IGNORE_COUNT) )
			set_modified();
		Wings[cur_wing].flags |= WF_IGNORE_COUNT;

	} else {
		if ( Wings[cur_wing].flags & WF_IGNORE_COUNT )
			set_modified();
		Wings[cur_wing].flags &= ~WF_IGNORE_COUNT;
	}

	if ( m_no_arrival_music ) {
		if ( !(Wings[cur_wing].flags & WF_NO_ARRIVAL_MUSIC) )
			set_modified();
		Wings[cur_wing].flags |= WF_NO_ARRIVAL_MUSIC;

	} else {
		if ( Wings[cur_wing].flags & WF_NO_ARRIVAL_MUSIC )
			set_modified();
		Wings[cur_wing].flags &= ~WF_NO_ARRIVAL_MUSIC;
	}

	// check the no message flag
	if ( m_no_arrival_message ) {
		if ( !(Wings[cur_wing].flags & WF_NO_ARRIVAL_MESSAGE) )
			set_modified();
		Wings[cur_wing].flags |= WF_NO_ARRIVAL_MESSAGE;

	} else {
		if ( Wings[cur_wing].flags & WF_NO_ARRIVAL_MESSAGE )
			set_modified();
		Wings[cur_wing].flags &= ~WF_NO_ARRIVAL_MESSAGE;
	}

	// set the no warp effect for wings flag
	if ( m_no_arrival_warp ) {
		if ( !(Wings[cur_wing].flags & WF_NO_ARRIVAL_WARP) )
			set_modified();
		Wings[cur_wing].flags |= WF_NO_ARRIVAL_WARP;
	} else {
		if ( Wings[cur_wing].flags & WF_NO_ARRIVAL_WARP )
			set_modified();
		Wings[cur_wing].flags &= ~WF_NO_ARRIVAL_WARP;
	}
	// set the no warp effect for wings flag
	if ( m_no_departure_warp ) {
		if ( !(Wings[cur_wing].flags & WF_NO_DEPARTURE_WARP) )
			set_modified();
		Wings[cur_wing].flags |= WF_NO_DEPARTURE_WARP;
	} else {
		if ( Wings[cur_wing].flags & WF_NO_DEPARTURE_WARP )
			set_modified();
		Wings[cur_wing].flags &= ~WF_NO_DEPARTURE_WARP;
	}

	if ( m_no_dynamic ) {
		if ( !(Wings[cur_wing].flags & WF_NO_DYNAMIC) )
			set_modified();
		Wings[cur_wing].flags |= WF_NO_DYNAMIC;
	} else {
		if ( Wings[cur_wing].flags & WF_NO_DYNAMIC )
			set_modified();
		Wings[cur_wing].flags &= ~WF_NO_DYNAMIC;
	}

	if (Wings[cur_wing].arrival_cue >= 0)
		free_sexp2(Wings[cur_wing].arrival_cue);
	Wings[cur_wing].arrival_cue = m_arrival_tree.save_tree();

	if (Wings[cur_wing].departure_cue >= 0)
		free_sexp2(Wings[cur_wing].departure_cue);
	Wings[cur_wing].departure_cue = m_departure_tree.save_tree();

	// copy squad stuff
	if(stricmp(m_wing_squad_filename, Wings[cur_wing].wing_squad_filename))
	{
		string_copy(Wings[cur_wing].wing_squad_filename, m_wing_squad_filename, MAX_FILENAME_LEN);
		set_modified();
	}

	if (modified)
		set_modified();
}

BOOL wing_editor::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id, wing;

	id = LOWORD(wParam);
	if (id >= ID_WING_MENU && id < ID_WING_MENU + MAX_WINGS) {
		if (!update_data()) {
			wing = id - ID_WING_MENU;
			mark_wing(wing);
			return 1;
		}
	}

	return CDialog::OnCommand(wParam, lParam);
}

void wing_editor::OnDeltaposSpinWaves(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int new_pos;
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	
	new_pos = pNMUpDown->iPos + pNMUpDown->iDelta;
	if (new_pos > 0 && new_pos < 100)	{
		*pResult = 0;
		modified = 1;

	} else
		*pResult = 1;
}

void wing_editor::OnRclickArrivalTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_arrival_tree.right_clicked();	
	*pResult = 0;
}

void wing_editor::OnRclickDepartureTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_departure_tree.right_clicked();
	*pResult = 0;
}

void wing_editor::OnBeginlabeleditArrivalTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if (m_arrival_tree.edit_label(pTVDispInfo->item.hItem) == 1)	{
		*pResult = 0;
		modified = 1;

	} else
		*pResult = 1;
}

void wing_editor::OnBeginlabeleditDepartureTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if (m_departure_tree.edit_label(pTVDispInfo->item.hItem) == 1)	{
		*pResult = 0;
		modified = 1;

	} else
		*pResult = 1;
}

void wing_editor::OnEndlabeleditArrivalTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	*pResult = m_arrival_tree.end_label_edit(pTVDispInfo->item.hItem, pTVDispInfo->item.pszText);
}

void wing_editor::OnEndlabeleditDepartureTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	*pResult = m_departure_tree.end_label_edit(pTVDispInfo->item.hItem, pTVDispInfo->item.pszText);
}

int wing_editor::verify()
{
	nprintf(("Fred routing", "Wing dialog verify\n"));
	if (!GetSafeHwnd() || !modified)
		return 0;

	if (bypass_errors)
		return 1;

	return 0;
}

// delete wing and all ships that are part of the wing
void wing_editor::OnDeleteWing()
{
	modified = 0;  // no need to run update checks, since wing will be gone shortly anyway.
	delete_wing(cur_wing);
}

// delete wing, but leave ships intact and wingless
void wing_editor::OnDisbandWing()
{
	modified = 0;  // no need to run update checks, since wing will be gone shortly anyway.
	remove_wing(cur_wing);
}

void wing_editor::OnGoals2()
{
	ShipGoalsDlg dlg_goals;

	Assert(cur_wing != -1);
	dlg_goals.self_wing = cur_wing;
	dlg_goals.DoModal();
	if (query_initial_orders_conflict(cur_wing))
		MessageBox("One or more ships of this wing also has initial orders",
			"Possible conflict");
}

void wing_editor::OnReinforcement()
{
	UpdateData(TRUE);
	UpdateData(TRUE);
	//if (m_reinforcement)
	//	m_arrival_tree.clear_tree("false");
}

void wing_editor::OnPrev() 
{
	int wing, count = 0;

	if (!update_data() && Num_wings) {
		wing = cur_wing - 1;
		if (wing < 0)
			wing = MAX_WINGS - 1;

		while (!Wings[wing].wave_count) {
			wing--;
			if (count++ > MAX_WINGS)
				return;

			if (wing < 0)
				wing = MAX_WINGS - 1;
		}

		mark_wing(wing);
		Wing_editor_dialog.initialize_data(1);
		Update_wing = 0;
	}

	return;
}

void wing_editor::OnNext() 
{
	int wing, count = 0;

	if (!update_data() && Num_wings) {
		wing = cur_wing + 1;
		if (wing >= MAX_WINGS)
			wing = 0;

		while (!Wings[wing].wave_count) {
			wing++;
			if (count++ > MAX_WINGS)
				return;

			if (wing >= MAX_WINGS)
				wing = 0;
		}

		mark_wing(wing);
		Wing_editor_dialog.initialize_data(1);
		Update_wing = 0;
	}

	return;
}

void wing_editor::OnSelchangedArrivalTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HTREEITEM h;

	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	h = pNMTreeView->itemNew.hItem;
	if (h)
		m_arrival_tree.update_help(h);

	*pResult = 0;
}

void wing_editor::OnSelchangedDepartureTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HTREEITEM h;

	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	h = pNMTreeView->itemNew.hItem;
	if (h)
		m_departure_tree.update_help(h);

	*pResult = 0;
}

void wing_editor::calc_cue_height()
{
	CRect cue;

	GetDlgItem(IDC_CUE_FRAME)->GetWindowRect(cue);
	cue_height = cue.bottom - cue.top + 10;
	if (Show_sexp_help)
		cue_height += SEXP_HELP_BOX_SIZE;

	if (Hide_wing_cues) {
		((CButton *) GetDlgItem(IDC_HIDE_CUES)) -> SetCheck(1);
		OnHideCues();
	}
}

void wing_editor::show_hide_sexp_help()
{
	CRect rect;

	if (Show_sexp_help)
		cue_height += SEXP_HELP_BOX_SIZE;
	else
		cue_height -= SEXP_HELP_BOX_SIZE;

	if (((CButton *) GetDlgItem(IDC_HIDE_CUES)) -> GetCheck())
		return;

	GetWindowRect(rect);
	if (Show_sexp_help)
		rect.bottom += SEXP_HELP_BOX_SIZE;
	else
		rect.bottom -= SEXP_HELP_BOX_SIZE;

	MoveWindow(rect);
}

void wing_editor::OnHideCues() 
{
	CRect rect;

	GetWindowRect(rect);
	if (((CButton *) GetDlgItem(IDC_HIDE_CUES)) -> GetCheck()) {
		rect.bottom -= cue_height;
		Hide_wing_cues = 1;

	} else {
		rect.bottom += cue_height;
		Hide_wing_cues = 0;
	}

	MoveWindow(rect);
}

void wing_editor::OnSelchangeArrivalLocation() 
{
	CComboBox *box;

	box = (CComboBox *)GetDlgItem(IDC_ARRIVAL_TARGET);
	UpdateData();
	if (m_arrival_location) {
		GetDlgItem(IDC_ARRIVAL_DISTANCE)->EnableWindow(TRUE);
		GetDlgItem(IDC_ARRIVAL_TARGET)->EnableWindow(TRUE);
		if (m_arrival_target < 0) {
			m_arrival_target = 0;
		}

		// determine which items we should put into the arrival target combo box
		if ( m_arrival_location == ARRIVE_FROM_DOCK_BAY ) {
			management_add_ships_to_combo( box, SHIPS_2_COMBO_DOCKING_BAY_ONLY );
		} else {
			management_add_ships_to_combo( box, SHIPS_2_COMBO_SPECIAL | SHIPS_2_COMBO_ALL_SHIPS );
		}
	} else {
		m_arrival_target = -1;
		GetDlgItem(IDC_ARRIVAL_DISTANCE)->EnableWindow(FALSE);
		GetDlgItem(IDC_ARRIVAL_TARGET)->EnableWindow(FALSE);
	}

	if (m_arrival_location == ARRIVE_FROM_DOCK_BAY)	{
		GetDlgItem(IDC_RESTRICT_ARRIVAL)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_RESTRICT_ARRIVAL)->EnableWindow(FALSE);
	}

	UpdateData(FALSE);
}

void wing_editor::OnSelchangeDepartureLocation() 
{
	CComboBox *box;

	box = (CComboBox *)GetDlgItem(IDC_DEPARTURE_TARGET);
	UpdateData();
	if (m_departure_location) {
		GetDlgItem(IDC_DEPARTURE_TARGET)->EnableWindow(TRUE);
		if (m_departure_target < 0) {
			m_departure_target = 0;
		}
		// we need to build up the list box content based on the departure type.  When
		// from a docking bay, only show ships in the list which have them.  Show all ships otherwise
		if ( m_departure_location == DEPART_AT_DOCK_BAY ) {
			management_add_ships_to_combo( box, SHIPS_2_COMBO_DOCKING_BAY_ONLY );
		} else {
			// I think that this section is currently illegal
			Int3();
		}
	} else {
		m_departure_target = -1;
		GetDlgItem(IDC_DEPARTURE_TARGET)->EnableWindow(FALSE);
	}

	if (m_departure_location == DEPART_AT_DOCK_BAY)	{
		GetDlgItem(IDC_RESTRICT_DEPARTURE)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_RESTRICT_DEPARTURE)->EnableWindow(FALSE);
	}

	UpdateData(FALSE);
}

// see if hotkey should possibly be reserved for player starting wing
void wing_editor::OnSelchangeHotkey() 
{
	char buf[256];
	int set_num, i;
	
	UpdateData(TRUE);
	set_num = m_hotkey - 1;	// hotkey sets are 1 index based

	// first, determine if we are currently working with a starting wing
	for ( i = 0; i < MAX_STARTING_WINGS; i++ ) {
		if ( !stricmp( Wings[cur_wing].name, Starting_wing_names[i]) )
			break;
	}
	if ( i == MAX_STARTING_WINGS )
		return;

	// we have a player starting wing.  See if we assigned a non-standard hotkey
	if ( (set_num >= MAX_STARTING_WINGS) || (set_num != i) ) {
		sprintf(buf, "Assigning nonstandard hotkey to wing %s (default is F%d)", Wings[cur_wing].name, 5+i);
		MessageBox(buf, NULL, MB_OK | MB_ICONEXCLAMATION );
	}
}

void wing_editor::OnSquadLogo()
{	
	int z;
	char *Logo_ext =	"Image Files (*.dds, *.pcx)|*.dds;*.pcx|"
						"DDS Files (*.dds)|*.dds|"
						"PCX Files (*.pcx)|*.pcx|"
						"All Files (*.*)|*.*|"
						"|";

	//phreak 05/05/2003
	//this needs to be here or else the data in the wing editor dialog will revert 
	//to what it was before it was opened.
	UpdateData(TRUE);

	// get list of squad images
	z = cfile_push_chdir(CF_TYPE_SQUAD_IMAGES);
	CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, Logo_ext);

	// if we have a result
	if (dlg.DoModal() == IDOK) {
		m_wing_squad_filename = dlg.GetFileName();		
	} else {
		m_wing_squad_filename = _T("");
	}
	UpdateData(FALSE);		

	// restore directory
	if (!z){
		cfile_pop_dir();
	}	
}

// Goober5000
void wing_editor::OnRestrictArrival()
{
	int arrive_from_ship;
	CComboBox *box;
	restrict_paths dlg;

	// grab stuff from GUI
	UpdateData(TRUE);

	if (m_arrival_location != ARRIVE_FROM_DOCK_BAY)
	{
		Int3();
		return;
	}

	box = (CComboBox *) GetDlgItem(IDC_ARRIVAL_TARGET);
	if (box->GetCount() == 0)
		return;

	arrive_from_ship = box->GetItemData(m_arrival_target);

	if (!ship_has_dock_bay(arrive_from_ship))
	{
		Int3();
		return;
	}

	dlg.m_arrival = true;
	dlg.m_ship_class = Ships[arrive_from_ship].ship_info_index;
	dlg.m_path_mask = &Wings[cur_wing].arrival_path_mask;

	dlg.DoModal();
}

// Goober5000
void wing_editor::OnRestrictDeparture()
{
	int depart_to_ship;
	CComboBox *box;
	restrict_paths dlg;

	// grab stuff from GUI
	UpdateData(TRUE);

	if (m_departure_location != DEPART_AT_DOCK_BAY)
	{
		Int3();
		return;
	}

	box = (CComboBox *) GetDlgItem(IDC_DEPARTURE_TARGET);
	if (box->GetCount() == 0)
		return;

	depart_to_ship = box->GetItemData(m_departure_target);

	if (!ship_has_dock_bay(depart_to_ship))
	{
		Int3();
		return;
	}

	dlg.m_arrival = false;
	dlg.m_ship_class = Ships[depart_to_ship].ship_info_index;
	dlg.m_path_mask = &Wings[cur_wing].departure_path_mask;

	dlg.DoModal();
}

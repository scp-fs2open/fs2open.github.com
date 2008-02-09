/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Fred2/MainFrm.cpp $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * MainFrm.cpp : implementation of the CMainFrame class
 * The main frame class of a document/view architechure, which we hate but must
 * deal with, due to Microsoft limiting our freedom and forcing us to use whether
 * we want to or not.  The main frame is basically the container window that other
 * view windows are within.  In Fred, our view window is always maximized inside
 * the main frame window, so you can't tell the difference between the two.  A few
 * old MFC events are handled here because the people working on the code before
 * me (Hoffoss) decided to put it here.  I've been putting it all in FredView.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.8  2006/01/16 00:34:34  Goober5000
 * final (I hope) IFF fixage involving the FRED dialogs :p
 * --Goober5000
 *
 * Revision 1.7  2005/12/29 08:21:00  wmcoolmon
 * No my widdle FRED, I didn't forget about you ^_^ (codebase commit)
 *
 * Revision 1.6  2005/09/26 03:37:35  Goober5000
 * species upgrade stuff for FRED
 * --Goober5000
 *
 * Revision 1.5  2005/07/13 01:03:10  Goober5000
 * improved species support and removed need for #define
 * --Goober5000
 *
 * Revision 1.4  2004/02/05 23:23:19  randomtiger
 * Merging OGL Fred into normal Fred, no conflicts, should be perfect
 *
 * Revision 1.3.2.1  2004/01/24 13:00:43  randomtiger
 * Implemented basic OGL HT&L but some drawing routines missing from cobe.lib so keeping turned off for now.
 * Added lighting menu option and made about dialog more useful.
 * Ship menu now defaults to the first entry.
 * Loads of debug to try and trap crashes.
 *
 * Revision 1.3  2003/10/15 22:10:34  Kazan
 * Da Species Update :D
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:10:59  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 8     10/13/99 2:55p Jefff
 * fixed unnumbered XSTRs
 * 
 * 7     10/13/99 9:22a Daveb
 * Fixed Fred jumpnode placing bug. Fixed 1024 glide tiled texture problem
 * related to movies. Fixed launcher spawning from PXO screen.
 * 
 * 6     9/14/99 11:07p Andsager
 * Improve freddoc error message.  Fix Frerd2 about box for "2" part of
 * Fred2
 * 
 * 5     9/08/99 12:07a Andsager
 * Add browser based help to Fred
 * 
 * 4     9/01/99 10:15a Dave
 * 
 * 3     11/19/98 8:36a Dave
 * Removed ST reordering of ships in Fred.
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
 * 31    9/16/98 6:54p Dave
 * Upped  max sexpression nodes to 1800 (from 1600). Changed FRED to sort
 * the ship list box. Added code so that tracker stats are not stored with
 * only 1 player.
 * 
 * 30    3/09/98 10:56a Hoffoss
 * Added jump node objects to Fred.
 * 
 * 29    2/20/98 12:56p Adam
 * Made ship class box even wider.
 * 
 * 28    2/20/98 12:07p Hoffoss
 * Made ship type combo box on toolbar wider.
 * 
 * 27    9/30/97 2:11p Hoffoss
 * Removed player start item from toolbar combo box.
 * 
 * 26    8/21/97 5:39p Hoffoss
 * Fixed warning when building optimized.  Was MFC code I copied, which
 * says something about MFC I guess.
 * 
 * 25    8/17/97 10:22p Hoffoss
 * Fixed several bugs in Fred with Undo feature.  In the process, recoded
 * a lot of CFile.cpp.
 * 
 * 24    8/14/97 2:32p Hoffoss
 * fixed bug where controlling an object doesn't cause screen updates, and
 * added a number of cool features to viewpoint/control object code.
 * 
 * 23    7/21/97 3:57p Hoffoss
 * Removed group combo box from toolbar, since I don't think I'll ever get
 * it working right.
 * 
 * 22    6/26/97 12:39p Mike
 * Add ship_type to ship_info and ships.tbl.
 * 
 * 21    6/09/97 4:57p Hoffoss
 * Added autosave and undo to Fred.
 * 
 * 20    5/05/97 1:35p Hoffoss
 * View window is now refocused when a new ship type selection is made.
 * 
 * 19    5/01/97 10:54a Hoffoss
 * Removed obsolete files/classes from Fred project, and any reference to
 * them.
 * 
 * 18    4/17/97 2:01p Hoffoss
 * All dialog box window states are saved between sessions now.
 * 
 * 17    4/03/97 11:35a Hoffoss
 * Fixed bugs: viewpoint didn't reset, initial orders not updated when
 * referenced ship is renamed or deleted.
 * 
 * 16    3/10/97 4:58p Hoffoss
 * Added waypoint and start types to drop down toolbar combo box and fixed
 * context menu new ship type selection to also work.
 * 
 * 15    3/10/97 12:54p Hoffoss
 * Added drop down combo box to toolbar and fixed compiling errors Mark
 * (maybe Mike?) introduced to code.
 * 
 * 14    2/28/97 11:31a Hoffoss
 * Implemented modeless dialog saving and restoring, and changed some
 * variables names.
 * 
 * 13    2/27/97 5:54p Hoffoss
 * Implemented support for saving and restoring window positions.
 * 
 * 12    2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "FRED.h"

#include "MainFrm.h"

#include "FREDDoc.h"
#include "FREDView.h"

#include "MessageEditorDlg.h"
#include "ShipClassEditorDlg.h"
#include "MissionNotesDlg.h"
#include "Grid.h"
#include "dialog1.h"
#include "species_defs/species_defs.h"
#include "iff_defs/iff_defs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_MESSAGE(WM_MENU_POPUP_EDIT, OnMenuPopupTest)
	ON_CBN_SELCHANGE(ID_NEW_SHIP_TYPE, OnNewShipTypeChange)

	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_EDITORS_AI_CLASSES, OnEditorsAiClasses)
	ON_COMMAND(ID_EDITORS_GOALS, OnEditorsGoals)
	ON_COMMAND(ID_EDITORS_ART, OnEditorsArt)
	ON_COMMAND(ID_EDITORS_MUSIC, OnEditorsMusic)
	ON_COMMAND(ID_EDITORS_SHIP_CLASSES, OnEditorsShipClasses)
	ON_COMMAND(ID_EDITORS_SOUND, OnEditorsSound)
	ON_COMMAND(ID_EDITORS_TERRAIN, OnEditorsTerrain)
	ON_COMMAND(ID_FILE_MISSIONNOTES, OnFileMissionnotes)
	ON_WM_LBUTTONUP()
	ON_WM_DESTROY()
	ON_COMMAND(ID_VIEW_STATUS_BAR, OnViewStatusBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STATUS_BAR, OnUpdateViewStatusBar)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_LEFT, OnUpdateLeft)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_RIGHT, OnUpdateRight)
	ON_COMMAND(ID_MIKE_GRIDCONTROL, OnMikeGridcontrol)
	ON_COMMAND(IDR_MENU_POPUP_TOGGLE1, OnMenuPopupToggle1)
	ON_UPDATE_COMMAND_UI(IDR_MENU_POPUP_TOGGLE1, OnUpdateMenuPopupToggle1)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_HELP_INPUT_INTERFACE, OnHelpInputInterface)
	ON_WM_CLOSE()
	ON_WM_INITMENU()
	ON_COMMAND(ID_HELP_FINDER, OnFredHelp)
	ON_COMMAND(ID_HELP, OnFredHelp)
	//ON_COMMAND(ID_CONTEXT_HELP, OnFredHelp)
	//ON_COMMAND(ID_DEFAULT_HELP, OnFredHelp)
	//}}AFX_MSG_MAP
	// Global help commands
END_MESSAGE_MAP()

#define FRED_HELP_URL "\\data\\freddocs\\index.html"

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_INDICATOR_MODIFIED,
	ID_SEPARATOR,
//	ID_INDICATOR_LEFT,
//	ID_INDICATOR_RIGHT,
//	ID_INDICATOR_CAPS,
//	ID_INDICATOR_NUM,
//	ID_INDICATOR_SCRL,
};

CMainFrame	*Fred_main_wnd;
color_combo_box	m_new_ship_type_combo_box;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int z;
	CRect rect;

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	// Create the combo box
	z = m_wndToolBar.CommandToIndex(ID_NEW_SHIP_TYPE);
	Assert(z != -1);
	m_wndToolBar.SetButtonInfo(z, ID_NEW_SHIP_TYPE, TBBS_SEPARATOR, 230);

	// Design guide advises 12 pixel gap between combos and buttons
//	m_wndToolBar.SetButtonInfo(1, ID_SEPARATOR, TBBS_SEPARATOR, 12);
	m_wndToolBar.GetItemRect(z, &rect);
	rect.top = 3;
	rect.bottom = rect.top + 550;
	if (!m_new_ship_type_combo_box.Create(CBS_DROPDOWNLIST | WS_VISIBLE | WS_VSCROLL | CBS_HASSTRINGS | LBS_OWNERDRAWFIXED,
		rect, &m_wndToolBar, ID_NEW_SHIP_TYPE))
	{
		TRACE0("Failed to create new ship type combo-box\n");
		return FALSE;
	}

/*	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
*/

/*	if (!m_wndStatusBar.Create(this,
		WS_CHILD | WS_VISIBLE | CBRS_BOTTOM, ID_MY_STATUS_BAR) ||
		!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))*/

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;

	} else {
		m_wndStatusBar.SetPaneInfo(0, 0, SBPS_STRETCH, 0);
		m_wndStatusBar.SetPaneInfo(1, 0, SBPS_NORMAL, 80);
		m_wndStatusBar.SetPaneInfo(2, 0, SBPS_NORMAL, 180);
//		m_wndStatusBar.SetPaneInfo(3, 0, SBPS_NORMAL, 100);
		m_wndStatusBar.SetPaneInfo(4, 0, SBPS_NORMAL, 130);
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	Fred_main_wnd = this;
	Ship_editor_dialog.Create();
	Wing_editor_dialog.Create();
	Waypoint_editor_dialog.Create();
	init_tools();
	LoadBarState("Tools state");
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

//void CMainFrame::OnEditorsShips() 
//{
//	CShipEditorDlg	dlg;
//
//	dlg.DoModal();
//		
//}

void CMainFrame::OnEditorsAiClasses() 
{
}

void CMainFrame::OnEditorsGoals() 
{
}

void CMainFrame::OnEditorsArt() 
{
}

void CMainFrame::OnEditorsMusic() 
{
}

void CMainFrame::OnEditorsShipClasses() 
{
}

void CMainFrame::OnEditorsSound() 
{
}

void CMainFrame::OnEditorsTerrain() 
{
}

void CMainFrame::OnFileMissionnotes() 
{
	CMissionNotesDlg	dlg;

	dlg.DoModal();
}

// I have been unable to get this message event to occur.
void CMainFrame::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CFrameWnd::OnLButtonUp(nFlags, point);
}

// This event is invoked when you click on the black X in the upper right corner
// or when you do File/Exit.
void CMainFrame::OnDestroy() 
{
	Fred_main_wnd = NULL;	
	CFrameWnd::OnDestroy();
}

void CMainFrame::OnViewStatusBar() 
{
	m_wndStatusBar.ShowWindow((m_wndStatusBar.GetStyle() & WS_VISIBLE) == 0);
	RecalcLayout();
}

void CMainFrame::OnUpdateViewStatusBar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck((m_wndStatusBar.GetStyle() & WS_VISIBLE) != 0);
}

void CMainFrame::OnUpdateLeft(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(::GetKeyState(VK_LBUTTON) < 0);
}

void CMainFrame::OnUpdateRight(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(::GetKeyState(VK_RBUTTON) < 0);
}

void CMainFrame::OnMikeGridcontrol() 
{
	CGrid	dlg;

	dlg.DoModal();
}

int	Toggle1_var = 0;

void CMainFrame::OnMenuPopupToggle1() 
{
	if (Toggle1_var == 0)
		Toggle1_var = 1;
	else
		Toggle1_var = 0;
	
}

void CMainFrame::OnUpdateMenuPopupToggle1(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(Toggle1_var);
}

LONG CMainFrame::OnMenuPopupTest(UINT wParam, LONG lParam)
{
	CMenu	menu;
	CPoint	point;

	point = * ((CPoint*) lParam);
	
	ClientToScreen(&point);

	menu.LoadMenu(IDR_MENU1);
	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);

	return 0L;
}

CPoint	Global_point2;

void CMainFrame::OnRButtonDown(UINT nFlags, CPoint point) 
{
	Global_point2 = point;

	PostMessage(WM_MENU_POPUP_TEST, nFlags, (int) &Global_point2);
	CFrameWnd::OnRButtonDown(nFlags, point);
}

void CMainFrame::OnHelpInputInterface() 
{
	dialog1	dlg;

	dlg.DoModal();
}

void CMainFrame::OnClose()
{
	theApp.write_ini_file();
	SaveBarState("Tools state");
	CFrameWnd::OnClose();
}

void CMainFrame::init_tools()
{
	static int count = 0;
	int i;
	//int highest_terran_index;
	//char ship_name[256];
	//int ship_index;

	Assert(count < 2);

	// add 
	if (count++) {
		for (i=0; i<Num_ship_classes; i++){
			// don't add the pirate ship
			if(Ship_info[i].flags & SIF_NO_FRED){
				m_new_ship_type_combo_box.AddString("");
				continue;
			}

			m_new_ship_type_combo_box.AddString(Ship_info[i].name);
		}

//		m_new_ship_type_combo_box.AddString("Player Start");		
		m_new_ship_type_combo_box.AddString("Jump Node");
		m_new_ship_type_combo_box.AddString("Waypoint");		

		/*
		// now we want to sort special ships (mission disk) ----------------------
		highest_terran_index = 0;
		memset(ship_name, 0, 256);
		while(m_new_ship_type_combo_box.GetLBText(highest_terran_index, ship_name) != CB_ERR){
			ship_index = ship_info_lookup(ship_name);
			if((ship_index < 0) || (ship_index >= Num_ship_classes) || (Ship_info[ship_index].species != 0)){
				break;
			}
			highest_terran_index++;
		}		
		*/
		m_new_ship_type_combo_box.SetCurSel(0);
	}  
}

void CMainFrame::OnNewShipTypeChange()
{
	if (Fred_view_wnd)
		Fred_view_wnd->SetFocus();
}

void color_combo_box::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	int m_cyText = 24, z;
	CString strText;
	char ship_name[256];

	// You must override DrawItem and MeasureItem for LBS_OWNERDRAWVARIABLE
	ASSERT((GetStyle() & (LBS_OWNERDRAWFIXED | CBS_HASSTRINGS)) ==
		(LBS_OWNERDRAWFIXED | CBS_HASSTRINGS));

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	// I think we need to do a lookup by ship name here	
	if(lpDrawItemStruct->itemID >= (uint)Num_ship_classes){
		z = lpDrawItemStruct->itemID;
	} else {		
		memset(ship_name, 0, 256);
		GetLBText(lpDrawItemStruct->itemID, ship_name);
		z = ship_info_lookup(ship_name);
	}

	if ((z >= 0) && (lpDrawItemStruct->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)))
	{
		int cyItem = GetItemHeight(z);
		BOOL fDisabled = !IsWindowEnabled();

		COLORREF newTextColor = RGB(0x80, 0x80, 0x80);  // light gray
		if (!fDisabled)
		{
			if (z >= Num_ship_classes)
				newTextColor = RGB(0, 0, 0);
			else
			{
				species_info *sinfo = &Species_info[Ship_info[z].species];
				newTextColor = RGB(sinfo->fred_color.rgb.r, sinfo->fred_color.rgb.g, sinfo->fred_color.rgb.b);
			}
		}

		COLORREF oldTextColor = pDC->SetTextColor(newTextColor);
		COLORREF newBkColor = GetSysColor(COLOR_WINDOW);
		COLORREF oldBkColor = pDC->SetBkColor(newBkColor);

		if (newTextColor == newBkColor)
			newTextColor = RGB(0xC0, 0xC0, 0xC0);   // dark gray

		if (!fDisabled && ((lpDrawItemStruct->itemState & ODS_SELECTED) != 0))
		{
			pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
			pDC->SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
		}

		if (m_cyText == 0)
			VERIFY(cyItem >= CalcMinimumItemHeight());

		if (z == Id_select_type_jump_node)
			strText = _T("Jump Node");
		else if (z == Id_select_type_start)
			strText = _T("Player Start");
		else if (z == Id_select_type_waypoint)
			strText = _T("Waypoint");
		else
			strText = _T(Ship_info[z].name);
//		GetLBText(lpDrawItemStruct->itemID, strText);

		pDC->ExtTextOut(lpDrawItemStruct->rcItem.left,
			lpDrawItemStruct->rcItem.top + max(0, (cyItem - m_cyText) / 2),
			ETO_OPAQUE, &(lpDrawItemStruct->rcItem), strText, strText.GetLength(), NULL);

		pDC->SetTextColor(oldTextColor);
		pDC->SetBkColor(oldBkColor);
	}

	if ((lpDrawItemStruct->itemAction & ODA_FOCUS) != 0)
		pDC->DrawFocusRect(&(lpDrawItemStruct->rcItem));
}

int color_combo_box::CalcMinimumItemHeight()
{
	int nResult = 1;

	if ((GetStyle() & (LBS_HASSTRINGS | LBS_OWNERDRAWFIXED)) ==
		(LBS_HASSTRINGS | LBS_OWNERDRAWFIXED))
	{
		CClientDC dc(this);
		CFont* pOldFont = dc.SelectObject(GetFont());
		TEXTMETRIC tm;
		VERIFY (dc.GetTextMetrics ( &tm ));
		dc.SelectObject(pOldFont);

		nResult = tm.tmHeight;
	}

	return nResult;
}

void color_combo_box::MeasureItem(LPMEASUREITEMSTRUCT)
{
	// You must override DrawItem and MeasureItem for LBS_OWNERDRAWVARIABLE
	ASSERT((GetStyle() & (LBS_OWNERDRAWFIXED | CBS_HASSTRINGS)) ==
		(LBS_OWNERDRAWFIXED | CBS_HASSTRINGS));
}

int color_combo_box::SetCurSelNEW(int model_index)
{	
	if((model_index < 0) || (model_index >= Num_ship_classes)){
		return SetCurSel(model_index);
	}	

	// lookup the ship name
	return FindString(0, Ship_info[model_index].name);
}

int color_combo_box::GetCurSelNEW()
{
	int cur_sel;
	int ship_info;
	char ship_name[256];
	char *hmmm = ship_name;

	// see if we have a special item (>= Num_ship_classes)
	cur_sel = GetCurSel();
	if(cur_sel >= Num_ship_classes){
		return cur_sel;
	}

	// otherwise lookup the ship by name
	memset(ship_name, 0, 256);
	if(GetLBText(cur_sel, hmmm) == CB_ERR){
		return CB_ERR;
	}
	ship_info = ship_info_lookup(ship_name);
	if((ship_info < 0) || (ship_info >= Num_ship_classes)){
		return CB_ERR;
	}
	return ship_info;
}

void CMainFrame::OnInitMenu(CMenu* pMenu) 
{
	int i;
	CString str;
	extern int ID_SHOW_IFF[MAX_IFFS];

	if (Undo_available && !FREDDoc_ptr->undo_desc[1].IsEmpty())
		str = "Undo " + FREDDoc_ptr->undo_desc[1] + "\tCtrl+Z";
	else
		str = "Undo\tCtrl+Z";

	if (pMenu->GetMenuState(ID_EDIT_UNDO, MF_BYCOMMAND) != -1)
		pMenu->ModifyMenu(ID_EDIT_UNDO, MF_BYCOMMAND, ID_EDIT_UNDO, str);

	// Goober5000 - do the IFF menu options
	for (i = 0; i < MAX_IFFS; i++)
	{
		if (i < Num_iffs)
		{
			char text[NAME_LENGTH + 7];
			sprintf(text, "Show %s", Iff_info[i].iff_name);

			pMenu->ModifyMenu(ID_SHOW_IFF[i], MF_BYCOMMAND | MF_STRING, ID_SHOW_IFF[i], text);
		}
		else
		{
			pMenu->DeleteMenu(ID_SHOW_IFF[i], MF_BYCOMMAND);
		}
	}	

	CFrameWnd::OnInitMenu(pMenu);
}


void url_launch(char *url)
{
	int r;

	r = (int) ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOW);
	if (r < 32) {
		char *txt = NULL;

		switch (r) {
			case 0:	txt = XSTR("The operating system is out of memory or resources.", 1107); break;
			case ERROR_BAD_FORMAT: txt = XSTR("The .EXE file is invalid (non-Win32 .EXE or error in .EXE image).", 1108); break;
			case SE_ERR_ACCESSDENIED: txt = XSTR("The operating system denied access to the specified file. ", 1109); break;
			case SE_ERR_ASSOCINCOMPLETE: txt = XSTR("The filename association is incomplete or invalid.\r\n(You need to have a default Internet browser installed)", 1110); break;
			case SE_ERR_DDEBUSY: txt = XSTR("The DDE transaction could not be completed because other DDE transactions were being processed.", 1111); break;
			case SE_ERR_DDEFAIL: txt = XSTR("The DDE transaction failed.", 1112); break;
			case SE_ERR_DDETIMEOUT: txt = XSTR("The DDE transaction could not be completed because the request timed out.", 1113); break;
			case SE_ERR_DLLNOTFOUND: txt = XSTR("The specified dynamic-link library was not found.", 1114); break;
			case SE_ERR_OOM: txt = XSTR("There was not enough memory to complete the operation.", 1115); break;
			case SE_ERR_SHARE: txt = XSTR("A sharing violation occurred.", 1116); break;

			// No browser installed message
			case SE_ERR_NOASSOC:
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND: txt =	XSTR("\r\nUnable to locate Fred Help file: \\data\\freddocs\\index.html\r\n", 1479); break;

			default: txt = XSTR("Unknown error occurred.", 1118); break;
		}
		AfxMessageBox(txt, MB_OK | MB_ICONERROR);
	}
}


void CMainFrame::OnFredHelp()
{
	char buffer[_MAX_PATH];

	// get exe path
	strcpy(buffer, Fred_exe_dir);

	// strip exe name
	char *last_slash = strrchr(buffer, '\\');
	if ( last_slash == NULL) {
		return;
	} else {
		*last_slash = 0;
	}

	// add rest of path
	strcat(buffer, FRED_HELP_URL);

	// shell_open url
	url_launch(buffer);
}


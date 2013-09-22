/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
	int i;
	//int highest_terran_index;
	//char ship_name[256];
	//int ship_index;

	// some bizarre Volition check:
	static int count = 0;
	count++;
	if (count == 1) {
		return;
	} else if (count >= 3) {
		Warning(LOCATION, "CMainFrame::init_tools was called more than twice!  Trace out and fix.");
		return;
	}

	for (i=0; i<Num_ship_classes; i++){
		// don't add the pirate ship
		if(Ship_info[i].flags & SIF_NO_FRED){
			m_new_ship_type_combo_box.AddString("");
			continue;
		}

		m_new_ship_type_combo_box.AddString(Ship_info[i].name);
	}

//	m_new_ship_type_combo_box.AddString("Player Start");		
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
		const char *txt = NULL;

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
	strcpy_s(buffer, Fred_exe_dir);

	// strip exe name
	char *last_slash = strrchr(buffer, '\\');
	if ( last_slash == NULL) {
		return;
	} else {
		*last_slash = 0;
	}

	// add rest of path
	strcat_s(buffer, FRED_HELP_URL);

	// shell_open url
	url_launch(buffer);
}


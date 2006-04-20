/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/fred2/FREDView.h $
 * $Revision: 1.2 $
 * $Date: 2006-04-20 06:32:01 $
 * $Author: Goober5000 $
 *
 * View class for a document/view architechure design program, which we don't
 * want or need, but MFC forces us to use.  This is the main place we handle
 * MFC messages, events, etc.  Sort of the interface between our code and MFC.
 * There is also a lot of our code in here related to these things.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.9  2006/01/15 00:48:25  Goober5000
 * this should complete the new IFF stuff for FRED
 * --Goober5000
 *
 * Revision 1.8  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.7  2005/04/13 20:02:08  chhogg
 * Changed vector to vec3d for Fred.
 *
 * Revision 1.6  2004/11/04 01:41:00  Goober5000
 * preliminary work on voice acting mananger for FRED
 * --Goober5000
 *
 * Revision 1.5  2004/09/29 17:26:32  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.4  2004/02/05 23:23:19  randomtiger
 * Merging OGL Fred into normal Fred, no conflicts, should be perfect
 *
 * Revision 1.3.2.4  2004/01/24 13:00:43  randomtiger
 * Implemented basic OGL HT&L but some drawing routines missing from cobe.lib so keeping turned off for now.
 * Added lighting menu option and made about dialog more useful.
 * Ship menu now defaults to the first entry.
 * Loads of debug to try and trap crashes.
 *
 * Revision 1.3.2.3  2004/01/20 19:11:29  randomtiger
 * Cut unneeded code, text now works in graphics window.
 *
 * Revision 1.3.2.2  2004/01/18 14:53:17  randomtiger
 * Textures work now, initialisation runs from cobe.lib not internally.
 *
 * Revision 1.3.2.1  2004/01/17 22:11:25  randomtiger
 * Large series of changes to make fred run though OGL. Requires code.lib to be up to date to compile.
 * Port still in progress, fred currently unusable, note this has been committed to fred_ogl branch.
 *
 * Revision 1.3  2003/09/05 05:32:36  Goober5000
 * moved custom buttons from shield sys dialog and added dialog to set
 * ship flags globally
 * --Goober5000
 *
 * Revision 1.2  2003/08/26 20:32:41  Goober5000
 * added support for saving in FS2 retail format
 * --Goober5000
 *
 * Revision 1.1.1.1  2002/07/15 03:10:58  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 6     6/04/99 2:20p Andsager
 * Add dump stats basic functionality
 * 
 * 5     3/26/99 4:49p Dave
 * Made cruisers able to dock with stuff. Made docking points and paths
 * visible in fred.
 * 
 * 4     1/27/99 4:09p Andsager
 * Added highlight to ship subsystems
 * 
 * 3     10/13/98 9:27a Dave
 * Started neatening up freespace.h
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:01p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 84    5/21/98 11:48a Hoffoss
 * Removed check in and check out options.
 * 
 * 83    5/14/98 5:32p Hoffoss
 * Added some more error checking.
 * 
 * 82    4/01/98 10:48a Hoffoss
 * Changed Fred to not allow command briefings in multiplayer missions.
 * 
 * 81    3/25/98 4:14p Hoffoss
 * Split ship editor up into ship editor and a misc dialog, which tracks
 * flags and such.
 * 
 * 80    3/09/98 10:56a Hoffoss
 * Added jump node objects to Fred.
 * 
 * 79    3/05/98 3:59p Hoffoss
 * Added a bunch of new command brief stuff, and asteroid initialization
 * to Fred.
 * 
 * 78    1/29/98 5:14p Hoffoss
 * Added error checking for more than 4 ships in a player starting wing.
 * 
 * 77    10/30/97 3:30p Hoffoss
 * Made anti-aliased gridlines an option in Fred.
 * 
 * 76    10/14/97 10:59a Allender
 * more persona work.  Made global error checker call funciton to assign
 * and check personas
 * 
 * 75    9/06/97 2:13p Mike
 * Replace support for TEAM_NEUTRAL
 * 
 * 74    9/01/97 6:59p Hoffoss
 * Added source safe checkin and checkout capabilities to Fred.
 * 
 * 73    8/26/97 4:18p Hoffoss
 * Added error checking to initial orders dialog when ok is clicked.
 * 
 * 72    8/25/97 5:58p Hoffoss
 * Created menu items for keypress functions in Fred, and fixed bug this
 * uncovered with wing_delete function.
 * 
 * 71    8/18/97 9:31p Hoffoss
 * Added grid adjustment dialog and shield system editor dialog.
 * 
 * 70    8/14/97 2:32p Hoffoss
 * fixed bug where controlling an object doesn't cause screen updates, and
 * added a number of cool features to viewpoint/control object code.
 * 
 * 69    8/12/97 6:32p Hoffoss
 * Added code to allow hiding of arrival and departure cues in editors.
 * 
 * 68    8/11/97 6:54p Hoffoss
 * Groups now supported in Fred.
 * 
 * 67    8/11/97 11:51a Allender
 * added stamp stuff to Fred
 * 
 * 66    8/07/97 6:01p Hoffoss
 * Added a rotate about selected object button to toolbar and
 * functionality, as requested by Comet.
 * 
 * 65    8/01/97 3:10p Hoffoss
 * Made Sexp help hidable.
 * 
 * 64    8/01/97 12:52p Hoffoss
 * Added variable, fixed bug with global error check.
 * 
 * 63    7/28/97 2:51p Hoffoss
 * Re-evaluated and improved global error checker in Fred.
 * 
 * 62    7/24/97 12:45p Hoffoss
 * Added camera position save and restore.
 * 
 * 61    7/08/97 2:03p Hoffoss
 * Debriefing editor coded and implemented.
 * 
 * 60    6/18/97 3:07p Hoffoss
 * Wing ship names are 1 indexes instead of 0 indexed now.
 * 
 * 59    6/09/97 4:57p Hoffoss
 * Added autosave and undo to Fred.
 * 
 * 58    6/05/97 6:10p Hoffoss
 * Added features: Autosaving, object hiding.  Also fixed some minor bugs.
 * 
 * 57    6/02/97 11:52a Hoffoss
 * Custom cursors displayed when over objects in different modes.
 * 
 * 56    5/21/97 5:43p Hoffoss
 * Added features requested on Todo list.
 * 
 * 55    5/06/97 3:01p Hoffoss
 * Added some accelerator keys.
 * 
 * 54    5/05/97 5:44p Hoffoss
 * Added specialized popup menu choices, save before running FreeSpace,
 * and display filters.
 * 
 * 53    5/05/97 9:42a Hoffoss
 * Single axis contraint code changed to operate better.
 * 
 * 52    4/24/97 5:15p Hoffoss
 * fixes to Fred.
 * 
 * 51    4/17/97 5:23p Hoffoss
 * Implemented ability to run FreeSpace from Fred.
 * 
 * 50    4/16/97 5:18p Hoffoss
 * Moved Asteroid field editor stuff to a seperate dialog box.
 * 
 * 49    4/11/97 4:22p Hoffoss
 * Fixed bug in Sexp trees, moved Show starfield option to view menu and
 * removed preferences dialog box.
 * 
 * 48    3/12/97 12:40p Hoffoss
 * Fixed bugs in wing object management functions, several small additions
 * and rearrangements.
 * 
 * 47    3/10/97 4:58p Hoffoss
 * Added waypoint and start types to drop down toolbar combo box and fixed
 * context menu new ship type selection to also work.
 * 
 * 46    3/10/97 12:54p Hoffoss
 * Added drop down combo box to toolbar and fixed compiling errors Mark
 * (maybe Mike?) introduced to code.
 * 
 * 45    3/06/97 3:35p Hoffoss
 * Added Show_outline stuff, moved show options to the view menu, fixed a
 * bug in message dialog editor.
 * 
 * 44    3/04/97 6:27p Hoffoss
 * Changes to Fred to handle new wing structure.
 * 
 * 43    2/28/97 11:31a Hoffoss
 * Implemented modeless dialog saving and restoring, and changed some
 * variables names.
 * 
 * 42    2/27/97 5:54p Hoffoss
 * Implemented support for saving and restoring window positions.
 * 
 * 41    2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 * 
 * 72    2/12/97 5:50p Hoffoss
 * Expanded on error checking.
 * 
 * 71    2/12/97 12:25p Hoffoss
 * Expanded on global error checker, added initial orders conflict
 * checking and warning, added waypoint editor dialog and code.
 * 
 * 70    2/10/97 5:18p Hoffoss
 * Global error checker work.  Added checks for many things, but still
 * more to add tomorrow!
 * 
 * 69    2/04/97 3:09p Hoffoss
 * Background bitmap editor implemented fully.
 * 
 * 68    1/30/97 2:24p Hoffoss
 * Added remaining mission file structures and implemented load/save of
 * them.
 *
 * $NoKeywords: $
 */

#ifndef STAMPER_PROGRAM

#include "FREDDoc.h"

#define WM_MENU_POPUP_SHIPS	(WM_USER+6)
#define WM_MENU_POPUP_EDIT		(WM_USER+7)
#define SEXP_HELP_BOX_SIZE 170

typedef struct Marking_box {
	int x1, y1, x2, y2;
} Marking_box;

typedef struct subsys_to_render
{
	bool				do_render;
	object			*ship_obj;
	ship_subsys		*cur_subsys;
} subsys_to_render;

class CShipEditorDlg;
class	CGrid;

class CFREDView : public CView
{
private:
	CGrid*		m_pGDlg;
	int global_error_check_player_wings(int multi);

protected: // create from serialization only
	CFREDView();
	DECLARE_DYNCREATE(CFREDView)

// Attributes
public:

	int global_error_check_mixed_player_wing(int w);
	int fred_check_sexp(int sexp, int type, char *msg, ...);
	int internal_error(char *msg, ...);
	int error(char *msg, ...);
	int global_error_check();
	void place_background_bitmap(vec3d v);
	void cycle_constraint();
	CFREDDoc *GetDocument();

	static CFREDView *GetView();
	
LONG OnGoodbye(UINT wParam, LONG lParam);
// LONG OnMenuPopupShips(CWnd *pWnd, CPoint point);
// LONG OnMenuPopupShips(UINT wParam, LONG lParam);

	//BOOL	m_ConfirmDeleting;
	//BOOL	m_ShowCapitalShips;
	//BOOL	m_ShowElevations;
	//BOOL	m_ShowFighters;
	//BOOL	m_ShowGrid;
	//BOOL	m_ShowMiscObjects;
	//BOOL	m_ShowPlanets;
	//BOOL	m_ShowWaypoints;	

	// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFREDView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void OnInitialUpdate();
	virtual BOOL DestroyWindow();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL



// Implementation
public:
	virtual ~CFREDView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	
//	virtual void SEDlg_destroy();

// Generated message map functions
protected:
	//{{AFX_MSG(CFREDView)
	afx_msg void OnViewGrid();
	afx_msg void OnUpdateViewGrid(CCmdUI* pCmdUI);
	afx_msg void OnViewWaypoints();
	afx_msg void OnUpdateViewWaypoints(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnEditorsShips();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMiscstuffShowshipsasicons();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditPopupShowShipIcons();
	afx_msg void OnUpdateEditPopupShowShipIcons(CCmdUI* pCmdUI);
	afx_msg void OnEditPopupShowShipModels();
	afx_msg void OnUpdateEditPopupShowShipModels(CCmdUI* pCmdUI);
	afx_msg void OnMiscStatistics();
	afx_msg void OnEditPopupShowCompass();
	afx_msg void OnUpdateEditPopupShowCompass(CCmdUI* pCmdUI);
	afx_msg void OnUpdateChangeViewpointExternal(CCmdUI* pCmdUI);
	afx_msg void OnChangeViewpointExternal();
	afx_msg void OnUpdateChangeViewpointFollow(CCmdUI* pCmdUI);
	afx_msg void OnChangeViewpointFollow();
	afx_msg void OnEditorsGoals();
	afx_msg void OnSpeed1();
	afx_msg void OnSpeed2();
	afx_msg void OnSpeed5();
	afx_msg void OnSpeed10();
	afx_msg void OnUpdateSpeed1(CCmdUI* pCmdUI);
	afx_msg void OnSpeed3();
	afx_msg void OnSpeed8();
	afx_msg void OnRot1();
	afx_msg void OnRot2();
	afx_msg void OnRot3();
	afx_msg void OnRot4();
	afx_msg void OnRot5();
	afx_msg void OnUpdateSpeed2(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSpeed3(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSpeed5(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSpeed8(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSpeed10(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRot1(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRot2(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRot3(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRot4(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRot5(CCmdUI* pCmdUI);
	afx_msg void OnControlModeCamera();
	afx_msg void OnUpdateControlModeCamera(CCmdUI* pCmdUI);
	afx_msg void OnControlModeShip();
	afx_msg void OnUpdateControlModeShip(CCmdUI* pCmdUI);
	afx_msg void OnShowGridPositions();
	afx_msg void OnUpdateShowGridPositions(CCmdUI* pCmdUI);
	afx_msg void OnShowCoordinates();
	afx_msg void OnUpdateShowCoordinates(CCmdUI* pCmdUI);
	afx_msg void OnSpeed50();
	afx_msg void OnUpdateSpeed50(CCmdUI* pCmdUI);
	afx_msg void OnSpeed100();
	afx_msg void OnUpdateSpeed100(CCmdUI* pCmdUI);
	afx_msg void OnSelect();
	afx_msg void OnUpdateSelect(CCmdUI* pCmdUI);
	afx_msg void OnSelectAndMove();
	afx_msg void OnUpdateSelectAndMove(CCmdUI* pCmdUI);
	afx_msg void OnSelectAndRotate();
	afx_msg void OnUpdateSelectAndRotate(CCmdUI* pCmdUI);
	afx_msg void OnConstrainX();
	afx_msg void OnUpdateConstrainX(CCmdUI* pCmdUI);
	afx_msg void OnConstrainY();
	afx_msg void OnUpdateConstrainY(CCmdUI* pCmdUI);
	afx_msg void OnConstrainZ();
	afx_msg void OnUpdateConstrainZ(CCmdUI* pCmdUI);
	afx_msg void OnConstrainXz();
	afx_msg void OnUpdateConstrainXz(CCmdUI* pCmdUI);
	afx_msg void OnSelectionLock();
	afx_msg void OnUpdateSelectionLock(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDoubleFineGridlines();
	afx_msg void OnUpdateDoubleFineGridlines(CCmdUI* pCmdUI);
	afx_msg void OnShowDistances();
	afx_msg void OnUpdateShowDistances(CCmdUI* pCmdUI);
	afx_msg void OnUniversalHeading();
	afx_msg void OnUpdateUniversalHeading(CCmdUI* pCmdUI);
	afx_msg void OnFlyingControls();
	afx_msg void OnUpdateFlyingControls(CCmdUI* pCmdUI);
	afx_msg void OnRotateLocally();
	afx_msg void OnUpdateRotateLocally(CCmdUI* pCmdUI);
	afx_msg void OnConstrainXy();
	afx_msg void OnUpdateConstrainXy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateConstrainYz(CCmdUI* pCmdUI);
	afx_msg void OnConstrainYz();
	afx_msg void OnSelectList();
	afx_msg void OnZoomExtents();
	afx_msg void OnZoomSelected();
	afx_msg void OnUpdateZoomSelected(CCmdUI* pCmdUI);
	afx_msg void OnFormWing();
	afx_msg void OnUpdateFormWing(CCmdUI* pCmdUI);
	afx_msg void OnDisbandWing();
	afx_msg void OnUpdateDisbandWing(CCmdUI* pCmdUI);
	afx_msg void OnShowHorizon();
	afx_msg void OnUpdateShowHorizon(CCmdUI* pCmdUI);
	afx_msg void OnEditorsWing();
	afx_msg void OnEditorsPlayer();
	afx_msg void OnEditorsOrient();
	afx_msg void OnEditorsEvents();
	afx_msg void OnUpdateEditorsOrient(CCmdUI* pCmdUI);
	afx_msg void OnEditorsMessage();
	afx_msg void OnEditorsStarfield();
	afx_msg void OnEditorsBgBitmaps();
	afx_msg void OnEditorsReinforcement();
	afx_msg void OnErrorChecker();
	afx_msg void OnEditorsWaypoint();
	afx_msg void OnViewOutlines();
	afx_msg void OnUpdateViewOutlines(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNewShipType(CCmdUI* pCmdUI);
	afx_msg void OnShowStarfield();
	afx_msg void OnUpdateShowStarfield(CCmdUI* pCmdUI);
	afx_msg void OnAsteroidEditor();
	afx_msg void OnRunFreeSpace();
	afx_msg void OnEditorCampaign();
	afx_msg void OnShowShips();
	afx_msg void OnUpdateShowShips(CCmdUI* pCmdUI);
	afx_msg void OnShowStarts();
	afx_msg void OnUpdateShowStarts(CCmdUI* pCmdUI);
	afx_msg void OnShowIFF0();
	afx_msg void OnUpdateShowIFF0(CCmdUI* pCmdUI);
	afx_msg void OnShowIFF1();
	afx_msg void OnUpdateShowIFF1(CCmdUI* pCmdUI);
	afx_msg void OnShowIFF2();
	afx_msg void OnUpdateShowIFF2(CCmdUI* pCmdUI);
	afx_msg void OnShowIFF3();
	afx_msg void OnUpdateShowIFF3(CCmdUI* pCmdUI);
	afx_msg void OnShowIFF4();
	afx_msg void OnUpdateShowIFF4(CCmdUI* pCmdUI);
	afx_msg void OnShowIFF5();
	afx_msg void OnUpdateShowIFF5(CCmdUI* pCmdUI);
	afx_msg void OnShowIFF6();
	afx_msg void OnUpdateShowIFF6(CCmdUI* pCmdUI);
	afx_msg void OnShowIFF7();
	afx_msg void OnUpdateShowIFF7(CCmdUI* pCmdUI);
	afx_msg void OnShowIFF8();
	afx_msg void OnUpdateShowIFF8(CCmdUI* pCmdUI);
	afx_msg void OnShowIFF9();
	afx_msg void OnUpdateShowIFF9(CCmdUI* pCmdUI);
	afx_msg void OnToggleViewpoint();
	afx_msg void OnRevert();
	afx_msg void OnUpdateRevert(CCmdUI* pCmdUI);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnHideObjects();
	afx_msg void OnShowHiddenObjects();
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditorsBriefing();
	afx_msg void OnEditorsDebriefing();
	afx_msg void OnSaveCamera();
	afx_msg void OnRestoreCamera();
	afx_msg void OnUpdateRestoreCamera(CCmdUI* pCmdUI);
	afx_msg void OnShowSexpHelp();
	afx_msg void OnUpdateShowSexpHelp(CCmdUI* pCmdUI);
	afx_msg void OnLookatObj();
	afx_msg void OnUpdateLookatObj(CCmdUI* pCmdUI);
	afx_msg void OnEditorsAdjustGrid();
	afx_msg void OnEditorsShieldSys();
	afx_msg void OnLevelObj();
	afx_msg void OnAlignObj();
	afx_msg void OnControlObj();
	afx_msg void OnNextObj();
	afx_msg void OnPrevObj();
	afx_msg void OnEditDeleteWing();
	afx_msg void OnMarkWing();
	afx_msg void OnUpdateControlObj(CCmdUI* pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnAaGridlines();
	afx_msg void OnUpdateAaGridlines(CCmdUI* pCmdUI);
	afx_msg void OnCmdBrief();
	afx_msg void OnDisableUndo();
	afx_msg void OnUpdateDisableUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCmdBrief(CCmdUI* pCmdUI);
	afx_msg void OnNextSubsys();
	afx_msg void OnPrevSubsys();
	afx_msg void OnCancelSubsys();
	afx_msg void OnDumpStats();
	afx_msg void OnShowPaths();
	afx_msg void OnUpdateShowPaths(CCmdUI* pCmdUI);
	afx_msg void OnShowDockPoints();
	afx_msg void OnUpdateShowDockPoints(CCmdUI* pCmdUI);
	afx_msg void OnFormatFs2Open();
	afx_msg void OnUpdateFormatFs2Open(CCmdUI* pCmdUI);
	afx_msg void OnFormatFs2Retail();
	afx_msg void OnUpdateFormatFs2Retail(CCmdUI* pCmdUI);
	afx_msg void OnFormatFs1Retail();
	afx_msg void OnUpdateFormatFs1Retail(CCmdUI* pCmdUI);
	afx_msg void OnEditorsSetGlobalShipFlags();
	afx_msg void OnEditorsVoiceManager();
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnViewLighting();
	afx_msg void OnUpdateViewLighting(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnGroup(UINT nID);
	afx_msg void OnSetGroup(UINT nID);

	void OnShowIFF(int iff);
	void OnUpdateShowIFF(int iff, CCmdUI* pCmdUI);

// LONG OnMenuPopupShips(CWnd *pWnd, CPoint point);
LONG OnMenuPopupShips(UINT wParam, LONG lParam);
LONG OnMenuPopupEdit(UINT wParam, LONG lParam);

	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in FREDView.cpp
inline CFREDDoc* CFREDView::GetDocument()
   { return (CFREDDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

void cancel_drag();
char *error_check_initial_orders(ai_goal *goals, int ship, int wing);
extern void fred_check_message_personas();

extern int Autosave_disabled;
extern int Show_sexp_help;
extern int Show_ships;
extern int Show_starts;
extern int physics_speed;
extern int physics_rot;
extern int viewpoint;
extern int view_obj;
extern int box_marking;		// Are we currently box marking? (i.e. draging out a box to mark)
extern int button_down;		// Is the left mouse button down and we are handling it?
extern int Marked;			// number of marked objects
extern int Show_compass;
extern int Show_ship_models;
extern int Show_ship_info;
extern int Show_dock_points;
extern int Lighting_on;
extern int Show_paths_fred;
extern int Selection_lock;
extern int Cursor_over;
extern int Cur_bitmap;
extern int Id_select_type_jump_node;
extern int Id_select_type_start;
extern int Id_select_type_waypoint;
extern int Hide_ship_cues, Hide_wing_cues;
extern Marking_box marking_box;
extern object_orient_pos	rotation_backup[MAX_OBJECTS];

// Goober5000 (currently, FS1 retail not implemented)
extern int Format_fs2_open;
extern int Format_fs2_retail;
extern int Format_fs1_retail;

extern CFREDView *Fred_view_wnd;

#endif		// if #ifndef STAMPER_PROGRAM

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
	int fred_check_sexp(int sexp, int type, const char *msg, ...);
	int internal_error(const char *msg, ...);
	int error(const char *msg, ...);
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
	afx_msg void OnFormatFs2OpenComp();
	afx_msg void OnUpdateFormatFs2OpenComp(CCmdUI* pCmdUI);
	afx_msg void OnFormatFs2Retail();
	afx_msg void OnUpdateFormatFs2Retail(CCmdUI* pCmdUI);
	afx_msg void OnFormatFs1Retail();
	afx_msg void OnUpdateFormatFs1Retail(CCmdUI* pCmdUI);
	afx_msg void OnEditorsSetGlobalShipFlags();
	afx_msg void OnEditorsVoiceManager();
	afx_msg void OnEditorsFiction();
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnViewLighting();
	afx_msg void OnUpdateViewLighting(CCmdUI* pCmdUI);
	afx_msg void OnViewFullDetail();
	afx_msg void OnUpdateViewFullDetail(CCmdUI *pCmdUI);
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
extern int FullDetail;
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

enum FSO_FORMAT
{
	FSO_FORMAT_RETAIL = 0,
	FSO_FORMAT_STANDARD = 1,
	FSO_FORMAT_COMPATIBILITY_MODE = 2
};

// Goober5000 (currently, FS1 retail not implemented)
extern int Format_fs2_open;
extern int Format_fs2_retail;
extern int Format_fs1_retail;

extern CFREDView *Fred_view_wnd;

#endif		// if #ifndef STAMPER_PROGRAM

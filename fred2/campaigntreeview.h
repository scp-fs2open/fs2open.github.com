/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// CampaignTreeView.h : header file
//

#include "mission/missioncampaign.h"

#define MAX_LEVELS	100
#define MAX_CAMPAIGN_TREE_LINKS	300

typedef struct campaign_tree_element {
	int from_links;	// total branches from this mission
	int to_links;		// total branches that lead to this mission
	CRect box;			// coordinates of drawn box
} campaign_tree_element;

typedef struct campaign_tree_link {
	int from;	// index of source mission
	int to;		// index of mission link leads to
	int sexp;	// sexp index of condition that allows this branch
	int node;	// node tracker when link is in sexp tree window
	int from_pos;	// from link drawing offset
	int to_pos;	// to link drawing offset
	bool is_mission_loop;	// whether link leads to mission loop
	bool is_mission_fork;	// whether link leads to mission fork
	char *mission_branch_txt;	// text describing mission loop
	char *mission_branch_brief_anim;	// filename of anim to play in the brief
	char *mission_branch_brief_sound;	// filename of anim to play in the brief
	CPoint p1;	// coordinates of line last draw for link, from p1 to p2
	CPoint p2;
} campaign_tree_link;

extern int Total_links;
extern int Level_counts[MAX_LEVELS];
extern int Sorted[MAX_CAMPAIGN_MISSIONS];
extern campaign_tree_element Elements[MAX_CAMPAIGN_MISSIONS];
extern campaign_tree_link Links[MAX_CAMPAIGN_TREE_LINKS];

/////////////////////////////////////////////////////////////////////////////
// campaign_tree_view view

class campaign_tree_view : public CScrollView
{
protected:
	campaign_tree_view();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(campaign_tree_view)

// Attributes
public:
	void drop_mission(int m, CPoint point);
	int add_link(int from, int to);
	void remove_mission(int m);
	void delete_link(int num);
	int get_root_mission();
	void horizontally_align_mission(int num, int dir);
	void correct_position(int num);
	void free_links();
	void sort_elements();
	int query_alternate_pos(const CPoint& p);
	int query_pos(const CPoint& p);
	int query_level(const CPoint& p);
	void sort_links();
	void realign_tree();
	int total_levels;
	int total_width;
	campaign_tree_link *first_link;

// Operations
public:
	void construct_tree();
	void initialize();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(campaign_tree_view)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~campaign_tree_view();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(campaign_tree_view)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRemoveMission();
	afx_msg void OnDeleteRow();
	afx_msg void OnInsertRow();
	afx_msg void OnAddRepeat();
	afx_msg void OnEndOfCampaign();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

extern campaign_tree_view *Campaign_tree_viewp;

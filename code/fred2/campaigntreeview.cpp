/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// CampaignTreeView.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "CampaignTreeView.h"
#include "CampaignEditorDlg.h"
#include "CampaignTreeWnd.h"
#include "mission/missionparse.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LOCAL int Bx, By, Mission_dragging = -1, Mission_dropping = -1, Context_mission;
int Total_links = 0;
int CTV_button_down = 0;
int Level_counts[MAX_LEVELS];
int Sorted[MAX_CAMPAIGN_MISSIONS];
campaign_tree_element Elements[MAX_CAMPAIGN_MISSIONS];
campaign_tree_link Links[MAX_CAMPAIGN_TREE_LINKS];
LOCAL CRect Dragging_rect;
LOCAL CSize Rect_offset, Last_draw_size;

/////////////////////////////////////////////////////////////////////////////
// campaign_tree_view

IMPLEMENT_DYNCREATE(campaign_tree_view, CScrollView)

campaign_tree_view *Campaign_tree_viewp;

campaign_tree_view::campaign_tree_view()
{
	total_levels = 1;
	total_width = 1;
}

campaign_tree_view::~campaign_tree_view()
{
}

BEGIN_MESSAGE_MAP(campaign_tree_view, CScrollView)
	//{{AFX_MSG_MAP(campaign_tree_view)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_REMOVE_MISSION, OnRemoveMission)
	ON_COMMAND(ID_DELETE_ROW, OnDeleteRow)
	ON_COMMAND(ID_INSERT_ROW, OnInsertRow)
	ON_COMMAND(ID_ADD_REPEAT, OnAddRepeat)
	ON_COMMAND(ID_END_OF_CAMPAIGN, OnEndOfCampaign)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// campaign_tree_view drawing

#define LEVEL_HEIGHT		75
#define CELL_WIDTH		150
#define CELL_TEXT_WIDTH	130

int campaign_tree_view::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void campaign_tree_view::OnDraw(CDC* pDC)
{
	char str[256];
	int i, x, y, f, t;
	BOOL r;
	CSize size;
	CRect rect;
	CPen black_pen, white_pen, red_pen, blue_pen, green_pen;
	CBrush gray_brush;
	TEXTMETRIC tm;

	// setup text drawing stuff
	pDC->SetTextAlign(TA_TOP | TA_CENTER);
	pDC->SetTextColor(RGB(0, 0, 0));
	pDC->SetBkMode(TRANSPARENT);

	// figure out text box sizes
	r = pDC->GetTextMetrics(&tm);
	Assert(r);
	Bx = CELL_TEXT_WIDTH + 4;
	By = tm.tmHeight + 4;

	r = gray_brush.CreateSolidBrush(RGB(192, 192, 192));
	Assert(r);
	pDC->FillRect(CRect(0, 0, total_width * CELL_WIDTH, total_levels * LEVEL_HEIGHT), &gray_brush);

	// create pens
	r = black_pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	Assert(r);
	r = white_pen.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	Assert(r);
	r = red_pen.CreatePen(PS_SOLID, 1, RGB(192, 0, 0));
	Assert(r);
	r = blue_pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 192));
	Assert(r);
	r = green_pen.CreatePen(PS_SOLID, 1, RGB(0, 192, 0));
	Assert(r);

	pDC->SelectObject(&black_pen);
	// draw level seperators
	for (i=1; i<total_levels; i++) {
		pDC->MoveTo(0, i * LEVEL_HEIGHT - 1);
		pDC->LineTo(total_width * CELL_WIDTH, i * LEVEL_HEIGHT - 1);
	}

	pDC->SelectObject(&white_pen);
	for (i=1; i<total_levels; i++) {
		pDC->MoveTo(0, i * LEVEL_HEIGHT);
		pDC->LineTo(total_width * CELL_WIDTH, i * LEVEL_HEIGHT);
	}


	// draw edges of the whole tree rectangle
	pDC->SelectObject(&black_pen);
	pDC->MoveTo(0, total_levels * LEVEL_HEIGHT);
	pDC->LineTo(total_width * CELL_WIDTH, total_levels * LEVEL_HEIGHT);
	pDC->LineTo(total_width * CELL_WIDTH, -1);

	// draw text boxes and text

	for (i=0; i<Campaign.num_missions; i++) {
		x = (Campaign.missions[i].pos + 1) * CELL_WIDTH / 2;
		y = Campaign.missions[i].level * LEVEL_HEIGHT + LEVEL_HEIGHT / 2;
		Elements[i].box.left = x - Bx / 2;
		Elements[i].box.right = Elements[i].box.left + Bx;
		Elements[i].box.top = y - By / 2;
		Elements[i].box.bottom = Elements[i].box.top + By;
  
		strcpy_s(str, Campaign.missions[i].name);
		str[strlen(str) - 4] = 0;  // strip extension from filename
		GetTextExtentPoint32(pDC->m_hDC, str, strlen(str), &size);
		if (size.cx > CELL_TEXT_WIDTH) {
			strcpy(str + strlen(str) - 1, "...");
			GetTextExtentPoint32(pDC->m_hDC, str, strlen(str), &size);
			while (size.cx > CELL_TEXT_WIDTH) {
				strcpy(str + strlen(str) - 4, "...");
				GetTextExtentPoint32(pDC->m_hDC, str, strlen(str), &size);
			}
		}

		if (i == Cur_campaign_mission) {
			pDC->SetTextColor(RGB(192, 0, 0));
			pDC->Draw3dRect(x - Bx / 2, y - By / 2, Bx, By, RGB(0, 0, 0), RGB(255, 255, 255));

		} else {
			pDC->SetTextColor(RGB(0, 0, 0));
			pDC->Draw3dRect(x - Bx / 2, y - By / 2, Bx, By, RGB(255, 255, 255), RGB(0, 0, 0));
		}

		pDC->TextOut(x, y - By / 2 + 2, str, strlen(str));
	}

	for (i=0; i<Total_links; i++) {
		f = Links[i].from;
		t = Links[i].to;

		if (t == -1) {
			continue;
		}
		Links[i].p1.x = Elements[f].box.left + Links[i].from_pos * Bx / (Elements[f].from_links + 1);
		Links[i].p1.y = Elements[f].box.bottom;
		Links[i].p2.x = Elements[t].box.left + Links[i].to_pos * Bx / (Elements[t].to_links + 1);
		Links[i].p2.y = Elements[t].box.top;

		// if special mission link, select blue pen
		if (Links[i].is_mission_loop || Links[i].is_mission_fork) {
			pDC->SelectObject(&blue_pen);
		}

		// if active link, select highlight pen (red - normal, green - special)
		if (i == Cur_campaign_link) {
			if (Links[i].is_mission_loop || Links[i].is_mission_fork) {
				pDC->SelectObject(&green_pen);
			} else {
				pDC->SelectObject(&red_pen);
			}
		}

		// draw a line between 'from' and 'to' mission.  to might be -1 in the case of end-campaign, so
		// don't draw line if so.
		if ( (f != t) && ( t != -1) ) {
			pDC->MoveTo(Links[i].p1);
			pDC->LineTo(Links[i].p2);
		}

		// select (defalt) black pen
		pDC->SelectObject(&black_pen);
	}

	pDC->SelectObject(&black_pen);
}

/////////////////////////////////////////////////////////////////////////////
// campaign_tree_view diagnostics

#ifdef _DEBUG
void campaign_tree_view::AssertValid() const
{
	CScrollView::AssertValid();
}

void campaign_tree_view::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// campaign_tree_view message handlers

void campaign_tree_view::OnInitialUpdate() 
{
	CScrollView::OnInitialUpdate();
	SetScrollSizes(MM_TEXT, CSize(320, 320));
}

void stuff_link_with_formula(int *link_idx, int formula, int mission_num)
{
	int j, node, node2, node3;
	if (formula >= 0) {
		if (!stricmp(CTEXT(formula), "cond")) {
			// sexp is valid

			node = CDR(formula);
			free_one_sexp(formula);
			while (node != -1) {
				node2 = CAR(node);
				Links[*link_idx].from = mission_num;
				Links[*link_idx].sexp = CAR(node2);
				Links[*link_idx].mission_branch_txt = NULL;
				Links[*link_idx].mission_branch_brief_anim = NULL;
				Links[*link_idx].mission_branch_brief_sound = NULL;
				sexp_mark_persistent(CAR(node2));
				free_one_sexp(node2);
				node3 = CADR(node2);
				if ( !stricmp( CTEXT(node3), "next-mission") ) {
					node3 = CDR(node3);
					for (j=0; j<Campaign.num_missions; j++)
						if (!stricmp(CTEXT(node3), Campaign.missions[j].name))
							break;

					if (j < Campaign.num_missions) {  // mission is in campaign (you never know..)
						Links[(*link_idx)++].to = j;
						Elements[mission_num].from_links++;
						Elements[j].to_links++;
					}

				} else if ( !stricmp( CTEXT(node3), "end-of-campaign") ) {
					Links[(*link_idx)++].to = -1;
					Elements[mission_num].from_links++;

				} else
					Int3();			// bogus operator in campaign file

				free_sexp(CDR(node2));
				free_one_sexp(node);
				node = CDR(node);
			}
		}
	}
}

// this function should only be called right after a campaign is loaded.  Calling it a second
// time without having loaded a campaign again will result in undefined behavior.
void campaign_tree_view::construct_tree()
{
	int i;
	free_links();

	// initialize mission link counts
	for (i=0; i<Campaign.num_missions; i++) {
		Elements[i].from_links = Elements[i].to_links = 0;
	}

	// analyze branching sexps and build links from them.
	int link_idx = 0;
	for (i=0; i<Campaign.num_missions; i++) {

		// do main campaign path
		stuff_link_with_formula(&link_idx, Campaign.missions[i].formula, i);

		// do special mission path
		if ( Campaign.missions[i].flags & CMISSION_FLAG_HAS_LOOP ) {
			stuff_link_with_formula(&link_idx, Campaign.missions[i].mission_loop_formula, i);
			Links[link_idx-1].mission_branch_txt = Campaign.missions[i].mission_branch_desc;
			Links[link_idx-1].mission_branch_brief_anim = Campaign.missions[i].mission_branch_brief_anim;
			Links[link_idx-1].mission_branch_brief_sound = Campaign.missions[i].mission_branch_brief_sound;
			Links[link_idx-1].is_mission_loop = true;
		}
		else if ( Campaign.missions[i].flags & CMISSION_FLAG_HAS_FORK ) {
			Campaign.missions[i].mission_loop_formula = -1;
			Links[link_idx-1].mission_branch_txt = Campaign.missions[i].mission_branch_desc;
			Links[link_idx-1].mission_branch_brief_anim = Campaign.missions[i].mission_branch_brief_anim;
			Links[link_idx-1].mission_branch_brief_sound = Campaign.missions[i].mission_branch_brief_sound;
			Links[link_idx-1].is_mission_fork = true;
		}
	}

	for (i=0; i<Campaign.num_missions; i++) {
		Sorted[i] = i;
	}

	Total_links = link_idx;
	if (Campaign.realign_required) {
		realign_tree();
		Campaign.realign_required = 0;
	}
}

void campaign_tree_view::initialize()
{
	int i, z;

	total_levels = total_width = 1;
	for (i=0; i<MAX_LEVELS; i++)
		Level_counts[i] = 0;

	for (i=0; i<Campaign.num_missions; i++) {
		z = Campaign.missions[i].level;
		if (z + 2 > total_levels)
			total_levels = z + 2;

		Level_counts[z]++;
		z = (Campaign.missions[i].pos + 3) / 2;
		if (z > total_width)
			total_width = z;
	}

	sort_links();
	SetScrollSizes(MM_TEXT, CSize(total_width * CELL_WIDTH, total_levels * LEVEL_HEIGHT));
	Invalidate();
}

void campaign_tree_view::free_links()
{
	int i;

	for (i=0; i<Total_links; i++) {
		sexp_unmark_persistent(Links[i].sexp);
		free_sexp2(Links[i].sexp);
	}

	Total_links = 0;
}

void campaign_tree_view::realign_tree()
{
	int i, j, z, offset, level, pos;

	// figure out what level each mission lies on and an initial position on that level
	level = pos = total_width = 0;
	for (i=0; i<Campaign.num_missions; i++) {
		z = Sorted[i];
		for (j=0; j<Total_links; j++)
			if (Links[j].to == z) {
				Assert(Campaign.missions[Links[j].from].level <= Campaign.missions[z].level);  // links can't go up the tree, only down
				if (Campaign.missions[Links[j].from].level == level) {
					level++;  // force to next level in tree
					pos = 0;
					break;
				}
			}

		Campaign.missions[z].level = level;
		Campaign.missions[z].pos = pos++;
		if (pos > total_width)
			total_width = pos;

		Level_counts[level] = pos;
		if (!z) {  // topmost node must always be alone on level
			level++;
			pos = 0;
		}
	}

	// now calculate the true x position of each mission
	for (i=0; i<Campaign.num_missions; i++) {
		offset = total_width - Level_counts[Campaign.missions[i].level];
		Campaign.missions[i].pos = Campaign.missions[i].pos * 2 + offset;
	}
}

void campaign_tree_view::sort_links()
{
	int i, j, k, z, to_count, from_count, swap;
	int to_list[MAX_CAMPAIGN_TREE_LINKS];
	int from_list[MAX_CAMPAIGN_TREE_LINKS];

	for (i=0; i<Campaign.num_missions; i++) {
		// build list of all to and from links for one mission at a time
		to_count = from_count = 0;
		for (j=0; j<Total_links; j++) {
			if ((Links[j].to == i) && (Links[j].from == i))
				continue;  // ignore 'repeat mission' links
			if (Links[j].to == i)
				to_list[to_count++] = j;
			if (Links[j].from == i)
				from_list[from_count++] = j;
		}

		// sort to links, left to right and top to bottom
		for (j=0; j<to_count-1; j++)
			for (k=j+1; k<to_count; k++) {
				swap = 0;
				z = Campaign.missions[Links[to_list[j]].from].pos -
					Campaign.missions[Links[to_list[k]].from].pos;

				if (z > 0)  // sort left to right
					swap = 1;

				else if (!z) {  // both have same position?
					z = Campaign.missions[Links[to_list[j]].from].level -
						Campaign.missions[Links[to_list[k]].from].level;

					// see where from link position is relative to to link position
					if (Campaign.missions[i].pos < Campaign.missions[Links[to_list[j]].from].pos) {
						if (z > 0)  // sort bottom to top
							swap = 1;

					} else {
						if (z < 0) // sort top to bottom
							swap = 1;
					}
				}

				if (swap) {
					z = to_list[j];
					to_list[j] = to_list[k];
					to_list[k] = z;
				}
			}

		// set all links to positions
		for (j=0; j<to_count; j++)
			Links[to_list[j]].to_pos = j + 1;

		// sort from links, left to right and bottom to top
		for (j=0; j<from_count-1; j++)
			for (k=j+1; k<from_count; k++) {
				swap = 0;
				z = Campaign.missions[Links[from_list[j]].to].pos -
					Campaign.missions[Links[from_list[k]].to].pos;

				if (z > 0)
					swap = 1;
					
				else if (!z) {
					z = Campaign.missions[Links[from_list[j]].to].level -
						Campaign.missions[Links[from_list[k]].to].level;

					if (Campaign.missions[i].pos < Campaign.missions[Links[from_list[j]].to].pos) {
						if (z < 0)
							swap = 1;

					} else {
						if (z > 0)
							swap = 1;
					}
				}

				if (swap) {
					z = from_list[j];
					from_list[j] = from_list[k];
					from_list[k] = z;
				}
			}

		// set all links from positions
		for (j=0; j<from_count; j++)
			Links[from_list[j]].from_pos = j + 1;
	}
}

void campaign_tree_view::OnLButtonDown(UINT nFlags, CPoint point) 
{
	int i;
	CString str;
	CEdit *box;
	CListBox *listbox;
	CClientDC dc(this);

	OnPrepareDC(&dc);
	dc.DPtoLP(&point);
	if (nFlags & MK_CONTROL) {
		listbox = (CListBox *) &Campaign_tree_formp->m_filelist;
		i = listbox->GetCurSel();

		Mission_dropping = -1;
		if (i != LB_ERR) {
			Mission_dropping = i;
			SetCapture();
		}

		Last_draw_size = CSize(0, 0);
		Dragging_rect.SetRect(0, 0, 1, 1);
		dc.DrawDragRect(Dragging_rect, Last_draw_size, NULL, CSize(0, 0));

	} else {
		if ( (Cur_campaign_link >= 0) && (Links[Cur_campaign_link].is_mission_loop || Links[Cur_campaign_link].is_mission_fork)) {
			// HACK!!  UPDATE mission loop/fork desc before changing selections
			// save mission loop/fork desc
			char buffer[MISSION_DESC_LENGTH];
			box = (CEdit *) Campaign_tree_formp->GetDlgItem(IDC_MISSISON_LOOP_DESC);
			box->GetWindowText(buffer, MISSION_DESC_LENGTH);
			if (strlen(buffer)) {
				if (Links[Cur_campaign_link].mission_branch_txt) {
					free(Links[Cur_campaign_link].mission_branch_txt);
				}
				Links[Cur_campaign_link].mission_branch_txt = strdup(buffer);
			} else {
				Links[Cur_campaign_link].mission_branch_txt = NULL;
			}

			// HACK!!  UPDATE mission loop/fork desc before changing selections
			// save mission loop/fork desc			
			box = (CEdit *) Campaign_tree_formp->GetDlgItem(IDC_LOOP_BRIEF_ANIM);
			box->GetWindowText(buffer, MISSION_DESC_LENGTH);
			if (strlen(buffer)) {
				if (Links[Cur_campaign_link].mission_branch_brief_anim) {
					free(Links[Cur_campaign_link].mission_branch_brief_anim);
				}
				Links[Cur_campaign_link].mission_branch_brief_anim = strdup(buffer);
			} else {
				Links[Cur_campaign_link].mission_branch_brief_anim = NULL;
			}

			// HACK!!  UPDATE mission loop/fork desc before changing selections
			// save mission loop/fork desc			
			box = (CEdit *) Campaign_tree_formp->GetDlgItem(IDC_LOOP_BRIEF_SOUND);
			box->GetWindowText(buffer, MISSION_DESC_LENGTH);
			if (strlen(buffer)) {
				if (Links[Cur_campaign_link].mission_branch_brief_sound) {
					free(Links[Cur_campaign_link].mission_branch_brief_sound);
				}
				Links[Cur_campaign_link].mission_branch_brief_sound = strdup(buffer);
			} else {
				Links[Cur_campaign_link].mission_branch_brief_sound = NULL;
			}
		}
		Mission_dragging = Cur_campaign_mission = Cur_campaign_link = -1;
		for (i=0; i<Campaign.num_missions; i++)
			if (Elements[i].box.PtInRect(point)) {
				SetCapture();

				Mission_dragging = Cur_campaign_mission = i;
				Dragging_rect = Elements[i].box;
				Rect_offset = Dragging_rect.TopLeft() - point;
				Last_draw_size = CSize(4, 4);
				if (Campaign.missions[Cur_campaign_mission].num_goals < 0)  // haven't loaded goal names yet (or notes)
					read_mission_goal_list(Cur_campaign_mission);

				if (Campaign.missions[Cur_campaign_mission].notes) {
					convert_multiline_string(str, Campaign.missions[Cur_campaign_mission].notes);
					box = (CEdit *) Campaign_tree_formp->GetDlgItem(IDC_HELP_BOX);
					if (box)
						box->SetWindowText(str);
				}

				Campaign_tree_formp->mission_selected(Cur_campaign_mission);
				break;
			}
	}
	
	Invalidate();
	UpdateWindow();
	Campaign_tree_formp->load_tree();
	if (Mission_dragging != -1) {
		CRect rect = Dragging_rect;

		dc.LPtoDP(&rect);
		dc.DrawDragRect(rect, Last_draw_size, NULL, CSize(0, 0));
	}

	CScrollView::OnLButtonDown(nFlags, point);
}

void campaign_tree_view::OnMouseMove(UINT nFlags, CPoint point)
{
	int i, level, pos, x, y;
	CSize draw_size;
	CRect rect, r1;
	CClientDC dc(this);

	OnPrepareDC(&dc);
	dc.DPtoLP(&point);
	if ((Mission_dragging >= 0) || (Mission_dropping >= 0)) {
		if (GetCapture() != this) {
			rect = Dragging_rect;
			dc.LPtoDP(&rect);
			dc.DrawDragRect(rect, CSize(0, 0), rect, Last_draw_size);
			Mission_dragging = Mission_dropping = -1;

		} else {
			for (i=0; i<Campaign.num_missions; i++)
				if (Elements[i].box.PtInRect(point))
					break;

			if ((i < Campaign.num_missions) && (Mission_dropping < 0)) {  // on a mission box?
				draw_size = CSize(4, 4);
				rect = Elements[i].box;

			} else {
				level = query_level(point);
				pos = query_pos(point);
				if ((level < 0) || (pos < 0)) {  // off table?
					draw_size = CSize(0, 0);
					rect = Dragging_rect;

				} else {
					draw_size = CSize(2, 2);
					for (i=0; i<Campaign.num_missions; i++)
						if ((Campaign.missions[i].level == level) && (Campaign.missions[i].pos + 1 == pos)) {
							pos = query_alternate_pos(point);
							break;
						}

					x = pos * CELL_WIDTH / 2 - Bx / 2;
					y = level * LEVEL_HEIGHT + LEVEL_HEIGHT / 2 - By / 2;
					rect.SetRect(x, y, x + Bx, y + By);
				}
			}

			r1 = rect;
			dc.LPtoDP(&r1);
			dc.LPtoDP(&Dragging_rect);
			dc.DrawDragRect(r1, draw_size, Dragging_rect, Last_draw_size);
			Dragging_rect = rect;
			Last_draw_size = draw_size;
		}
	}

	CScrollView::OnMouseMove(nFlags, point);
}

void campaign_tree_view::OnLButtonUp(UINT nFlags, CPoint point) 
{
	int i, j, z, level, pos;
	CClientDC dc(this);

	OnPrepareDC(&dc);
	dc.DPtoLP(&point);
	if (Mission_dropping >= 0) {  // dropping a new mission into campaign?
		z = Mission_dropping;
		Mission_dropping = -1;
		if (GetCapture() == this) {
			ReleaseCapture();
			dc.LPtoDP(&Dragging_rect);
			dc.DrawDragRect(Dragging_rect, CSize(0, 0), Dragging_rect, Last_draw_size);

			drop_mission(z, point);
			return;
		}

	} else if (Mission_dragging >= 0) {  // moving position of a mission?
		z = Mission_dragging;
		Mission_dragging = -1;
		if (GetCapture() == this) {
			ReleaseCapture();
			dc.LPtoDP(&Dragging_rect);
			dc.DrawDragRect(Dragging_rect, CSize(0, 0), Dragging_rect, Last_draw_size);
			for (i=0; i<Campaign.num_missions; i++)
				if (Elements[i].box.PtInRect(point)) {  // see if released on another mission
					if (i == z)  // released on the same mission?
						return;

					for (j=0; j<Total_links; j++)
						if ((Links[j].from == z) && (Links[j].to == i))
							return;  // already linked

					if (Total_links >= MAX_CAMPAIGN_TREE_LINKS) {
						MessageBox("Too many links exist.  Can't add any more.");
						return;
					}

					if (Campaign.missions[z].level >= Campaign.missions[i].level) {
						MessageBox("A branch can only be set to a mission on a lower level");
						return;
					}

					add_link(z, i);
					return;
				}

			// at this point, we are dragging a mission to a new place
			level = query_level(point);
			pos = query_pos(point);
			if ((level < 0) || (pos < 0))
				return;

			if (!level && (get_root_mission() >= 0)) {
				MessageBox("Can't move mission to this level.  There is already a top level mission");
				return;
			}

			for (i=0; i<Campaign.num_missions; i++)
				if ((Campaign.missions[i].level == level) && (Campaign.missions[i].pos + 1 == pos)) {
					pos = query_alternate_pos(point);
					break;
				}

			for (i=0; i<Total_links; i++)
				if (Links[i].to == z)
					if (level <= Campaign.missions[Links[i].from].level) {
						MessageBox("Can't move mission to that level, as it would be\n"
							"above a parent mission", "Error");

						return;
					}

			Campaign.missions[z].level = level;
			Campaign.missions[z].pos = pos - 1;
			correct_position(z);
			sort_links();
			SetScrollSizes(MM_TEXT, CSize(total_width * CELL_WIDTH, total_levels * LEVEL_HEIGHT));
			Invalidate();
			Campaign_modified = 1;
			return;
		}
	}
	
	CScrollView::OnLButtonUp(nFlags, point);
}

int campaign_tree_view::add_link(int from, int to)
{
	if (Total_links >= MAX_CAMPAIGN_TREE_LINKS)
		return -1;

	Campaign_tree_formp->load_tree(1);
	Links[Total_links].from = from;
	Links[Total_links].to = to;
	Links[Total_links].sexp = Locked_sexp_true;
	Links[Total_links].is_mission_loop = false;
	Links[Total_links].is_mission_fork = false;
	Links[Total_links].mission_branch_txt = NULL;
	Links[Total_links].mission_branch_brief_anim = NULL;
	Links[Total_links].mission_branch_brief_sound = NULL;
	Total_links++;
	if (from != to) {
		Elements[from].from_links++;
		Elements[to].to_links++;
	}

	sort_links();
	Campaign_tree_formp->load_tree(0);
	Invalidate();
	Campaign_modified = 1;
	return 0;
}

int campaign_tree_view::query_level(const CPoint& p)
{
	int level;

	if ((p.y < 0) || (p.y >= total_levels * LEVEL_HEIGHT))
		return -1;

	level = p.y / LEVEL_HEIGHT;
	Assert((level >= 0) && (level < total_levels));
	return level;
}

int campaign_tree_view::query_pos(const CPoint& p)
{
	int pos;
	
	if ((p.x < 0) || (p.x >= total_width * CELL_WIDTH))
		return -1;

	pos = ((p.x * 4 / CELL_WIDTH) + 1) / 2;
	Assert((pos >= 0) && (pos <= total_width * 2));
	return pos;
}

int campaign_tree_view::query_alternate_pos(const CPoint& p)
{
	int x, pos;
	
	if ((p.x < 0) || (p.x >= total_width * CELL_WIDTH))
		return -1;

	x = p.x * 4 / CELL_WIDTH;
	pos = (x + 1) / 2;
	if (x & 1) // odd number
		pos--;
	else  // even number
		pos++;

	Assert((pos >= 0) && (pos <= total_width * 2));
	return pos;
}
/*
DROPEFFECT campaign_tree_view::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	CClientDC dc(this);
	OnPrepareDC(&dc);

	if (!pDataObject->IsDataAvailable((unsigned short)Mission_filename_cb_format))
		return DROPEFFECT_NONE;  // data isn't a mission filename, the only valid data to drop here

	Mission_dropping = 1;
	Last_draw_size = CSize(0, 0);
	Dragging_rect.SetRect(0, 0, 1, 1);
	dc.DrawDragRect(Dragging_rect, Last_draw_size, NULL, CSize(0, 0));
	return DROPEFFECT_MOVE;
}

void campaign_tree_view::OnDragLeave()
{
	CScrollView::OnDragLeave();
	if (Mission_dropping >= 0) {
		CClientDC dc(this);
		OnPrepareDC(&dc);

		dc.LPtoDP(&Dragging_rect);
		dc.DrawDragRect(Dragging_rect, CSize(0, 0), Dragging_rect, Last_draw_size);
		Mission_dropping = -1;
	}
}

DROPEFFECT campaign_tree_view::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	int i, level, pos, x, y;
	CSize draw_size;
	CRect rect, r1;
	DROPEFFECT r = DROPEFFECT_MOVE;

	if (Mission_dropping < 0)
		return DROPEFFECT_NONE;

	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);

	level = query_level(point);
	pos = query_pos(point);
	if ((level < 0) || (pos < 0)) {  // off table?
		draw_size = CSize(0, 0);
		rect = Dragging_rect;
		r = DROPEFFECT_NONE;

	} else {
		draw_size = CSize(2, 2);
		for (i=0; i<Campaign.num_missions; i++)
			if ((Campaign.missions[i].level == level) && (Campaign.missions[i].pos + 1 == pos)) {
				pos = query_alternate_pos(point);
				break;
			}

		x = pos * CELL_WIDTH / 2 - Dragging_rect.Width() / 2;
		y = level * LEVEL_HEIGHT + LEVEL_HEIGHT / 2 - Dragging_rect.Height() / 2;
		rect.SetRect(x, y, x + Bx, y + By);
	}

	r1 = rect;
	dc.LPtoDP(&r1);
	dc.LPtoDP(&Dragging_rect);
	dc.DrawDragRect(r1, draw_size, Dragging_rect, Last_draw_size);
	Dragging_rect = rect;
	Last_draw_size = draw_size;

	return r;
}

BOOL campaign_tree_view::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) 
{
	int i, level, pos;
	cmission *cm;
	HGLOBAL hGlobal;
	LPCSTR pData;
	mission a_mission;

	if (Mission_dropping < 0)
		return FALSE;

	// If the dropEffect requested is not a MOVE, return FALSE to 
	// signal no drop. (No COPY into trashcan)
	if ((dropEffect & DROPEFFECT_MOVE) != DROPEFFECT_MOVE)
		return FALSE;

	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);

	dc.LPtoDP(&Dragging_rect);
	dc.DrawDragRect(Dragging_rect, CSize(0, 0), Dragging_rect, Last_draw_size);
	Mission_dropping = -1;

	if (!pDataObject->IsDataAvailable((unsigned short)Mission_filename_cb_format))
		return FALSE;  // data isn't a mission filename, the only valid data to drop here

	// Get text data from COleDataObject
	hGlobal = pDataObject->GetGlobalData((unsigned short)Mission_filename_cb_format);

	// Get pointer to data
	pData = (LPCSTR) GlobalLock(hGlobal);
	ASSERT(pData);

	if (Campaign.num_missions >= MAX_CAMPAIGN_MISSIONS) {  // Can't add any more
		GlobalUnlock(hGlobal);
		MessageBox("Too many missions.  Can't add more to Campaign.", "Error");
		return FALSE;
	}

	level = query_level(point);
	pos = query_pos(point);
	Assert((level >= 0) && (pos >= 0));  // this should be impossible
	if (!level && (get_root_mission() >= 0)) {
		GlobalUnlock(hGlobal);
		MessageBox("Only 1 mission may be in the top level");
		return FALSE;
	}

	// check the number of players in a multiplayer campaign against the number of players
	// in the mission that was just dropped
	if ( Campaign.type != CAMPAIGN_TYPE_SINGLE ) {
		get_mission_info((char *)pData, &a_mission);
		if ( !(a_mission.game_type & MISSION_TYPE_MULTI) ) {
			char buf[256];

			sprintf( buf, "Mission \"%s\" is not a multiplayer mission", pData );
			MessageBox(buf, "Error");
			GlobalUnlock(hGlobal);
			return FALSE;
		}

		if ( Campaign.num_players != 0 ) {
			if ( Campaign.num_players != a_mission.num_players ) {
				char buf[512];

				sprintf(buf, "Mission \"%s\" has %d players.  Campaign has %d players.  Campaign will not play properly in FreeSpace", pData, a_mission.num_players, Campaign.num_players );
				MessageBox(buf, "Warning");
			}
		} else {
			Campaign.num_players = a_mission.num_players;
		}
	}

	Elements[Campaign.num_missions].from_links = Elements[Campaign.num_missions].to_links = 0;
	cm = &(Campaign.missions[Campaign.num_missions++]);
	cm->name = strdup(pData);
	cm->formula = Locked_sexp_true;
	cm->num_goals = -1;
	cm->notes = NULL;
	cm->briefing_cutscene[0] = 0;
	for (i=0; i<Campaign.num_missions - 1; i++)
		if ((Campaign.missions[i].level == level) && (Campaign.missions[i].pos + 1 == pos)) {
			pos = query_alternate_pos(point);
			break;
		}

	cm->level = level;
	cm->pos = pos - 1;
	correct_position(Campaign.num_missions - 1);
	sort_links();
	SetScrollSizes(MM_TEXT, CSize(total_width * CELL_WIDTH, total_levels * LEVEL_HEIGHT));
	Invalidate();

	// update and reinitialize dialog items
	if ( Campaign.type != CAMPAIGN_TYPE_SINGLE ) {
		Campaign_tree_formp->update();
		Campaign_tree_formp->initialize(0);
	}

	// Unlock memory - Send dropped text into the "bit-bucket"
	GlobalUnlock(hGlobal);
	Campaign_modified = 1;
	return TRUE;
}
*/

void campaign_tree_view::drop_mission(int m, CPoint point)
{
	char name[MAX_FILENAME_LEN + 1];
	int i, item, level, pos;
	cmission *cm;
	mission a_mission;
	CListBox *listbox;

	level = query_level(point);
	pos = query_pos(point);
	Assert((level >= 0) && (pos >= 0));  // this should be impossible

	listbox = (CListBox *) &Campaign_tree_formp->m_filelist;
	item = listbox->GetCurSel();
	if (item == LB_ERR) {
		MessageBox("Select a mission from listbox to add.", "Error");
		return;
	}

	if (listbox->GetTextLen(item) > MAX_FILENAME_LEN) {
		char buf[256];

		sprintf(buf, "Filename is too long.  Must be %d or less characters.", MAX_FILENAME_LEN);
		MessageBox(buf, "Error");
		return;  // filename is too long.  Would overflow our buffer
	}

	// grab the filename selected from the listbox
	listbox->GetText(item, name);

	if (Campaign.num_missions >= MAX_CAMPAIGN_MISSIONS) {  // Can't add any more
		MessageBox("Too many missions.  Can't add more to Campaign.", "Error");
		return;
	}

	if (!level && (get_root_mission() >= 0)) {
		MessageBox("Only 1 mission may be in the top level");
		return;
	}

	// check the number of players in a multiplayer campaign against the number of players
	// in the mission that was just dropped
	if ( Campaign.type != CAMPAIGN_TYPE_SINGLE ) {
		get_mission_info(name, &a_mission);
		if ( !(a_mission.game_type & MISSION_TYPE_MULTI) ) {
			char buf[256];

			sprintf(buf, "Mission \"%s\" is not a multiplayer mission", name);
			MessageBox(buf, "Error");
			return;
		}

		if (Campaign.num_players != 0) {
			if (Campaign.num_players != a_mission.num_players) {
				char buf[512];

				sprintf(buf, "Mission \"%s\" has %d players.  Campaign has %d players.  Campaign will not play properly in FreeSpace", name, a_mission.num_players, Campaign.num_players );
				MessageBox(buf, "Warning");
			}

		} else {
			Campaign.num_players = a_mission.num_players;
		}
	}

	Elements[Campaign.num_missions].from_links = Elements[Campaign.num_missions].to_links = 0;
	cm = &(Campaign.missions[Campaign.num_missions++]);
	cm->name = strdup(name);
	cm->formula = Locked_sexp_true;
	cm->num_goals = -1;
	cm->notes = NULL;
	cm->briefing_cutscene[0] = 0;
	for (i=0; i<Campaign.num_missions - 1; i++)
		if ((Campaign.missions[i].level == level) && (Campaign.missions[i].pos + 1 == pos)) {
			pos = query_alternate_pos(point);
			break;
		}

	cm->level = level;
	cm->pos = pos - 1;
	correct_position(Campaign.num_missions - 1);
	sort_links();
	SetScrollSizes(MM_TEXT, CSize(total_width * CELL_WIDTH, total_levels * LEVEL_HEIGHT));
	Invalidate();

	// update and reinitialize dialog items
	if ( Campaign.type != CAMPAIGN_TYPE_SINGLE ) {
		Campaign_tree_formp->update();
		Campaign_tree_formp->initialize(0);
	}

	listbox->DeleteString(item);
	Campaign_modified = 1;
	return;
}

void campaign_tree_view::sort_elements()
{
	int i, j, s1, s2;

	for (i=0; i<Campaign.num_missions; i++)
		Sorted[i] = i;

	// sort the tree, so realignment will work property
	for (i=1; i<Campaign.num_missions; i++) {
		s1 = Sorted[i];
		for (j=i-1; j>=0; j--) {
			s2 = Sorted[j];
			if ((Campaign.missions[s1].level > Campaign.missions[s2].level) ||
				((Campaign.missions[s1].level == Campaign.missions[s2].level) &&
				(Campaign.missions[s1].pos > Campaign.missions[s2].pos)))
					break;

			Sorted[j + 1] = s2;
		}

		Sorted[j + 1] = s1;
	}
}

void campaign_tree_view::correct_position(int num)
{
	int i, z;

	// move missions down if required
	if (Campaign.missions[num].level + 2 > total_levels)
		total_levels = Campaign.missions[num].level + 2;

	for (i=0; i<Total_links; i++)
		if (Links[i].from == num) {
			z = Links[i].to;
			if ( (num != z) && (Campaign.missions[num].level >= Campaign.missions[z].level) ) {
				Campaign.missions[z].level = Campaign.missions[num].level + 1;
				correct_position(z);
			}
		}

	// space out horizontally to avoid overlap of missions
	horizontally_align_mission(num, -1);
	horizontally_align_mission(num, 1);
}

void campaign_tree_view::horizontally_align_mission(int num, int dir)
{
	int i, z;

	if ((Campaign.missions[num].pos == -1) || (Campaign.missions[num].pos + 1 == total_width * 2)) {  // need to expand total_width
		for (i=0; i<Campaign.num_missions; i++)
			Campaign.missions[i].pos++;

		total_width++;
	}

	for (i=0; i<Campaign.num_missions; i++) {
		if (i == num)
			continue;

		if (Campaign.missions[i].level == Campaign.missions[num].level) {
			z = Campaign.missions[i].pos - Campaign.missions[num].pos;
			if (dir < 0) {
				if (!z || (z == -1)) {
					Campaign.missions[i].pos = Campaign.missions[num].pos - 2;
					horizontally_align_mission(i, -1);
				}

			} else {
				if (!z || (z == 1)) {
					Campaign.missions[i].pos = Campaign.missions[num].pos + 2;
					horizontally_align_mission(i, 1);
				}
			}
		}
	}
}

void campaign_tree_view::delete_link(int num)
{
	Assert((num >= 0) && (num < Total_links));
	if (Links[num].from != Links[num].to) {
		Elements[Links[num].from].from_links--;
		Elements[Links[num].to].to_links--;
	}

	sexp_unmark_persistent(Links[num].sexp);
	free_sexp2(Links[num].sexp);
	while (num < Total_links - 1) {
		Links[num] = Links[num + 1];
		num++;
	}

	Total_links--;
	sort_links();
	Invalidate();
	Campaign_modified = 1;
	return;
}

int campaign_tree_view::get_root_mission()
{
	int i;

	for (i=0; i<Campaign.num_missions; i++)
		if (!Campaign.missions[i].level)
			return i;

	return -1;
}

void campaign_tree_view::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	int i;
	CMenu menu, *popup;
	CPoint p = point;
	CClientDC dc(this);

	OnPrepareDC(&dc);
	dc.DPtoLP(&p);

	ScreenToClient(&p);
	for (i=0; i<Campaign.num_missions; i++)
		if (Elements[i].box.PtInRect(p))
			break;

	if (i < Campaign.num_missions) {  // clicked on a mission
		Context_mission = i;
		if (menu.LoadMenu(IDR_CPGN_VIEW_ON)) {
			popup = menu.GetSubMenu(0);
			ASSERT(popup);
			popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
		}

	} else {
		Context_mission = query_level(p);
		if ((Context_mission >= 0) && (Context_mission < total_levels))
			if (menu.LoadMenu(IDR_CPGN_VIEW_OFF)) {
				popup = menu.GetSubMenu(0);
				ASSERT(popup);
				popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
			}
	}
}

void campaign_tree_view::OnRemoveMission() 
{
	remove_mission(Context_mission);
	Invalidate();
	UpdateWindow();

	// for multiplayer missions, update the data and reiniailize the dialog -- the number of player
	// in the mission might have changed because of deletion of the first mission
	if ( Campaign.type != CAMPAIGN_TYPE_SINGLE ) {
		Campaign_tree_formp->update();
		Campaign_tree_formp->initialize();
	}
}

void campaign_tree_view::remove_mission(int m)
{
	int i, z;
	CEdit *box;

	Assert(m >= 0);
	Campaign_tree_formp->m_filelist.AddString(Campaign.missions[m].name);

	z = --Campaign.num_missions;
	i = Total_links;
	while (i--) {
		if ((Links[i].from == m) || (Links[i].to == m))
			delete_link(i);
		if (Links[i].from == z)
			Links[i].from = m;
		if (Links[i].to == z)
			Links[i].to = m;
	}

	Elements[m] = Elements[z];
	Campaign.missions[m] = Campaign.missions[z];
	if (m == Cur_campaign_mission) {
		Cur_campaign_mission = -1;
		box = (CEdit *) Campaign_tree_formp->GetDlgItem(IDC_HELP_BOX);
		if (box)
			box->SetWindowText("");

		Campaign_tree_formp->load_tree(0);
	}

	Campaign_modified = 1;
}

void campaign_tree_view::OnDeleteRow() 
{
	int i, z;

	if (!Context_mission) {
		MessageBox("Can't delete the top level");
		return;
	}

	for (i=z=0; i<Campaign.num_missions; i++)
		if (Campaign.missions[i].level == Context_mission)
			z++;

	if (z) {
		z = MessageBox("Deleting row will remove all missions on this row", "Notice", MB_ICONEXCLAMATION | MB_OKCANCEL);
		if (z == IDCANCEL)
			return;
	}

	while (i--)
		if (Campaign.missions[i].level == Context_mission)
			remove_mission(i);

	for (i=0; i<Campaign.num_missions; i++)
		if (Campaign.missions[i].level > Context_mission)
			Campaign.missions[i].level--;

	total_levels--;
	SetScrollSizes(MM_TEXT, CSize(total_width * CELL_WIDTH, total_levels * LEVEL_HEIGHT));
	Invalidate();
	UpdateWindow();
	Campaign_modified = 1;
}

void campaign_tree_view::OnInsertRow() 
{
	int i;

	for (i=0; i<Campaign.num_missions; i++)
		if (Campaign.missions[i].level >= Context_mission)
			Campaign.missions[i].level++;

	total_levels++;
	SetScrollSizes(MM_TEXT, CSize(total_width * CELL_WIDTH, total_levels * LEVEL_HEIGHT));
	Invalidate();
	UpdateWindow();
	Campaign_modified = 1;
}

void campaign_tree_view::OnAddRepeat() 
{
	if (add_link(Context_mission, Context_mission)) {
		MessageBox("Too many links exist.  Can't add any more.");
		return;
	}
}

void campaign_tree_view::OnEndOfCampaign() 
{
	if ( add_link(Context_mission, -1) ) {
		MessageBox("Too many links exist.  Cannot add any more.");
		return;
	}
}

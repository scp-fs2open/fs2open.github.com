/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/BriefingEditorDlg.h $
 * $Revision: 1.3 $
 * $Date: 2006-05-30 02:13:22 $
 * $Author: Goober5000 $
 *
 * Briefing editor dialog box class.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/01/26 04:01:58  Goober5000
 * spelling
 *
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.6  2005/12/29 00:55:25  phreak
 * Briefing icons are now able to be flipped.  Left becomes right and vice versa.
 *
 * Revision 1.5  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.4  2005/04/13 20:02:08  chhogg
 * Changed vector to vec3d for Fred.
 *
 * Revision 1.3  2004/09/29 17:26:32  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:10:52  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 3     5/20/99 1:46p Andsager
 * Add briefing view copy and paste between stages
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
 * 24    4/20/98 4:40p Hoffoss
 * Added a button to 4 editors to play the chosen wave file.
 * 
 * 23    4/07/98 4:51p Dave
 * (Hoffoss) Fixed a boat load of bugs caused by the new change to
 * multiple briefings.  Allender's code changed to support this in the
 * briefing editor wasn't quite correct.
 * 
 * 22    4/06/98 5:37p Hoffoss
 * Added sexp tree support to briefings in Fred.
 * 
 * 21    2/18/98 6:45p Hoffoss
 * Added support for lines between icons in briefings for Fred.
 * 
 * 20    2/09/98 9:25p Allender
 * team v team support.  multiple pools and briefings
 * 
 * 19    2/04/98 4:32p Allender
 * support for multiple briefings and debriefings.  Changes to mission
 * type (now a bitfield).  Bitfield defs for multiplayer modes
 * 
 * 18    11/04/97 4:33p Hoffoss
 * Made saving keep the current briefing state intact.
 * 
 * 17    10/19/97 11:32p Hoffoss
 * Added support for briefing cuts in Fred.
 * 
 * 16    9/30/97 5:56p Hoffoss
 * Added music selection combo boxes to Fred.
 * 
 * 15    8/19/97 10:15a Hoffoss
 * Made escape close briefing window through normal channels.
 * 
 * 14    8/14/97 6:37p Hoffoss
 * Added briefing icon id support to Fred.
 * 
 * 13    8/07/97 3:45p Hoffoss
 * Added in several requested features to the briefing editor.
 * 
 * 12    8/06/97 12:25p Hoffoss
 * Fixed bugs: Briefing editor dialog wasn't getting reset when new
 * mission created, and hitting MFC assert when quitting while briefing
 * editor open.
 * 
 * 11    7/03/97 9:42a Hoffoss
 * Added a ship type field in briefing editor.
 * 
 * 10    6/26/97 6:04p Hoffoss
 * Briefing icons now are selected before normal objects.
 * 
 * 9     6/26/97 5:18p Hoffoss
 * Major rework of briefing editor functionality.
 * 
 * 8     6/25/97 4:13p Hoffoss
 * Added functionality to the make icon button.
 * 
 * 7     6/25/97 10:37a Hoffoss
 * Added icon text field support to briefing editor.
 * 
 * 6     6/24/97 3:03p Hoffoss
 * New stages now copy another stage if possible.
 * 
 * 5     6/24/97 11:31a Hoffoss
 * Added code for handling icon editing.
 * 
 * 4     6/24/97 10:16a Hoffoss
 * Changes to briefing editor code.
 * 
 * 3     6/23/97 2:58p Hoffoss
 * Added more functionality to briefing editor.
 * 
 * 2     6/18/97 2:39p Hoffoss
 * Improved on briefing editor.  Still far from done, though.
 *
 * $NoKeywords: $
 */

#ifndef __BRIEFINGEDITORDLG_H__
#define __BRIEFINGEDITORDLG_H__

#include "parse/sexp.h"
#include "mission/missionbriefcommon.h"

/////////////////////////////////////////////////////////////////////////////
// briefing_editor_dlg dialog

class briefing_editor_dlg : public CDialog
{
// Construction
public:
	void focus_sexp(int select_sexp_node);
	int calc_num_lines_for_icons(int num);
	void batch_render();
	void save_editor_state();
	void restore_editor_state();
	void reset_icon_loop(int stage);
	int get_next_icon(int id);
	void OnOK();
	void OnCancel();
	int find_icon(int id, int stage);
	void propagate_icon(int num);
	void reset_editor();
	int check_mouse_hit(int x, int y);
	void delete_icon(int num);
	void update_positions();
	void icon_select(int num);
	void draw_icon(object *objp);
	void create();
	void update_data(int update = 1);
	briefing_editor_dlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(briefing_editor_dlg)
	enum { IDD = IDD_BRIEFING_EDITOR };
	sexp_tree	m_tree;
	CButton	m_lines;
	BOOL	m_hilight;
	int		m_icon_image;
	CString	m_icon_label;
	CString	m_stage_title;
	CString	m_text;
	CString	m_time;
	CString	m_voice;
	CString	m_icon_text;
	int		m_icon_team;
	int		m_ship_type;
	BOOL	m_change_local;
	int		m_id;
	int		m_briefing_music;
	CString	m_substitute_briefing_music;
	BOOL	m_cut_next;
	BOOL	m_cut_prev;
	int		m_current_briefing;
	BOOL	m_flipicon;
	//}}AFX_DATA

	CBitmap m_play_bm;

	// copy view variables
	int		m_copy_view_set;
	vec3d	m_copy_view_pos;
	matrix	m_copy_view_orient;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(briefing_editor_dlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	brief_icon *iconp;
	briefing *save_briefing;
	int icon_loop, stage_loop;
	int m_cur_stage;
	int m_last_stage;
	int m_cur_icon;
	int m_last_icon;
	int stage_saved;
	int icon_saved;
	int modified;
//	int point_obj;
	int icon_obj[MAX_STAGE_ICONS];
	int icon_marked[MAX_STAGE_ICONS];
	int line_marked[MAX_BRIEF_STAGE_LINES];

	void copy_stage(int from, int to);

	// Generated message map functions
	//{{AFX_MSG(briefing_editor_dlg)
	afx_msg void OnClose();
	afx_msg void OnNext();
	afx_msg void OnPrev();
	afx_msg void OnBrowse();
	afx_msg void OnAddStage();
	afx_msg void OnDeleteStage();
	afx_msg void OnInsertStage();
	afx_msg void OnMakeIcon();
	afx_msg void OnDeleteIcon();
	afx_msg void OnGotoView();
	afx_msg void OnSaveView();
	afx_msg void OnSelchangeIconImage();
	afx_msg void OnSelchangeTeam();
	afx_msg void OnPropagateIcons();
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnLines();
	afx_msg void OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPlay();
	afx_msg void OnCopyView();
	afx_msg void OnPasteView();
	afx_msg void OnFlipIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
};

#endif

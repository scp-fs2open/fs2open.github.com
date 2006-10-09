/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Fred2/EventEditor.h $
 * $Revision: 1.2 $
 * $Date: 2006-10-09 05:25:18 $
 * $Author: Goober5000 $
 *
 * Event editor dialog box class and event tree class (used for dialog)
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.4  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.3  2004/09/29 17:26:32  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:10:53  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 5     8/26/99 8:52p Dave
 * Gave multiplayer TvT messaging a heavy dose of sanity. Cheat codes.
 * 
 * 4     5/04/99 5:21p Andsager
 * 
 * 3     2/17/99 2:11p Dave
 * First full run of squad war. All freespace and tracker side stuff
 * works.
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
 * 27    5/15/98 5:51p Hoffoss
 * Fixed escape key and cancel button bugs.
 * 
 * 26    4/22/98 9:56a Sandeep
 * 
 * 25    4/20/98 4:40p Hoffoss
 * Added a button to 4 editors to play the chosen wave file.
 * 
 * 24    4/03/98 5:20p Hoffoss
 * Changed code so that changing a message's wave file will update the
 * persona as well, if the wave file has the proper prefix.
 * 
 * 23    2/16/98 6:25p Hoffoss
 * Did major rework of the whole right_clicked() handler to simplify it
 * all, break it down and make it more flexible.  Should be a lot easier
 * to work with from now on.
 * 
 * 22    2/16/98 2:42p Hoffoss
 * Added new code in preparation to simplify the sexp_tree monster.
 * Checking in code now as a good foundation point that I can revert back
 * to if needed.
 * 
 * 21    1/08/98 11:18a Hoffoss
 * Fixed several bugs in new Event Editor.
 * 
 * 20    1/07/98 5:58p Hoffoss
 * Combined message editor into event editor.
 * 
 * 19    1/06/98 8:25p Hoffoss
 * Added insert event functionality to event editor.
 * 
 * 18    10/10/97 6:21p Hoffoss
 * Put in Fred support for training object list editing.
 * 
 * 17    10/09/97 1:03p Hoffoss
 * Renaming events or goals now updates sexp references as well.
 * 
 * 16    9/30/97 10:01a Hoffoss
 * Added event chaining support to Fred and FreeSpace.
 * 
 * 15    8/12/97 3:33p Hoffoss
 * Fixed the "press cancel to go to reference" code to work properly.
 * 
 * 14    7/30/97 5:23p Hoffoss
 * Removed Sexp tree verification code, since it duplicates normal sexp
 * verification, and is just another set of code to keep maintained.
 * 
 * 13    7/25/97 3:05p Allender
 * added score field to goals and events editor
 * 
 * 12    7/24/97 12:45p Hoffoss
 * Added sexp help system to sexp trees and some dialog boxes.
 * 
 * 11    7/17/97 4:10p Hoffoss
 * Added drag and drop to sexp trees for reordering root items.
 * 
 * 10    7/15/97 10:30a Allender
 * added repeat count and interval time to event editor.  For use in
 * repeating events at regular intervals
 * 
 * 9     6/02/97 8:47p Hoffoss
 * Fixed bug with inserting an operator at root position, but under a
 * label.
 * 
 * 8     5/20/97 2:28p Hoffoss
 * Added message box queries for close window operation on all modal
 * dialog boxes.
 * 
 * 7     5/01/97 4:12p Hoffoss
 * Added return handling to dialogs.
 * 
 * 6     4/17/97 2:01p Hoffoss
 * All dialog box window states are saved between sessions now.
 * 
 * 5     4/11/97 10:10a Hoffoss
 * Name fields supported by Fred for Events and Mission Goals.
 * 
 * 4     4/07/97 3:48p Hoffoss
 * Event editor now supports a sporty new delete button!
 * 
 * 3     1/22/97 11:01a Hoffoss
 * Many bug fixes (those pointed out by Mark during Fred testing trying to
 * make mission 5).
 * 
 * 2     1/13/97 4:54p Hoffoss
 * Added event editor.
 *
 * $NoKeywords: $
 */

#ifndef _EVENTEDITOR_H
#define _EVENTEDITOR_H

#include "Sexp_tree.h"
#include "mission/missiongoals.h"
#include "mission/missionmessage.h"

#define MAX_SEARCH_MESSAGE_DEPTH		5		// maximum search number of event nodes with message text

class event_sexp_tree : public sexp_tree
{
};

/////////////////////////////////////////////////////////////////////////////
// event_editor dialog

class event_editor : public CDialog
{
// Construction
public:
	void update_persona();
	void save();
	char *current_message_name(int index);
	char *get_message_list_item(int i);
	int save_message(int num);
	void update_cur_message();
	HTREEITEM get_event_handle(int num);
	void reset_event(int num, HTREEITEM after);
	void save_event(int e);
	void swap_handler(int node1, int node2);
	void insert_handler(int old, int node);
	int query_modified();
	void OnOK();
	void OnCancel();
	int handler(int code, int node, char *str = NULL);
	void create_tree();
	void load_tree();
	int modified;
	int select_sexp_node;
	event_editor(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(event_editor)
	enum { IDD = IDD_EVENT_EDITOR };
	event_sexp_tree	m_event_tree;
	UINT	m_repeat_count;
	UINT	m_interval;
	int		m_event_score;
	int		m_chain_delay;
	BOOL	m_chained;
	CString	m_obj_text;
	CString	m_obj_key_text;
	CString	m_avi_filename;
	CString	m_message_name;
	CString	m_message_text;
	int		m_persona;
	CString	m_wave_filename;
	int		m_cur_msg;
	int		m_team;
	int		m_message_team;
	int		m_last_message_node;
	//}}AFX_DATA

	CBitmap m_play_bm;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(event_editor)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(event_editor)
	virtual BOOL OnInitDialog();
	afx_msg void OnRclickEventTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditEventTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditEventTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonNewEvent();
	afx_msg void OnDelete();
	afx_msg void OnOk();
	afx_msg void OnClose();
	afx_msg void OnSelchangedEventTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateRepeatCount();
	afx_msg void OnChained();
	afx_msg void OnInsert();
	afx_msg void OnSelchangeMessageList();
	afx_msg void OnNewMsg();
	afx_msg void OnDeleteMsg();
	afx_msg void OnBrowseAvi();
	afx_msg void OnBrowseWave();
	afx_msg void OnSelchangeWaveFilename();
	afx_msg void OnPlay();
	afx_msg void OnUpdate();
	afx_msg void On_Cancel();
	afx_msg void OnSelchangeTeam();
	afx_msg void OnSelchangeMessageTeam();
	afx_msg void OnDblclkMessageList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	int cur_event;
	void update_cur_event();
	int m_num_events;
	int m_sig[MAX_MISSION_EVENTS];
	mission_event m_events[MAX_MISSION_EVENTS];
	int m_num_messages;
	MMessage m_messages[MAX_MISSION_MESSAGES];
	int m_msg_sig[MAX_MISSION_MESSAGES];
};

extern event_editor *Event_editor_dlg; // global reference needed by event tree class

#endif

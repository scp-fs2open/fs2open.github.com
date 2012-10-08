/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
	UINT	m_trigger_count;
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
	int		m_log_true;
	int		m_log_false;
	int		m_log_always_true;
	int		m_log_always_false;
	int		m_log_1st_repeat;
	int		m_log_last_repeat;
	int		m_log_1st_trigger;
	int		m_log_last_trigger;
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
	afx_msg void OnUpdateTriggerCount();
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
	SCP_vector<MMessage> m_messages;
	int m_wave_id;
};

extern event_editor *Event_editor_dlg; // global reference needed by event tree class

#endif

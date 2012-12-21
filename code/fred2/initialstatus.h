/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// InitialStatus.h : header file
//


typedef struct dockpoint_information {
	int dockee_shipnum;
	int dockee_point;
} dockpoint_information;

/////////////////////////////////////////////////////////////////////////////
// initial_status dialog

class initial_status : public CDialog
{
// Construction
public:
	void dock(object *objp1, int dockpoint1, object *objp2, int dockpoint2);
	void undock(object *objp1, object *objp2);
	void OnOK();
	int inited;
	void change_subsys();
	void change_docker_point(bool store_selection);
	void change_dockee(bool store_selection);
	void change_dockee_point(bool store_selection);

	initial_status(CWnd* pParent = NULL);   // standard constructor
	~initial_status();

	int m_ship;
	int m_multi_edit;

// Dialog Data
	//{{AFX_DATA(initial_status)
	enum { IDD = IDD_INITIAL_STATUS };
	CSpinButtonCtrl	m_hull_spin;
	CSpinButtonCtrl	m_velocity_spin;
	CSpinButtonCtrl	m_shields_spin;
	CSpinButtonCtrl	m_damage_spin;
	int		m_damage;
	int		m_shields;
	int		m_force_shields;
	int		m_velocity;
	int		m_hull;
	BOOL	m_has_shields;
	int		m_ship_locked;
	int		m_weapons_locked;
	CString	m_cargo_name;
	int		m_primaries_locked;
	int		m_secondaries_locked;
	int		m_turrets_locked;
	int		m_afterburner_locked;
	CComboBox m_team_color_setting;

	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(initial_status)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(initial_status)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeSubsys();
	afx_msg void OnSelchangeDockerPoint();
	afx_msg void OnSelchangeDockee();
	afx_msg void OnSelchangeDockeePoint();
	afx_msg void OnHasShields();
	afx_msg void OnForceShields();
	afx_msg void OnShipLocked();
	afx_msg void OnWeaponsLocked();
	afx_msg void OnPrimariesLocked();
	afx_msg void OnSecondariesLocked();
	afx_msg void OnTurretsLocked();
	afx_msg void OnAfterburnersLocked();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void list_dockees(int dock_types);
	void list_dockee_points(int shipnum);
	void update_docking_info();
	int cur_subsys;

	dockpoint_information *dockpoint_array;
	int num_dock_points;

	CListBox *lstDockerPoints;
	CComboBox *cboDockees;
	CComboBox *cboDockeePoints;

	int cur_docker_point;
	int cur_dockee;
	int cur_dockee_point;
};

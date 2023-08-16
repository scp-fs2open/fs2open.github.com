/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */



#include "Management.h"

/////////////////////////////////////////////////////////////////////////////
// warp_params_dlg dialog

class warp_params_dlg : public CDialog
{
// Construction
public:	
	warp_params_dlg(CWnd* pParent = nullptr);   // standard constructor

	// parameters for the dialog
	bool m_warp_in;


// Dialog Data
	//{{AFX_DATA(warp_params_dlg)
	enum { IDD = IDD_WARP_PARAMS };
	int	m_warp_type;
	CString	m_start_sound;
	CString	m_end_sound;
	CString	m_warpout_engage_time;
	CString	m_speed;
	CString	m_time;
	CString	m_accel_exp;
	CString	m_radius;
	CString	m_anim;
	BOOL m_special_warp_physics;
	CString	m_player_warpout_speed;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(warp_params_dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(warp_params_dlg)
	virtual BOOL OnInitDialog();
	void OnCancel();	
	void OnOK();
	afx_msg void OnSelchangeWarpType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// regenerate all controls
	void reset_controls();
};

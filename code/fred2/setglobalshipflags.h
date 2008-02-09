#if !defined(AFX_SETGLOBALSHIPFLAGS_H__9D023C1A_2EED_451B_AEEB_F23347FFACAE__INCLUDED_)
#define AFX_SETGLOBALSHIPFLAGS_H__9D023C1A_2EED_451B_AEEB_F23347FFACAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetGlobalShipFlags.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// SetGlobalShipFlags dialog

class SetGlobalShipFlags : public CDialog
{
// Construction
public:
	SetGlobalShipFlags(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(SetGlobalShipFlags)
	enum { IDD = IDD_SET_GLOBAL_SHIP_FLAGS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SetGlobalShipFlags)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(SetGlobalShipFlags)
	afx_msg void OnNoShields();
	afx_msg void OnNoSubspaceDrive();
	afx_msg void OnPrimitiveSensors();
	afx_msg void OnAffectedByGravity();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETGLOBALSHIPFLAGS_H__9D023C1A_2EED_451B_AEEB_F23347FFACAE__INCLUDED_)

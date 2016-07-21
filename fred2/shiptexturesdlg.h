// ShipTexturesDlg.h : header file
// Goober5000

#include "Management.h"

#ifndef _SHIPTEXTURESDLG_H
#define _SHIPTEXTURESDLG_H

#define SORT_OLD	0
#define SORT_NEW	1

/////////////////////////////////////////////////////////////////////////////
// CShipTexturesDlg dialog

class CShipTexturesDlg : public CDialog
{
// Construction
public:
	CShipTexturesDlg(CWnd* pParent = NULL);   // standard constructor

	int self_ship;
	int active_texture_index;
	int modified;
	int texture_count;

// Dialog Data
	//{{AFX_DATA(CShipTexturesDlg)
	enum { IDD = IDD_SHIP_TEXTURES };
	CString	m_new_texture;
	int		m_old_texture_list;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShipTexturesDlg)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CShipTexturesDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSelchangeOldTextureList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	int num_model_textures;
	char old_texture_name[MAX_REPLACEMENT_TEXTURES][MAX_FILENAME_LEN];
	char new_texture_name[MAX_REPLACEMENT_TEXTURES][MAX_FILENAME_LEN];
	int query_modified();
	void sort_textures(int test = SORT_OLD);
	void swap_strings(char *str1, char *str2);
	texture_replace *texture_set(texture_replace *dest, const texture_replace *src);
};

//{{AFX_INSERT_LOCATION(CShipTexturesDlg)
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
//}}AFX_INSERT_LOCATION

#endif

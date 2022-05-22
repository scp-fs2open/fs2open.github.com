// CalcRelativeCoordsDlg.h : header file

#ifndef _CALCRELATIVECOORDSDLG_H
#define _CALCRELATIVECOORDSDLG_H

class calc_relative_coords_dlg : public CDialog
{
public:
	calc_relative_coords_dlg(CWnd *pParent = nullptr);

	// Dialog Data
	//{{AFX_DATA(calc_relative_coords_dlg)
	enum { IDD = IDD_CALC_RELATIVE_COORDS };
	CListBox	m_origin_list;
	CListBox	m_satellite_list;
	CString		m_distance;
	CString		m_orientation_p;
	CString		m_orientation_b;
	CString		m_orientation_h;
	//}}AFX_DATA

	SCP_vector<int> m_object_indexes;

protected:
	virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

	//{{AFX_MSG(calc_relative_coords_dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeOriginList();
	afx_msg void OnSelchangeSatelliteList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void update_coords();
};
#endif	// _CALCRELATIVECOORDSDLG_H

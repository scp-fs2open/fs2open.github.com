/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#pragma once

class CEditContainerAddNewDlg : public CDialog
{
public:
	CEditContainerAddNewDlg(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_ADD_NEW_CONTAINER };

	CString	m_new_container_name;
	bool cancelled;
	
protected:
	void DoDataExchange(CDataExchange* pDX) override;

	void OnOK() override;
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
};

/*
 * Created by Hassan "Karajorma" Kazmi and Josh "jg18" Glatt for The FreeSpace 2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#pragma once

// used for naming new containers and renaming existing ones
class CEditContainerNameDlg : public CDialog
{
public:
	CEditContainerNameDlg(const SCP_string &window_title,
		const SCP_string&old_name,
		CWnd *pParent = nullptr);

	enum { IDD = IDD_EDIT_CONTAINER_NAME };

	bool cancelled() const { return m_cancelled; }
	const char *new_container_name() const { return m_new_container_name; }

protected:
	void DoDataExchange(CDataExchange *pDX) override;

	void OnOK() override;
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()

	const CString m_window_title;

	CString	m_new_container_name;
	bool m_cancelled;
};

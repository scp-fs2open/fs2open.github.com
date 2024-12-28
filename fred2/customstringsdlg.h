/*
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#ifndef _CUSTOMSTRINGS_H
#define _CUSTOMSTRINGS_H


/////////////////////////////////////////////////////////////////////////////
// Edit (Add/Modify) Container dialog

class CustomStringsDlg : public CDialog {
  public:
	CustomStringsDlg(CWnd* pParent = nullptr);

	enum {
		IDD = IDD_EDIT_CUSTOM_STRINGS
	};

	CListBox m_data_lister;

  protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// intercept Enter and Escape
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnOK() override;     // default MFC OK behavior
	afx_msg void OnCancel() override; // default MFC Cancel behavior

	afx_msg void OnButtonOk();     // button handler
	afx_msg void OnButtonCancel(); // button handler
	afx_msg void OnClose();

	BOOL OnInitDialog() override;

	afx_msg void OnStringAdd();
	afx_msg void OnStringRemove();
	afx_msg void OnStringUpdate();

	afx_msg void OnListerSelectionChange();

	afx_msg void OnEnSetFocusEditString();

	afx_msg void OnEnKillFocusEditString();

	void update_data_lister();
	void update_text_edit_boxes(const SCP_string& key, const SCP_string& value, const SCP_string& text);
	void update_help_text(const SCP_string& desc);

	bool edit_boxes_have_valid_data(bool update = false);
	bool data_edit_box_has_valid_data();
	bool key_edit_box_has_valid_data(bool update = false);
	bool text_edit_box_has_valid_data();

	void add_pair_entry();

	DECLARE_MESSAGE_MAP()

  private:
	bool query_modified() const;
	bool m_modified;

	bool m_text_edit_focus;

	SCP_vector<custom_string> m_custom_strings;

	// read-only view of data pair keys
	SCP_vector<SCP_string> m_lister_keys;
};

#endif // _CUSTOMDATA_H
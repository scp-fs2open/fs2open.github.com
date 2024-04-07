#pragma once

/*
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */


/////////////////////////////////////////////////////////////////////////////
// Edit (Add/Modify) Container dialog

class CustomDataDlg : public CDialog {
  public:
	CustomDataDlg(CWnd* pParent = nullptr);

	enum {
		IDD = IDD_EDIT_CUSTOM_DATA
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

	afx_msg void OnPairAdd();
	afx_msg void OnPairRemove();
	afx_msg void OnPairUpdate();

	afx_msg void OnListerSelectionChange();

	void update_data_lister();
	void update_text_edit_boxes(const SCP_string& key, const SCP_string& data);
	void update_help_text(const SCP_string& desc);

	bool edit_boxes_have_valid_data(bool dup_key_ok);
	bool data_edit_box_has_valid_data();
	bool key_edit_box_has_valid_data(bool dup_key_ok);

	void add_pair_entry(int insert_index);

	DECLARE_MESSAGE_MAP()

  private:
	bool query_modified() const;

	SCP_map<SCP_string, SCP_string> m_custom_data;

	// read-only view of data pair keys
	SCP_vector<SCP_string> m_lister_keys;
};
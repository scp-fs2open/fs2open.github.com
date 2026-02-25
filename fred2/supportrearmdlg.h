#pragma once

class CSupportRearmDlg : public CDialog {
  public:
	CSupportRearmDlg(CWnd* pParent = nullptr);
	enum {
		IDD = IDD_SUPPORT_REARM_OPTIONS
	};

	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;
	void OnOK() override;

  protected:
	afx_msg void OnSelchangeWeaponList();
	afx_msg void OnSetPoolAmount();
	afx_msg void OnSetPoolUnlimited();
	afx_msg void OnSetPoolZero();
	afx_msg void OnSetAllPoolAmount();
	afx_msg void OnSetAllPoolUnlimited();
	afx_msg void OnSetAllPoolZero();
	afx_msg void OnOptionChanged();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	DECLARE_MESSAGE_MAP()

  private:
	void update_control_states();
	void populate_weapon_list();
	void update_weapon_amount_display();
	int get_selected_weapon_class() const;
	void set_selected_weapon_amount(int amount);
	void set_all_weapon_amount(int amount);
	CString format_weapon_pool_entry(int weapon_class) const;

	BOOL m_disallow_support_ships;
	BOOL m_limit_rearm_to_pool;
	BOOL m_support_repairs_hull;
	BOOL m_disallow_support_rearm;
	BOOL m_allow_weapon_precedence;
	BOOL m_rearm_pool_from_loadout;
	float m_max_hull_repair_val;
	float m_max_subsys_repair_val;
	int m_weapon_pool_amount;
	int m_rearm_weapon_pool[MAX_WEAPON_TYPES];
};

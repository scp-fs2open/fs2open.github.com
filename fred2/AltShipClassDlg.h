/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#ifndef _ALTSHIPCLASSDLG_H
#define _ALTSHIPCLASSDLG_H

/**
 * @class AltShipClassDlg
 *
 * @brief Alternate Ship Class Editor, called from the Alt Ship Class button (Ships Editor)
 */
class AltShipClassDlg : public CDialog
{
public:
	/**
	 * @brief Standard constructor, nothing special
	 *
	 * @see OnInitDialog
	 */
	AltShipClassDlg(CWnd* pParent = NULL);

	//{{AFX_DATA(AltShipClassDlg)
	enum
	{
		IDD = IDD_ALT_SHIP_CLASS
	};

	CButton m_default_to_class; //!< That's no button, its a checkbox! Filled if the selected alt ship type in the class list is considered the default.
	CListBox  m_alt_class_list; //!< Listbox of alternative ship types allowed for the selected ship

	CComboBox m_set_from_ship_class;    //!< Choicebox of available ship types
	int m_selected_class;               //!< The selected ship class in the m_set_from_ship_class choicebox

	CComboBox m_set_from_variables;     //!< Choicebox of available SEXP variables
	int m_selected_variable;            //!< The selected SEXP variable in the m_set_from_variables choicebox
	//}}AFX_DATA

	//{{AFX_VIRTUAL(AltShipClassDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	//{{AFX_MSG(AltShipClassDlg)
	/**
	 * @brief Handler for dialog initialization
	 */
	virtual BOOL OnInitDialog();

	/**
	 * @brief Handler for the Cancel button
	 */
	virtual void OnCancel();

	/**
	 * @brief Handler for the OK button
	 */
	virtual void OnOK();

	/**
	 * @brief Handler for the AltClass::Add button
	 */
	afx_msg void OnAltClassAdd();

	/**
	 * @brief Handler for the AltClass::Insert button
	 */
	afx_msg void OnAltClassInsert();

	/**
	 * @brief Handler for the AltClass::Delete button
	 */
	afx_msg void OnAltClassDelete();

	/**
	 * @brief Handler for the MoveUp button
	 *
	 * @details Moves the selected item up by one
	 */
	afx_msg void OnMoveUp();

	/**
	 * @brief Handler for the MoveDown button
	 *
	 * @details Moves the selected item down by one
	 */
	afx_msg void OnMoveDown();

	afx_msg void OnSelendokSetFromShipClass();
	afx_msg void OnSelendokSetFromVariables();
	afx_msg void OnSelchangeAltClassList();

	/**
	 * @brief Handler for the Default to Class checkbox
	 */
	afx_msg void OnDefaultToClass();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool player_ships_only; //!< True if we can only add player ships to the alt classes, false if we can add any

	//! Pool of alt ship classes for the selected item.
	//! The pool is shown in the m_alt_class_list but this vector is easier to work with internally.
	SCP_vector<alt_class> alt_class_pool;

	int num_string_variables;                           //!< Number of string variables in the mission
	int string_variable_indices[MAX_SEXP_VARIABLES];    //!< maps string variables to their index in Sexp_variables
	int ship_class_indices[MAX_SHIP_CLASSES];           //!< maps ships in the ships combobox to their index in Ship_info

	//!@
	//! Multi-edit member.
	bool multi_edit;                    //!< True if multiple ships are selected
	int m_selected_ships[MAX_SHIPS];    //!< Array containing the indices of the selected ships.
	int num_selected_ships;             //!< Number of ships selected
	//!@}

	/**
	 * @brief Helper function, rebuilds the alt ship pool and repopulates the m_alt_class_list listbox
	 */
	void AltShipClassDlg::alt_class_list_rebuild();

	/**
	 * @brief Helper function, updates the selected alt ship entry
	 */
	void AltShipClassDlg::alt_class_update_entry(alt_class &list_item);
};

//{{AFX_INSERT_LOCATION}}

#endif // _ALTSHIPCLASSDLG_H

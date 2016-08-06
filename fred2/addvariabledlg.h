/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */
#ifndef _ADDVARIABLEDLG_H
#define _ADDVARIABLEDLG_H

/**
* @class CAddVariableDlg
*
* @brief The Add Variable dialog, created when adding a new variable to a sexp in the Mission Event editor
*/
class CAddVariableDlg : public CDialog
{
public:
	/**
	* @brief Standard constructor.
	*
	* @see CAddVariableDlg::OnInitDialog()
	*/
	CAddVariableDlg(CWnd* pParent = NULL);

	// Dialog Data
	//{{AFX_DATA(CAddVariableDlg)
	enum
	{
		IDD = IDD_ADD_VARIABLE
	};

	CString m_default_value;    //!< The default value for the new variable
	CString m_variable_name;    //!< The name of the new variable
	bool m_name_validated;      //!< True if the name has been validated
	bool m_data_validated;      //!< True if the data has been validated

	// Radiobutton group
	bool m_type_number;     //!< True if the type is a number, false if the type is a string

	// Checkbox group
	bool m_type_campaign_persistent;    //!< True if the variable is campaign persistant
	bool m_type_player_persistent;      //!< True if the variable is player persistant
	bool m_type_network_variable;       //!< True if the variable is a network variable (multiplayer)

	bool m_create;  //!< True if the variable should be created upon the dialog's closure

	int m_sexp_var_index;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddVariableDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CAddVariableDlg)
	/**
	 * @brief Handler for the OK button
	 */
	virtual void OnOK();

	/**
	 * @brief Handler for dialog initialization.
	 */
	virtual BOOL OnInitDialog();

	/**
	 * @brief Validates the new variable name
	 *
	 * @param[in] set_focus If equal to @p SET_FOCUS a message box will inform the user of the error.
	 *
	 * @details Checks if the variable name's value has changed, nonzero length, and does not already exist
	 */
	afx_msg void validate_variable_name(int set_focus);

	/**
	 * @brief Validates the new variable's data
	 *
	 * @param[in] set_focus If equal to @p SET_FOCUS a message box will inform the user of the error.
	 *
	 * @details Checks if the data string's value is nonzero length, and (if a number type) is a valid number string
	 */
	afx_msg void validate_data(int set_focus);
	
	/**
	 * @brief Handler for the Type::Number radio button
	 */
	afx_msg void OnTypeNumber();

	/**
	 * @brief Handler for the Type::String radio button
	 */
	afx_msg void OnTypeString();

	/**
	 * @brief Handler for the Player Persistent radio checkbox
	 *
	 * @details A variable may be player persistent, campaign persitent, or not persetent.
	 */
	afx_msg void OnTypePlayerPersistent();

	/**
	 * @brief Handler for the Campaign Persistent radio checkbox
	 *
	 * @details A variable may be player persistent, campaign persitent, or not persetent.
	 */
	afx_msg void OnTypeCampaignPersistent();

	/**
	 * @brief Handler for the Network Variable checkbox
	 */
	afx_msg void OnTypeNetworkVariable();

	/**
	 * @brief Handler for the type checkboxes and radio buttons.
	 *
	 * @details Sets each of the widget's Check state according to the member values
	 */
	afx_msg void set_variable_type();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // _ADDVARIABLEDLG_H

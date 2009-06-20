/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */



#include "model/model.h"
#include "parse/parselo.h"

/////////////////////////////////////////////////////////////////////////////
// restrict_paths dialog

class restrict_paths : public CDialog
{
// Construction
public:	
	restrict_paths(CWnd* pParent = NULL);   // standard constructor

	// parameters for the dialog
	bool m_arrival;
	int	m_ship_class;
	int *m_path_mask;


// Dialog Data
	//{{AFX_DATA(restrict_paths)
	enum { IDD = IDD_RESTRICT_PATHS };
	CCheckListBox		m_path_list;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(restrict_paths)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(restrict_paths)
	virtual BOOL OnInitDialog();
	void OnCancel();	
	void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// model info
	polymodel *m_model;
	int m_num_paths;

	// regenerate all controls
	void reset_controls();
};

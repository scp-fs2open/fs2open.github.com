/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */



#include "stdafx.h"
#include "FRED.h"
#include "restrictpaths.h"
#include "ship/ship.h"
#include "model/model.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// restrict_paths dialog

restrict_paths::restrict_paths(CWnd* pParent) : CDialog(restrict_paths::IDD, pParent)
{
	//{{AFX_DATA_INIT(restrict_paths)
	//}}AFX_DATA_INIT
}

void restrict_paths::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(restrict_paths)
	DDX_Control(pDX, IDC_PATH_LIST, m_path_list);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(restrict_paths, CDialog)
	//{{AFX_MSG_MAP(restrict_paths)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// restrict_paths message handlers

BOOL restrict_paths::OnInitDialog() 
{
	// get stuff from params
	m_model = model_get(Ship_info[m_ship_class].model_num);
	Assert(m_model->ship_bay);
	m_num_paths = m_model->ship_bay->num_paths;
	Assert(m_num_paths > 0);
				
	// misc window crap
	CDialog::OnInitDialog();
	theApp.init_window(&Player_wnd_data, this);

	// set up gui
	reset_controls();

	return TRUE;
}

void restrict_paths::reset_controls()
{
	int i;
	BOOL allowed;

	// initialize checkbox
	m_path_list.ResetContent();
	for (i = 0; i < m_num_paths; i++)
	{
		// add name
		m_path_list.AddString(m_model->paths[m_model->ship_bay->path_indexes[i]].name);

		// toggle according to mask
		if (m_path_mask == 0)
			allowed = TRUE;
		else
			allowed = (*m_path_mask & (1 << i)) ? TRUE : FALSE;

		m_path_list.SetCheck(i, allowed);
	}

	// set up the label
	CStatic *label = (CStatic *) GetDlgItem(IDC_RESTRICT_PATHS_LABEL);
	if (m_arrival)
		label->SetWindowText("Restrict arrival paths to the following:");
	else
		label->SetWindowText("Restrict departure paths to the following:");

	// be sure that nothing is selected	
	m_path_list.SetCurSel(-1);

	// store stuff to gui
	UpdateData(FALSE);
}

// cancel
void restrict_paths::OnCancel()
{
	theApp.record_window_data(&Player_wnd_data, this);
	CDialog::OnCancel();
}

// ok
void restrict_paths::OnOK()
{
	int i, num_allowed = 0;

	// grab stuff from GUI
	UpdateData(TRUE);

	// store mask data
	*m_path_mask = 0;
	for (i = 0; i < m_num_paths; i++)
	{
		if (m_path_list.GetCheck(i))
		{
			*m_path_mask |= (1 << i);
			num_allowed++;
		}
	}

	// if all allowed, mask is 0
	if (num_allowed == m_num_paths)
		*m_path_mask = 0;

	theApp.record_window_data(&Player_wnd_data, this);
	CDialog::OnOK();
}

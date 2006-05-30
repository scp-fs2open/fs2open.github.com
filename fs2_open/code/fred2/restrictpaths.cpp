/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

/*
 * $Logfile: /Freespace2/code/FRED2/RestrictPaths.cpp $
 * $Revision: 1.2 $
 * $Date: 2006-05-30 06:01:05 $
 * $Author: Goober5000 $
 *
 * Code for restricting arrival/departure to specific bays
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/05/30 05:58:59  Goober5000
 * I should probably add these files too
 * --Goober5000
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "FRED.h"
#include "restrictpaths.h"
#include "ship/ship.h"
#include "model/model.h"

#ifdef _DEBUG
#define new DEBUG_NEW
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
	int i;
				
	m_model = model_get(Ship_info[m_ship_class].modelnum);
	Assert(m_model->ship_bay);
	m_num_paths = m_model->ship_bay->num_paths;
	Assert(m_num_paths > 0);

	// initialize path data
	for (i = 0; i < MAX_SHIP_BAY_PATHS; i++)
	{
		// initialize to either the name or a blank
		if (i < m_num_paths)
			strcpy(m_ship_bay_data[i].name, m_model->paths[m_model->ship_bay->paths[i]].name);
		else
			strcpy(m_ship_bay_data[i].name, "");

		// initialize to true
		m_ship_bay_data[i].allowed = true;
	}

	// set up the label
	CStatic *label = (CStatic *) GetDlgItem(IDC_RESTRICT_PATHS_LABEL);
	if (m_arrival)
		label->SetWindowText("Restrict arrival paths to the following:");
	else
		label->SetWindowText("Restrict departure paths to the following:");

	// misc window crap
	CDialog::OnInitDialog();
	theApp.init_window(&Player_wnd_data, this);

	// regenerate all the controls
	reset_controls();

	return TRUE;
}

// regenerate all controls
void restrict_paths::reset_controls()
{	
	int i;

	// create a checklistbox for each bay path
	m_path_list.ResetContent();
	for (i = 0; i < m_num_paths; i++)
	{
		m_path_list.AddString(m_ship_bay_data[i].name);
		m_path_list.SetCheck(i, m_ship_bay_data[i].allowed ? TRUE : FALSE);
	}

	// be sure that nothing is selected	
	m_path_list.SetCurSel(-1);

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
	// store whatever

	theApp.record_window_data(&Player_wnd_data, this);
	CDialog::OnOK();
}

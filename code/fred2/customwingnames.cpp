// CustomWingNames.cpp : implementation file
//

#include "stdafx.h"
#include "fred.h"
#include "CustomWingNames.h"
#include "ship/ship.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CustomWingNames dialog


CustomWingNames::CustomWingNames(CWnd* pParent /*=NULL*/)
	: CDialog(CustomWingNames::IDD, pParent)
{
	//{{AFX_DATA_INIT(CustomWingNames)
	m_squadron_1 = _T("");
	m_squadron_2 = _T("");
	m_squadron_3 = _T("");
	m_squadron_4 = _T("");
	m_squadron_5 = _T("");
	m_starting_1 = _T("");
	m_starting_2 = _T("");
	m_starting_3 = _T("");
	m_tvt_1 = _T("");
	m_tvt_2 = _T("");
	//}}AFX_DATA_INIT
}


void CustomWingNames::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CustomWingNames)
	DDX_Text(pDX, IDC_SQUADRON_WING_NAME_1, m_squadron_1);
	DDX_Text(pDX, IDC_SQUADRON_WING_NAME_2, m_squadron_2);
	DDX_Text(pDX, IDC_SQUADRON_WING_NAME_3, m_squadron_3);
	DDX_Text(pDX, IDC_SQUADRON_WING_NAME_4, m_squadron_4);
	DDX_Text(pDX, IDC_SQUADRON_WING_NAME_5, m_squadron_5);
	DDX_Text(pDX, IDC_STARTING_WING_NAME_1, m_starting_1);
	DDX_Text(pDX, IDC_STARTING_WING_NAME_2, m_starting_2);
	DDX_Text(pDX, IDC_STARTING_WING_NAME_3, m_starting_3);
	DDX_Text(pDX, IDC_TVT_WING_NAME_1, m_tvt_1);
	DDX_Text(pDX, IDC_TVT_WING_NAME_2, m_tvt_2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CustomWingNames, CDialog)
	//{{AFX_MSG_MAP(CustomWingNames)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CustomWingNames message handlers

BOOL CustomWingNames::OnInitDialog()
{
	// init starting wings
	m_starting_1 = _T(Starting_wing_names[0]);
	m_starting_2 = _T(Starting_wing_names[1]);
	m_starting_3 = _T(Starting_wing_names[2]);

	// init squadron wings
	m_squadron_1 = _T(Squadron_wing_names[0]);
	m_squadron_2 = _T(Squadron_wing_names[1]);
	m_squadron_3 = _T(Squadron_wing_names[2]);
	m_squadron_4 = _T(Squadron_wing_names[3]);
	m_squadron_5 = _T(Squadron_wing_names[4]);

	// init tvt wings
	m_tvt_1 = _T(TVT_wing_names[0]);
	m_tvt_2 = _T(TVT_wing_names[1]);

	CDialog::OnInitDialog();
	UpdateData(FALSE);
	return TRUE;
}

void CustomWingNames::OnOK() 
{
	UpdateData(TRUE);

	if (strcmp(m_starting_1, m_tvt_1))
	{
		MessageBox("The first starting wing and the first team-versus-team wing must have the same wing name.");
		return;
	}

	if (!stricmp(m_starting_1, m_starting_2) || !stricmp(m_starting_1, m_starting_3)
		|| !stricmp(m_starting_2, m_starting_3))
	{
		MessageBox("Duplicate wing names in starting wing list.");
		return;
	}

	if (!stricmp(m_squadron_1, m_squadron_2) || !stricmp(m_squadron_1, m_squadron_3) || !stricmp(m_squadron_1, m_squadron_4) || !stricmp(m_squadron_1, m_squadron_5)
		|| !stricmp(m_squadron_2, m_squadron_3) || !stricmp(m_squadron_2, m_squadron_4) || !stricmp(m_squadron_2, m_squadron_5)
		|| !stricmp(m_squadron_3, m_squadron_4) || !stricmp(m_squadron_3, m_squadron_5)
		|| !stricmp(m_squadron_4, m_squadron_5))
	{
		MessageBox("Duplicate wing names in squadron wing list.");
		return;
	}

	if (!stricmp(m_tvt_1, m_tvt_2))
	{
		MessageBox("Duplicate wing names in team-versus-team wing list.");
		return;
	}


	// copy starting wings
	strcpy_s(Starting_wing_names[0], m_starting_1);
	strcpy_s(Starting_wing_names[1], m_starting_2);
	strcpy_s(Starting_wing_names[2], m_starting_3);

	// copy squadron wings
	strcpy_s(Squadron_wing_names[0], m_squadron_1);
	strcpy_s(Squadron_wing_names[1], m_squadron_2);
	strcpy_s(Squadron_wing_names[2], m_squadron_3);
	strcpy_s(Squadron_wing_names[3], m_squadron_4);
	strcpy_s(Squadron_wing_names[4], m_squadron_5);

	// copy tvt wings
	strcpy_s(TVT_wing_names[0], m_tvt_1);
	strcpy_s(TVT_wing_names[1], m_tvt_2);

	update_custom_wing_indexes();

	CDialog::OnOK();
}

void CustomWingNames::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CustomWingNames::OnClose()
{
	int z;

	UpdateData(TRUE);

	if (query_modified()) {
		z = MessageBox("Do you want to keep your changes?", "Close", MB_ICONQUESTION | MB_YESNOCANCEL);
		if (z == IDCANCEL){
			return;
		}

		if (z == IDYES) {
			OnOK();
			return;
		}
	}

	CDialog::OnClose();
}

int CustomWingNames::query_modified()
{
	return strcmp(Starting_wing_names[0], m_starting_1) || strcmp(Starting_wing_names[1], m_starting_2) || strcmp(Starting_wing_names[2], m_starting_3)
		|| strcmp(Squadron_wing_names[0], m_squadron_1) || strcmp(Squadron_wing_names[1], m_squadron_2) || strcmp(Squadron_wing_names[2], m_squadron_3) || strcmp(Squadron_wing_names[3], m_squadron_4) || strcmp(Squadron_wing_names[4], m_squadron_5)
		|| strcmp(TVT_wing_names[0], m_tvt_1) || strcmp(TVT_wing_names[1], m_tvt_2);
}

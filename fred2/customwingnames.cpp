// CustomWingNames.cpp : implementation file
//

#include "stdafx.h"
#include "fred.h"
#include "CustomWingNames.h"
#include "ship/ship.h"
#include "management.h"

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
	m_tvt_1_1 = _T("");
	m_tvt_2_1 = _T("");
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
	DDX_Text(pDX, IDC_TVT_WING_NAME_1, m_tvt_1_1);
	DDX_Text(pDX, IDC_TVT_WING_NAME_2, m_tvt_2_1);
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
	if (!Starting_wing_names.empty()){
		m_starting_1 = _T(Starting_wing_names[0]);
	} else {
		m_starting_1 = "";
	}

	if (Starting_wing_names.size() > 1){
		m_starting_2 = _T(Starting_wing_names[1]);
	} else {
		m_starting_2 = "";
	}

	if (Starting_wing_names.size() > 2){
		m_starting_3 = _T(Starting_wing_names[2]);
	} else {
		m_starting_3 = "";
	}

	// init squadron wings
	if (!Squadron_wing_names.empty()){
		m_squadron_1 = _T(Squadron_wing_names[0].c_str());
	} else {
		m_squadron_1 = "";
	}

	if (Squadron_wing_names.size() > 1){
		m_squadron_2 = _T(Squadron_wing_names[1].c_str());
	} else {
		m_squadron_2 = "";
	}

	if (Squadron_wing_names.size() > 2){
		m_squadron_3 = _T(Squadron_wing_names[2].c_str());
	} else {
		m_squadron_3 = "";
	}

	if (Squadron_wing_names.size() > 3){
		m_squadron_4 = _T(Squadron_wing_names[3].c_str());
	} else {
		m_squadron_4 = "";
	}
	
	if (Squadron_wing_names.size() > 4){
		m_squadron_5 = _T(Squadron_wing_names[4].c_str());
	} else {
		m_squadron_5 = "";
	}

	// init tvt wings
	if (!TVT_wing_names[0].empty()){
		m_tvt_1_1 = _T(TVT_wing_names[0]);
	} else {
		m_tvt_1_1 = "";
	}

	if (!TVT_wing_names[1].empty()){
		m_tvt_2_1 = _T(TVT_wing_names[1]);
	} else {
		m_tvt_2_1 = "";
	}

	CDialog::OnInitDialog();
	UpdateData(FALSE);
	return TRUE;
}

void CustomWingNames::OnOK() 
{
	UpdateData(TRUE);

	strip_quotation_marks(m_starting_1);
	strip_quotation_marks(m_starting_2);
	strip_quotation_marks(m_starting_3);

	strip_quotation_marks(m_squadron_1);
	strip_quotation_marks(m_squadron_2);
	strip_quotation_marks(m_squadron_3);
	strip_quotation_marks(m_squadron_4);
	strip_quotation_marks(m_squadron_5);

	strip_quotation_marks(m_tvt_1_1);
	strip_quotation_marks(m_tvt_2_1);

	if (strcmp(m_starting_1, m_tvt_1_1))
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

	if (!stricmp(m_tvt_1_1, m_tvt_2_1))
	{
		MessageBox("Duplicate wing names in team-versus-team wing list.");
		return;
	}

	// copy starting wings
	Starting_wing_names.clear();

	if (m_starting_1 != ""){
		Starting_wing_names.emplace_back(m_starting_1);
	}

	if (m_starting_2 != ""){
		Starting_wing_names.emplace_back(m_starting_2);
	}

	if (m_starting_3 != ""){
		Starting_wing_names.emplace_back(m_starting_3);
	}

	// copy squadron wings
	Squadron_wing_names.clear();

	if (m_squadron_1 != ""){
		Squadron_wing_names.emplace_back(m_squadron_1);
	}

	if (m_squadron_2 != ""){
		Squadron_wing_names.emplace_back(m_squadron_2);
	}

	if (m_squadron_3 != ""){
		Squadron_wing_names.emplace_back(m_squadron_3);
	}

	if (m_squadron_4 != ""){
		Squadron_wing_names.emplace_back(m_squadron_4);
	}

	if (m_squadron_5 != ""){
		Squadron_wing_names.emplace_back(m_squadron_5);
	}

	// copy tvt wings
	TVT_wing_names[0].clear();
	TVT_wing_names[1].clear();

	if (m_tvt_1_1 != ""){
		TVT_wing_names[0].emplace_back(m_tvt_1_1);
	}

	if (m_tvt_2_1 != ""){
		TVT_wing_names[1].emplace_back(m_tvt_2_1);
	}

	strcpy_s(TVT_wing_names[0], m_tvt_1_1);
	strcpy_s(TVT_wing_names[1], m_tvt_2_1);

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
	// Because starting wing names are now dynamic, best to check manually and safely, and methodically for clarity.

	// First Starting wing names entry changed.
	if (!Starting_wing_names.empty() && strcmp(Starting_wing_names[0].c_str(), m_starting_1)){
		return true;
	}

	// Added a wing name to a previously empty Starting_wing_names vector.
	if (Starting_wing_names.empty() && strlen(m_starting_1)){
		return true;
	}
	
	// Second wing etc...
	if (Starting_wing_names.size() > 1 && strcmp(Starting_wing_names[1].c_str(), m_starting_2)){
		return true;
	}

	if (Starting_wing_names.size() < 2 && strlen(m_starting_2)){
		return true;
	}

	// third wing...
	if (Starting_wing_names.size() > 2 && strcmp(Starting_wing_names[2].c_str(), m_starting_3)){
		return true;
	}

	if (Starting_wing_names.size() < 3 && strlen(m_starting_2)){
		return true;
	}

	// First squadron wing changed.
	if (!Squadron_wing_names.empty() && strcmp(Squadron_wing_names[0].c_str(), m_squadron_1))
		return true;
	
	// Added a first squadron wing
	if (Squadron_wing_names.empty() && strlen(m_squadron_1))
		return true;
	
	// second Squadron wing name, etc...
	if (Squadron_wing_names.size() > 1 && strcmp(Squadron_wing_names[1].c_str(), m_squadron_2))
		return true;

	if (Squadron_wing_names.size() < 2 && strlen(m_squadron_2))
		return true;

	// third Squadron wing
	if (Squadron_wing_names.size() > 2 && strcmp(Squadron_wing_names[2].c_str(), m_squadron_3))
		return true;

	if (Squadron_wing_names.size() < 3 && strlen(m_squadron_3))
		return true;

	// Fourth Squadron Wing
	if (Squadron_wing_names.size() > 3 && strcmp(Squadron_wing_names[3].c_str(), m_squadron_4))
		return true;

	if (Squadron_wing_names.size() < 4 && strlen(m_squadron_4))
		return true;

	// Fifth Squadron wing
	if (Squadron_wing_names.size() > 4 && strcmp(Squadron_wing_names[4].c_str(), m_squadron_5))
		return true;

	if (Squadron_wing_names.size() < 5 && strlen(m_squadron_5))
		return true;


	// Team 1 Wing 1 name changed.
	if (!TVT_wing_names[0].empty() && strcmp(TVT_wing_names[0][0].c_str(), m_tvt_1_1))
		return true;
	
	// Team 1 added a wing 1 name
	if (TVT_wing_names[0].empty() && strlen(m_tvt_1_1))
		return true;

	// Team 2 Wing 1 name changed.
	if (!TVT_wing_names[1].empty() && strcmp(TVT_wing_names[1][0].c_str(), m_tvt_2_1))
		return true;
	
	// Team 2 added a wing 1 name
	if (TVT_wing_names[1].empty() && strlen(m_tvt_2_1))
		return true;
}

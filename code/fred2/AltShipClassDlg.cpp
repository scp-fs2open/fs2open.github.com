/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

// AltShipClassDlg.cpp : implementation file
//

#include "stdafx.h"
#include "fred.h"
#include "AltShipClassDlg.h"
#include "globalincs/linklist.h"
#include "ship/ship.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// AltShipClassDlg dialog


AltShipClassDlg::AltShipClassDlg(CWnd* pParent /*=NULL*/)
	: CDialog(AltShipClassDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(AltShipClassDlg)
	m_selected_variable = -1;
	m_selected_class = -1;
	//}}AFX_DATA_INIT

	multi_edit = false;
	player_ships_only = true;
	num_string_variables = 0;

	memset(string_variable_indices, -1, sizeof (int)*MAX_SEXP_VARIABLES); 
	memset(ship_class_indices, -1, sizeof (int)*MAX_SHIP_CLASSES);
}


void AltShipClassDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AltShipClassDlg)
	DDX_Control(pDX, IDC_DEFAULT_TO_CLASS, m_default_to_class);
	DDX_Control(pDX, IDC_ALT_CLASS_LIST, m_alt_class_list);
	DDX_Control(pDX, IDC_SET_FROM_SHIP_CLASS, m_set_from_ship_class);
	DDX_Control(pDX, IDC_SET_FROM_VARIABLES, m_set_from_variables);
	DDX_CBIndex(pDX, IDC_SET_FROM_VARIABLES, m_selected_variable);
	DDX_CBIndex(pDX, IDC_SET_FROM_SHIP_CLASS, m_selected_class);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(AltShipClassDlg, CDialog)
	//{{AFX_MSG_MAP(AltShipClassDlg)
	ON_BN_CLICKED(IDC_ALT_CLASS_ADD, OnAltClassAdd)
	ON_BN_CLICKED(IDC_ALT_CLASS_INSERT, OnAltClassInsert)
	ON_BN_CLICKED(IDC_ALT_CLASS_DELETE, OnAltClassDelete)
	ON_BN_CLICKED(IDC_ALT_CLASS_UP, OnMoveUp)
	ON_BN_CLICKED(IDC_ALT_CLASS_DOWN, OnMoveDown)
	ON_CBN_SELENDOK(IDC_SET_FROM_SHIP_CLASS, OnSelendokSetFromShipClass)
	ON_CBN_SELENDOK(IDC_SET_FROM_VARIABLES, OnSelendokSetFromVariables)
	ON_LBN_SELCHANGE(IDC_ALT_CLASS_LIST, OnSelchangeAltClassList)
	ON_BN_CLICKED(IDC_DEFAULT_TO_CLASS, OnDefaultToClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AltShipClassDlg message handlers


BOOL AltShipClassDlg::OnInitDialog() 
{
	int i, count;
	char buff[TOKEN_LENGTH + TOKEN_LENGTH + 2];  // VariableName[VariableValue]

	CDialog::OnInitDialog();

	// have we got multiple selected ships?
	num_selected_ships = 0;
	object *objp;

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
			if (objp->flags & OF_MARKED) {
				m_selected_ships[num_selected_ships++] = objp->instance;
			}
		}
		objp = GET_NEXT(objp);
	}

	Assert (num_selected_ships > 0);
	Assert (Objects[cur_object_index].flags & OF_MARKED);

	if (num_selected_ships > 1) 
	{
		multi_edit = true;
	}

	// Fill the variable combo box	
	m_set_from_variables.ResetContent();
	m_set_from_variables.AddString("Set From Ship Class");
	for (i=0; i < MAX_SEXP_VARIABLES; i++) 
	{
		if (Sexp_variables[i].type & SEXP_VARIABLE_STRING) 
		{
			sprintf(buff, "%s[%s]", Sexp_variables[i].variable_name, Sexp_variables[i].text);
			m_set_from_variables.AddString(buff);
			string_variable_indices[num_string_variables++] = i;
		}
	}
	m_set_from_variables.SetCurSel(0);
	if (!num_string_variables) {
		m_set_from_variables.EnableWindow(FALSE);
	}
	
	// Fill the ship classes combo box
	m_set_from_ship_class.ResetContent();
	// Add the default entry if we need one followed by all the ship classes
	if (num_string_variables) {
		m_set_from_ship_class.AddString("Set From Variable");
	}
	count = 0; 
	for (i=0; i<Num_ship_classes; i++)
	{
		if (player_ships_only && !(Ship_info[i].flags & SIF_PLAYER_SHIP)) {
			continue;
		}

		ship_class_indices[count++] = i;
		m_set_from_ship_class.AddString(Ship_info[i].name);
	}
	m_set_from_ship_class.SetCurSel(num_string_variables?1:0); // Set to the first ship class

	// Set up the actual list of alt classes
	alt_class_pool.clear();	
	alt_class_pool = Ships[Objects[cur_object_index].instance].s_alt_classes;
	alt_class_list_rebuild(); 
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void AltShipClassDlg::alt_class_list_rebuild()
{	
	int i, j;
	char buff[TOKEN_LENGTH + TOKEN_LENGTH + 2];  // VariableName[VariableValue]

	j = m_alt_class_list.GetCount();
	for ( i=0; i < j ; i++) {
		m_alt_class_list.DeleteString(0); 
	}

	for (i = 0; i < (int)alt_class_pool.size() ; i++) {
		// Default classes should be at the end of the list
		if(alt_class_pool[i].default_to_this_class && (i < (int)alt_class_pool.size() - 1) ) {
			// Int3();
			// TO DO : we should fix this.
		}

		if (alt_class_pool[i].variable_index != -1) {
			Assert (alt_class_pool[i].variable_index > -1 && alt_class_pool[i].variable_index < MAX_SEXP_VARIABLES);
			Assert (Sexp_variables[alt_class_pool[i].variable_index].type & SEXP_VARIABLE_STRING); 
			
			sprintf(buff, "%s[%s]", Sexp_variables[alt_class_pool[i].variable_index].variable_name, Sexp_variables[alt_class_pool[i].variable_index].text);			

			// add it to the list
			m_alt_class_list.AddString(buff); 
		}
		else {
			if (alt_class_pool[i].ship_class >= 0 && alt_class_pool[i].ship_class < MAX_SHIP_CLASSES) {
				m_alt_class_list.AddString(Ship_info[alt_class_pool[i].ship_class].name);
			}
			else {
				m_alt_class_list.AddString("Invalid Ship Class");
				Int3();
			}
		}		
	}
}

void AltShipClassDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void AltShipClassDlg::OnOK() 
{
	// TODO: Add extra validation here

	for (int i=0; i < num_selected_ships; i++) {
		Ships[m_selected_ships[i]].s_alt_classes = alt_class_pool;
	}
	
	CDialog::OnOK();
}

void AltShipClassDlg::OnAltClassAdd() 
{
	alt_class new_list_item; 

	alt_class_update_entry(new_list_item); 

	alt_class_pool.push_back(new_list_item); 
	alt_class_list_rebuild(); 
}

void AltShipClassDlg::OnAltClassInsert() 
{
	int index; 
	alt_class new_list_item; 
	alt_class_update_entry(new_list_item); 

	index = m_alt_class_list.GetCurSel(); 

	// If nothing is selected just add it
	if ( index == -1) {
		alt_class_pool.push_back(new_list_item); 
	}
	// Stick it in front of the current selection
	else {
		alt_class_pool.insert(alt_class_pool.begin() + index, new_list_item );
	}

	alt_class_list_rebuild(); 
	
	// If we inserted the user will probably want to edit the selection
	if (index > -1) {
		m_alt_class_list.SetCurSel(index); 
	}
}

// Updates the currently selected list entry
void AltShipClassDlg::alt_class_update_entry(alt_class &list_item)
{
	int index; 
	
	// Add a string variable to the list
	if (num_string_variables && m_set_from_variables.GetCurSel() > 0) {
		index = string_variable_indices[m_set_from_variables.GetCurSel() -1]; 

		Assert (index >= 0); 
		list_item.variable_index = index; 
		list_item.ship_class = ship_info_lookup(Sexp_variables[index].text); 
	}
	// Add a ship class to the list
	else {
		index = m_set_from_ship_class.GetCurSel(); 

		// Correct the index if the first entry isn't actually a ship class
		if (num_string_variables) {
			Assert (index > 0);
			index--;
		}
		
		list_item.variable_index = -1;
		list_item.ship_class = ship_class_indices[index];		
	}
	
	// check the default tickbox
	list_item.default_to_this_class = m_default_to_class.GetCheck() ? true:false;
}

void AltShipClassDlg::OnAltClassDelete() 
{
	int index = m_alt_class_list.GetCurSel();
	
	// Nothing selected
	if (index == -1) {
		return;
	}

	Assert (index < (int) alt_class_pool.size());
	alt_class_pool.erase(alt_class_pool.begin()+index);  
	alt_class_list_rebuild(); 
}

void AltShipClassDlg::OnMoveUp() 
{
	int index = m_alt_class_list.GetCurSel();
	
	// Nothing selected or already at the top
	if (index < 1) {
		return;
	}

	std::swap(alt_class_pool[index], alt_class_pool[index-1]);
	alt_class_list_rebuild(); 
	m_alt_class_list.SetCurSel(index-1); 
}

void AltShipClassDlg::OnMoveDown() 
{
	int index = m_alt_class_list.GetCurSel();
	int inext = index + 1;
	
	// Nothing selected or already at the bottom
	if (index == -1 || (index >= m_alt_class_list.GetCount() - 1)  ) {
		return;
	}

	std::swap(alt_class_pool[index], alt_class_pool[inext]);
	alt_class_list_rebuild(); 
	m_alt_class_list.SetCurSel(index); 
}

void AltShipClassDlg::OnSelendokSetFromShipClass() 
{
	int current_selection; 

	if (m_set_from_ship_class.GetCurSel() == 0 && num_string_variables) {
		m_set_from_variables.SetCurSel(1);
	}
	else {
		m_set_from_variables.SetCurSel(0); 
	}

	// if a list entry is selected we should update it
	current_selection = m_alt_class_list.GetCurSel(); 
	if (current_selection >= 0) {
		alt_class_update_entry(alt_class_pool[m_alt_class_list.GetCurSel()]); 
		alt_class_list_rebuild(); 
	}
}

void AltShipClassDlg::OnSelendokSetFromVariables() 
{
	int current_selection; 

	if (m_set_from_variables.GetCurSel() == 0) {
		m_set_from_ship_class.SetCurSel(num_string_variables ? 1 : 0);
	}
	else {
		m_set_from_ship_class.SetCurSel(0); 
	}

	// if a list entry is selected we should update it
	current_selection = m_alt_class_list.GetCurSel(); 
	if (current_selection >= 0) {
		alt_class_update_entry(alt_class_pool[m_alt_class_list.GetCurSel()]); 
		alt_class_list_rebuild(); 
	}
}

void AltShipClassDlg::OnSelchangeAltClassList() 
{
	int i;
	int variable_selection = 0;
	int ship_selection = 0; 
	int index = m_alt_class_list.GetCurSel();
	
	// Selected nothing
	if (index == -1) {
		return;
	}

	// If we have a variable selected
	if (alt_class_pool[index].variable_index != -1) {
		for (i=0; i < MAX_SEXP_VARIABLES; i++) 
		{
			if (string_variable_indices[i] == alt_class_pool[index].variable_index) {
				variable_selection = i+1; 
				break;
			}
		}
	}
	// Ship selected
	else {
		for (i=0; i < MAX_SHIP_CLASSES; i++) {
			if (ship_class_indices[i] == alt_class_pool[index].ship_class) {
				ship_selection = i; 
				break;
			}
		}

		// if we have string variables in the mission the first entry on the list won't be a ship
		if (num_string_variables) {
			ship_selection++; 
		}
	}
	
	m_set_from_variables.SetCurSel(variable_selection); 
	m_set_from_ship_class.SetCurSel(ship_selection);
	m_default_to_class.SetCheck(alt_class_pool[index].default_to_this_class ? 1:0);
}

void AltShipClassDlg::OnDefaultToClass() 
{
	int index = m_alt_class_list.GetCurSel();
	
	// Nothing selected
	if (index == -1) {
		return;
	}

	alt_class_pool[index].default_to_this_class = m_default_to_class.GetCheck() ? true:false; 
}




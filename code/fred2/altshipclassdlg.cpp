// AltShipClassDlg.cpp : implementation file
//

#include "stdafx.h"
#include "fred.h"
#include "AltShipClassDlg.h"
#include "globalincs/linklist.h"
#include "FREDDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// AltShipClassDlg dialog


AltShipClassDlg::AltShipClassDlg(CWnd* pParent /*=NULL*/)
	: CDialog(AltShipClassDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(AltShipClassDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	multi_edit = false;

	for (int i=0; i < MAX_ALT_CLASS_1; i++) 
	{
		type1_radio_button_selection[i] = 0;
	}
}


void AltShipClassDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AltShipClassDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(AltShipClassDlg, CDialog)
	//{{AFX_MSG_MAP(AltShipClassDlg)
	ON_BN_CLICKED(IDC_ASCT1_CLASS_RADIO1, OnASCT1ClassRadio)
	ON_BN_CLICKED(IDC_ASCT1_VARIABLES_RADIO1, OnASCT1VariablesRadio)
	ON_BN_CLICKED(IDC_ASCT1_CLASS_RADIO2, OnAsct1ClassRadio2)
	ON_BN_CLICKED(IDC_ASCT1_CLASS_RADIO3, OnAsct1ClassRadio3)
	ON_BN_CLICKED(IDC_ASCT1_VARIABLES_RADIO2, OnAsct1VariablesRadio2)
	ON_BN_CLICKED(IDC_ASCT1_VARIABLES_RADIO3, OnAsct1VariablesRadio3)
	ON_BN_CLICKED(IDC_ASCT2_CLASS_RADIO1, OnAsct2ClassRadio1)
	ON_BN_CLICKED(IDC_ASCT2_VARIABLES_RADIO1, OnAsct2VariablesRadio1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AltShipClassDlg message handlers

BOOL AltShipClassDlg::OnInitDialog() 
{
	int i;

	CDialog::OnInitDialog();

	// Set up pointers to all the controls to make things easier later
	type1_class_selections[0] = (CComboBox *) GetDlgItem(IDC_ASCT1_CLASS_COMBO1);
	type1_class_selections[1] = (CComboBox *) GetDlgItem(IDC_ASCT1_CLASS_COMBO2);
	type1_class_selections[2] = (CComboBox *) GetDlgItem(IDC_ASCT1_CLASS_COMBO3);

	type1_variable_selections[0] = (CComboBox *) GetDlgItem(IDC_ASCT1_VARIABLES_COMBO1);
	type1_variable_selections[1] = (CComboBox *) GetDlgItem(IDC_ASCT1_VARIABLES_COMBO2);
	type1_variable_selections[2] = (CComboBox *) GetDlgItem(IDC_ASCT1_VARIABLES_COMBO3);

	type1_class_radio_button[0] = IDC_ASCT1_CLASS_RADIO1; 
	type1_class_radio_button[1] = IDC_ASCT1_CLASS_RADIO2;
	type1_class_radio_button[2] = IDC_ASCT1_CLASS_RADIO3; 

	type1_variable_radio_button[0] = IDC_ASCT1_VARIABLES_RADIO1; 
	type1_variable_radio_button[1] = IDC_ASCT1_VARIABLES_RADIO2;
	type1_variable_radio_button[2] = IDC_ASCT1_VARIABLES_RADIO3; 

	type2_class_selections[0] = (CComboBox *) GetDlgItem(IDC_ASCT2_CLASS_COMBO1);

	type2_variable_selections[0] = (CComboBox *) GetDlgItem(IDC_ASCT2_VARIABLES_COMBO1);

	type2_class_radio_button[0] = IDC_ASCT2_CLASS_RADIO1; 

	type2_variable_radio_button[0] = IDC_ASCT2_VARIABLES_RADIO1; 

	// get selected ships
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
	if (num_selected_ships > 1) 
	{
		multi_edit = true;
	}

	// Set up the ship and variable combo boxes. 

	// Type 1
	for (i=0; i < MAX_ALT_CLASS_1; i++)
	{
		SetupType1ComboBoxes(type1_class_selections[i], type1_variable_selections[i], i);
		// Setup the type 1 radio buttons
		
		switch (type1_radio_button_selection[i])
		{
			case 0:
				CheckDlgButton(type1_class_radio_button[i], 1); 
				break;
			case 1:
				CheckDlgButton(type1_variable_radio_button[i], 1); 
				break;
			default:
				CheckDlgButton(type1_class_radio_button[i], 1); 
		}
	}

	// Type 2
	for (i=0; i < MAX_ALT_CLASS_2; i++)
	{
		SetupType2ComboBoxes(type2_class_selections[i], type2_variable_selections[i], i);
		// Setup the type 2 radio buttons
		
		switch (type2_radio_button_selection[i])
		{
			case 0:
				CheckDlgButton(type2_class_radio_button[i], 1); 
				break;
			case 1:
				CheckDlgButton(type2_variable_radio_button[i], 1); 
				break;
			default:
				CheckDlgButton(type2_class_radio_button[i], 1); 
		}
	}
	DisableComponents();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Fills the combo box with the list of ships and an entry to select no ship. 
void AltShipClassDlg::FillClassComboBox(CComboBox *ptr)
{
	ptr->ResetContent();
	// Add the default entry followed by all the ship classes
	ptr->AddString("None");
	for (int i=0; i<Num_ship_classes; i++)
	{
		ptr->AddString(Ship_info[i].name);
	}
}

// Returns the position in the variables combo box that a supplied string variable will have
int AltShipClassDlg::GetStringVariableIndex(int alt_variable_index)
{
	// If it's -1 then this wasn't set by a sexp_variable in the first place
	if (alt_variable_index != -1)
	{
		// Loop through Sexp_variables until we have found the one corresponding to the argument
		int count = 0;
		for (int i=0; i < MAX_SEXP_VARIABLES; i++)
		{
			if (Sexp_variables[i].type & SEXP_VARIABLE_STRING) 
			{
				if (i == alt_variable_index)
				{
					return count + ALT_SHIP_CLASS_COMBO_OFFSET; 
				}

				count++;
			}
		}
		// Shouldn't ever get here
		Assert (false);
		return 0; 
	}
	
	return alt_variable_index + ALT_SHIP_CLASS_COMBO_OFFSET;
}

// Returns the Sexp_variables index of the selected item
int AltShipClassDlg::GetVariableIndexFromControl(CComboBox *variable_ptr)
{
	char selection[TOKEN_LENGTH]; 
	int result = variable_ptr->GetLBText(variable_ptr->GetCurSel(), selection); 
	if (!(strcmp(selection, "None")) || (result == CB_ERR))
	{
		return -1;
	}
	return get_index_sexp_variable_name(selection);
}

// Fills the combo box with the list of string variables and entry to select none 
void AltShipClassDlg::FillVariableComboBox(CComboBox *ptr)
{
	ptr->ResetContent();
	ptr->AddString("None");
	for (int k=0; k < MAX_SEXP_VARIABLES; k++) 
	{
		if (Sexp_variables[k].type & SEXP_VARIABLE_STRING) 
		{
			ptr->AddString(Sexp_variables[k].variable_name);
		}
	}
}

void AltShipClassDlg::SetupType1ComboBoxes(CComboBox *class_ptr, CComboBox *variable_ptr, int alt_class_index)
{
	FillClassComboBox(class_ptr);
	FillVariableComboBox(variable_ptr);

	// Get the data from the first ship 

	bool all_the_same = true;
	int first_alt_class = Ships[m_selected_ships[0]].alt_class_one[alt_class_index];
	int first_alt_variable = Ships[m_selected_ships[0]].alt_class_one_variable[alt_class_index];

	// If we have multiple ships selected we'll need to check if they have the same alt class before we display anything
	if (multi_edit) 
	{
		// Start at 1 not 0 as we have already read in the data for the first ship
		for (int j=1; j < num_selected_ships; j++)
		{
			if (first_alt_class != Ships[m_selected_ships[j]].alt_class_one[alt_class_index] || 
				first_alt_variable != Ships[m_selected_ships[j]].alt_class_one_variable[alt_class_index])
			{
				all_the_same = false;
				break;
			}
		}
	}
	
	// If all the selected ships have the same entry already or we only have one ship we set the combo boxes to 
	// display the selected value. Otherwise we set nothing and leave it showing no entry. 
	if (all_the_same) 
	{
		if (first_alt_variable > -1)
		{
			// Set the Radio Button to select the variable combo
			type1_radio_button_selection[alt_class_index] = 1; 
		}
		else 
		{
			// Set the Radio Button to select the class combo
			type1_radio_button_selection[alt_class_index] = 0; 
		}
		
		class_ptr->SetCurSel( first_alt_class + ALT_SHIP_CLASS_COMBO_OFFSET); // Offset because we added the "none" entry 

		variable_ptr->SetCurSel(GetStringVariableIndex(first_alt_variable)); 
	}
}

void AltShipClassDlg::SetupType2ComboBoxes(CComboBox *class_ptr, CComboBox *variable_ptr, int alt_class_index)
{
	FillClassComboBox(class_ptr);
	FillVariableComboBox(variable_ptr);

	// Get the data from the first ship 

	bool all_the_same = true;
	int first_alt_class = Ships[m_selected_ships[0]].alt_class_two[alt_class_index];
	int first_alt_variable = Ships[m_selected_ships[0]].alt_class_two_variable[alt_class_index];

	// If we have multiple ships selected we'll need to check if they have the same alt class before we display anything
	if (multi_edit) 
	{
		// Start at 1 not 0 as we have already read in the data for the first ship
		for (int j=1; j < num_selected_ships; j++)
		{
			if (first_alt_class != Ships[m_selected_ships[j]].alt_class_two[alt_class_index] || 
				first_alt_variable != Ships[m_selected_ships[j]].alt_class_two_variable[alt_class_index])
			{
				all_the_same = false;
				break;
			}
		}
	}
	
	// If all the selected ships have the same entry already or we only have one ship we set the combo boxes to 
	// display the selected value. Otherwise we set nothing and leave it showing no entry. 
	if (all_the_same) 
	{
		if (first_alt_variable > -1)
		{
			// Set the Radio Button to select the variable combo
			type2_radio_button_selection[alt_class_index] = 1; 
		}
		else 
		{
			// Set the Radio Button to select the class combo
			type2_radio_button_selection[alt_class_index] = 0; 
		}
		
		class_ptr->SetCurSel( first_alt_class + ALT_SHIP_CLASS_COMBO_OFFSET); // Offset because we added the "none" entry 

		variable_ptr->SetCurSel(GetStringVariableIndex(first_alt_variable)); 
	}
}

void AltShipClassDlg::DisableComponents()
{
	int i;
	for (i=0; i < MAX_ALT_CLASS_1; i++)
	{
		if (type1_radio_button_selection[i])
		{			
			type1_class_selections[i]->EnableWindow(FALSE);
			type1_variable_selections[i]->EnableWindow(TRUE);
		}
		else
		{		
			type1_class_selections[i]->EnableWindow(TRUE);
			type1_variable_selections[i]->EnableWindow(FALSE);
		}
	}
	for (i=0; i < MAX_ALT_CLASS_2; i++)
	{
		if (type2_radio_button_selection[i])
		{			
			type2_class_selections[i]->EnableWindow(FALSE);
			type2_variable_selections[i]->EnableWindow(TRUE);
		}
		else
		{		
			type2_class_selections[i]->EnableWindow(TRUE);
			type2_variable_selections[i]->EnableWindow(FALSE);
		}
	}
}

void AltShipClassDlg::OnCancel() 
{
	// Don't need any extra cleanup here yet
	
	CDialog::OnCancel();
}

void AltShipClassDlg::WriteType1DataToShip(CComboBox *class_ptr, CComboBox *variable_ptr, int alt_class_index)
{
	// Ship Class
	if (type1_radio_button_selection[alt_class_index] == 0)
	{
		int selected_ship_class = class_ptr->GetCurSel() - ALT_SHIP_CLASS_COMBO_OFFSET;
		for (int i=0; i < num_selected_ships; i++)
		{
			MODIFY (Ships[m_selected_ships[i]].alt_class_one[alt_class_index], selected_ship_class);
			MODIFY (Ships[m_selected_ships[i]].alt_class_one_variable[alt_class_index], -1); 
		}
	}
	// Variable
	else 
	{
		int selected_ship_index = GetVariableIndexFromControl(variable_ptr);
		
		for (int i=0; i < num_selected_ships; i++)
		{
			MODIFY (Ships[m_selected_ships[i]].alt_class_one[alt_class_index], -1); 
			MODIFY (Ships[m_selected_ships[i]].alt_class_one_variable[alt_class_index], selected_ship_index);
		}
	}	
}

void AltShipClassDlg::WriteType2DataToShip(CComboBox *class_ptr, CComboBox *variable_ptr, int alt_class_index)
{
	// Ship Class
	if (type2_radio_button_selection[alt_class_index] == 0)
	{
		int selected_ship_class = class_ptr->GetCurSel() - ALT_SHIP_CLASS_COMBO_OFFSET;
		for (int i=0; i < num_selected_ships; i++)
		{
			MODIFY (Ships[m_selected_ships[i]].alt_class_two[alt_class_index], selected_ship_class);
			MODIFY (Ships[m_selected_ships[i]].alt_class_two_variable[alt_class_index], -1); 
		}
	}
	// Variable
	else 
	{
		int selected_ship_index = GetVariableIndexFromControl(variable_ptr);
		
		for (int i=0; i < num_selected_ships; i++)
		{
			MODIFY (Ships[m_selected_ships[i]].alt_class_two[alt_class_index], -1); 
			MODIFY (Ships[m_selected_ships[i]].alt_class_two_variable[alt_class_index], selected_ship_index);
		}
	}	
}


void AltShipClassDlg::OnOK() 
{	
	int i;

	UpdateData(TRUE);
	
	// Read the selection from the ship and variable combo boxes. 	
	for (i=0; i < MAX_ALT_CLASS_1; i++)
	{
		WriteType1DataToShip(type1_class_selections[i], type1_variable_selections[i], i);
	}

	for (i=0; i < MAX_ALT_CLASS_2; i++)
	{
		WriteType2DataToShip(type2_class_selections[i], type2_variable_selections[i], i);
	}
	
	CDialog::OnOK();
}


void AltShipClassDlg::OnASCT1ClassRadio() 
{
	type1_radio_button_selection[0] = 0;
	DisableComponents();
}

void AltShipClassDlg::OnAsct1ClassRadio2() 
{
	type1_radio_button_selection[1] = 0;
	DisableComponents();
}

void AltShipClassDlg::OnAsct1ClassRadio3() 
{
	type1_radio_button_selection[2] = 0;
	DisableComponents();
}

void AltShipClassDlg::OnASCT1VariablesRadio() 
{
	type1_radio_button_selection[0] = 1;
	DisableComponents();
}

void AltShipClassDlg::OnAsct1VariablesRadio2() 
{
	type1_radio_button_selection[1] = 1;
	DisableComponents();
}

void AltShipClassDlg::OnAsct1VariablesRadio3() 
{
	type1_radio_button_selection[2] = 1;
	DisableComponents();
}

void AltShipClassDlg::OnAsct2ClassRadio1() 
{
	type2_radio_button_selection[0] = 0;
	DisableComponents();
}

void AltShipClassDlg::OnAsct2VariablesRadio1() 
{
	type2_radio_button_selection[0] = 1;
	DisableComponents();
}

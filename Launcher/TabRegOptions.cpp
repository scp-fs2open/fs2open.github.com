// TabRegOptions.cpp : implementation file
//

#include "stdafx.h"
#include "Launcher.h"
#include "TabRegOptions.h"
#include "win32func.h"
#include "settings.h"
#include "tabcommline.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum
{
	REG_TYPE_SZ,
	REG_TYPE_DWORD,
	REG_TYPE_UNSUPPORTED,
	MAX_REG_TYPES
};

char *string_reps[MAX_REG_TYPES] =
{
	"String", 
	"DWORD", 
	"N\\A"
};
						   
/////////////////////////////////////////////////////////////////////////////
// CTabRegOptions dialog


CTabRegOptions::CTabRegOptions(CWnd* pParent /*=NULL*/)
	: CDialog(CTabRegOptions::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabRegOptions)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTabRegOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabRegOptions)
	DDX_Control(pDX, IDC_REG_LIST, m_reg_option_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabRegOptions, CDialog)
	//{{AFX_MSG_MAP(CTabRegOptions)
	ON_BN_CLICKED(IDC_SET, OnSet)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_REG_LIST, OnItemchangedRegList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabRegOptions message handlers

/**
 *
 */
BOOL CTabRegOptions::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_reg_option_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_reg_option_list.InsertColumn(0, "Name", LVCFMT_LEFT, 125);
	m_reg_option_list.InsertColumn(1, "Type", LVCFMT_LEFT, 60);
	m_reg_option_list.InsertColumn(2, "Data", LVCFMT_LEFT, 129);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/**
 *
 */
void CTabRegOptions::InsertItem(char *name, int type, void *data, DWORD size)
{
	// Insert name and get new value, may differ from requested index value
	int new_value = m_reg_option_list.InsertItem(m_list_count, name, 0);

	// Buffer for REG_TYPE_DWORD (have to declare now because of use in switch)
	char *buffer = NULL;

	// Insert type
	LVITEM item;

	item.iItem    = new_value;
	item.iSubItem = 1;
	item.mask	  = LVIF_TEXT;
	item.pszText  = string_reps[type];

	if(m_reg_option_list.SetItem(&item) == FALSE)
	{
		MessageBox("Failed to set item");
		return;
	}

	item.iSubItem = 2;

	if(data)
	{
		switch(type)
		{
			case REG_TYPE_SZ:
				item.pszText = (char *) data;
				m_reg_option_list.SetItem(&item);
				break;
			case REG_TYPE_DWORD:
				buffer = new char[size+2];
				if (buffer == NULL)
				{
					break;
				}
				// Restrict the data copy to passed "size" rather than the actual size of "buffer"
				_snprintf(buffer, size, "%d", *(DWORD *) data);
				item.pszText = buffer;
				m_reg_option_list.SetItem(&item);
				delete[] buffer;
				break;
			case REG_TYPE_UNSUPPORTED: break;
		}
	}

	m_reg_option_list.SetItemData(new_value, type);
}

/**
 *
 */
void CTabRegOptions::FillRegList()
{
	int count = 0;
	LONG result;

	m_reg_option_list.DeleteAllItems();

	if(Settings::exe_type == EXE_TYPE_NONE)
	{
		GetDlgItem(IDC_REG_LOCATION)->SetWindowText("No valid game exe chosen");
		return;
	}

	if(strlen(Settings::reg_path) == 0)
	{
		return;
	}

	HKEY hkey = reg_open_dir(Settings::reg_path);

	// Try to open the correct registry dir
	if(hkey == NULL)
	{
		return;
	}

	GetDlgItem(IDC_REG_LOCATION)->SetWindowText(Settings::reg_path);

	// Now find and display all the items at this path
	do
	{
		unsigned long subkey_size = MAX_PATH;
		char subkey_name[MAX_PATH];
		DWORD subkey_type;
		DWORD data_len = 100;
		BYTE *pdata = NULL;

		{ // Cheap way to determine size of data space needed, data_len is updated with the actual size of the value
			result = RegEnumValue(
						hkey,
						count,
						subkey_name,
						&subkey_size,
						0, // reserved
						&subkey_type,
						NULL,
						&data_len);

			if (result == ERROR_NO_MORE_ITEMS)
			{
				break;
			}

			pdata = new BYTE[data_len+2];

			if (pdata == NULL)
			{
				MessageBox("An error has occured reading from the registry; cancelling...");
				break;
			}
		} // end cheap data space check

		// account for the NULL char, otherwise we lose the last character of the key name
		subkey_size++;

		result = RegEnumValue(
				  	hkey, 
					count, 
					subkey_name, 
					&subkey_size,
					0, // reserved
					&subkey_type,
					pdata,
					&data_len);

		if(result == ERROR_NO_MORE_ITEMS)
		{
			delete[] pdata;
			break;
		}

		if(result != ERROR_SUCCESS && result != ERROR_NO_MORE_ITEMS && result != ERROR_MORE_DATA)
		{
			delete[] pdata;
			MessageBox("An error has occured reading from the registry; cancelling...");
			break;
		}

		int reg_type = REG_TYPE_UNSUPPORTED;

		switch(subkey_type)
		{
			case REG_DWORD:	reg_type = REG_TYPE_DWORD; break;
			case REG_SZ:	reg_type = REG_TYPE_SZ; break;
		}

		InsertItem(subkey_name, reg_type, (void *) pdata, data_len);
		delete[] pdata;
		count++;

	} while(1);

	// Close this path now we have all the info we need
	RegCloseKey(hkey);

	// Nothing is selected so this should be blank
	GetDlgItem(IDC_NEW_VALUE)->SetWindowText("");
}

/**
 *
 */
void CTabRegOptions::OnSet() 
{
	// Check the paths are valid
	if(strlen(Settings::reg_path) == 0)
	{
		MessageBox("No reg path for this exe");
		FillRegList();
		return;
	}

	// Check a valid item is selected
	int selection = (int) m_reg_option_list.GetFirstSelectedItemPosition() - 1;

	if(selection == -1)
	{
		MessageBox("No entry chosen");
		return;
	}

	CString new_value, name = m_reg_option_list.GetItemText(selection, 0);

	// Get the new value
	GetDlgItem(IDC_NEW_VALUE)->GetWindowText(new_value); 

	const int data_type = m_reg_option_list.GetItemData(selection);
	bool result;

	switch(data_type)
	{
		case REG_TYPE_SZ:
		{
			result = reg_set_sz(Settings::reg_path, name, (LPCTSTR) new_value); 
			break;
		}
		case REG_TYPE_DWORD:
		{
			DWORD number = atoi( (LPCTSTR) new_value );
			result = reg_set_dword(Settings::reg_path, name, number);
			break;
		}
		default:
		case REG_TYPE_UNSUPPORTED:
			MessageBox("Viewing or changing this registry type is not supported");
			return;
	}

	if(result == false)
	{
		MessageBox("Failed to set reg value");
	}

	// Lets update the list the slow way and ensure everything is how it should be
	FillRegList();
}

/**
 *
 */
void CTabRegOptions::OnItemchangedRegList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	// Get the name and type and data
	POSITION selection = m_reg_option_list.GetFirstSelectedItemPosition();
	CString data = m_reg_option_list.GetItemText((int) selection - 1, 2);

	GetDlgItem(IDC_NEW_VALUE)->SetWindowText(data);

	*pResult = 0;
}

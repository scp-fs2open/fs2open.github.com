// ShipTexturesDlg.cpp : implementation file
// Goober5000

#include "stdafx.h"
#include "fred.h"
#include "ShipTexturesDlg.h"
#include "model/model.h"
#include "bmpman/bmpman.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShipTexturesDlg dialog


CShipTexturesDlg::CShipTexturesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShipTexturesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShipTexturesDlg)
	m_new_texture = _T("");
	m_old_texture_list = 0;
	//}}AFX_DATA_INIT

	self_ship = -1;
	active_texture_index = -1;
	modified = 0;
	texture_count = 0;
}


void CShipTexturesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CShipTexturesDlg)
	DDX_Text(pDX, IDC_NEW_TEXTURE, m_new_texture);
	DDX_CBIndex(pDX, IDC_OLD_TEXTURE_LIST, m_old_texture_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CShipTexturesDlg, CDialog)
	//{{AFX_MSG_MAP(CShipTexturesDlg)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_OLD_TEXTURE_LIST, OnSelchangeOldTextureList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShipTexturesDlg message handlers

void CShipTexturesDlg::OnOK() 
{
	int i, z, not_found, temp_bmp, temp_frames, temp_fps;
	CString missing_files, message;
	char buf[10];

	// update, in case of a last-minute edit
	OnSelchangeOldTextureList();

	// quick skip if nothing modified
	if (query_modified())
	{
		// sort according to new
		sort_textures(SORT_NEW);

		// check for filenames not found
		not_found = 0;
		missing_files = _T("");
		for (i=0; i<texture_count; i++)
		{
			// make sure we have a texture
			if (strlen(new_texture_name[i]))
			{
				// allow invisible textures without doing a file check
				if (!stricmp(new_texture_name[i], "invisible"))
					continue;

				// try loading the texture (bmpman should take care of eventually unloading it)
				temp_bmp = bm_load( new_texture_name[i] );

				// if PCX not found, look for ANI
				if (temp_bmp < 0)
				{					
					temp_bmp = bm_load_animation(new_texture_name[i],  &temp_frames, &temp_fps, NULL, 1);
				}

				// check if loaded
				if (temp_bmp < 0)
				{
					not_found++;
					missing_files += "   ";
					missing_files += new_texture_name[i];
					missing_files += '\n';
				}
			}
		}

		// alert user if any textures were not found
		if (not_found)
		{
			sprintf(buf, "%d", not_found);
			message = "FRED was unable to find ";
			message += buf;
			message += ((not_found > 1) ? " files:\n" : " file:\n");
			message += missing_files;
			message += "\nContinue anyway?";

			z = MessageBox(message, ((not_found > 1) ? "Some textures were not found." : "A texture was not found."), MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2);
			if (z == IDCANCEL)
			{
				return;
			}
		}

		// re-sort according to old
		sort_textures();

		// overwrite all the old entries that refer to this ship
		SCP_vector<texture_replace>::iterator ii, end;
		end = Fred_texture_replacements.end();
		for (ii = Fred_texture_replacements.begin(); ii != end; ++ii)
		{
			if (!stricmp(ii->ship_name, Ships[self_ship].ship_name))
			{
				end--;
				if (end == ii)
					break;
				texture_set(&(*ii), &(*end));
			}
		}
		if (Fred_texture_replacements.size() > 0)
			Fred_texture_replacements.erase(end);

		// now put the new entries on the end of the list
		for (i=0; i<texture_count; i++)
		{
			// make sure there is an entry
			if (strlen(new_texture_name[i]))
			{
				texture_replace tr;

				strcpy_s(tr.old_texture, old_texture_name[i]);
				strcpy_s(tr.new_texture, new_texture_name[i]);
				strcpy_s(tr.ship_name, Ships[self_ship].ship_name);
				tr.new_texture_id = -1;

				// assign to global FRED array
				Fred_texture_replacements.push_back(tr);
			}
		}
	}	// skipped here if nothing modified

	CDialog::OnOK();
}

BOOL CShipTexturesDlg::OnInitDialog() 
{	
	int i, j, k, z, duplicate;
	char *p = NULL;
	char texture_file[MAX_FILENAME_LEN];
	CComboBox *box;

	// get our model
	polymodel *pm = model_get(Ship_info[Ships[self_ship].ship_info_index].model_num);

	// empty old and new fields
	texture_count = 0;
	for (i=0; i<MAX_REPLACEMENT_TEXTURES; i++)
	{
		*old_texture_name[i] = 0;
		*new_texture_name[i] = 0;
	}

	// set up pointer to combo box
	box = (CComboBox *) GetDlgItem(IDC_OLD_TEXTURE_LIST);
	box->ResetContent();

	// look for textures to populate the combo box
	for (i=0; i<pm->n_textures; i++)
	{
		for(j = 0; j < TM_NUM_TYPES; j++)
		{
			// get texture file name
			bm_get_filename(pm->maps[i].textures[j].GetOriginalTexture(), texture_file);

			// skip blank textures
			if (!strlen(texture_file))
				continue;

			// get rid of file extension
			p = strchr( texture_file, '.' );
			if ( p )
			{
				//mprintf(( "ignoring extension on file '%s'\n", texture_file ));
				*p = 0;
			}

			// check for duplicate textures in list
			duplicate = -1;
			for (k=0; k<texture_count; k++)
			{
				if (!stricmp(old_texture_name[k], texture_file))
				{
					duplicate = k;
					break;
				}
			}

			if (duplicate >= 0)
				continue;

			// make old texture lowercase
			strlwr(texture_file);

			// now add it to the box
			z = box->AddString(texture_file);

			// and add it to the field as well
			strcpy_s(old_texture_name[texture_count], texture_file);

			// increment
			texture_count++;

			// sort
			sort_textures();
		}
	}

	// now look for new textures
	for (SCP_vector<texture_replace>::iterator ii = Fred_texture_replacements.begin(); ii != Fred_texture_replacements.end(); ++ii)
	{
		if (!stricmp(Ships[self_ship].ship_name, ii->ship_name))
		{
			// look for corresponding old texture
			for (i=0; i<texture_count; i++)
			{
				// if match
				if (!stricmp(old_texture_name[i], ii->old_texture))
				{
					// assign new texture
					strcpy_s(new_texture_name[i], ii->new_texture);

					// we found one, so no more to check
					break;
				}
			}
		}
	}
	// end of new texture check

	// set indexes and flags
	m_old_texture_list = 0;
	active_texture_index = 0;
	modified = 0;

	// display new texture, if we have one
	m_new_texture = CString(new_texture_name[0]);

	CDialog::OnInitDialog();
	UpdateData(FALSE);
 
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CShipTexturesDlg::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{	
	return CDialog::Create(IDD, pParentWnd);
}

void CShipTexturesDlg::OnClose() 
{
	int z;

	if (query_modified()) {
		z = MessageBox("Do you want to keep your changes?", "Close", MB_ICONQUESTION | MB_YESNOCANCEL);
		if (z == IDCANCEL)
			return;

		if (z == IDYES) {
			OnOK();
			return;
		}
	}
	
	CDialog::OnClose();
}

int CShipTexturesDlg::query_modified()
{
	return modified;
}

void CShipTexturesDlg::OnSelchangeOldTextureList() 
{
	UpdateData(TRUE);

	char *p;

	// see if we edited anything
	if (stricmp(new_texture_name[active_texture_index], m_new_texture))
	{
		// assign it
		strcpy_s(new_texture_name[active_texture_index], m_new_texture);
	
		// make it lowercase
		strlwr(new_texture_name[active_texture_index]);

		// get rid of file extension
		p = strchr( new_texture_name[active_texture_index], '.' );
		if ( p )
		{
			mprintf(( "ignoring extension on file '%s'\n", new_texture_name[active_texture_index] ));
			*p = 0;
		}

		// set modified flag
		modified = 1;
	}

	// bring active texture index up to date
	active_texture_index = m_old_texture_list;

	// display appropriate texture
	m_new_texture = CString(new_texture_name[active_texture_index]);

	UpdateData(FALSE);
}

// bubble sort
void CShipTexturesDlg::sort_textures(int test)
{
	int i, j, str_check = 0;

	for (i = 0; i < texture_count; i++)
	{
		for (j = 0; j < i; j++)
		{
			switch(test)
			{
				case SORT_OLD:
					str_check = stricmp(old_texture_name[i], old_texture_name[j]);
					break;
				case SORT_NEW:
					str_check = stricmp(new_texture_name[i], new_texture_name[j]);
					break;
				default:
					Int3();
			}

			if (str_check < 0)
			{
				// swap old
				swap_strings(old_texture_name[i], old_texture_name[j]);

				// swap new
				swap_strings(new_texture_name[i], new_texture_name[j]);
			}
		}
	}
}

void CShipTexturesDlg::swap_strings(char *str1, char *str2)
{
/*
	char *temp;
	temp = str1;
	str1 = str2;
	str2 = temp;
*/

	char temp[256];
	strcpy_s(temp, str1);
	strcpy(str1, str2);
	strcpy(str2, temp);
}

texture_replace *CShipTexturesDlg::texture_set(texture_replace *dest, const texture_replace *src)
{
	dest->new_texture_id = src->new_texture_id;
	strcpy_s(dest->ship_name, src->ship_name);
	strcpy_s(dest->old_texture, src->old_texture);
	strcpy_s(dest->new_texture, src->new_texture);

	return dest;
}

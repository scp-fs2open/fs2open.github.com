// DX8Disp.cpp : implementation file
//
#include "stdafx.h"

#include <d3d8.h>
#include <D3d8types.h>

#include "launcher.h"
#include "tabcommline.h"
#include "DX8Disp.h"

#include "win32func.h"
#include "dbugfile.h"
#include "settings.h"

typedef struct
{
	int cdepth;
	char text[10];
} CDepth_DX;

CDepth_DX color_depthDX[2];

// Declared here to avoid having to include DX8 files in the header file
IDirect3D8* d3d_interface = NULL;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDX8Disp dialog


CDX8Disp::CDX8Disp(CWnd* pParent /*=NULL*/)
	: CDialog(CDX8Disp::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDX8Disp)
	//}}AFX_DATA_INIT

	m_dx8_initialised_ok = false;
}


void CDX8Disp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDX8Disp)
	DDX_Control(pDX, IDC_ADAPTER_LIST, m_adapter_list);
	DDX_Control(pDX, IDC_RES_LIST, m_res_list);
	DDX_Control(pDX, IDC_CDEPTH_LIST, m_cdepth_list);
	DDX_Control(pDX, IDC_ANTIALIAS_LIST, m_antialias_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDX8Disp, CDialog)
	//{{AFX_MSG_MAP(CDX8Disp)
	ON_CBN_SELCHANGE(IDC_RES_LIST, OnSelchangeResList)
	ON_CBN_SELCHANGE(IDC_CDEPTH_LIST, OnChangeCDepth)
	ON_CBN_SELCHANGE(IDC_ANTIALIAS_LIST, OnSelchangeAntialiasList)
	ON_CBN_SELCHANGE(IDC_ADAPTER_LIST, OnSelchangeAdapterList)
	ON_BN_CLICKED(IDC_GEN_CAPS, OnGenCaps)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDX8Disp message handlers

void CDX8Disp::OnSelchangeAdapterList() 
{
	UpdateResList();
}

void CDX8Disp::OnSelchangeResList() 
{
	UpdateAntialiasList();
}

void CDX8Disp::OnSelchangeAntialiasList() 
{
}

BOOL CDX8Disp::OnInitDialog() 
{
	CDialog::OnInitDialog();

	d3d_interface = Direct3DCreate8( D3D_SDK_VERSION );

	// User does not have DX8 installed disable all functionality
   	if( d3d_interface == NULL ) 
	{
		GetDlgItem(IDC_DX8_NOT_INSTALLED)->ShowWindow(TRUE);
		GetDlgItem(IDC_GEN_CAPS)->ShowWindow(FALSE);
		GetDlgItem(IDC_ADAPTER_LIST)->ShowWindow(FALSE);
		GetDlgItem(IDC_RES_LIST)->ShowWindow(FALSE);
		GetDlgItem(IDC_CDEPTH_LIST)->ShowWindow(FALSE);
		GetDlgItem(IDC_ANTIALIAS_LIST)->ShowWindow(FALSE);
		MessageBox("You do not have DX8.1 installed; cannot offer D3D8 options", "Error", MB_ICONERROR);
		return TRUE;
	}

	color_depthDX[0].cdepth = 16;
	strcpy(color_depthDX[0].text, "16-bit");
	color_depthDX[1].cdepth = 32;
	strcpy(color_depthDX[1].text, "32-bit");

	m_dx8_initialised_ok = true;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDX8Disp::UpdateAdapterList(int select)
{
	DBUGFILE_OUTPUT_1("Requesting adapter %d", select);

	if (d3d_interface == NULL)
		return;

	// set color depth defaults
	m_cdepth_list.ResetContent();
	m_cdepth_list.AddString(color_depthDX[0].text);
	m_cdepth_list.AddString(color_depthDX[1].text);

	m_adapter_list.ResetContent();

	D3DADAPTER_IDENTIFIER8 identifier;
	int num_adapters = d3d_interface->GetAdapterCount();

	if(num_adapters == 0)
	{
		MessageBox("Fatal error, no adapters!", "Error", MB_ICONERROR);
		return;
	}

	for(int i = 0; i < num_adapters; i++)
	{
		if(FAILED(d3d_interface->GetAdapterIdentifier(i, D3DENUM_NO_WHQL_LEVEL, &identifier)))
		{
			MessageBox("Failed GetAdapterIdentifier in UpdateAdapterList; trying with WHQL_LEVEL", "Error", MB_ICONWARNING);
			if(FAILED(d3d_interface->GetAdapterIdentifier(i, 0, &identifier)))
			{
				MessageBox("Failed again, cannot get adapter", "Error", MB_ICONERROR);
				return;
			}

		}
		m_adapter_list.AddString(identifier.Description);
	}

	if(select >= num_adapters)
	{
		select = 0;
	}

	m_adapter_list.SetCurSel(select);
	UpdateResList();
}

int CDX8Disp::GetCDepth(int cdepth)
{
	int idx = m_cdepth_list.GetCurSel();

	if (cdepth == -1) {
		if (idx < 0) {
			m_cdepth_list.SetCurSel(0);
			idx = 0;
		}

		return color_depthDX[idx].cdepth;
	}

	for (idx = 0; idx < 2; idx++) {
		if (cdepth == color_depthDX[idx].cdepth) {
			m_cdepth_list.SetCurSel(idx);
			return color_depthDX[idx].cdepth;
		}
	}

	return cdepth;
}

/**
 * Return bit type for modes, those not listed or simply not valid for FS2
 *
 * @return int, 32, 16 or 0 if not valid 
 * @param D3DFORMAT type 
 */
int d3d8_get_mode_bit(D3DFORMAT type)
{
	switch(type)
	{
		case D3DFMT_X8R8G8B8: 
		case D3DFMT_A8R8G8B8:		
		case D3DFMT_A2B10G10R10:	
			return 32;
			
		case D3DFMT_R8G8B8:
		case D3DFMT_R5G6B5:   
		case D3DFMT_X1R5G5B5: 
		case D3DFMT_X4R4G4B4:
		case D3DFMT_A1R5G5B5:		
		case D3DFMT_A4R4G4B4:		
		case D3DFMT_A8R3G3B2:		
			return 16;
	}

	return 0;
}

void CDX8Disp::UpdateResList(
	unsigned int requested_width, unsigned int requested_height, int requested_cdepth)
{
	int selected_sel = -1;
	bool standard_match = TRUE;

 	m_res_list.ResetContent();

 	if (d3d_interface == NULL)
		return;

	requested_cdepth = GetCDepth(requested_cdepth);

	int selected_adapter  = m_adapter_list.GetCurSel();
	int num_modes		  = d3d_interface->GetAdapterModeCount(selected_adapter);

	int index = -1;
	char mode_string[20] = "";

	// Only list different resolutions and bit type NOT different refresh rates
	int index_count = 0;
	for(int i = 0; i < num_modes; i++)
	{
		D3DDISPLAYMODE mode;
		if(FAILED(d3d_interface->EnumAdapterModes(selected_adapter, i, &mode)))
		{
			MessageBox("Failed EnumAdapterModes", "Error", MB_ICONERROR);
			return;
		}

		if (d3d8_get_mode_bit(mode.Format) != requested_cdepth)
			continue;

		// skip anything below the minimum
		if(	!(mode.Width == 640 && mode.Height == 480) &&
			(mode.Width < 800 || mode.Height < 600))
		{
		  	continue;
		}

/*
		// Lets look ahead and get the lowest refresh rate
		// Can't assume there will be at least 60 or 0
		while(i > 0)
		{
			D3DDISPLAYMODE next_mode;
			if(FAILED(d3d_interface->EnumAdapterModes(selected_adapter, i-1, &next_mode)))
			{
				MessageBox("Failed EnumAdapterModes", "Fatal Error", MB_ICONERROR);
				return;
			}

			// Modes are the same execpt refresh rate
			if( mode.Width == next_mode.Width && 
				mode.Height == next_mode.Height && 
				d3d8_get_mode_bit(mode.Format) == d3d8_get_mode_bit(next_mode.Format)) 
			{
				DBUGFILE_OUTPUT_2("Changing stored mode from %d to %d", i, i-1);
				i--;
			}
			else
			{
				DBUGFILE_OUTPUT_0("Not the same, next!");
				break;
			}
		}
		*/
/*
		char new_string[20];
		sprintf(new_string, "%dx%dx%d", mode.Width, mode.Height, d3d8_get_mode_bit(mode.Format));
		
		if(stricmp(new_string, mode_string) == 0)
		{
			continue;
		}
		strcpy(mode_string,new_string); 
		*/

		// If not the first mode
		if(index > -1)
		{
			bool ignore_this_mode = false;
			int count_back = index;

			while(count_back > -1)
			{
				int back_mode_index = m_res_list.GetItemData(count_back);

				D3DDISPLAYMODE last_mode;
				if(FAILED(d3d_interface->EnumAdapterModes(selected_adapter, back_mode_index, &last_mode)))
				{
					MessageBox("Failed EnumAdapterModes", "Fatal Error", MB_ICONERROR);
					return;
				}

				// If we already have this mode ignore it
				if( mode.Width == last_mode.Width && 
					mode.Height == last_mode.Height && 
					d3d8_get_mode_bit(mode.Format) == d3d8_get_mode_bit(last_mode.Format))
				{
					ignore_this_mode = true;
					break;
				}

				count_back--;
			}

			if(ignore_this_mode == true)
			{
				continue;
			}
		}

		sprintf(mode_string, "%d x %d", mode.Width, mode.Height);
		index = m_res_list.InsertString(index_count, mode_string);
		m_res_list.SetItemData(index, i);

		if( requested_width  == mode.Width &&
			requested_height == mode.Height &&
			requested_cdepth == d3d8_get_mode_bit(mode.Format))

		{
	   		selected_sel = index_count;

			// identify whether this is one of the standard video modes or not
			standard_match = ((mode.Width == 1024 && mode.Height == 768) ||
									(mode.Width == 640 && mode.Height == 480));
		}
				
	//	char buffer[100];
	//	sprintf(buffer,"refresh %d", mode.RefreshRate);
	//	MessageBox(buffer, "DEBUG", MB_OK);

		index_count++;
	}

	if (selected_sel < 0)
		selected_sel = 0;

	// show or hide the non-standard video mode text if needed
	GetDlgItem(IDC_NSVM_TEXT)->ShowWindow( (standard_match) ? SW_HIDE : SW_SHOW );

	m_res_list.SetCurSel(selected_sel);
 	UpdateAntialiasList();
}

void CDX8Disp::UpdateAntialiasList(int select)
{
 	m_antialias_list.ResetContent();
	if(d3d_interface == NULL) 
	{
		DBUGFILE_OUTPUT_0("d3d_interface == NULL");
		return;
	}

	int selected_adapter  = m_adapter_list.GetCurSel();
	int selected_mode	  = m_res_list.GetItemData(m_res_list.GetCurSel());

	D3DDISPLAYMODE mode;
	if(FAILED(d3d_interface->EnumAdapterModes(selected_adapter, selected_mode, &mode))) 
	{
		MessageBox("Failed EnumAdapterModes in UpdateAntialiasList", "Fatal Error", MB_ICONERROR);
		return;
	}

	{
		// do a quick standard res check while we're here...

		// identify whether this is one of the standard video modes or not
		bool standard_match = ((mode.Width == 1024 && mode.Height == 768) ||
							(mode.Width == 640 && mode.Height == 480));

		// show or hide the non-standard video mode text if needed
		GetDlgItem(IDC_NSVM_TEXT)->ShowWindow( (standard_match) ? SW_HIDE : SW_SHOW );
	}


	// Now fill the avaliable anti aliasing  for this mode
	typedef struct {
		int   type;
		char *text;
	} MultisampleInfo;

	const int MAX_MULTISAMPLE = 16;
	const MultisampleInfo multisample_info[MAX_MULTISAMPLE] =
	{
 		D3DMULTISAMPLE_NONE,     "D3DMULTISAMPLE_NONE",
		D3DMULTISAMPLE_2_SAMPLES,"D3DMULTISAMPLE_2_SAMPLES",
		D3DMULTISAMPLE_3_SAMPLES,"D3DMULTISAMPLE_3_SAMPLES",
		D3DMULTISAMPLE_4_SAMPLES,"D3DMULTISAMPLE_4_SAMPLES",
		D3DMULTISAMPLE_5_SAMPLES,"D3DMULTISAMPLE_5_SAMPLES",
		D3DMULTISAMPLE_6_SAMPLES,"D3DMULTISAMPLE_6_SAMPLES",
		D3DMULTISAMPLE_7_SAMPLES,"D3DMULTISAMPLE_7_SAMPLES",
		D3DMULTISAMPLE_8_SAMPLES,"D3DMULTISAMPLE_8_SAMPLES",
		D3DMULTISAMPLE_9_SAMPLES,"D3DMULTISAMPLE_9_SAMPLES",
		D3DMULTISAMPLE_10_SAMPLES,"D3DMULTISAMPLE_10_SAMPLES",
		D3DMULTISAMPLE_11_SAMPLES,"D3DMULTISAMPLE_11_SAMPLES",
		D3DMULTISAMPLE_12_SAMPLES,"D3DMULTISAMPLE_12_SAMPLES",
		D3DMULTISAMPLE_13_SAMPLES,"D3DMULTISAMPLE_13_SAMPLES",
		D3DMULTISAMPLE_14_SAMPLES,"D3DMULTISAMPLE_14_SAMPLES",
		D3DMULTISAMPLE_15_SAMPLES,"D3DMULTISAMPLE_15_SAMPLES",
		D3DMULTISAMPLE_16_SAMPLES,"D3DMULTISAMPLE_16_SAMPLES"
	};


	for(int i = 0; i < MAX_MULTISAMPLE; i++) 
	{		
		if( FAILED( d3d_interface->CheckDeviceMultiSampleType( 
			selected_adapter, 
			D3DDEVTYPE_HAL , 
			mode.Format, FALSE, 
			(D3DMULTISAMPLE_TYPE) multisample_info[i].type ) ) ) {
			 break;
		}

		m_antialias_list.InsertString(i, multisample_info[i].text);
	}

	if(select > MAX_MULTISAMPLE)
		select = 0;


	m_antialias_list.SetCurSel(select);
}

void CDX8Disp::OnChangeCDepth()
{
	int cur_pos = m_res_list.GetCurSel();

	if (cur_pos == CB_ERR)
		return;

	int selected_adapter = m_adapter_list.GetCurSel();

	int offset = m_res_list.GetItemData(cur_pos);

	if (offset < 0)
		offset = 0;

	D3DDISPLAYMODE mode;

	if ( SUCCEEDED(d3d_interface->EnumAdapterModes(selected_adapter, offset, &mode)) )
		UpdateResList(mode.Width, mode.Height);
}

#include <Dxerr8.h>

/**
 * Generate textfile version of D3D8 caps
 * Allow user to save it where they want then open it
 */
void CDX8Disp::OnGenCaps() 
{
	if(d3d_interface == NULL) return;

	int selected_adapter  = m_adapter_list.GetCurSel();
	char mass_buffer[50000];
	int  i = 0;

	// OS
	char os_string[100];
  	os_get_type_text(os_string);
 	i += sprintf(mass_buffer + i, "Operating System: %s\n", os_string); 
 	i += sprintf(mass_buffer + i, "Memory: %d MB\n", memory_get_ammount() / 1048576);


	// Adapter data
	D3DADAPTER_IDENTIFIER8 identifier;
	if(FAILED(d3d_interface->GetAdapterIdentifier(selected_adapter, D3DENUM_NO_WHQL_LEVEL, &identifier)))
	{
		MessageBox("Failed GetAdapterIdentifier in OnGenCaps", "Fatal Error", MB_ICONERROR);
		return;
	}

	// Lets get the all mode data
	int num_modes = d3d_interface->GetAdapterModeCount(selected_adapter);

	i += sprintf(mass_buffer + i, "Adapter: %s\n\n", identifier.Description); 
	i += sprintf(mass_buffer + i, "Modes:\n\n"); 
	for(int j = num_modes - 1; j >= 0; j--)
	{
		D3DDISPLAYMODE mode;
		if(FAILED(d3d_interface->EnumAdapterModes(selected_adapter, j, &mode))) 
		{
			MessageBox("Failed EnumAdapterModes in OnGenCaps", "Fatal Error", MB_ICONERROR);
			return;
		}

		i += sprintf(mass_buffer + i, 
			"%d: (%dx%d)x%d bit, %d rr\n",
			j, mode.Width, mode.Height, 
			d3d8_get_mode_bit(mode.Format),
			mode.RefreshRate);
	}

	// Now output the caps
	D3DCAPS8 caps;
	HRESULT hr = d3d_interface->GetDeviceCaps(selected_adapter, D3DDEVTYPE_HAL, &caps);

	// Don't quit, continue on; sometimes it doesn't matter!
   	if(FAILED(hr))
	{
		char buffer[100];
		sprintf(buffer, "Failed to generate caps, please report to coder");
		MessageBox(buffer, "Error", MB_ICONERROR);
	}
	else
	{
		char *temp;

// Internal MACRO, only for use in this function
#define SPRINTF_FLAG(cap, flag) \
		temp = #cap "." #flag ":"; \
		i += sprintf(mass_buffer + i, "%-63s %s\n", temp, (cap & flag) ? "yes" : "no");

// Internal MACRO, only for use in this function
#define SPRINTF_NL() i += sprintf(mass_buffer + i, "\n")

		// This outputs all the DX8 caps to text
		i += sprintf(mass_buffer + i, "\n\nD3D8 Caps:\n\n"); 
		SPRINTF_FLAG(caps.Caps, D3DCAPS_READ_SCANLINE);
		SPRINTF_NL();
		SPRINTF_FLAG(caps.Caps2, D3DCAPS2_CANCALIBRATEGAMMA); 
		SPRINTF_FLAG(caps.Caps2, D3DCAPS2_CANRENDERWINDOWED); 
		SPRINTF_FLAG(caps.Caps2, D3DCAPS2_CANMANAGERESOURCE); 
		SPRINTF_FLAG(caps.Caps2, D3DCAPS2_DYNAMICTEXTURES); 
		SPRINTF_FLAG(caps.Caps2, D3DCAPS2_FULLSCREENGAMMA); 
   		SPRINTF_FLAG(caps.Caps2, D3DCAPS2_NO2DDURING3DSCENE);
		SPRINTF_NL();
		SPRINTF_FLAG(caps.Caps3, D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD);
		SPRINTF_NL();
		SPRINTF_FLAG(caps.PresentationIntervals, D3DPRESENT_INTERVAL_IMMEDIATE); 
		SPRINTF_FLAG(caps.PresentationIntervals, D3DPRESENT_INTERVAL_ONE); 
		SPRINTF_FLAG(caps.PresentationIntervals, D3DPRESENT_INTERVAL_TWO); 
		SPRINTF_FLAG(caps.PresentationIntervals, D3DPRESENT_INTERVAL_THREE); 
		SPRINTF_FLAG(caps.PresentationIntervals, D3DPRESENT_INTERVAL_FOUR); 
		SPRINTF_NL();
		SPRINTF_FLAG(caps.CursorCaps, D3DCURSORCAPS_COLOR); 
		SPRINTF_FLAG(caps.CursorCaps, D3DCURSORCAPS_LOWRES); 
		SPRINTF_NL();
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_CANBLTSYSTONONLOCAL); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_CANRENDERAFTERFLIP); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_DRAWPRIMTLVERTEX); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_EXECUTESYSTEMMEMORY); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_EXECUTEVIDEOMEMORY); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_HWRASTERIZATION); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_HWTRANSFORMANDLIGHT); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_NPATCHES); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_PUREDEVICE); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_QUINTICRTPATCHES); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_RTPATCHES); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_RTPATCHHANDLEZERO); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_SEPARATETEXTUREMEMORIES); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_TEXTURENONLOCALVIDMEM); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_TEXTURESYSTEMMEMORY); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_TEXTUREVIDEOMEMORY); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_TLVERTEXSYSTEMMEMORY); 
		SPRINTF_FLAG(caps.DevCaps, D3DDEVCAPS_TLVERTEXVIDEOMEMORY); 
		SPRINTF_NL();
		SPRINTF_FLAG(caps.PrimitiveMiscCaps, D3DPMISCCAPS_BLENDOP);  
		SPRINTF_FLAG(caps.PrimitiveMiscCaps, D3DPMISCCAPS_CLIPPLANESCALEDPOINTS);  
		SPRINTF_FLAG(caps.PrimitiveMiscCaps, D3DPMISCCAPS_CLIPTLVERTS);  
		SPRINTF_FLAG(caps.PrimitiveMiscCaps, D3DPMISCCAPS_COLORWRITEENABLE);  
		SPRINTF_FLAG(caps.PrimitiveMiscCaps, D3DPMISCCAPS_CULLCCW);  
		SPRINTF_FLAG(caps.PrimitiveMiscCaps, D3DPMISCCAPS_CULLCW);  
		SPRINTF_FLAG(caps.PrimitiveMiscCaps, D3DPMISCCAPS_CULLNONE);  
		SPRINTF_FLAG(caps.PrimitiveMiscCaps, D3DPMISCCAPS_LINEPATTERNREP);  
		SPRINTF_FLAG(caps.PrimitiveMiscCaps, D3DPMISCCAPS_MASKZ);  
		SPRINTF_FLAG(caps.PrimitiveMiscCaps, D3DPMISCCAPS_TSSARGTEMP); 
		SPRINTF_NL();
		SPRINTF_FLAG(caps.RasterCaps, D3DPRASTERCAPS_ANISOTROPY); 
		SPRINTF_FLAG(caps.RasterCaps, D3DPRASTERCAPS_ANTIALIASEDGES); 
		SPRINTF_FLAG(caps.RasterCaps, D3DPRASTERCAPS_COLORPERSPECTIVE); 
		SPRINTF_FLAG(caps.RasterCaps, D3DPRASTERCAPS_DITHER); 
		SPRINTF_FLAG(caps.RasterCaps, D3DPRASTERCAPS_FOGRANGE); 
		SPRINTF_FLAG(caps.RasterCaps, D3DPRASTERCAPS_FOGTABLE); 
		SPRINTF_FLAG(caps.RasterCaps, D3DPRASTERCAPS_FOGVERTEX); 
		SPRINTF_FLAG(caps.RasterCaps, D3DPRASTERCAPS_MIPMAPLODBIAS); 
		SPRINTF_FLAG(caps.RasterCaps, D3DPRASTERCAPS_PAT); 
		SPRINTF_FLAG(caps.RasterCaps, D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE); 
		SPRINTF_FLAG(caps.RasterCaps, D3DPRASTERCAPS_WBUFFER); 
		SPRINTF_FLAG(caps.RasterCaps, D3DPRASTERCAPS_WFOG); 
		SPRINTF_FLAG(caps.RasterCaps, D3DPRASTERCAPS_ZBIAS); 
		SPRINTF_FLAG(caps.RasterCaps, D3DPRASTERCAPS_ZBUFFERLESSHSR); 
		SPRINTF_FLAG(caps.RasterCaps, D3DPRASTERCAPS_ZFOG); 
		SPRINTF_FLAG(caps.RasterCaps, D3DPRASTERCAPS_ZTEST); 
		SPRINTF_NL();
		SPRINTF_FLAG(caps.ZCmpCaps, D3DPCMPCAPS_ALWAYS); 
		SPRINTF_FLAG(caps.ZCmpCaps, D3DPCMPCAPS_EQUAL); 
		SPRINTF_FLAG(caps.ZCmpCaps, D3DPCMPCAPS_GREATER); 
		SPRINTF_FLAG(caps.ZCmpCaps, D3DPCMPCAPS_GREATEREQUAL); 
		SPRINTF_FLAG(caps.ZCmpCaps, D3DPCMPCAPS_LESS); 
		SPRINTF_FLAG(caps.ZCmpCaps, D3DPCMPCAPS_LESSEQUAL); 
		SPRINTF_FLAG(caps.ZCmpCaps, D3DPCMPCAPS_NEVER); 
		SPRINTF_FLAG(caps.ZCmpCaps, D3DPCMPCAPS_NOTEQUAL); 
		SPRINTF_NL();
		SPRINTF_FLAG(caps.SrcBlendCaps,D3DPBLENDCAPS_BOTHINVSRCALPHA); 
		SPRINTF_FLAG(caps.SrcBlendCaps,D3DPBLENDCAPS_BOTHSRCALPHA); 
		SPRINTF_FLAG(caps.SrcBlendCaps,D3DPBLENDCAPS_DESTALPHA); 
		SPRINTF_FLAG(caps.SrcBlendCaps,D3DPBLENDCAPS_DESTCOLOR); 
		SPRINTF_FLAG(caps.SrcBlendCaps,D3DPBLENDCAPS_INVDESTALPHA); 
		SPRINTF_FLAG(caps.SrcBlendCaps,D3DPBLENDCAPS_INVDESTCOLOR); 
		SPRINTF_FLAG(caps.SrcBlendCaps,D3DPBLENDCAPS_INVSRCALPHA); 
		SPRINTF_FLAG(caps.SrcBlendCaps,D3DPBLENDCAPS_INVSRCCOLOR); 
		SPRINTF_FLAG(caps.SrcBlendCaps,D3DPBLENDCAPS_ONE); 
		SPRINTF_FLAG(caps.SrcBlendCaps,D3DPBLENDCAPS_SRCALPHA); 
		SPRINTF_FLAG(caps.SrcBlendCaps,D3DPBLENDCAPS_SRCALPHASAT); 
		SPRINTF_FLAG(caps.SrcBlendCaps,D3DPBLENDCAPS_SRCCOLOR); 
		SPRINTF_FLAG(caps.SrcBlendCaps,D3DPBLENDCAPS_ZERO); 
		SPRINTF_NL();
		SPRINTF_FLAG(caps.DestBlendCaps,D3DPBLENDCAPS_BOTHINVSRCALPHA);  
		SPRINTF_FLAG(caps.DestBlendCaps,D3DPBLENDCAPS_BOTHSRCALPHA);     
		SPRINTF_FLAG(caps.DestBlendCaps,D3DPBLENDCAPS_DESTALPHA);        
		SPRINTF_FLAG(caps.DestBlendCaps,D3DPBLENDCAPS_DESTCOLOR);        
		SPRINTF_FLAG(caps.DestBlendCaps,D3DPBLENDCAPS_INVDESTALPHA);     
		SPRINTF_FLAG(caps.DestBlendCaps,D3DPBLENDCAPS_INVDESTCOLOR);     
		SPRINTF_FLAG(caps.DestBlendCaps,D3DPBLENDCAPS_INVSRCALPHA);      
		SPRINTF_FLAG(caps.DestBlendCaps,D3DPBLENDCAPS_INVSRCCOLOR);      
		SPRINTF_FLAG(caps.DestBlendCaps,D3DPBLENDCAPS_ONE);              
		SPRINTF_FLAG(caps.DestBlendCaps,D3DPBLENDCAPS_SRCALPHA);         
		SPRINTF_FLAG(caps.DestBlendCaps,D3DPBLENDCAPS_SRCALPHASAT);      
		SPRINTF_FLAG(caps.DestBlendCaps,D3DPBLENDCAPS_SRCCOLOR);         
		SPRINTF_FLAG(caps.DestBlendCaps,D3DPBLENDCAPS_ZERO);             
		SPRINTF_NL();
		SPRINTF_FLAG(caps.AlphaCmpCaps, D3DPCMPCAPS_ALWAYS);        
		SPRINTF_FLAG(caps.AlphaCmpCaps, D3DPCMPCAPS_EQUAL);         
		SPRINTF_FLAG(caps.AlphaCmpCaps, D3DPCMPCAPS_GREATER);       
		SPRINTF_FLAG(caps.AlphaCmpCaps, D3DPCMPCAPS_GREATEREQUAL);  
		SPRINTF_FLAG(caps.AlphaCmpCaps, D3DPCMPCAPS_LESS);          
		SPRINTF_FLAG(caps.AlphaCmpCaps, D3DPCMPCAPS_LESSEQUAL);     
		SPRINTF_FLAG(caps.AlphaCmpCaps, D3DPCMPCAPS_NEVER);         
		SPRINTF_FLAG(caps.AlphaCmpCaps, D3DPCMPCAPS_NOTEQUAL);      
		SPRINTF_NL();
		SPRINTF_FLAG(caps.ShadeCaps,D3DPSHADECAPS_ALPHAGOURAUDBLEND); 
		SPRINTF_FLAG(caps.ShadeCaps,D3DPSHADECAPS_COLORGOURAUDRGB); 
		SPRINTF_FLAG(caps.ShadeCaps,D3DPSHADECAPS_FOGGOURAUD); 
		SPRINTF_FLAG(caps.ShadeCaps,D3DPSHADECAPS_SPECULARGOURAUDRGB); 
		SPRINTF_NL();
		SPRINTF_FLAG(caps.TextureCaps, D3DPTEXTURECAPS_ALPHA); 
		SPRINTF_FLAG(caps.TextureCaps, D3DPTEXTURECAPS_ALPHAPALETTE); 
		SPRINTF_FLAG(caps.TextureCaps, D3DPTEXTURECAPS_CUBEMAP); 
		SPRINTF_FLAG(caps.TextureCaps, D3DPTEXTURECAPS_CUBEMAP_POW2); 
		SPRINTF_FLAG(caps.TextureCaps, D3DPTEXTURECAPS_MIPCUBEMAP); 
		SPRINTF_FLAG(caps.TextureCaps, D3DPTEXTURECAPS_MIPMAP); 
		SPRINTF_FLAG(caps.TextureCaps, D3DPTEXTURECAPS_MIPVOLUMEMAP); 
		SPRINTF_FLAG(caps.TextureCaps, D3DPTEXTURECAPS_NONPOW2CONDITIONAL); 
		SPRINTF_FLAG(caps.TextureCaps, D3DPTEXTURECAPS_PERSPECTIVE); 
		SPRINTF_FLAG(caps.TextureCaps, D3DPTEXTURECAPS_POW2); 
		SPRINTF_FLAG(caps.TextureCaps, D3DPTEXTURECAPS_PROJECTED); 
		SPRINTF_FLAG(caps.TextureCaps, D3DPTEXTURECAPS_SQUAREONLY); 
		SPRINTF_FLAG(caps.TextureCaps, D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE); 
		SPRINTF_FLAG(caps.TextureCaps, D3DPTEXTURECAPS_VOLUMEMAP); 
		SPRINTF_FLAG(caps.TextureCaps, D3DPTEXTURECAPS_VOLUMEMAP_POW2); 
		SPRINTF_NL();
		SPRINTF_FLAG(caps.TextureFilterCaps, D3DPTFILTERCAPS_MAGFAFLATCUBIC); 
		SPRINTF_FLAG(caps.TextureFilterCaps, D3DPTFILTERCAPS_MAGFANISOTROPIC); 
		SPRINTF_FLAG(caps.TextureFilterCaps, D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC); 
		SPRINTF_FLAG(caps.TextureFilterCaps, D3DPTFILTERCAPS_MAGFLINEAR); 
		SPRINTF_FLAG(caps.TextureFilterCaps, D3DPTFILTERCAPS_MAGFPOINT); 
		SPRINTF_FLAG(caps.TextureFilterCaps, D3DPTFILTERCAPS_MINFANISOTROPIC); 
		SPRINTF_FLAG(caps.TextureFilterCaps, D3DPTFILTERCAPS_MINFLINEAR); 
		SPRINTF_FLAG(caps.TextureFilterCaps, D3DPTFILTERCAPS_MINFPOINT); 
		SPRINTF_FLAG(caps.TextureFilterCaps, D3DPTFILTERCAPS_MIPFLINEAR); 
		SPRINTF_FLAG(caps.TextureFilterCaps, D3DPTFILTERCAPS_MIPFPOINT); 
		SPRINTF_NL();
		SPRINTF_FLAG(caps.CubeTextureFilterCaps, D3DPTFILTERCAPS_MAGFAFLATCUBIC);   
		SPRINTF_FLAG(caps.CubeTextureFilterCaps, D3DPTFILTERCAPS_MAGFANISOTROPIC);  
		SPRINTF_FLAG(caps.CubeTextureFilterCaps, D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC);
		SPRINTF_FLAG(caps.CubeTextureFilterCaps, D3DPTFILTERCAPS_MAGFLINEAR);       
		SPRINTF_FLAG(caps.CubeTextureFilterCaps, D3DPTFILTERCAPS_MAGFPOINT);        
		SPRINTF_FLAG(caps.CubeTextureFilterCaps, D3DPTFILTERCAPS_MINFANISOTROPIC);  
		SPRINTF_FLAG(caps.CubeTextureFilterCaps, D3DPTFILTERCAPS_MINFLINEAR);       
		SPRINTF_FLAG(caps.CubeTextureFilterCaps, D3DPTFILTERCAPS_MINFPOINT);        
		SPRINTF_FLAG(caps.CubeTextureFilterCaps, D3DPTFILTERCAPS_MIPFLINEAR);       
		SPRINTF_FLAG(caps.CubeTextureFilterCaps, D3DPTFILTERCAPS_MIPFPOINT);        
		SPRINTF_NL();
		SPRINTF_FLAG(caps.VolumeTextureFilterCaps, D3DPTFILTERCAPS_MAGFAFLATCUBIC);   
		SPRINTF_FLAG(caps.VolumeTextureFilterCaps, D3DPTFILTERCAPS_MAGFANISOTROPIC);  
		SPRINTF_FLAG(caps.VolumeTextureFilterCaps, D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC);
		SPRINTF_FLAG(caps.VolumeTextureFilterCaps, D3DPTFILTERCAPS_MAGFLINEAR);       
		SPRINTF_FLAG(caps.VolumeTextureFilterCaps, D3DPTFILTERCAPS_MAGFPOINT);        
		SPRINTF_FLAG(caps.VolumeTextureFilterCaps, D3DPTFILTERCAPS_MINFANISOTROPIC);  
		SPRINTF_FLAG(caps.VolumeTextureFilterCaps, D3DPTFILTERCAPS_MINFLINEAR);       
		SPRINTF_FLAG(caps.VolumeTextureFilterCaps, D3DPTFILTERCAPS_MINFPOINT);        
		SPRINTF_FLAG(caps.VolumeTextureFilterCaps, D3DPTFILTERCAPS_MIPFLINEAR);       
		SPRINTF_FLAG(caps.VolumeTextureFilterCaps, D3DPTFILTERCAPS_MIPFPOINT);        
		SPRINTF_NL();
		SPRINTF_FLAG(caps.TextureAddressCaps, D3DPTADDRESSCAPS_BORDER); 
		SPRINTF_FLAG(caps.TextureAddressCaps, D3DPTADDRESSCAPS_CLAMP); 
		SPRINTF_FLAG(caps.TextureAddressCaps, D3DPTADDRESSCAPS_INDEPENDENTUV); 
		SPRINTF_FLAG(caps.TextureAddressCaps, D3DPTADDRESSCAPS_MIRROR); 
		SPRINTF_FLAG(caps.TextureAddressCaps, D3DPTADDRESSCAPS_MIRRORONCE); 
		SPRINTF_FLAG(caps.TextureAddressCaps, D3DPTADDRESSCAPS_WRAP); 
		SPRINTF_NL();
		SPRINTF_FLAG(caps.VolumeTextureAddressCaps, D3DPTADDRESSCAPS_BORDER);       
		SPRINTF_FLAG(caps.VolumeTextureAddressCaps, D3DPTADDRESSCAPS_CLAMP);        
		SPRINTF_FLAG(caps.VolumeTextureAddressCaps, D3DPTADDRESSCAPS_INDEPENDENTUV);
		SPRINTF_FLAG(caps.VolumeTextureAddressCaps, D3DPTADDRESSCAPS_MIRROR);       
		SPRINTF_FLAG(caps.VolumeTextureAddressCaps, D3DPTADDRESSCAPS_MIRRORONCE);   
		SPRINTF_FLAG(caps.VolumeTextureAddressCaps, D3DPTADDRESSCAPS_WRAP);         
		SPRINTF_NL();
		SPRINTF_FLAG(caps.LineCaps, D3DLINECAPS_ALPHACMP); 
		SPRINTF_FLAG(caps.LineCaps, D3DLINECAPS_BLEND); 
		SPRINTF_FLAG(caps.LineCaps, D3DLINECAPS_FOG); 
		SPRINTF_FLAG(caps.LineCaps, D3DLINECAPS_TEXTURE); 
		SPRINTF_FLAG(caps.LineCaps, D3DLINECAPS_ZTEST); 
		SPRINTF_NL();
		i += sprintf(mass_buffer + i, "Max texture width:  %d\n", caps.MaxTextureWidth);
		i += sprintf(mass_buffer + i, "Max texture height: %d\n", caps.MaxTextureHeight);
		i += sprintf(mass_buffer + i, "Max volume extent:  %d\n", caps.MaxVolumeExtent);
		SPRINTF_NL();
		i += sprintf(mass_buffer + i, "Max texture repeat: %d\n", caps.MaxTextureRepeat);
		i += sprintf(mass_buffer + i, "Max texture aspect: %d\n", caps.MaxTextureAspectRatio);
		i += sprintf(mass_buffer + i, "Max anisotropy:     %d\n", caps.MaxAnisotropy);
		i += sprintf(mass_buffer + i, "Max vertex W:       %f\n", caps.MaxVertexW);
		SPRINTF_NL();
		i += sprintf(mass_buffer + i, "Guard band (l,t,r,b): %f %f %f %f\n",
			caps.GuardBandLeft,
			caps.GuardBandTop,
			caps.GuardBandRight,
			caps.GuardBandBottom);
		SPRINTF_NL();
		i += sprintf(mass_buffer + i, "Extents adjust: %d\n", caps.ExtentsAdjust);
		SPRINTF_NL();
		SPRINTF_FLAG(caps.StencilCaps, D3DSTENCILCAPS_DECR); 
		SPRINTF_FLAG(caps.StencilCaps, D3DSTENCILCAPS_DECRSAT); 
		SPRINTF_FLAG(caps.StencilCaps, D3DSTENCILCAPS_INCR); 
		SPRINTF_FLAG(caps.StencilCaps, D3DSTENCILCAPS_INCRSAT); 
		SPRINTF_FLAG(caps.StencilCaps, D3DSTENCILCAPS_INVERT); 
		SPRINTF_FLAG(caps.StencilCaps, D3DSTENCILCAPS_KEEP); 
		SPRINTF_FLAG(caps.StencilCaps, D3DSTENCILCAPS_REPLACE); 
		SPRINTF_FLAG(caps.StencilCaps, D3DSTENCILCAPS_ZERO); 
		SPRINTF_NL();
		SPRINTF_FLAG(caps.FVFCaps, D3DFVFCAPS_DONOTSTRIPELEMENTS); 
		SPRINTF_FLAG(caps.FVFCaps, D3DFVFCAPS_PSIZE); 
		SPRINTF_FLAG(caps.FVFCaps, D3DFVFCAPS_TEXCOORDCOUNTMASK); 
		SPRINTF_NL();
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_ADD); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_ADDSIGNED); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_ADDSIGNED2X); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_ADDSMOOTH); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_BLENDCURRENTALPHA); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_BLENDDIFFUSEALPHA); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_BLENDFACTORALPHA); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_BLENDTEXTUREALPHA); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_BLENDTEXTUREALPHAPM); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_BUMPENVMAP); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_BUMPENVMAPLUMINANCE); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_DISABLE); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_DOTPRODUCT3); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_LERP); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_MODULATE); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_MODULATE2X); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_MODULATE4X); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_MULTIPLYADD); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_PREMODULATE); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_SELECTARG1); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_SELECTARG2); 
		SPRINTF_FLAG(caps.TextureOpCaps, D3DTEXOPCAPS_SUBTRACT); 
		SPRINTF_NL();
		i += sprintf(mass_buffer + i, "Max texture blend stages:  %d\n", caps.MaxTextureBlendStages);
		i += sprintf(mass_buffer + i, "Max simultaneous textures: %d\n", caps.MaxSimultaneousTextures);
		SPRINTF_NL();
		SPRINTF_FLAG(caps.VertexProcessingCaps, D3DVTXPCAPS_DIRECTIONALLIGHTS); 
		SPRINTF_FLAG(caps.VertexProcessingCaps, D3DVTXPCAPS_LOCALVIEWER); 
		SPRINTF_FLAG(caps.VertexProcessingCaps, D3DVTXPCAPS_MATERIALSOURCE7); 
		SPRINTF_FLAG(caps.VertexProcessingCaps, D3DVTXPCAPS_POSITIONALLIGHTS); 
		SPRINTF_FLAG(caps.VertexProcessingCaps, D3DVTXPCAPS_TEXGEN); 
		SPRINTF_FLAG(caps.VertexProcessingCaps, D3DVTXPCAPS_TWEENING); 
		SPRINTF_FLAG(caps.VertexProcessingCaps, D3DVTXPCAPS_NO_VSDT_UBYTE4); 
		SPRINTF_NL();
		i += sprintf(mass_buffer + i, "Max active lights:             %d\n", caps.MaxActiveLights);
		i += sprintf(mass_buffer + i, "Max user clip planes:          %d\n", caps.MaxUserClipPlanes);
		i += sprintf(mass_buffer + i, "Max vertex blend matrices:     %d\n", caps.MaxVertexBlendMatrices);
		i += sprintf(mass_buffer + i, "Max vertex blend matrix index: %d\n", caps.MaxVertexBlendMatrixIndex);
		SPRINTF_NL();
		i += sprintf(mass_buffer + i, "Max point size: %f\n", caps.MaxPointSize);
		i += sprintf(mass_buffer + i, "Max Prim count:    %d\n", caps.MaxPrimitiveCount);
		i += sprintf(mass_buffer + i, "Max Vertrx index:  %d\n", caps.MaxVertexIndex);
		i += sprintf(mass_buffer + i, "Max Streams:       %d\n", caps.MaxStreams);
		i += sprintf(mass_buffer + i, "Max Stream stride: %d\n", caps.MaxStreamStride);
		SPRINTF_NL();
		i += sprintf(mass_buffer + i, "Vertex shader version:   %d.%d\n", HIBYTE(caps.VertexShaderVersion), LOBYTE(caps.VertexShaderVersion));
		i += sprintf(mass_buffer + i, "Max vertex shader const: %d\n", caps.MaxVertexShaderConst);
		SPRINTF_NL();
		i += sprintf(mass_buffer + i, "Pixel shader version:    %d.%d\n", HIBYTE(caps.PixelShaderVersion), LOBYTE(caps.PixelShaderVersion));
		i += sprintf(mass_buffer + i, "Max pixel shader value:  %d\n", caps.MaxPixelShaderValue); 
		SPRINTF_NL(); 
	}
														  
	char filename[MAX_PATH] = {"D3D8 CAPS "};

	unsigned long len = MAX_PATH - strlen(filename); 
	GetUserName(filename + strlen(filename), &len); 

	FILE *fp = browse_for_and_open_save_file(this->GetSafeHwnd(), filename, "txt", "Save D3D8 CAPS textfile"); 

	if(fp == NULL)
	{
		MessageBox("Cancelled D3D cap file save", "Warning", MB_ICONWARNING);
		return;
	}
		
	fwrite(mass_buffer, strlen(mass_buffer), 1, fp);

	fclose(fp);

#ifdef _DEBUG
	char sys_command[MAX_PATH] = "notepad \"";

	strcat(sys_command, filename);
	strcat(sys_command, "\"");

	system(sys_command);
#endif
}

/**
 * Cleanup function
 */
void CDX8Disp::OnDestroy() 
{
	CDialog::OnDestroy();

	if(d3d_interface)
	{
		d3d_interface->Release();
	}
}

/**
 * The user has chosen to accept these settings
 *
 * @param char *reg_path - Registry path that any settings should be saved to
 */
void CDX8Disp::OnApply(int flags)
{
	if(m_dx8_initialised_ok == false)
	{
		MessageBox("Cannot apply valid settings, install DX8!", "Error", MB_ICONERROR);
		return;
	}

	int result = 0;
	
	int selected_adapter = m_adapter_list.GetCurSel();
	int mode_num		 = m_res_list.GetItemData(m_res_list.GetCurSel());

	int aatype = m_antialias_list.GetCurSel();
	if(aatype == -1) {
		aatype = 0;
	}

	result += reg_set_dword(Settings::reg_path, "D3D8_AAType",  aatype);
	result += reg_set_dword(Settings::reg_path, "D3D8_Adapter", selected_adapter); 

	if(!result)
	{
		MessageBox("Failed to set an important registry entry", "Error", MB_ICONERROR);
	}

	// Lets set those video card settings
	D3DDISPLAYMODE mode;
	d3d_interface->EnumAdapterModes(selected_adapter, mode_num, &mode); 

	char video_card[100];
	int cdepth = d3d8_get_mode_bit(mode.Format);

	char *reg_name = "VideocardFs2open";

	sprintf(video_card, "D3D8-(%dx%d)x%d bit", mode.Width, mode.Height, cdepth);
	reg_set_sz(Settings::reg_path, reg_name, video_card);
}

void CDX8Disp::LoadSettings(int flags)
{
   	if(m_dx8_initialised_ok == false)
	{
		return;
	}

	DWORD adapter, aatype;

	if(reg_get_dword(Settings::reg_path, "D3D8_Adapter", &adapter) == false)
	{
		UpdateAdapterList();
		return;
	}

	UpdateAdapterList(adapter);

	// Getting mode options is a bit more complicated
	char mode_string[1024];
	bool result = false;
	unsigned int width, height;
	int cdepth;

	char *reg_name = "VideocardFs2open";

	if(reg_get_sz(Settings::reg_path, reg_name, mode_string, 1024))
		if(sscanf(mode_string, "D3D8-(%dx%d)x%d bit", &width, &height, &cdepth)  == 3) 
			result = true;

	// No mode found, show user choice and return
	if(result == false)
	{
		UpdateResList();
		return;
	}

	// Show user choice and set if next setting is already set
	UpdateResList(width, height, cdepth);

	if(reg_get_dword(Settings::reg_path, "D3D8_AAType", &aatype) == false)
	{
		// No setting found, show user choice and return
		UpdateAntialiasList();
		return;
	}

	UpdateAntialiasList(aatype);
}

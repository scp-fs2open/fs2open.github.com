// TabSound.cpp : implementation file
//

#include "stdafx.h"
#include "launcher.h"
#include "TabSound.h"

#include <mmsystem.h>
#include <vector>
#include <string>
#include <regstr.h>

#include "win32func.h"
#include "settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int NUM_SND_MODES = 4;

char *sound_card_modes[NUM_SND_MODES] =
{
	"DirectSound",
	"EAX",
	"Aureal A3D",
	"No Sound"
}; 

// OpenAL stuff
typedef struct ALCdevice_struct ALCdevice;
typedef char ALCchar;
typedef int ALCenum;
#define ALC_DEFAULT_DEVICE_SPECIFIER	0x1004
#define ALC_DEVICE_SPECIFIER			0x1005
HINSTANCE hOAL;
const ALCchar* (*alcGetString)( ALCdevice *device, ALCenum param ) = NULL;

std::vector<std::string> OpenAL_sound_devices;

/////////////////////////////////////////////////////////////////////////////
// CTabSound dialog

CTabSound::CTabSound(CWnd* pParent /*=NULL*/)
	: CDialog(CTabSound::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabSound)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTabSound::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabSound)
	DDX_Control(pDX, IDC_JOYSTICK, m_joystick_list);
	DDX_Control(pDX, IDC_SOUND_CARD, m_sound_api_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabSound, CDialog)
	//{{AFX_MSG_MAP(CTabSound)
	ON_BN_CLICKED(IDC_CALIBRATE, OnCalibrate)
	ON_CBN_SELCHANGE(IDC_SOUND_CARD, OnSelChangeSoundDevice)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabSound message handlers

void CTabSound::OnApply()
{
	// Sound settings
	int index = m_sound_api_list.GetCurSel();

	char string[50];
	m_sound_api_list.GetLBText(index, string);

	if ( Settings::openal_build == true ) {
		reg_set_sz(Settings::reg_path, "SoundDeviceOAL", string);
	} else {
		reg_set_sz(Settings::reg_path, "Soundcard", string);
	}

	// Joystick settings
	const int CHECKED = 1; 

	int ff = (((CButton *) GetDlgItem(IDC_FORCE_FREEDBACK))->GetCheck() == CHECKED) ? 1 : 0;
	int dh = (((CButton *) GetDlgItem(IDC_DIR_HIT))->GetCheck() == CHECKED) ? 1 : 0;
		
	reg_set_dword(Settings::reg_path, "EnableJoystickFF", ff);
	reg_set_dword(Settings::reg_path, "EnableHitEffect", dh);

	// Set joystick
	index = m_joystick_list.GetCurSel();

	if (index < 0)
		index = 0;

    int enum_id = m_joystick_list.GetItemData(index);

	reg_set_dword(Settings::reg_path, "CurrentJoystick", enum_id);
}

void CTabSound::SetupOpenAL()
{
	if ( OpenAL_sound_devices.size() < 1 ) {
		if ( (hOAL = LoadLibrary("OpenAL32.dll")) == 0 ) {
			MessageBox("Build needs OpenAL but an OpenAL DLL cannot be found!", "Error", MB_OK);
			return;
		}

		alcGetString = (const ALCchar* (*)(ALCdevice *device, ALCenum param)) GetProcAddress( hOAL, "alcGetString" );

		if (alcGetString == NULL) {
			FreeLibrary( hOAL );
			MessageBox("Unable to acquire alcGetString pointer!", "Error", MB_OK);
			return;
		}

		const char *my_devices = (const char*) alcGetString( NULL, ALC_DEVICE_SPECIFIER );
		char *pos = NULL;

		pos = (char*)my_devices;

		while (*pos && (strlen(pos) > 0)) {
			OpenAL_sound_devices.push_back(pos);

			pos += (strlen(pos) + 1);
		}

		OpenAL_sound_devices.push_back("no sound");

		FreeLibrary( hOAL );
	}

	m_sound_api_list.ResetContent();

	unsigned int i;
	int selection = 0;

	for (i = 0; i < OpenAL_sound_devices.size(); i++) {
		m_sound_api_list.InsertString(i, OpenAL_sound_devices[i].c_str());

		if ( !stricmp(OpenAL_sound_devices[i].c_str(), "Generic Software") ) {
			GetDlgItem(IDC_OAL_WARN_STATIC)->ShowWindow(false);
			selection = i;
		}
	}

	m_sound_api_list.SetCurSel(selection);
}

BOOL CTabSound::OnInitDialog() 
{
	int i, list_index;
	JOYCAPS jc;
	JOYINFO ji;
	char joy_name[256];
	TCHAR szKey[256];
	TCHAR szValue[256];
	UCHAR szOEMKey[256];
	HKEY hKey;
	DWORD dwcb;
	LONG lr;

	CDialog::OnInitDialog();

	for (i = 0; i < NUM_SND_MODES; i++)
		m_sound_api_list.InsertString(i, sound_card_modes[i]);

	m_sound_api_list.SetCurSel(0);

	GetDlgItem(IDC_OAL_WARN_STATIC)->ShowWindow(false);

	// Joystick
	list_index = m_joystick_list.AddString("No Joystick");
	m_joystick_list.SetItemData(list_index, 99999);

	int num_sticks = joyGetNumDevs();

	for (i = 0; i < num_sticks; i++) {
		memset( &jc, 0, sizeof(JOYCAPS) );
		memset( joy_name, 0, sizeof(joy_name) );

		MMRESULT mr = joyGetDevCaps( i, &jc, sizeof(JOYCAPS) );

		// make sure that our device/driver is good
		if (mr != JOYERR_NOERROR)
			continue;

		// if the joystick is unplugged or otherwise not available then just ignore it
		if (joyGetPos(i, &ji) != JOYERR_NOERROR)
			continue;

		// copy basic device name, in case getting the OEM name from the registry fails
		strcpy( joy_name, jc.szPname );

		// now try to grab the full OEM name for this joystick ...

		sprintf(szKey, "%s\\%s\\%s", REGSTR_PATH_JOYCONFIG, jc.szRegKey, REGSTR_KEY_JOYCURR);
		lr = RegOpenKeyEx(HKEY_CURRENT_USER, (LPTSTR) &szKey, 0, KEY_QUERY_VALUE, &hKey);

		if (lr != ERROR_SUCCESS)
			goto Done;

		dwcb = sizeof(szOEMKey);
		// NOTE: normal joystick values are 0 based, but it's 1 based in the registry, hence the "+1"
		sprintf(szValue, "Joystick%d%s", i+1, REGSTR_VAL_JOYOEMNAME);
		lr = RegQueryValueEx(hKey, szValue, 0, 0, (LPBYTE) &szOEMKey, (LPDWORD) &dwcb);
		RegCloseKey(hKey);

		if (lr != ERROR_SUCCESS)
			goto Done;

		sprintf(szKey, "%s\\%s", REGSTR_PATH_JOYOEM, szOEMKey);
		lr = RegOpenKeyEx(HKEY_CURRENT_USER, szKey, 0, KEY_QUERY_VALUE, &hKey);

		if (lr != ERROR_SUCCESS)
			goto Done;

		// grab the OEM name
		dwcb = sizeof(szValue);
		lr = RegQueryValueEx(hKey, REGSTR_VAL_JOYOEMNAME, 0, 0, (LPBYTE) joy_name, (LPDWORD) &dwcb);
		RegCloseKey(hKey);

Done:
		list_index = m_joystick_list.AddString(joy_name);
		m_joystick_list.SetItemData(list_index, i);
	}


	m_joystick_list.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTabSound::LoadSettings()
{
	DWORD ff, dh;

	if ( Settings::openal_build == true ) {
		GetDlgItem(IDC_OAL_WARN_STATIC)->ShowWindow(true);
		GetDlgItem(IDC_EAX_STATIC)->ShowWindow(false);
		GetDlgItem(IDC_SOUND_STATIC)->SetWindowText("Currently selected OpenAL Sound Device");

		SetupOpenAL();
	} else {
		GetDlgItem(IDC_EAX_STATIC)->ShowWindow(true);
		GetDlgItem(IDC_SOUND_STATIC)->SetWindowText("Currently selected Sound Card");
	}

	reg_get_dword(Settings::reg_path, "EnableJoystickFF", &ff);
	reg_get_dword(Settings::reg_path, "EnableHitEffect", &dh);

	((CButton *) GetDlgItem(IDC_FORCE_FREEDBACK))->SetCheck(ff ? 1 : 0);
	((CButton *) GetDlgItem(IDC_DIR_HIT))->SetCheck(dh ? 1 : 0);

	char local_port_text[50];

	if ( Settings::openal_build == false ) {
		reg_get_sz(Settings::reg_path, "SoundCard", local_port_text, 50);

		for(int i = 0; i < NUM_SND_MODES; i++)
		{
			if(stricmp(sound_card_modes[i], local_port_text) == 0)
			{
				m_sound_api_list.SetCurSel(i);
				break;;
			}
		}
	} else {
		reg_get_sz(Settings::reg_path, "SoundDeviceOAL", local_port_text, 50);

		for (unsigned int i = 0; i < OpenAL_sound_devices.size(); i++) {
			if ( !stricmp(OpenAL_sound_devices[i].c_str(), local_port_text) ) {
				m_sound_api_list.SetCurSel(i);

				// if not software device then show warning message
				if ( stricmp(OpenAL_sound_devices[i].c_str(), "no sound") && 
						stricmp(OpenAL_sound_devices[i].c_str(), "Generic Software") )
				{
					GetDlgItem(IDC_OAL_WARN_STATIC)->ShowWindow(true);
				}

				break;
			}
		}
	}

	// Get joystick
	DWORD enum_id = 0;
	reg_get_dword(Settings::reg_path, "CurrentJoystick", &enum_id);

	int num_joy = m_joystick_list.GetCount();
	for (int i = 0; i < num_joy; i++) {
		if (m_joystick_list.GetItemData(i) == enum_id) {
			m_joystick_list.SetCurSel(i);
			break;
		}
	}
}

void CTabSound::OnCalibrate() 
{
	int index = m_joystick_list.GetCurSel();

	if (index < 0)
		return;

    int enum_id = m_joystick_list.GetItemData(index);

	if (enum_id == 99999) {
		MessageBox("No Joystick is setup", "Error");
		return;
	}

	// launch the joystick control panel applet
	WinExec("rundll32.exe shell32.dll,Control_RunDLL joy.cpl", SW_SHOWNORMAL);
}

void CTabSound::OnDestroy() 
{
	CDialog::OnDestroy();
}

void CTabSound::OnSelChangeSoundDevice()
{
	if ( Settings::openal_build == false )
		return;

	char device_str[50];
	int index = m_sound_api_list.GetCurSel();

	m_sound_api_list.GetLBText(index, device_str);

	// since anything other than the generic software device
	// may not have enough sound sources to also use music/voices
	// be sure to give a warning text to that effect
	if ( stricmp(device_str, "no sound") && stricmp(device_str, "Generic Software") )
		GetDlgItem(IDC_OAL_WARN_STATIC)->ShowWindow(true);
	else
		GetDlgItem(IDC_OAL_WARN_STATIC)->ShowWindow(false);
}

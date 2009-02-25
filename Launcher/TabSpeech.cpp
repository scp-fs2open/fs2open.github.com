// TabSpeech.cpp : implementation file
//
#include "stdafx.h"

#ifdef FS2_SPEECH
#include <sapi.h>           
#include <sphelper.h>
#include <spuihelp.h>
#endif

#include "launcher.h"
#include "TabSpeech.h"

#include "win32func.h"
#include "speech.h"
#include "settings.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabSpeech dialog


CTabSpeech::CTabSpeech(CWnd* pParent /*=NULL*/)
	: CDialog(CTabSpeech::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabSpeech)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTabSpeech::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabSpeech)
	DDX_Control(pDX, IDC_VOLUME_SLIDER, m_volume_control);
	DDX_Control(pDX, IDC_SPEECH_TECHROOM, m_checkbox_techroom);
	DDX_Control(pDX, IDC_SPEECH_INGAME, m_checkbox_ingame);
	DDX_Control(pDX, IDC_SPEECH_BRIEFINGS, m_checkbox_briefings);
	DDX_Control(pDX, IDC_EDIT, m_edit_box);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabSpeech, CDialog)
	//{{AFX_MSG_MAP(CTabSpeech)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	ON_WM_HSCROLL()
	ON_CBN_SELCHANGE(IDC_VOICE_COMBO, OnSelchangeVoiceCombo)
	ON_BN_CLICKED(IDC_GET_VOICES, OnGetVoices)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabSpeech message handlers

BOOL CTabSpeech::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_speech_supported = speech_init(); 

   	if(m_speech_supported == false) 
	{
		EnableWindow(FALSE);
		m_edit_box.SetWindowText("Speech API 5.1 not installed");
		m_checkbox_techroom.SetCheck(0);
		m_checkbox_briefings.SetCheck(0);
		m_checkbox_ingame.SetCheck(0);
	} else {
		m_edit_box.SetWindowText("Press play to test this string.");

#if FS2_SPEECH
		// Offer the user a choice of voices
		SpInitTokenComboBox( GetDlgItem(IDC_VOICE_COMBO )->GetSafeHwnd(), SPCAT_VOICES );
#endif

	}

	// Setup the volume
	m_volume_control.SetRange(0, 100);
	m_volume_control.SetPos(100);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTabSpeech::OnApply()
{
	if(m_speech_supported == false) return;

	// Set the basic use of speech
	const int CHECKED = 1; 

	DWORD use;
	use = (m_checkbox_techroom.GetCheck() == CHECKED);
	reg_set_dword(Settings::reg_path, "SpeechTechRoom", use);

	use = (m_checkbox_briefings.GetCheck() == CHECKED);
	reg_set_dword(Settings::reg_path, "SpeechBriefings", use);

	use = (m_checkbox_ingame.GetCheck() == CHECKED);
	reg_set_dword(Settings::reg_path, "SpeechIngame", use);

	reg_set_dword(Settings::reg_path, "SpeechVolume", m_speech_volume);

	// Get the selected voice num in the combo box
	int selected = ((CComboBox *) GetDlgItem(IDC_VOICE_COMBO))->GetCurSel();
	if(selected == CB_ERR) selected = -1;


	// Set the voice name as number (sorry, you wouldnt believe how complicated text would be!) 
	reg_set_dword(Settings::reg_path, "SpeechVoice", selected);
}

void CTabSpeech::LoadSettings()
{
	if(m_speech_supported == false) return;

	const int CHECKED = 1; 

	// Set the basic use of speech parameters
	m_checkbox_techroom.SetCheck(CHECKED);
	m_checkbox_briefings.SetCheck(CHECKED);
	m_checkbox_ingame.SetCheck(CHECKED);

	DWORD use;
	if(reg_get_dword(Settings::reg_path, "SpeechTechRoom", &use) && use == 0) {
		m_checkbox_techroom.SetCheck(0);
	}

	if(reg_get_dword(Settings::reg_path, "SpeechBriefings", &use) && use == 0) {
		m_checkbox_briefings.SetCheck(0);
	}

	if(reg_get_dword(Settings::reg_path, "SpeechIngame", &use) && use == 0) {
		m_checkbox_ingame.SetCheck(0);
	}

	if(reg_get_dword(Settings::reg_path, "SpeechVolume", &m_speech_volume) == false) {
		m_speech_volume = 100;
	}

	m_volume_control.SetPos(m_speech_volume);
	speech_set_volume(m_speech_volume);

	// Get the voice name as number
	DWORD selected;
	reg_get_dword(Settings::reg_path, "SpeechVoice", &selected);

	if(selected == -1) return;
	((CComboBox *) GetDlgItem(IDC_VOICE_COMBO))->SetCurSel(selected);
}


void CTabSpeech::OnPlay() 
{
	// Make sure we are using the right voice
	UpdateVoiceFromCombo();

	const int MAX_CHARS = 1024;
	char text_to_play[MAX_CHARS];

	m_edit_box.GetWindowText(text_to_play, MAX_CHARS);
	speech_play(text_to_play);	
}

// Free everything here
BOOL CTabSpeech::DestroyWindow() 
{
   	if(m_speech_supported == true) 
	{
#ifdef FS2_SPEECH
		SpDestroyTokenComboBox( GetDlgItem(IDC_VOICE_COMBO )->GetSafeHwnd() );
#endif
   		speech_deinit();
	}
	
	return CDialog::DestroyWindow();
}

// User has changed volume control
void CTabSpeech::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if(pScrollBar)
	{
		m_speech_volume = m_volume_control.GetPos();
	 	speech_set_volume(m_speech_volume);
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

// User has selected new voice from combo box
void CTabSpeech::OnSelchangeVoiceCombo() 
{
	UpdateVoiceFromCombo();
	OnPlay();

}

void CTabSpeech::UpdateVoiceFromCombo()
{
#if FS2_SPEECH
    ISpObjectToken* pToken = SpGetCurSelComboBoxToken( GetDlgItem(IDC_VOICE_COMBO )->GetSafeHwnd() );
	if(	pToken )
	{
		speech_set_voice(pToken);
	}
#endif
}

void CTabSpeech::OnGetVoices() 
{
	MessageBox("The fs2_open project is not responsible for the content of this site or the functionality of the installer on it", "Warning", MB_ICONWARNING);	
	open_web_page("http://www.microsoft.com/downloads/details.aspx?FamilyId=5E86EC97-40A7-453F-B0EE-6583171B4530&displaylang=en");
}

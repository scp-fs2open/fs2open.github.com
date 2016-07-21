// soundenvironmentdlg.cpp : implementation file
//

#include "stdafx.h"
#include "fred.h"
#include "soundenvironmentdlg.h"
#include "sound/audiostr.h"
#include "sound/sound.h"
#include "sound/ds.h"
#include "cfile/cfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SoundEnvironment dialog


SoundEnvironment::SoundEnvironment(CWnd* pParent /*=NULL*/)
	: CDialog(SoundEnvironment::IDD, pParent)
{
	//{{AFX_DATA_INIT(SoundEnvironment)
	m_environment = -1;
	m_damping = 0.0f;
	m_decay_time = 0.0f;
	m_volume = 0.0f;
	m_wave_filename = _T("");
	m_wave_id = -1;
	//}}AFX_DATA_INIT
}


void SoundEnvironment::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SoundEnvironment)
	DDX_CBIndex(pDX, IDC_SOUND_ENVIRONMENT, m_environment);
	DDX_Text(pDX, IDC_SOUND_ENVIRONMENT_DAMPING, m_damping);
	DDV_MinMaxFloat(pDX, m_damping, 0.1f, 2.0f);
	DDX_Text(pDX, IDC_SOUND_ENVIRONMENT_DECAY, m_decay_time);
	DDV_MinMaxFloat(pDX, m_decay_time, 0.1f, 20.0f);
	DDX_Text(pDX, IDC_SOUND_ENVIRONMENT_VOLUME, m_volume);
	DDV_MinMaxFloat(pDX, m_volume, 0.0f, 1.0f);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SoundEnvironment, CDialog)
	//{{AFX_MSG_MAP(SoundEnvironment)
	ON_CBN_SELCHANGE(IDC_SOUND_ENVIRONMENT, OnSelChangeSoundEnvironment)
	ON_BN_CLICKED(IDC_BROWSE_WAVE, OnBrowseWave)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SoundEnvironment message handlers

BOOL SoundEnvironment::OnInitDialog()
{
	m_play_bm.LoadBitmap(IDB_PLAY);
	((CButton *) GetDlgItem(IDC_PLAY)) -> SetBitmap(m_play_bm);

	// fill environment list ...

	CComboBox *box = (CComboBox *) GetDlgItem(IDC_SOUND_ENVIRONMENT);

	// add empty default string (for no environment set)
	box->AddString("");

	for (size_t i = 0; i < EFX_presets.size(); i++) {
		box->AddString(EFX_presets[i].name.c_str());
	}

	// set values ...

	sound_env *m_env = &The_mission.sound_environment;

	// make sure we are set by default to mission values
	if (m_env->id >= 0) {
		m_environment = m_env->id + 1;

		m_volume = m_env->volume;
		m_damping = m_env->damping;
		m_decay_time = m_env->decay;

		// make it active
		sound_env_set(m_env);
	} else {
		m_environment = 0;

		m_volume = 0.0f;
		m_damping = 0.1f;
		m_decay_time = 0.1f;

		sound_env_disable();
	}

	CDialog::OnInitDialog();
	UpdateData(FALSE);
	return TRUE;
}

void SoundEnvironment::OnOK()
{
	UpdateData(TRUE);

	sound_env *m_env = &The_mission.sound_environment;

	int env_id = m_environment - 1;

	// save anything if we need to
	if (env_id >= 0) {
		m_env->id = env_id;
		m_env->volume = m_volume;
		m_env->damping = m_damping;
		m_env->decay = m_decay_time;
	} else {
		m_env->id = -1;
	}

	CDialog::OnOK();
}

void SoundEnvironment::OnCancel()
{
	CDialog::OnCancel();
}

void SoundEnvironment::OnClose()
{
	int z;

	UpdateData(TRUE);

	if (query_modified()) {
		z = MessageBox("Do you want to keep your changes?", "Close", MB_ICONQUESTION | MB_YESNOCANCEL);
		if (z == IDCANCEL) {
			return;
		}

		if (z == IDYES) {
			OnOK();
			return;
		}
	}

	CDialog::OnClose();
}

int SoundEnvironment::query_modified()
{
	int env_id = m_environment - 1;
	sound_env *m_env = &The_mission.sound_environment;

	return ( (env_id != m_env->id) || (m_volume != m_env->volume) || (m_damping != m_env->damping)
				|| (m_decay_time != m_env->decay) );
}

void SoundEnvironment::OnSelChangeSoundEnvironment()
{
	UpdateData(TRUE);

	int index = m_environment - 1;

	if (index >= 0) {
		m_volume = EFX_presets[index].flGain;
		m_damping = EFX_presets[index].flDecayHFRatio;
		m_decay_time = EFX_presets[index].flDecayTime;
	} else {
		m_volume = 0.0f;
		m_damping = 0.1f;
		m_decay_time = 0.1f;
	}

	UpdateData(FALSE);
}

BOOL SoundEnvironment::DestroyWindow()
{
	sound_env_disable();

	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	m_play_bm.DeleteObject();
	return CDialog::DestroyWindow();
}

void SoundEnvironment::OnPlay()
{
	sound_env m_env;

	if ( !sound_env_supported() ) {
		MessageBox("Sound environment effects are not available! Unable to preview!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	if (m_wave_id < 0) {
		OnBrowseWave();
		return;
	}

	UpdateData(TRUE);

	if (m_environment > 0) {
		m_env.id = m_environment - 1;
		m_env.volume = m_volume;
		m_env.damping = m_damping;
		m_env.decay = m_decay_time;

		sound_env_set(&m_env);
	}

	audiostream_play(m_wave_id, 1.0f, 0);
}

void SoundEnvironment::OnBrowseWave()
{
	int z;

	if (m_wave_id >= 0) {
		audiostream_close_file(m_wave_id, 0);
		m_wave_id = -1;
	}

	UpdateData(TRUE);

	m_wave_filename = _T("");

	z = cfile_push_chdir(CF_TYPE_DATA);

	CFileDialog dlg(TRUE, "wav", m_wave_filename, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR,
		"Voice Files (*.ogg, *.wav)|*.ogg;*.wav|Ogg Vorbis Files (*.ogg)|*.ogg|Wave Files (*.wav)|*.wav||");

	if (dlg.DoModal() == IDOK) {
		m_wave_filename = dlg.GetFileName();
		m_wave_id = audiostream_open((char *)(LPCSTR) m_wave_filename, ASF_SOUNDFX);
	}

	if ( !z ) {
		cfile_pop_dir();
	}
}

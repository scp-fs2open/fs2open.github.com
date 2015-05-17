// FictionViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "fred.h"
#include "FictionViewerDlg.h"
#include "missionui/fictionviewer.h"
#include "gamesnd/eventmusic.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// FictionViewerDlg dialog


FictionViewerDlg::FictionViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(FictionViewerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(FictionViewerDlg)
	m_story_file = _T("");
	m_font_file = _T("");
	m_voice_file = _T("");
	m_fiction_music = 0;
	//}}AFX_DATA_INIT
}


void FictionViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FictionViewerDlg)
	DDX_Text(pDX, IDC_STORY_FILE, m_story_file);
	DDX_Text(pDX, IDC_FONT_FILE, m_font_file);
	DDX_Text(pDX, IDC_VOICE_FILE, m_voice_file);
	DDX_CBIndex(pDX, IDC_FICTION_MUSIC, m_fiction_music);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(FictionViewerDlg, CDialog)
	//{{AFX_MSG_MAP(FictionViewerDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FictionViewerDlg message handlers

BOOL FictionViewerDlg::OnInitDialog()
{
	int i;
	CComboBox *box;

	box = (CComboBox *) GetDlgItem(IDC_FICTION_MUSIC);
	box->AddString("None");
	for (i=0; i<Num_music_files; i++){
		box->AddString(Spooled_music[i].name);		
	}

	// init variables
	m_story_file = _T(fiction_file());
	m_font_file = _T(fiction_font());
	m_voice_file = _T(fiction_voice());
	m_fiction_music = Mission_music[SCORE_FICTION_VIEWER] + 1;

	CDialog::OnInitDialog();
	UpdateData(FALSE);
	return TRUE;
}

void FictionViewerDlg::OnOK() 
{
	UpdateData(TRUE);

	// load it up
	fiction_viewer_reset();
	fiction_viewer_load((const char *)(LPCSTR)m_story_file, (const char *)(LPCSTR)m_font_file, (const char *)(LPCSTR)m_voice_file);

	// set music
	Mission_music[SCORE_FICTION_VIEWER] = m_fiction_music - 1;

	CDialog::OnOK();
}

void FictionViewerDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void FictionViewerDlg::OnClose()
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

int FictionViewerDlg::query_modified()
{
	return strcmp(m_story_file, fiction_file()) || strcmp(m_font_file, fiction_font()) || 
		strcmp(m_voice_file, fiction_voice()) || m_fiction_music != (Mission_music[SCORE_FICTION_VIEWER] + 1);
}

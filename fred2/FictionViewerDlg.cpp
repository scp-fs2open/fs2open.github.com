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
	CComboBox *box;

	box = (CComboBox *) GetDlgItem(IDC_FICTION_MUSIC);
	box->AddString("None");
	for (auto &sm: Spooled_music) {
		box->AddString(sm.name);
	}

	if (Fiction_viewer_stages.size() > 1)
	{
		MessageBox("You have multiple fiction viewer stages defined for this mission.  At present, FRED will only allow you to edit the first stage.");
	}

	// init fields based on first fiction viewer stage
	if (!Fiction_viewer_stages.empty())
	{
		fiction_viewer_stage *stagep = &Fiction_viewer_stages.at(0);
		m_story_file = _T(stagep->story_filename);
		m_font_file = _T(stagep->font_filename);
		m_voice_file = _T(stagep->voice_filename);
	}

	// music is managed through the mission
	m_fiction_music = Mission_music[SCORE_FICTION_VIEWER] + 1;

	CDialog::OnInitDialog();
	UpdateData(FALSE);
	return TRUE;
}

void FictionViewerDlg::OnOK() 
{
	UpdateData(TRUE);

	// remove whitespace
	m_story_file.Trim();
	m_font_file.Trim();
	m_voice_file.Trim();

	// if we don't have a story file, remove the fiction stuff
	if (m_story_file.GetLength() == 0)
	{
		// we only edit stage 0, so remove that
		if (!Fiction_viewer_stages.empty())
			Fiction_viewer_stages.erase(Fiction_viewer_stages.begin());

		// set music
		Mission_music[SCORE_FICTION_VIEWER] = -1;
	}
	// we have a story file, so save our data
	else
	{
		// make sure we have at least one stage
		if (Fiction_viewer_stages.empty())
		{
			fiction_viewer_stage stage;
			memset(&stage, 0, sizeof(fiction_viewer_stage));
			stage.formula = Locked_sexp_true;

			Fiction_viewer_stages.push_back(stage);
		}

		// store the fields in the data structure
		fiction_viewer_stage *stagep = &Fiction_viewer_stages.at(0);
		strcpy_s(stagep->story_filename, (LPCSTR)m_story_file);
		strcpy_s(stagep->font_filename, (LPCSTR)m_font_file);
		strcpy_s(stagep->voice_filename, (LPCSTR)m_voice_file);

		// set music
		Mission_music[SCORE_FICTION_VIEWER] = m_fiction_music - 1;
	}

	CDialog::OnOK();
}

void FictionViewerDlg::OnCancel() 
{
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
	const char *story, *font, *voice;
	if (Fiction_viewer_stages.empty())
	{
		story = "";
		font = "";
		voice = "";
	}
	else
	{
		auto stagep = &Fiction_viewer_stages.at(0);
		story = (LPCSTR)stagep->story_filename;
		font = (LPCSTR)stagep->font_filename;
		voice = (LPCSTR)stagep->voice_filename;
	}

	return strcmp(m_story_file, story) || strcmp(m_font_file, font) || strcmp(m_voice_file, voice)
		|| m_fiction_music != (Mission_music[SCORE_FICTION_VIEWER] + 1);
}

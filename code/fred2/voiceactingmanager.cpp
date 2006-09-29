// VoiceActingManager.cpp : implementation file
//

#include "stdafx.h"
#include "fred.h"
#include "VoiceActingManager.h"
#include "missionui/missioncmdbrief.h"
#include "mission/missionbriefcommon.h"
#include "mission/missionmessage.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// VoiceActingManager dialog


VoiceActingManager::VoiceActingManager(CWnd* pParent /*=NULL*/)
	: CDialog(VoiceActingManager::IDD, pParent)
{
	//{{AFX_DATA_INIT(VoiceActingManager)
	m_abbrev_briefing = _T("");
	m_abbrev_campaign = _T("");
	m_abbrev_command_briefing = _T("");
	m_abbrev_debriefing = _T("");
	m_abbrev_message = _T("");
	m_abbrev_mission = _T("");
	m_example = _T("");
	//}}AFX_DATA_INIT
}


void VoiceActingManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VoiceActingManager)
	DDX_Text(pDX, IDC_ABBREV_BRIEFING, m_abbrev_briefing);
	DDX_Text(pDX, IDC_ABBREV_CAMPAIGN, m_abbrev_campaign);
	DDX_Text(pDX, IDC_ABBREV_COMMAND_BRIEFING, m_abbrev_command_briefing);
	DDX_Text(pDX, IDC_ABBREV_DEBRIEFING, m_abbrev_debriefing);
	DDX_Text(pDX, IDC_ABBREV_MESSAGE, m_abbrev_message);
	DDX_Text(pDX, IDC_ABBREV_MISSION, m_abbrev_mission);
	DDX_Text(pDX, IDC_EXAMPLE, m_example);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VoiceActingManager, CDialog)
	//{{AFX_MSG_MAP(VoiceActingManager)
	ON_EN_SETFOCUS(IDC_ABBREV_BRIEFING, OnSetfocusAbbrevBriefing)
	ON_EN_SETFOCUS(IDC_ABBREV_CAMPAIGN, OnSetfocusAbbrevCampaign)
	ON_EN_SETFOCUS(IDC_ABBREV_COMMAND_BRIEFING, OnSetfocusAbbrevCommandBriefing)
	ON_EN_SETFOCUS(IDC_ABBREV_DEBRIEFING, OnSetfocusAbbrevDebriefing)
	ON_EN_SETFOCUS(IDC_ABBREV_MESSAGE, OnSetfocusAbbrevMessage)
	ON_EN_SETFOCUS(IDC_ABBREV_MISSION, OnSetfocusAbbrevMission)
	ON_EN_CHANGE(IDC_ABBREV_BRIEFING, OnChangeAbbrevBriefing)
	ON_EN_CHANGE(IDC_ABBREV_CAMPAIGN, OnChangeAbbrevCampaign)
	ON_EN_CHANGE(IDC_ABBREV_COMMAND_BRIEFING, OnChangeAbbrevCommandBriefing)
	ON_EN_CHANGE(IDC_ABBREV_DEBRIEFING, OnChangeAbbrevDebriefing)
	ON_EN_CHANGE(IDC_ABBREV_MESSAGE, OnChangeAbbrevMessage)
	ON_EN_CHANGE(IDC_ABBREV_MISSION, OnChangeAbbrevMission)
	ON_BN_CLICKED(IDC_GENERATE_FILE_NAMES, OnGenerateFileNames)
	ON_CBN_SELCHANGE(IDC_SUFFIX, OnChangeOtherSuffix)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VoiceActingManager message handlers

BOOL VoiceActingManager::OnInitDialog()
{
	CComboBox *box = ((CComboBox *) GetDlgItem(IDC_SUFFIX));
	box->AddString(".WAV");
	box->AddString(".OGG");
	box->SetCurSel(0);

	return TRUE;
}

CString VoiceActingManager::get_suffix()
{
	// assign suffix
	if (((CComboBox *) GetDlgItem(IDC_SUFFIX))->GetCurSel() == 1)
		return ".ogg";
	else
		return ".wav";
}

int VoiceActingManager::calc_digits(int size)
{
	if (size >= 10000)
		return 5;
	else if (size >= 1000)	// I hope we never hit this!
		return 4;
	else if (size >= 100)
		return 3;
	else
		return 2;
}

void VoiceActingManager::build_example()
{
	// pick a default

	if (m_abbrev_command_briefing != "")
		build_example(m_abbrev_command_briefing);
	else if (m_abbrev_briefing != "")
		build_example(m_abbrev_briefing);
	else if (m_abbrev_debriefing != "")
		build_example(m_abbrev_debriefing);
	else if (m_abbrev_message != "")
		build_example(m_abbrev_message);
	else
		build_example("");
}

void VoiceActingManager::build_example(CString section)
{
	if (section == "")
	{
		m_example = "";
		return;
	}

	m_example = generate_filename(section, 1, 2);
}

CString VoiceActingManager::generate_filename(CString section, int number, int digits)
{
	if (section == "")
		return "none.wav";

	int i;
	CString str = "";
	CString num;

	// build prefix
	if (m_abbrev_campaign != "")
		str = str + m_abbrev_campaign;
	if (m_abbrev_mission != "")
		str = str + m_abbrev_mission;
	str = str + section;
	
	// build number
	num.Format("%d", number);;
	digits -= num.GetLength();
	for (i = 0; i < digits; i++)
	{
		num = "0" + num;
	}
	str = str + num;

	// suffix
	str = str + get_suffix();

	return str;
}

void VoiceActingManager::OnGenerateFileNames() 
{
	int i;
	int digits;

	// command briefings
	digits = calc_digits(Cmd_briefs[0].num_stages);
	for (i = 0; i < Cmd_briefs[0].num_stages; i++)
	{
		strcpy(Cmd_briefs[0].stage[i].wave_filename, LPCTSTR(generate_filename(m_abbrev_command_briefing, i + 1, digits)));
	}

	// briefings
	digits = calc_digits(Briefings[0].num_stages);
	for (i = 0; i < Briefings[0].num_stages; i++)
	{
		strcpy(Briefings[0].stages[i].voice, LPCTSTR(generate_filename(m_abbrev_briefing, i + 1, digits)));
	}

	// debriefings
	digits = calc_digits(Debriefings[0].num_stages);
	for (i = 0; i < Debriefings[0].num_stages; i++)
	{
		strcpy(Debriefings[0].stages[i].voice, LPCTSTR(generate_filename(m_abbrev_debriefing, i + 1, digits)));
	}

	// messages
	digits = calc_digits(Num_messages - Num_builtin_messages);
	for (i = 0; i < Num_messages - Num_builtin_messages; i++)
	{
		// free existing filename
		if (Messages[i + Num_builtin_messages].wave_info.name != NULL)
			free(Messages[i + Num_builtin_messages].wave_info.name);

		// allocate new filename
		Messages[i + Num_builtin_messages].wave_info.name = strdup(LPCTSTR(generate_filename(m_abbrev_message, i + 1, digits)));
	}

	// notify
	MessageBox("Filename generation complete.", "Woohoo!");
}

void VoiceActingManager::OnSetfocusAbbrevBriefing() 
{
	UpdateData(TRUE);

	build_example(m_abbrev_briefing);

	UpdateData(FALSE);
}

void VoiceActingManager::OnSetfocusAbbrevCampaign() 
{
	UpdateData(TRUE);

	build_example();

	UpdateData(FALSE);
}

void VoiceActingManager::OnSetfocusAbbrevCommandBriefing() 
{
	UpdateData(TRUE);

	build_example(m_abbrev_command_briefing);

	UpdateData(FALSE);
}

void VoiceActingManager::OnSetfocusAbbrevDebriefing() 
{
	UpdateData(TRUE);

	build_example(m_abbrev_debriefing);

	UpdateData(FALSE);
}

void VoiceActingManager::OnSetfocusAbbrevMessage() 
{
	UpdateData(TRUE);

	build_example(m_abbrev_message);

	UpdateData(FALSE);
}

void VoiceActingManager::OnSetfocusAbbrevMission() 
{
	UpdateData(TRUE);

	build_example();

	UpdateData(FALSE);
}

void VoiceActingManager::OnChangeAbbrevBriefing() 
{
	UpdateData(TRUE);

	build_example(m_abbrev_briefing);

	UpdateData(FALSE);
}

void VoiceActingManager::OnChangeAbbrevCampaign() 
{
	UpdateData(TRUE);

	build_example();

	UpdateData(FALSE);
}

void VoiceActingManager::OnChangeAbbrevCommandBriefing() 
{
	UpdateData(TRUE);

	build_example(m_abbrev_command_briefing);

	UpdateData(FALSE);
}

void VoiceActingManager::OnChangeAbbrevDebriefing() 
{
	UpdateData(TRUE);

	build_example(m_abbrev_debriefing);

	UpdateData(FALSE);
}

void VoiceActingManager::OnChangeAbbrevMessage() 
{
	UpdateData(TRUE);

	build_example(m_abbrev_message);

	UpdateData(FALSE);
}

void VoiceActingManager::OnChangeAbbrevMission() 
{
	UpdateData(TRUE);

	build_example();

	UpdateData(FALSE);
}

void VoiceActingManager::OnChangeOtherSuffix() 
{
	UpdateData(TRUE);

	build_example();

	UpdateData(FALSE);
}

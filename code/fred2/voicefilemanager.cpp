// VoiceFileManager.cpp : implementation file
//

#include "stdafx.h"
#include "fred.h"
#include "VoiceFileManager.h"
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
// VoiceFileManager dialog


VoiceFileManager::VoiceFileManager(CWnd* pParent /*=NULL*/)
	: CDialog(VoiceFileManager::IDD, pParent)
{
	//{{AFX_DATA_INIT(VoiceFileManager)
	m_abbrev_briefing = _T("");
	m_abbrev_campaign = _T("");
	m_abbrev_command_briefing = _T("");
	m_abbrev_debriefing = _T("");
	m_abbrev_message = _T("");
	m_example = _T("");
	m_abbrev_mission = _T("");
	//}}AFX_DATA_INIT
}


void VoiceFileManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VoiceFileManager)
	DDX_Text(pDX, IDC_ABBREV_BRIEFING, m_abbrev_briefing);
	DDX_Text(pDX, IDC_ABBREV_CAMPAIGN, m_abbrev_campaign);
	DDX_Text(pDX, IDC_ABBREV_COMMAND_BRIEFING, m_abbrev_command_briefing);
	DDX_Text(pDX, IDC_ABBREV_DEBRIEFING, m_abbrev_debriefing);
	DDX_Text(pDX, IDC_ABBREV_MESSAGE, m_abbrev_message);
	DDX_Text(pDX, IDC_EXAMPLE, m_example);
	DDX_Text(pDX, IDC_ABBREV_MISSION, m_abbrev_mission);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VoiceFileManager, CDialog)
	//{{AFX_MSG_MAP(VoiceFileManager)
	ON_BN_CLICKED(IDC_SEP_DASHES, OnSepDashes)
	ON_BN_CLICKED(IDC_SEP_NOTHING, OnSepNothing)
	ON_BN_CLICKED(IDC_SEP_UNDERSCORES, OnSepUnderscores)
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
	ON_BN_CLICKED(IDC_AUTOGENERATE, OnAutogenerate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VoiceFileManager message handlers

BOOL VoiceFileManager::OnInitDialog()
{
	((CButton *) GetDlgItem(IDC_SEP_UNDERSCORES))->SetCheck(1);	

	return TRUE;
}

int calc_digits(int size)
{
	int x = (int) floor(log10((double) size)) + 1;

	return (x > 2) ? x : 2;
}

CString VoiceFileManager::get_separator()
{
	// assign separator
	if (((CButton *) GetDlgItem(IDC_SEP_UNDERSCORES))->GetCheck())
		return "_";
	else if (((CButton *) GetDlgItem(IDC_SEP_DASHES))->GetCheck())
		return "-";
	else
		return "";
}

void VoiceFileManager::OnAutogenerate() 
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
}

CString VoiceFileManager::generate_filename(CString section, int number, int digits)
{
	if (section == "")
		return "none.wav";

	int i;
	CString str = "";
	CString num;
	CString sep = get_separator();

	// build prefix
	if (m_abbrev_campaign != "")
		str = str + m_abbrev_campaign + sep;
	if (m_abbrev_mission != "")
		str = str + m_abbrev_mission + sep;
	str = str + section;
	
	// build number
	num.Format("%d", number);;
	digits -= num.GetLength();
	for (i = 0; i < digits; i++)
	{
		num = "0" + num;
	}
	str = str + num;

	// build suffix
	str = str + ".wav";

	return str;
}

void VoiceFileManager::OnSepDashes() 
{
	UpdateData(TRUE);

	build_example();

	UpdateData(FALSE);
}

void VoiceFileManager::OnSepNothing() 
{
	UpdateData(TRUE);

	build_example();

	UpdateData(FALSE);
}

void VoiceFileManager::OnSepUnderscores() 
{
	UpdateData(TRUE);

	build_example();

	UpdateData(FALSE);
}

void VoiceFileManager::OnSetfocusAbbrevBriefing() 
{
	UpdateData(TRUE);

	build_example(m_abbrev_briefing);

	UpdateData(FALSE);
}

void VoiceFileManager::OnSetfocusAbbrevCampaign() 
{
	UpdateData(TRUE);

	build_example();

	UpdateData(FALSE);
}

void VoiceFileManager::OnSetfocusAbbrevCommandBriefing() 
{
	UpdateData(TRUE);

	build_example(m_abbrev_command_briefing);

	UpdateData(FALSE);
}

void VoiceFileManager::OnSetfocusAbbrevDebriefing() 
{
	UpdateData(TRUE);

	build_example(m_abbrev_debriefing);

	UpdateData(FALSE);
}

void VoiceFileManager::OnSetfocusAbbrevMessage() 
{
	UpdateData(TRUE);

	build_example(m_abbrev_message);

	UpdateData(FALSE);
}

void VoiceFileManager::OnSetfocusAbbrevMission() 
{
	UpdateData(TRUE);

	build_example();

	UpdateData(FALSE);
}

void VoiceFileManager::OnChangeAbbrevBriefing() 
{
	UpdateData(TRUE);

	build_example(m_abbrev_briefing);

	UpdateData(FALSE);
}

void VoiceFileManager::OnChangeAbbrevCampaign() 
{
	UpdateData(TRUE);

	build_example();

	UpdateData(FALSE);
}

void VoiceFileManager::OnChangeAbbrevCommandBriefing() 
{
	UpdateData(TRUE);

	build_example(m_abbrev_command_briefing);

	UpdateData(FALSE);
}

void VoiceFileManager::OnChangeAbbrevDebriefing() 
{
	UpdateData(TRUE);

	build_example(m_abbrev_debriefing);

	UpdateData(FALSE);
}

void VoiceFileManager::OnChangeAbbrevMessage() 
{
	UpdateData(TRUE);

	build_example(m_abbrev_message);

	UpdateData(FALSE);
}

void VoiceFileManager::OnChangeAbbrevMission() 
{
	UpdateData(TRUE);

	build_example();

	UpdateData(FALSE);
}

void VoiceFileManager::build_example()
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

void VoiceFileManager::build_example(CString section)
{
	if (section == "")
	{
		m_example = "";
		return;
	}

	m_example = generate_filename(section, 1, 2);
}

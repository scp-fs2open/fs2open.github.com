// VoiceActingManager.cpp : implementation file
//

#include "stdafx.h"
#include <string.h>
#include "fred.h"
#include "freddoc.h"
#include "VoiceActingManager.h"
#include "globalincs/vmallocator.h"
#include "missionui/missioncmdbrief.h"
#include "mission/missionbriefcommon.h"
#include "mission/missionmessage.h"
#include "mission/missiongoals.h"
#include "hud/hudtarget.h"
#include "parse/sexp.h"
#include "iff_defs/iff_defs.h"
#include "mission/missiongoals.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define INVALID_MESSAGE ((MMessage*)SIZE_T_MAX)

// to keep track of data
char Voice_abbrev_briefing[NAME_LENGTH];
char Voice_abbrev_campaign[NAME_LENGTH];
char Voice_abbrev_command_briefing[NAME_LENGTH];
char Voice_abbrev_debriefing[NAME_LENGTH];
char Voice_abbrev_message[NAME_LENGTH];
char Voice_abbrev_mission[NAME_LENGTH];
bool Voice_no_replace_filenames;
char Voice_script_entry_format[NOTES_LENGTH];
int Voice_export_selection;
bool Voice_group_messages;

constexpr int WINGMAN_PERSONAS = 0;
constexpr int NON_WINGMAN_PERSONAS = 1;
constexpr int SPECIFIC_PERSONAS_START_AT = 2;


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
	m_use_sender_in_filename = FALSE;
	m_example = _T("");
	m_no_replace = FALSE;
	m_script_entry_format = _T("");
	m_export_everything = FALSE;
	m_export_command_briefings = FALSE;
	m_export_briefings = FALSE;
	m_export_debriefings = FALSE;
	m_export_messages = FALSE;
	m_group_messages = FALSE;
	m_which_persona_to_sync = 0;
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
	DDX_Check(pDX, IDC_NO_REPLACE, m_no_replace);
	DDX_Check(pDX, IDC_INCLUDE_SENDER, m_use_sender_in_filename);
	DDX_Text(pDX, IDC_ENTRY_FORMAT, m_script_entry_format);
	DDX_Check(pDX, IDC_EXPORT_EVERYTHING, m_export_everything);
	DDX_Check(pDX, IDC_EXPORT_COMMAND_BRIEFINGS, m_export_command_briefings);
	DDX_Check(pDX, IDC_EXPORT_BRIEFINGS, m_export_briefings);
	DDX_Check(pDX, IDC_EXPORT_DEBRIEFINGS, m_export_debriefings);
	DDX_Check(pDX, IDC_EXPORT_MESSAGES, m_export_messages);
	DDX_Check(pDX, IDC_GROUP_MESSAGES, m_group_messages);
	DDX_CBIndex(pDX, IDC_WHICH_TO_SYNC, m_which_persona_to_sync);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VoiceActingManager, CDialog)
	//{{AFX_MSG_MAP(VoiceActingManager)
	ON_WM_CLOSE()
	ON_EN_SETFOCUS(IDC_ABBREV_BRIEFING, OnSetfocusAbbrevBriefing)
	ON_EN_SETFOCUS(IDC_ABBREV_CAMPAIGN, OnSetfocusAbbrevCampaign)
	ON_EN_SETFOCUS(IDC_ABBREV_COMMAND_BRIEFING, OnSetfocusAbbrevCommandBriefing)
	ON_EN_SETFOCUS(IDC_ABBREV_DEBRIEFING, OnSetfocusAbbrevDebriefing)
	ON_EN_SETFOCUS(IDC_ABBREV_MESSAGE, OnSetfocusAbbrevMessage)
	ON_EN_SETFOCUS(IDC_ABBREV_MISSION, OnSetfocusAbbrevMission)
	ON_CBN_SETFOCUS(IDC_SUFFIX, OnSetfocusSuffix)
	ON_EN_CHANGE(IDC_ABBREV_BRIEFING, OnChangeAbbrevBriefing)
	ON_EN_CHANGE(IDC_ABBREV_CAMPAIGN, OnChangeAbbrevCampaign)
	ON_EN_CHANGE(IDC_ABBREV_COMMAND_BRIEFING, OnChangeAbbrevCommandBriefing)
	ON_EN_CHANGE(IDC_ABBREV_DEBRIEFING, OnChangeAbbrevDebriefing)
	ON_EN_CHANGE(IDC_ABBREV_MESSAGE, OnChangeAbbrevMessage)
	ON_EN_CHANGE(IDC_ABBREV_MISSION, OnChangeAbbrevMission)
	ON_CBN_SELCHANGE(IDC_SUFFIX, OnChangeOtherSuffix)
	ON_BN_CLICKED(IDC_NO_REPLACE, OnChangeNoReplace)
	ON_BN_CLICKED(IDC_GENERATE_FILE_NAMES, OnGenerateFileNames)
	ON_BN_CLICKED(IDC_GENERATE_SCRIPT, OnGenerateScript)
	ON_BN_CLICKED(IDC_EXPORT_EVERYTHING, OnExportEverything)
	ON_BN_CLICKED(IDC_EXPORT_COMMAND_BRIEFINGS, OnExportCommandBriefings)
	ON_BN_CLICKED(IDC_EXPORT_BRIEFINGS, OnExportBriefings)
	ON_BN_CLICKED(IDC_EXPORT_DEBRIEFINGS, OnExportDebriefings)
	ON_BN_CLICKED(IDC_EXPORT_MESSAGES, OnExportMessages)
	ON_BN_CLICKED(IDC_INCLUDE_SENDER, OnBnClickedIncludeSender)
	ON_CBN_SELCHANGE(IDC_WHICH_TO_SYNC, OnChangeWhichToSync)
	ON_BN_CLICKED(IDC_MESSAGE_PERSONAS_TO_SHIPS, OnCopyMessagePersonasToShips)
	ON_BN_CLICKED(IDC_SHIP_PERSONAS_TO_MESSAGES, OnCopyShipPersonasToMessages)
	ON_BN_CLICKED(IDC_SET_HEAD_ANIS_USING_MESSAGES_TBL, OnSetHeadANIsUsingMessagesTbl)
	ON_BN_CLICKED(IDC_CLEAR_PERSONAS_FROM_NON_SENDERS, OnClearPersonasFromNonSenders)
	ON_BN_CLICKED(IDC_CHECK_ANY_WINGMAN_PERSONAS, OnCheckAnyWingmanPersonas)
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

	// this text is too long for the .rc file, so set it here
	GetDlgItem(IDC_ENTRY_FORMAT_DESC)->SetWindowText(
		"$name - name of the message\r\n"
		"$filename - name of the message file\r\n"
		"$message - text of the message\r\n"
		"$persona - persona of the sender\r\n"
		"$sender - name of the sender\r\n"
		"$note - message notes\r\n\r\n"
		"Note that $persona and $sender will only appear for the Message section."
	);

	// load saved data for file names
	m_abbrev_briefing = _T(Voice_abbrev_briefing);
	m_abbrev_campaign = _T(Voice_abbrev_campaign);
	m_abbrev_command_briefing = _T(Voice_abbrev_command_briefing);
	m_abbrev_debriefing = _T(Voice_abbrev_debriefing);
	m_abbrev_message = _T(Voice_abbrev_message);
	m_abbrev_mission = _T(Voice_abbrev_mission);
	m_no_replace = Voice_no_replace_filenames;

	// load saved data for script
	m_script_entry_format = _T(Voice_script_entry_format);
	m_export_everything = m_export_command_briefings = m_export_briefings = m_export_debriefings = m_export_messages = FALSE;
	if (Voice_export_selection == 1)
		m_export_command_briefings = TRUE;
	else if (Voice_export_selection == 2)
		m_export_briefings = TRUE;
	else if (Voice_export_selection == 3)
		m_export_debriefings = TRUE;
	else if (Voice_export_selection == 4)
		m_export_messages = TRUE;
	else
		m_export_everything = TRUE;
	m_group_messages = Voice_group_messages;
	
	CButton *button = ((CButton *) GetDlgItem(IDC_GROUP_MESSAGES));
	button->EnableWindow(m_export_everything || m_export_messages);

	// populate persona dropdown
	box = ((CComboBox *) GetDlgItem(IDC_WHICH_TO_SYNC));
	box->AddString("<Wingman Personas>");
	box->AddString("<Non-Wingman Personas>");
	for (const auto& persona : Personas)
		box->AddString(persona.name);

	UpdateData(FALSE);

	return TRUE;
}

void VoiceActingManager::OnClose()
{
	UpdateData(TRUE);

	// save data for file names
	strcpy_s(Voice_abbrev_briefing, m_abbrev_briefing);
	strcpy_s(Voice_abbrev_campaign, m_abbrev_campaign);
	strcpy_s(Voice_abbrev_command_briefing, m_abbrev_command_briefing);
	strcpy_s(Voice_abbrev_debriefing, m_abbrev_debriefing);
	strcpy_s(Voice_abbrev_message, m_abbrev_message);
	strcpy_s(Voice_abbrev_mission, m_abbrev_mission);
	Voice_no_replace_filenames = (m_no_replace == TRUE) ? true : false;

	// save data for script
	strcpy_s(Voice_script_entry_format, m_script_entry_format);
	if (m_export_command_briefings)
		Voice_export_selection = 1;
	else if (m_export_briefings)
		Voice_export_selection = 2;
	else if (m_export_debriefings)
		Voice_export_selection = 3;
	else if (m_export_messages)
		Voice_export_selection = 4;
	else
		Voice_export_selection = 0;
	Voice_group_messages = (m_group_messages == TRUE) ? true : false;

	CDialog::OnClose();
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

	m_example = generate_filename(section, 1, 2, INVALID_MESSAGE);
}

CString VoiceActingManager::generate_filename(CString section, int number, int digits, const MMessage *message)
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


	const CString suffix = get_suffix();
	// append sender name if supposed to and I have been passed a message
	// to get the sender from
	if ( message != NULL && m_use_sender_in_filename ) {
		size_t allow_to_copy = NAME_LENGTH - suffix.GetLength() - str.GetLength();
		char sender[NAME_LENGTH];
		if ( message == INVALID_MESSAGE ) {
			strcpy_s(sender, "Alpha 1");
		} else {
			get_valid_sender(sender, sizeof(sender), message);
		}

		// truncate sender to that we don't overflow filename
		sender[allow_to_copy] = '\0';
		size_t j;
		for( j = 0; sender[j] != '\0'; j++ ) {
			// lower case letter
			sender[j] = SCP_tolower(sender[j]);

			// replace any non alpha numeric with a underscore
			if ( !isalnum( sender[j] ) )
				sender[j] = '_';
		}

		// flatten multiple underscores
		j = 1;
		while( sender[j] != '\0' ) {
			if ( sender[j-1] == '_' && sender[j] == '_' ) {
				size_t k;
				for (k = j + 1; sender[k] != '\0'; k++ )
					sender[k - 1] = sender[k];
				sender[k - 1] = '\0';
			} else {
				// only increment on rounds when I am not moving the string down
				j++;
			}
		}
		str = str + sender;
	}

	// suffix
	str = str + get_suffix();

	Assert( str.GetLength() < NAME_LENGTH );

	return str;
}

#define STRCPY_IF_MODIFIED(dest, src)	{ auto temp = src; if (stricmp(dest, temp)) { strcpy(dest, temp); num_modified++; } }
#define COPY_IF_MODIFIED(dest, src)		{ auto temp = src; if (dest != temp) { dest = temp; num_modified++; } }
#define STRDUP_IF_MODIFIED(dest, src)	{ auto temp = src; if (dest == nullptr || stricmp(dest, temp)) { if (dest != nullptr) { free(dest); } dest = strdup(temp); num_modified++; } }

void VoiceActingManager::OnGenerateFileNames() 
{
	int i;
	int digits;
	size_t num_modified = 0;

	// stuff data to variables
	UpdateData(TRUE);

	// command briefings
	digits = calc_digits(Cmd_briefs[0].num_stages);
	for (i = 0; i < Cmd_briefs[0].num_stages; i++)
	{
		char *filename = Cmd_briefs[0].stage[i].wave_filename;

		// generate only if we're replacing or if it has a replaceable name
		if (!m_no_replace || !strlen(filename) || message_filename_is_generic(filename))
			STRCPY_IF_MODIFIED(filename, generate_filename(m_abbrev_command_briefing, i + 1, digits));
	}

	// briefings
	digits = calc_digits(Briefings[0].num_stages);
	for (i = 0; i < Briefings[0].num_stages; i++)
	{
		char *filename = Briefings[0].stages[i].voice;

		// generate only if we're replacing or if it has a replaceable name
		if (!m_no_replace || !strlen(filename) || message_filename_is_generic(filename))
			STRCPY_IF_MODIFIED(filename, generate_filename(m_abbrev_briefing, i + 1, digits));
	}

	// debriefings
	digits = calc_digits(Debriefings[0].num_stages);
	for (i = 0; i < Debriefings[0].num_stages; i++)
	{
		char *filename = Debriefings[0].stages[i].voice;

		// generate only if we're replacing or if it has a replaceable name
		if (!m_no_replace || !strlen(filename) || message_filename_is_generic(filename))
			STRCPY_IF_MODIFIED(filename, generate_filename(m_abbrev_debriefing, i + 1, digits));
	}

	// messages
	digits = calc_digits(Num_messages - Num_builtin_messages);
	for (i = 0; i < Num_messages - Num_builtin_messages; i++)
	{
		MMessage *message = &Messages[i + Num_builtin_messages];
		const char *filename = message->wave_info.name;

		// generate only if we're replacing or if it has a replaceable name
		if (!m_no_replace || !strlen(filename) || message_filename_is_generic(filename))
			STRDUP_IF_MODIFIED(message->wave_info.name, generate_filename(m_abbrev_message, i + 1, digits, message));
	}

	if ( num_modified > 0 ) {
		// Tell FRED that we actually modified something
		set_modified(TRUE);
	}

	// notify user that we are done and how many filenames were changed
	SCP_string message;
	sprintf(message, "File name generation complete.  " SIZE_T_ARG " file %s modified.", num_modified, (num_modified == 1) ? "name was" : "names were");
	MessageBox(message.c_str(), "Voice Acting Manager");
}

void VoiceActingManager::OnGenerateScript()
{
	char pathname[256];

	// stuff data to variables
	UpdateData(TRUE);

	// prompt to save script
	CFileDialog dlg(FALSE, "txt", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Text files (*.txt)|*.txt||");
	if (dlg.DoModal() != IDOK)
		return;

	CString dlgPathName = dlg.GetPathName( );
	string_copy(pathname, dlgPathName, 256 - 1);
	fp = cfopen(pathname, "wt");
	if (!fp)
	{
		MessageBox("Can't open file to save.", "Error!");
		return;
	}

	fout("%s\n", Mission_filename);
	fout("%s\n\n", The_mission.name);

	if (m_export_everything || m_export_command_briefings)
	{
		fout("\n\nCommand Briefings\n-----------------\n\n");

		for (int i = 0; i < Cmd_briefs[0].num_stages; i++)
		{
			CString entry = m_script_entry_format;
			entry.Replace("\r\n", "\n");

			cmd_brief_stage *stage = &Cmd_briefs[0].stage[i];
			entry.Replace("$filename", stage->wave_filename);
			entry.Replace("$message", stage->text.c_str());
			entry.Replace("$persona", "<no persona specified>");
			entry.Replace("$sender", "<no sender specified>");
			entry.Replace("$name", "<no name specified>");
			entry.Replace("$note", "<no note specified>");

			fout("%s\n\n\n", (char *) (LPCTSTR) entry);
		}
	}

	if (m_export_everything || m_export_briefings)
	{
		fout("\n\nBriefings\n---------\n\n");

		for (int i = 0; i < Briefings[0].num_stages; i++)
		{
			CString entry = m_script_entry_format;
			entry.Replace("\r\n", "\n");

			brief_stage *stage = &Briefings[0].stages[i];
			entry.Replace("$filename", stage->voice);
			entry.Replace("$message", stage->text.c_str());
			entry.Replace("$persona", "<no persona specified>");
			entry.Replace("$sender", "<no sender specified>");
			entry.Replace("$name", "<no name specified>");
			entry.Replace("$note", "<no note specified>");

			fout("%s\n\n\n", (char *) (LPCTSTR) entry);
		}
	}

	if (m_export_everything || m_export_debriefings)
	{
		fout("\n\nDebriefings\n-----------\n\n");

		for (int i = 0; i < Debriefings[0].num_stages; i++)
		{
			CString entry = m_script_entry_format;
			entry.Replace("\r\n", "\n");

			debrief_stage *stage = &Debriefings[0].stages[i];
			entry.Replace("$filename", stage->voice);
			entry.Replace("$message", stage->text.c_str());
			entry.Replace("$persona", "<no persona specified>");
			entry.Replace("$sender", "<no sender specified>");
			entry.Replace("$name", "<no name specified>");
			entry.Replace("$note", "<no note specified>");
	
			fout("%s\n\n\n", (char *) (LPCTSTR) entry);
		}
	}

	if (m_export_everything || m_export_messages)
	{
		fout("\n\nMessages\n--------\n\n");

		if (m_group_messages)
		{
			SCP_vector<int> message_indexes;
			for (int i = 0; i < Num_messages - Num_builtin_messages; i++)
				message_indexes.push_back(i + Num_builtin_messages);

			group_message_indexes(message_indexes);

			for (size_t index = 0; index < message_indexes.size(); index++)
			{
				MMessage *message = &Messages[message_indexes[index]];
				export_one_message(message);
			}
		}
		else
		{
			for (int i = 0; i < Num_messages - Num_builtin_messages; i++)
			{
				MMessage *message = &Messages[i + Num_builtin_messages];
				export_one_message(message);
			}
		}
	}

	cfclose(fp);

	// notify
	MessageBox("Script generation complete.", "Woohoo!");
}

void VoiceActingManager::export_one_message(const MMessage *message)
{
	CString entry = m_script_entry_format;
	entry.Replace("\r\n", "\n");

	entry.Replace("$name", message->name);

	// replace file name
	entry.Replace("$filename", message->wave_info.name);

	// replace message
	entry.Replace("$message", message->message);

	entry.Replace("$note", message->note.c_str());

	// determine and replace persona
	if (message->persona_index >= 0)
		entry.Replace("$persona", Personas[message->persona_index].name);
	else
		entry.Replace("$persona", "<none>");

	// determine sender
	char sender[NAME_LENGTH+1];
	get_valid_sender(sender, sizeof(sender), message);

	// replace sender (but print #Command as Command)
	if (*sender == '#')
		entry.Replace("$sender", &sender[1]);
	else
		entry.Replace("$sender", sender);

	fout("%s\n\n\n", (char *) (LPCTSTR) entry);
}

/** Passed sender string will have either have the senders name
or '\<none\>'*/
void VoiceActingManager::get_valid_sender(char *sender, size_t sender_size, const MMessage *message, int *sender_shipnum, bool *is_command)
{
	Assert( sender != NULL );
	Assert( message != NULL );

	memset(sender, 0, sender_size);
	strncpy(sender, get_message_sender(message), sender_size - 1);

	if (!strcmp("#Command", sender))
	{
		if (is_command)
			*is_command = true;

		// check if we're overriding #Command
		if (The_mission.flags[Mission::Mission_Flags::Override_hashcommand])
		{
			memset(sender, 0, sender_size);
			strncpy(sender, The_mission.command_sender, sender_size - 1);
		}
	}
	else
	{
		if (is_command)
			*is_command = false;
	}

	// strip hash if present
	if ( sender[0] == '#' )
	{
		size_t i = 1;
		for(; sender[i] != '\0'; i++ ) {
			sender[i-1] = sender[i];
		}
		sender[i-1] = '\0';
	}

	int shipnum = ship_name_lookup(sender, 1); // The player's ship is valid for this search.

	if (sender_shipnum != nullptr)
		*sender_shipnum = shipnum;

	if (shipnum >= 0)
	{
		ship *shipp = &Ships[shipnum];

		// we may have to use the callsign
		if (*Fred_callsigns[shipnum])
		{
			hud_stuff_ship_callsign(sender, shipp);
		}
		// account for hidden ship names
		else if ( ((Iff_info[shipp->team].flags & IFFF_WING_NAME_HIDDEN) && (shipp->wingnum != -1)) || (shipp->flags[Ship::Ship_Flags::Hide_ship_name]) )
		{
			hud_stuff_ship_class(sender, shipp);
		}
		// use the regular sender display name
		else
		{
			memset(sender, 0, sender_size);
			strncpy(sender, shipp->get_display_name(), sender_size - 1);
		}
	}
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

void VoiceActingManager::OnSetfocusSuffix()
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

void VoiceActingManager::OnChangeNoReplace()
{
	UpdateData(TRUE);
}

int VoiceActingManager::fout(const char *format, ...)
{
	SCP_string str;
	va_list args;
	
	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);

	cfputs(str.c_str(), fp);
	return 0;
}

// Loops through all the sexps and finds the sender of the specified message.  This assumes there is only one possible
// sender of the message, which is probably nearly always true (especially for voice-acted missions).
const char *VoiceActingManager::get_message_sender(const MMessage *message)
{
	int i;

	for (i = 0; i < Num_sexp_nodes; i++)
	{
		if (Sexp_nodes[i].type == SEXP_NOT_USED)
			continue;

		// stuff
		int op = get_operator_const(i);
		int n = CDR(i);

		// find the message sexps
		if (op == OP_SEND_MESSAGE)
		{
			// the first argument is the sender; the third is the message
			if (!strcmp(message->name, Sexp_nodes[CDDR(n)].text))
				return Sexp_nodes[n].text;
		}
		else if (op == OP_SEND_MESSAGE_LIST || op == OP_SEND_MESSAGE_CHAIN)
		{
			// skip the event argument
			if (op == OP_SEND_MESSAGE_CHAIN)
				n = CDR(n);

			// check the argument list
			while (n != -1)
			{
				// as before
				if (!strcmp(message->name, Sexp_nodes[CDDR(n)].text))
					return Sexp_nodes[n].text;

				// iterate along the list
				n = CDDDDR(n);
			}
		}
		else if (op == OP_SEND_RANDOM_MESSAGE)
		{
			// as before, sort of
			char *sender = Sexp_nodes[n].text;

			// check the argument list
			n = CDDR(n);
			while (n != -1)
			{
				if (!strcmp(message->name, Sexp_nodes[n].text))
					return sender;

				// iterate along the list
				n = CDR(n);
			}
		}
		else if (op == OP_TRAINING_MSG)
		{
			// just check the message
			if (!strcmp(message->name, Sexp_nodes[n].text))
				return "Training Message";
		}
	}

	return "<none>";
}

void VoiceActingManager::group_message_indexes(SCP_vector<int> &message_indexes)
{
#ifndef NDEBUG
	size_t initial_size = message_indexes.size();
#endif

	SCP_vector<int> temp_message_indexes = message_indexes;
	message_indexes.clear();

	// add all messages found in send-message-list or send-random-message node trees
	for (const auto &event: Mission_events)
		group_message_indexes_in_tree(event.formula, temp_message_indexes, message_indexes);

	// add remaining messages
	for (size_t index = 0; index < temp_message_indexes.size(); index++)
		message_indexes.push_back(temp_message_indexes[index]);

#ifndef NDEBUG
	if (initial_size > message_indexes.size())
	{
		Warning(LOCATION, "Initial size is greater than size after sorting!");
	}
	else if (initial_size < message_indexes.size())
	{
		Warning(LOCATION, "Initial size is less than size after sorting!");
	}
#endif
}

void VoiceActingManager::group_message_indexes_in_tree(int node, SCP_vector<int> &source_list, SCP_vector<int> &destination_list)
{
	int op, n;

	if (node < 0)
		return;
	if (Sexp_nodes[node].type == SEXP_NOT_USED)
		return;

	// stuff
	op = get_operator_const(node);
	n = CDR(node);

	if (op == OP_SEND_MESSAGE_LIST || op == OP_SEND_MESSAGE_CHAIN)
	{
		// skip the event argument
		if (op == OP_SEND_MESSAGE_CHAIN)
			n = CDR(n);

		// check the argument list
		while (n != -1)
		{
			// the third argument is a message
			char *message_name = Sexp_nodes[CDDR(n)].text;

			// check source messages
			for (size_t i = 0; i < source_list.size(); i++)
			{
				if (!strcmp(message_name, Messages[source_list[i]].name))
				{
					// move it from source to destination
					destination_list.push_back(source_list[i]);
					source_list.erase(source_list.begin() + i);
					break;
				}
			}

			// iterate along the list
			n = CDDDDR(n);
		}
	}
	else if (op == OP_SEND_RANDOM_MESSAGE)
	{
		// check the argument list
		n = CDDR(n);
		while (n != -1)
		{
			// each argument from this point on is a message
			char *message_name = Sexp_nodes[n].text;

			// check source messages
			for (size_t i = 0; i < source_list.size(); i++)
			{
				if (!strcmp(message_name, Messages[source_list[i]].name))
				{
					// move it from source to destination
					destination_list.push_back(source_list[i]);
					source_list.erase(source_list.begin() + i);
					break;
				}
			}

			// iterate along the list
			n = CDR(n);
		}
	}

	// iterate on first element
	group_message_indexes_in_tree(CAR(node), source_list, destination_list);

	// iterate on rest of elements
	group_message_indexes_in_tree(CDR(node), source_list, destination_list);
}

void VoiceActingManager::OnExportEverything()
{
	CButton *button = ((CButton *) GetDlgItem(IDC_GROUP_MESSAGES));
	button->EnableWindow(TRUE);
}

void VoiceActingManager::OnExportCommandBriefings()
{
	CButton *button = ((CButton *) GetDlgItem(IDC_GROUP_MESSAGES));
	button->EnableWindow(FALSE);
}

void VoiceActingManager::OnExportBriefings()
{
	CButton *button = ((CButton *) GetDlgItem(IDC_GROUP_MESSAGES));
	button->EnableWindow(FALSE);
}

void VoiceActingManager::OnExportDebriefings()
{
	CButton *button = ((CButton *) GetDlgItem(IDC_GROUP_MESSAGES));
	button->EnableWindow(FALSE);
}

void VoiceActingManager::OnExportMessages()
{
	CButton *button = ((CButton *) GetDlgItem(IDC_GROUP_MESSAGES));
	button->EnableWindow(TRUE);
}

void VoiceActingManager::OnBnClickedIncludeSender()
{
	UpdateData(TRUE);

	build_example();

	UpdateData(FALSE);
}

void VoiceActingManager::OnChangeWhichToSync()
{
	UpdateData(TRUE);
}

bool VoiceActingManager::check_persona(int persona)
{
	Assertion(SCP_vector_inbounds(Personas, persona), "The persona index provided to check_persona() is not in range!");

	if (m_which_persona_to_sync == WINGMAN_PERSONAS)
	{
		return (Personas[persona].flags & PERSONA_FLAG_WINGMAN) != 0;
	}
	else if (m_which_persona_to_sync == NON_WINGMAN_PERSONAS)
	{
		return (Personas[persona].flags & PERSONA_FLAG_WINGMAN) == 0;
	}
	else
	{
		int real_persona_to_sync = m_which_persona_to_sync - SPECIFIC_PERSONAS_START_AT;
		Assertion(SCP_vector_inbounds(Personas, real_persona_to_sync), "The m_which_persona_to_sync dropdown index is not in range!");
		return real_persona_to_sync == persona;
	}
}

void VoiceActingManager::OnCopyPersonas(bool messages_to_ships)
{
	char sender_buf[NAME_LENGTH];
	int sender_shipnum;
	bool is_command;
	size_t num_modified = 0;

	SCP_unordered_set<int> already_assigned;
	SCP_string inconsistent_copy_msg;

	// go through all messages in the mission
	for (int i = 0; i < Num_messages - Num_builtin_messages; i++)
	{
		auto message = &Messages[i + Num_builtin_messages];

		// find whoever sent this message
		get_valid_sender(sender_buf, NAME_LENGTH, message, &sender_shipnum, &is_command);
		auto sender_shipp = (sender_shipnum < 0) ? nullptr : &Ships[sender_shipnum];

		// NOTE: It is possible for a ship that is behaving as a "command source" to have a different persona than the mission's "command persona".
		// If this is so, any messages sent by #Command will have the mission's "command persona", and any messages sent by the ship will have the ship's persona.

		int persona_to_copy;
		if (messages_to_ships)
			persona_to_copy = message->persona_index;
		else if (is_command)
			persona_to_copy = The_mission.command_persona;
		else if (sender_shipp)
			persona_to_copy = sender_shipp->persona_index;
		else
			persona_to_copy = -1;

		// don't copy None, and only copy the personas we want
		if (persona_to_copy >= 0 && check_persona(persona_to_copy))
		{
			if (messages_to_ships && sender_shipp)
			{
				if (already_assigned.count(sender_shipnum) > 0 && sender_shipp->persona_index != persona_to_copy)
				{
					inconsistent_copy_msg += "\n\u2022 ";
					inconsistent_copy_msg += sender_shipp->ship_name;
				}
				already_assigned.insert(sender_shipnum);

				COPY_IF_MODIFIED(sender_shipp->persona_index, persona_to_copy);
			}
			else if (!messages_to_ships)
			{
				if (already_assigned.count(i) > 0 && message->persona_index != persona_to_copy)
				{
					inconsistent_copy_msg += "\n\u2022 ";
					inconsistent_copy_msg += message->name;
				}
				already_assigned.insert(i);

				COPY_IF_MODIFIED(message->persona_index, persona_to_copy);
			}
		}
	}

	if (num_modified > 0)
		set_modified(TRUE);

	SCP_string message;
	if (messages_to_ships)
		sprintf(message, "Personas have been copied from messages to ships.  " SIZE_T_ARG " %s modified.", num_modified, (num_modified == 1) ? "ship was" : "ships were");
	else
		sprintf(message, "Personas have been copied from ships to messages.  " SIZE_T_ARG " %s modified.", num_modified, (num_modified == 1) ? "message was" : "messages were");

	if (!inconsistent_copy_msg.empty())
	{
		if (messages_to_ships)
			message += "\n\nThe following ships send messages with inconsistent personas.  You may want to review them.\n";
		else
			message += "\n\nThe following messages are sent by ships with inconsistent personas.  You may want to review them.\n";
		message += inconsistent_copy_msg;
	}

	MessageBox(message.c_str(), "Voice Acting Manager");
}

void VoiceActingManager::OnClearPersonasFromNonSenders()
{
	SCP_unordered_set<int> all_senders;

	char sender_buf[NAME_LENGTH];
	int sender_shipnum;
	size_t num_modified = 0;

	// go through all messages in the mission
	for (int i = 0; i < Num_messages - Num_builtin_messages; i++)
	{
		auto message = &Messages[i + Num_builtin_messages];

		// find whoever sent this message
		get_valid_sender(sender_buf, NAME_LENGTH, message, &sender_shipnum);

		// if it's a ship, save the shipnum
		if (sender_shipnum >= 0)
			all_senders.insert(sender_shipnum);
	}

	// go through all ships in the mission
	for (auto objp : list_range(&obj_used_list))
	{
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP))
		{
			// for ships that aren't message senders
			if (all_senders.count(objp->instance) == 0)
			{
				// only for the personas we want
				if ((Ships[objp->instance].persona_index >= 0) && check_persona(Ships[objp->instance].persona_index))
				{
					// clear the persona
					COPY_IF_MODIFIED(Ships[objp->instance].persona_index, -1);
				}
			}
		}
	}

	if (num_modified > 0)
		set_modified(TRUE);

	SCP_string message;
	sprintf(message, "Personas have been cleared from all ships that do not send messages.  " SIZE_T_ARG " %s modified.", num_modified, (num_modified == 1) ? "ship was" : "ships were");
	MessageBox(message.c_str(), "Voice Acting Manager");
}

void VoiceActingManager::OnCopyMessagePersonasToShips()
{
	OnCopyPersonas(true);
}

void VoiceActingManager::OnCopyShipPersonasToMessages()
{
	OnCopyPersonas(false);
}

void VoiceActingManager::OnSetHeadANIsUsingMessagesTbl()
{
	size_t num_modified = 0;

	// go through all messages in the mission
	for (int i = 0; i < Num_messages - Num_builtin_messages; i++)
	{
		auto message = &Messages[i + Num_builtin_messages];

		// only messages with personas
		if (message->persona_index < 0)
			continue;

		// only the personas we want
		if (!check_persona(message->persona_index))
			continue;

		// find the corresponding head for this persona
		bool found = false;
		for (int j = 0; j < Num_builtin_messages; j++)
		{
			auto builtin_message = &Messages[j];

			if (message->persona_index == builtin_message->persona_index)
			{
				// either assign the correct head from scratch, or change the head to the correct one
				STRDUP_IF_MODIFIED(message->avi_info.name, builtin_message->avi_info.name);

				// done searching
				found = true;
				break;
			}
		}

		if (!found)
		{
			Warning(LOCATION, "Persona index %d was not found in list of messages.tbl personas!", message->persona_index);
		}
	}

	if (num_modified > 0)
		set_modified(TRUE);

	SCP_string message;
	sprintf(message, "Message head ANIs have been assigned from builtin messages.  " SIZE_T_ARG " %s modified.", num_modified, (num_modified == 1) ? "message was" : "messages were");
	MessageBox(message.c_str(), "Voice Acting Manager");
}

void VoiceActingManager::OnCheckAnyWingmanPersonas()
{
	char sender_buf[NAME_LENGTH];
	SCP_string output_msg;
	bool any_any_wingman = false;
	int issue_count = 0;

	// go through all messages in the mission
	for (int i = 0; i < Num_messages - Num_builtin_messages; i++)
	{
		auto message = &Messages[i + Num_builtin_messages];

		// find whoever sent this message
		get_valid_sender(sender_buf, NAME_LENGTH, message);

		// only check <any wingman>
		if (stricmp(sender_buf, "<any wingman>") != 0)
			continue;
		any_any_wingman = true;

		// check the message itself
		if (message->persona_index < 0)
		{
			issue_count++;
			output_msg += "\n\"";
			output_msg += message->name;
			output_msg += "\" - does not have a persona";
			continue;
		}
		if (!(Personas[message->persona_index].flags & PERSONA_FLAG_WINGMAN))
		{
			issue_count++;
			output_msg += "\n\"";
			output_msg += message->name;
			output_msg += "\" - does not have a wingman persona";
			continue;
		}

		bool found_potential_sender = false;

		// go through all ships in the mission
		for (auto objp : list_range(&obj_used_list))
		{
			if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP))
			{
				if (Ships[objp->instance].persona_index == message->persona_index)
				{
					found_potential_sender = true;
					break;
				}
			}
		}

		if (!found_potential_sender)
		{
			issue_count++;
			output_msg += "\n\"";
			output_msg += message->name;
			output_msg += "\" - no ship with persona \"";
			output_msg += Personas[message->persona_index].name;
			output_msg += "\" was found";
		}
	}

	if (!output_msg.empty())
	{
		if (issue_count == 1)
			output_msg = "The following issue was found for messages sent by <any wingman>:\n" + output_msg;
		else
			output_msg = "The following issues were found for messages sent by <any wingman>:\n" + output_msg;

		MessageBox(output_msg.c_str(), "Voice Acting Manager");
	}
	else if (!any_any_wingman)
		MessageBox("All messages have been checked.  There are no messages sent by <any wingman>.", "Voice Acting Manager");
	else
		MessageBox("All messages have been checked.  All messages sent by <any wingman> have at least one candidate sender.", "Voice Acting Manager");
}

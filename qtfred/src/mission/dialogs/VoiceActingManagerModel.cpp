#include "VoiceActingManagerModel.h"

#include "globalincs/linklist.h"
#include "cfile/cfile.h"
#include "hud/hudtarget.h"
#include "iff_defs/iff_defs.h"
#include "mission/missiongoals.h"
#include "missioneditor/common.h"
#include "missionui/missioncmdbrief.h"
#include "mission/missionbriefcommon.h"
#include "parse/sexp.h"

namespace fso::fred::dialogs {

VoiceActingManagerModel::VoiceActingManagerModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{

	initializeData();
}

bool VoiceActingManagerModel::apply()
{
	// Persist dialog settings back into globals
	strcpy_s(Voice_abbrev_briefing, _abbrevBriefing.c_str());
	strcpy_s(Voice_abbrev_campaign, _abbrevCampaign.c_str());
	strcpy_s(Voice_abbrev_command_briefing, _abbrevCommandBriefing.c_str());
	strcpy_s(Voice_abbrev_debriefing, _abbrevDebriefing.c_str());
	strcpy_s(Voice_abbrev_message, _abbrevMessage.c_str());
	strcpy_s(Voice_abbrev_mission, _abbrevMission.c_str());

	Voice_no_replace_filenames = _noReplace;

	strcpy_s(Voice_script_entry_format, _scriptEntryFormat.c_str());

	switch (_exportSelection) {
	case ExportSelection::CommandBriefings:
		Voice_export_selection = 1;
		break;
	case ExportSelection::Briefings:
		Voice_export_selection = 2;
		break;
	case ExportSelection::Debriefings:
		Voice_export_selection = 3;
		break;
	case ExportSelection::Messages:
		Voice_export_selection = 4;
		break;
	case ExportSelection::Everything:
	default:
		Voice_export_selection = 0;
		break;
	}
	Voice_group_messages = _groupMessages;

	// Nothing in apply() modifies mission data directly.
	return true;
}

void VoiceActingManagerModel::reject()
{
	// do nothing
}

void VoiceActingManagerModel::initializeData()
{
	_abbrevBriefing = Voice_abbrev_briefing;
	_abbrevCampaign = Voice_abbrev_campaign;
	_abbrevCommandBriefing = Voice_abbrev_command_briefing;
	_abbrevDebriefing = Voice_abbrev_debriefing;
	_abbrevMessage = Voice_abbrev_message;
	_abbrevMission = Voice_abbrev_mission;

	_noReplace = Voice_no_replace_filenames;
	_scriptEntryFormat = Voice_script_entry_format;

	if (_scriptEntryFormat.empty()) {
		_scriptEntryFormat = Voice_script_default_string;
	}

	switch (Voice_export_selection) {
	case 1:
		_exportSelection = ExportSelection::CommandBriefings;
		break;
	case 2:
		_exportSelection = ExportSelection::Briefings;
		break;
	case 3:
		_exportSelection = ExportSelection::Debriefings;
		break;
	case 4:
		_exportSelection = ExportSelection::Messages;
		break;
	default:
		_exportSelection = ExportSelection::Everything;
		break;
	}
	_groupMessages = Voice_group_messages;

	_suffix = Suffix::WAV;
	_includeSenderInFilename = false;
	_whichPersonaToSync = 0;
}

SCP_vector<SCP_string> VoiceActingManagerModel::personaChoices()
{
	SCP_vector<SCP_string> out;
	out.emplace_back("<Wingman Personas>");
	out.emplace_back("<Non-Wingman Personas>");
	for (const auto& persona : Personas) {
		out.emplace_back(persona.name);
	}
	return out;
}

SCP_vector<SCP_string> VoiceActingManagerModel::fileChoices()
{
	SCP_vector<SCP_string> out;

	for (int i = 0; i < static_cast<int>(Suffix::numSuffixes); i++) {
		switch (static_cast<Suffix>(i)) {
		case Suffix::OGG:
			out.emplace_back(".ogg");
			break;
		case Suffix::WAV:
			out.emplace_back(".wav");
			break;
		default:
			Assertion(false, "Invalid file type selected!");
			break;
		}
	}

	return out;
}

SCP_string VoiceActingManagerModel::buildExampleFilename() const
{
	return generateFilename(_previewSelection, 1, 2, INVALID_MESSAGE);
}

SCP_string VoiceActingManagerModel::pickExampleSection() const
{
	if (!_abbrevCommandBriefing.empty())
		return _abbrevCommandBriefing;
	if (!_abbrevBriefing.empty())
		return _abbrevBriefing;
	if (!_abbrevDebriefing.empty())
		return _abbrevDebriefing;
	if (!_abbrevMessage.empty())
		return _abbrevMessage;
	return "";
}

SCP_string VoiceActingManagerModel::getSuffixString() const
{
	switch (_suffix) {
		case Suffix::OGG:
			return ".ogg";
		case Suffix::WAV:
			return ".wav";
		default:
			Assertion(false, "Invalid file type selected!");
			return ".wav";
	}
}

int VoiceActingManagerModel::calcDigits(int size)
{
	if (size >= 10000)
		return 5;
	if (size >= 1000)
		return 4;
	if (size >= 100)
		return 3;
	return 2;
}

SCP_string VoiceActingManagerModel::generateFilename(ExportSelection sel, int number, int digits, const MMessage* message) const
{
	SCP_string prefix = _abbrevCampaign + _abbrevMission;
	switch (sel) {
		case ExportSelection::CommandBriefings:
			prefix += _abbrevCommandBriefing;
			break;
		case ExportSelection::Briefings:
			prefix += _abbrevBriefing;
			break;
		case ExportSelection::Debriefings:
			prefix += _abbrevDebriefing;
			break;
		case ExportSelection::Messages:
			prefix += _abbrevMessage;
			break;
		default:
			Assertion(false, "Invalid export selection for filename generation!");
			prefix = ""; // Fallback, shouldn't happen
	}

	SCP_string num = std::to_string(number);
	while (static_cast<int>(num.size()) < digits)
		num.insert(0, "0");

	SCP_string out = prefix + num;

	// optional sender suffix
	if (message != nullptr && _includeSenderInFilename) {
		const auto suffix = getSuffixString();
		const auto currently = out + suffix;

		const size_t allow_to_copy = NAME_LENGTH - size_t(currently.size());
		char sender[NAME_LENGTH]{};

		if (message == INVALID_MESSAGE) {
			strcpy_s(sender, "Alpha 1");
		} else {
			getValidSender(sender, sizeof(sender), message);
		}

		// truncate to avoid overflow
		if (allow_to_copy < strlen(sender)) {
			sender[allow_to_copy] = '\0';
		}

		// sanitize -> lowercase and replace non-alnum with '_'
		for (size_t j = 0; sender[j] != '\0'; ++j) {
			sender[j] = SCP_tolower(sender[j]);
			if (!isalnum(static_cast<unsigned char>(sender[j])))
				sender[j] = '_';
		}
		// compress consecutive underscores
		for (size_t j = 1; sender[j] != '\0';) {
			if (sender[j - 1] == '_' && sender[j] == '_') {
				// shift left
				size_t k = j + 1;
				while (sender[k] != '\0') {
					sender[k - 1] = sender[k];
					++k;
				}
				sender[k - 1] = '\0';
			} else {
				++j;
			}
		}
		out += sender;
	}

	out += getSuffixString();
	Assertion(out.size() < NAME_LENGTH, "Generated filename exceeds NAME_LENGTH");
	return out;
}

int VoiceActingManagerModel::generateFilenames()
{
	int num_modified = 0;

	// Command Briefings
	{
		const int digits = calcDigits(Cmd_briefs[0].num_stages);
		for (int i = 0; i < Cmd_briefs[0].num_stages; ++i) {
			auto* filename = Cmd_briefs[0].stage[i].wave_filename;
			if (!_noReplace || !strlen(filename) || message_filename_is_generic(filename)) {
				SCP_string s = generateFilename(ExportSelection::CommandBriefings, i + 1, digits, INVALID_MESSAGE);
				if (strcmp(filename, s.c_str()) != 0) {
					strcpy(filename, s.c_str());
					++num_modified;
				}
			}
		}
	}

	// Briefings
	{
		const int digits = calcDigits(Briefings[0].num_stages);
		for (int i = 0; i < Briefings[0].num_stages; ++i) {
			auto* filename = Briefings[0].stages[i].voice;
			if (!_noReplace || !strlen(filename) || message_filename_is_generic(filename)) {
				SCP_string s = generateFilename(ExportSelection::Briefings, i + 1, digits, INVALID_MESSAGE);
				if (strcmp(filename, s.c_str()) != 0) {
					strcpy(filename, s.c_str());
					++num_modified;
				}
			}
		}
	}

	// Debriefings
	{
		const int digits = calcDigits(Debriefings[0].num_stages);
		for (int i = 0; i < Debriefings[0].num_stages; ++i) {
			auto* filename = Debriefings[0].stages[i].voice;
			if (!_noReplace || !strlen(filename) || message_filename_is_generic(filename)) {
				SCP_string s = generateFilename(ExportSelection::Debriefings, i + 1, digits, INVALID_MESSAGE);
				if (strcmp(filename, s.c_str()) != 0) {
					strcpy(filename, s.c_str());
					++num_modified;
				}
			}
		}
	}

	// Messages
	{
		const int digits = calcDigits(Num_messages - Num_builtin_messages);
		for (int i = 0; i < Num_messages - Num_builtin_messages; ++i) {
			auto* message = &Messages[i + Num_builtin_messages];
			const char* filename = message->wave_info.name;
			if (!_noReplace || !strlen(filename) || message_filename_is_generic(filename)) {
				SCP_string s = generateFilename(ExportSelection::Messages, i + 1, digits, message);
				if (message->wave_info.name == nullptr || strcmp(message->wave_info.name, s.c_str()) != 0) {
					if (message->wave_info.name)
						free(message->wave_info.name);
					message->wave_info.name = strdup(s.c_str());
					++num_modified;
				}
			}
		}
	}

	if (num_modified > 0)
		set_modified();
	return num_modified;
}

bool VoiceActingManagerModel::fout(void* fp, const char* format, ...)
{
	SCP_string str;
	va_list args;
	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);
	cfputs(str.c_str(), static_cast<CFILE*>(fp));
	return true;
}

static inline void replace_all(std::string& s, const std::string& from, const std::string& to)
{
	if (from.empty())
		return;
	std::size_t pos = 0;
	while ((pos = s.find(from, pos)) != std::string::npos) {
		s.replace(pos, from.size(), to);
		pos += to.size();
	}
}

bool VoiceActingManagerModel::generateScript(const SCP_string& absoluteFilePath)
{
	auto* fp = cfopen(absoluteFilePath.c_str(), "wt");
	if (!fp)
		return false;

	// Mission metadata
	fout(fp, "%s\n", Mission_filename);
	fout(fp, "%s\n\n", The_mission.name);

	auto writeMessageEntry = [&](const char* filename,
								 const SCP_string& text,
								 const char* persona,
								 const char* sender,
								 const char* name,
								 const char* note) {
		SCP_string entry = _scriptEntryFormat;
		replace_all(entry, "\r\n", "\n"); // normalize endings

		// map nulls
		const char* filename_safe = filename ? filename : "<none>";
		const char* persona_safe = persona ? persona : "<none>";
		const char* sender_safe = sender ? sender : "<none>";
		const char* name_safe = name ? name : "<none>";
		const char* note_safe = note ? note : "<none>";

		// replace
		replace_all(entry, "$filename", filename_safe);
		replace_all(entry, "$message", text);
		replace_all(entry, "$persona", persona_safe);
		replace_all(entry, "$sender", sender_safe);
		replace_all(entry, "$name", name_safe);
		replace_all(entry, "$note", note_safe);

		fout(fp, "%s\n\n\n", entry.c_str());
	};

	// Command Briefings
	if (_exportSelection == ExportSelection::Everything || _exportSelection == ExportSelection::CommandBriefings) {
		fout(fp, "\n\nCommand Briefings\n-----------------\n\n");
		for (int i = 0; i < Cmd_briefs[0].num_stages; ++i) {
			auto* stage = &Cmd_briefs[0].stage[i];
			writeMessageEntry(stage->wave_filename,
				stage->text,
				"<no persona specified>",
				"<no sender specified>",
				"<no name specified>",
				"<no note specified>");
		}
	}

	// Briefings
	if (_exportSelection == ExportSelection::Everything || _exportSelection == ExportSelection::Briefings) {
		fout(fp, "\n\nBriefings\n---------\n\n");
		for (int i = 0; i < Briefings[0].num_stages; ++i) {
			auto* stage = &Briefings[0].stages[i];
			writeMessageEntry(stage->voice,
				stage->text,
				"<no persona specified>",
				"<no sender specified>",
				"<no name specified>",
				"<no note specified>");
		}
	}

	// Debriefings
	if (_exportSelection == ExportSelection::Everything || _exportSelection == ExportSelection::Debriefings) {
		fout(fp, "\n\nDebriefings\n-----------\n\n");
		for (int i = 0; i < Debriefings[0].num_stages; ++i) {
			auto* stage = &Debriefings[0].stages[i];
			writeMessageEntry(stage->voice,
				stage->text,
				"<no persona specified>",
				"<no sender specified>",
				"<no name specified>",
				"<no note specified>");
		}
	}

	// Messages
	if (_exportSelection == ExportSelection::Everything || _exportSelection == ExportSelection::Messages) {
		fout(fp, "\n\nMessages\n--------\n\n");

		if (_groupMessages || _exportSelection == ExportSelection::Everything) {
			SCP_vector<int> messageIndexes;
			messageIndexes.reserve(Num_messages - Num_builtin_messages);
			for (int i = 0; i < Num_messages - Num_builtin_messages; ++i)
				messageIndexes.emplace_back(i + Num_builtin_messages);

			groupMessageIndexes(messageIndexes);
			for (int idx : messageIndexes) {
				const auto* msg = &Messages[idx];

				char sender[NAME_LENGTH + 1]{};
				getValidSender(sender, sizeof(sender), msg);

				const char* persona = (msg->persona_index >= 0) ? Personas[msg->persona_index].name : "<none>";
				const char* senderOut = (sender[0] == '#') ? &sender[1] : sender;
				writeMessageEntry(msg->wave_info.name, msg->message, persona, senderOut, msg->name, msg->note.c_str());
			}
		} else {
			for (int i = 0; i < Num_messages - Num_builtin_messages; ++i) {
				const auto* msg = &Messages[i + Num_builtin_messages];

				char sender[NAME_LENGTH + 1]{};
				getValidSender(sender, sizeof(sender), msg);

				const char* persona = (msg->persona_index >= 0) ? Personas[msg->persona_index].name : "<none>";
				const char* senderOut = (sender[0] == '#') ? &sender[1] : sender;
				writeMessageEntry(msg->wave_info.name, msg->message, persona, senderOut, msg->name, msg->note.c_str());
			}
		}
	}

	cfclose(fp);
	return true;
}

static inline void assign_if_different(int& dest, int src, int& modified)
{
	if (dest != src) {
		dest = src;
		++modified;
	}
}
static inline void strdup_if_different(char*& dest, const char* src, int& modified)
{
	if (dest == nullptr || strcmp(dest, src) != 0) {
		if (dest)
			free(dest);
		dest = strdup(src);
		++modified;
	}
}

int VoiceActingManagerModel::copyMessagePersonasToShips()
{
	int modified = 0;
	SCP_unordered_set<int> alreadyAssigned;
	SCP_string inconsistent;

	char senderBuf[NAME_LENGTH]{};
	int senderShip = -1;
	bool isCommand = false;

	for (int i = 0; i < Num_messages - Num_builtin_messages; ++i) {
		auto* msg = &Messages[i + Num_builtin_messages];

		getValidSender(senderBuf, NAME_LENGTH, msg, &senderShip, &isCommand);
		auto* shipp = (senderShip < 0) ? nullptr : &Ships[senderShip];

		int personaToCopy = msg->persona_index;
		if (personaToCopy >= 0 && checkPersonaFilter(personaToCopy) && shipp) {
			if (alreadyAssigned.count(senderShip) && shipp->persona_index != personaToCopy) {
				inconsistent += "\n\u2022 ";
				inconsistent += shipp->ship_name;
			}
			alreadyAssigned.insert(senderShip);
			assign_if_different(shipp->persona_index, personaToCopy, modified);
		}
	}

	if (modified > 0)
		set_modified();

	return modified;
}

int VoiceActingManagerModel::copyShipPersonasToMessages()
{
	int modified = 0;
	SCP_unordered_set<int> alreadyAssigned;
	SCP_string inconsistent;

	char senderBuf[NAME_LENGTH]{};
	int senderShip = -1;
	bool isCommand = false;

	for (int i = 0; i < Num_messages - Num_builtin_messages; ++i) {
		auto* msg = &Messages[i + Num_builtin_messages];

		getValidSender(senderBuf, NAME_LENGTH, msg, &senderShip, &isCommand);
		const auto* shipp = (senderShip < 0) ? nullptr : &Ships[senderShip];

		int personaToCopy = -1;
		if (isCommand)
			personaToCopy = The_mission.command_persona;
		else if (shipp)
			personaToCopy = shipp->persona_index;

		if (personaToCopy >= 0 && checkPersonaFilter(personaToCopy)) {
			if (alreadyAssigned.count(i) && msg->persona_index != personaToCopy) {
				inconsistent += "\n\u2022 ";
				inconsistent += msg->name;
			}
			alreadyAssigned.insert(i);
			assign_if_different(msg->persona_index, personaToCopy, modified);
		}
	}

	if (modified > 0)
		set_modified();
	return modified;
}

int VoiceActingManagerModel::clearPersonasFromNonSenders()
{
	SCP_unordered_set<int> allSenders;

	char senderBuf[NAME_LENGTH]{};
	int senderShip = -1;

	// Gather all ships that actually send a message
	for (int i = 0; i < Num_messages - Num_builtin_messages; ++i) {
		auto* msg = &Messages[i + Num_builtin_messages];
		getValidSender(senderBuf, NAME_LENGTH, msg, &senderShip);
		if (senderShip >= 0)
			allSenders.insert(senderShip);
	}

	int modified = 0;
	for (auto objp : list_range(&obj_used_list)) {
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
			auto& ship = Ships[objp->instance];
			if (allSenders.count(objp->instance) == 0) {
				if (ship.persona_index >= 0 && checkPersonaFilter(ship.persona_index)) {
					assign_if_different(ship.persona_index, -1, modified);
				}
			}
		}
	}

	if (modified > 0)
		set_modified();
	return modified;
}

int VoiceActingManagerModel::setHeadAnisUsingMessagesTbl()
{
	int modified = 0;

	for (int i = 0; i < Num_messages - Num_builtin_messages; ++i) {
		auto* msg = &Messages[i + Num_builtin_messages];
		if (msg->persona_index < 0)
			continue;
		if (!checkPersonaFilter(msg->persona_index))
			continue;

		// find builtin message that shares this persona
		bool found = false;
		for (int j = 0; j < Num_builtin_messages; ++j) {
			const auto* builtin = &Messages[j];
			if (builtin->persona_index == msg->persona_index) {
				strdup_if_different(msg->avi_info.name, builtin->avi_info.name, modified);
				found = true;
				break;
			}
		}
		if (!found) {
			Warning(LOCATION, "Persona index %d not found in builtin messages (messages.tbl)!", msg->persona_index);
		}
	}

	if (modified > 0)
		set_modified();
	return modified;
}

AnyWingmanCheckResult VoiceActingManagerModel::checkAnyWingmanPersonas()
{
	AnyWingmanCheckResult result;
	char senderBuf[NAME_LENGTH]{};

	for (int i = 0; i < Num_messages - Num_builtin_messages; ++i) {
		const auto* msg = &Messages[i + Num_builtin_messages];

		getValidSender(senderBuf, NAME_LENGTH, msg, nullptr, nullptr);
		if (stricmp(senderBuf, "<any wingman>") != 0)
			continue;

		result.anyWingmanFound = true;

		// message must have a wingman persona and at least one ship with that persona
		if (msg->persona_index < 0) {
			++result.issueCount;
			result.report += SCP_string("\n\"") + msg->name + "\" - does not have a persona";
			continue;
		}
		if ((Personas[msg->persona_index].flags & PERSONA_FLAG_WINGMAN) == 0) {
			++result.issueCount;
			result.report += SCP_string("\n\"") + msg->name + "\" - does not have a wingman persona";
			continue;
		}

		bool foundPotentialSender = false;
		for (auto objp : list_range(&obj_used_list)) {
			if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
				if (Ships[objp->instance].persona_index == msg->persona_index) {
					foundPotentialSender = true;
					break;
				}
			}
		}
		if (!foundPotentialSender) {
			++result.issueCount;

			const char* msg_name = msg->name;
			const char* persona_name = Personas[msg->persona_index].name;

			result.report += std::string("\n\"") + msg_name + "\" - no ship with persona \"" + persona_name + "\" was found";
		}
	}
	return result;
}

const char* VoiceActingManagerModel::getMessageSender(const MMessage* message)
{
	for (int i = 0; i < Num_sexp_nodes; ++i) {
		if (Sexp_nodes[i].type == SEXP_NOT_USED)
			continue;

		const int op = get_operator_const(i);
		int n = CDR(i);

		if (op == OP_SEND_MESSAGE) {
			if (!strcmp(message->name, Sexp_nodes[CDDR(n)].text))
				return Sexp_nodes[n].text;
		} else if (op == OP_SEND_MESSAGE_LIST || op == OP_SEND_MESSAGE_CHAIN) {
			if (op == OP_SEND_MESSAGE_CHAIN)
				n = CDR(n);
			while (n != -1) {
				if (!strcmp(message->name, Sexp_nodes[CDDR(n)].text))
					return Sexp_nodes[n].text;
				n = CDDDDR(n);
			}
		} else if (op == OP_SEND_RANDOM_MESSAGE) {
			char* sender = Sexp_nodes[n].text;
			n = CDDR(n);
			while (n != -1) {
				if (!strcmp(message->name, Sexp_nodes[n].text))
					return sender;
				n = CDR(n);
			}
		} else if (op == OP_TRAINING_MSG) {
			if (!strcmp(message->name, Sexp_nodes[n].text))
				return "Training Message";
		}
	}
	return "<none>";
}

void VoiceActingManagerModel::getValidSender(char* sender,
	size_t sender_size,
	const MMessage* message,
	int* sender_shipnum,
	bool* is_command)
{
	Assert(sender != nullptr);
	Assert(message != nullptr);

	memset(sender, 0, sender_size);
	strncpy(sender, getMessageSender(message), sender_size - 1);

	if (!strcmp("#Command", sender)) {
		if (is_command)
			*is_command = true;

		if (The_mission.flags[Mission::Mission_Flags::Override_hashcommand]) {
			memset(sender, 0, sender_size);
			strncpy(sender, The_mission.command_sender, sender_size - 1);
		}
	} else {
		if (is_command)
			*is_command = false;
	}

	// strip leading '#'
	if (sender[0] == '#') {
		size_t i = 1;
		for (; sender[i] != '\0'; ++i)
			sender[i - 1] = sender[i];
		sender[i - 1] = '\0';
	}

	const int shipnum = ship_name_lookup(sender, 1);
	if (sender_shipnum)
		*sender_shipnum = shipnum;

	if (shipnum >= 0) {
		ship* shipp = &Ships[shipnum];

		if (*Fred_callsigns[shipnum]) {
			hud_stuff_ship_callsign(sender, shipp);
		} else if (((Iff_info[shipp->team].flags & IFFF_WING_NAME_HIDDEN) && (shipp->wingnum != -1)) ||
				   (shipp->flags[Ship::Ship_Flags::Hide_ship_name])) {
			hud_stuff_ship_class(sender, shipp);
		} else {
			memset(sender, 0, sender_size);
			strncpy(sender, shipp->get_display_name(), sender_size - 1);
		}
	}
}

void VoiceActingManagerModel::groupMessageIndexes(SCP_vector<int>& messageIndexes)
{
	const auto initialSize = messageIndexes.size();
	SCP_vector<int> source = messageIndexes;
	messageIndexes.clear();

	for (const auto& ev : Mission_events)
		groupMessageIndexesInTree(ev.formula, source, messageIndexes);

	// append remaining
	for (int idx : source)
		messageIndexes.push_back(idx);

#ifndef NDEBUG
	if (initialSize != messageIndexes.size()) {
		// parity check
		Warning(LOCATION, "groupMessageIndexes changed list size (%d -> %d)", static_cast<int>(initialSize), static_cast<int>(messageIndexes.size()));
	}
#endif
}

void VoiceActingManagerModel::groupMessageIndexesInTree(int node, SCP_vector<int>& source, SCP_vector<int>& dest)
{
	if (node < 0)
		return;
	if (Sexp_nodes[node].type == SEXP_NOT_USED)
		return;

	const int op = get_operator_const(node);
	int n = CDR(node);

	if (op == OP_SEND_MESSAGE_LIST || op == OP_SEND_MESSAGE_CHAIN) {
		if (op == OP_SEND_MESSAGE_CHAIN)
			n = CDR(n);
		while (n != -1) {
			char* message_name = Sexp_nodes[CDDR(n)].text;
			for (int i = 0; i < static_cast<int>(source.size()); ++i) {
				if (!strcmp(message_name, Messages[source[i]].name)) {
					dest.push_back(source[i]);
					source.erase(source.begin() + i);
					break;
				}
			}
			n = CDDDDR(n);
		}
	} else if (op == OP_SEND_RANDOM_MESSAGE) {
		n = CDDR(n);
		while (n != -1) {
			char* message_name = Sexp_nodes[n].text;
			for (int i = 0; i < static_cast<int>(source.size()); ++i) {
				if (!strcmp(message_name, Messages[source[i]].name)) {
					dest.push_back(source[i]);
					source.erase(source.begin() + i);
					break;
				}
			}
			n = CDR(n);
		}
	}

	groupMessageIndexesInTree(CAR(node), source, dest);
	groupMessageIndexesInTree(CDR(node), source, dest);
}

bool VoiceActingManagerModel::checkPersonaFilter(int persona) const
{
	Assertion(SCP_vector_inbounds(Personas, persona), "Persona index out of range in checkPersonaFilter()");
	if (_whichPersonaToSync == static_cast<int>(PersonaSyncIndex::Wingman)) {
		return (Personas[persona].flags & PERSONA_FLAG_WINGMAN) != 0;
	} else if (_whichPersonaToSync == static_cast<int>(PersonaSyncIndex::NonWingman)) {
		return (Personas[persona].flags & PERSONA_FLAG_WINGMAN) == 0;
	} else {
		const int specific = _whichPersonaToSync - static_cast<int>(PersonaSyncIndex::PersonasStart);
		Assertion(SCP_vector_inbounds(Personas, specific), "Dropdown persona index out of range");
		return specific == persona;
	}
}

SCP_string VoiceActingManagerModel::abbrevBriefing() const
{
	return _abbrevBriefing;
}
SCP_string VoiceActingManagerModel::abbrevCampaign() const
{
	return _abbrevCampaign;
}
SCP_string VoiceActingManagerModel::abbrevCommandBriefing() const
{
	return _abbrevCommandBriefing;
}
SCP_string VoiceActingManagerModel::abbrevDebriefing() const
{
	return _abbrevDebriefing;
}
SCP_string VoiceActingManagerModel::abbrevMessage() const
{
	return _abbrevMessage;
}
SCP_string VoiceActingManagerModel::abbrevMission() const
{
	return _abbrevMission;
}

void VoiceActingManagerModel::setAbbrevBriefing(const SCP_string& v)
{
	modify(_abbrevBriefing, v);
}
void VoiceActingManagerModel::setAbbrevCampaign(const SCP_string& v)
{
	modify(_abbrevCampaign, v);
}
void VoiceActingManagerModel::setAbbrevCommandBriefing(const SCP_string& v)
{
	modify(_abbrevCommandBriefing, v);
}
void VoiceActingManagerModel::setAbbrevDebriefing(const SCP_string& v)
{
	modify(_abbrevDebriefing, v);
}
void VoiceActingManagerModel::setAbbrevMessage(const SCP_string& v)
{
	modify(_abbrevMessage, v);
}
void VoiceActingManagerModel::setAbbrevMission(const SCP_string& v)
{
	modify(_abbrevMission, v);
}

void VoiceActingManagerModel::setAbbrevSelection(ExportSelection sel)
{
	switch (sel) {
	case ExportSelection::CommandBriefings:
		_previewSelection = ExportSelection::CommandBriefings;
		break;
	case ExportSelection::Briefings:
		_previewSelection = ExportSelection::Briefings;
		break;
	case ExportSelection::Debriefings:
		_previewSelection = ExportSelection::Debriefings;
		break;
	case ExportSelection::Messages:
		_previewSelection = ExportSelection::Messages;
		break;
	default: // Other options not allowed so no change!
		break;
	}
}

bool VoiceActingManagerModel::includeSenderInFilename() const
{
	return _includeSenderInFilename;
}
void VoiceActingManagerModel::setIncludeSenderInFilename(bool v)
{
	modify(_includeSenderInFilename, v);
}

bool VoiceActingManagerModel::noReplace() const
{
	return _noReplace;
}
void VoiceActingManagerModel::setNoReplace(bool v)
{
	modify(_noReplace, v);
}

Suffix VoiceActingManagerModel::suffix() const
{
	return _suffix;
}
void VoiceActingManagerModel::setSuffix(Suffix s)
{
	modify(_suffix, s);
}

SCP_string VoiceActingManagerModel::scriptEntryFormat() const
{
	return _scriptEntryFormat;
}
void VoiceActingManagerModel::setScriptEntryFormat(const SCP_string& v)
{
	modify(_scriptEntryFormat, v);
}

ExportSelection VoiceActingManagerModel::exportSelection() const
{
	return _exportSelection;
}
void VoiceActingManagerModel::setExportSelection(ExportSelection sel)
{
	modify(_exportSelection, sel);
}

bool VoiceActingManagerModel::groupMessages() const
{
	return _groupMessages;
}
void VoiceActingManagerModel::setGroupMessages(bool v)
{
	modify(_groupMessages, v);
}

int VoiceActingManagerModel::whichPersonaToSync() const
{
	return _whichPersonaToSync;
}
void VoiceActingManagerModel::setWhichPersonaToSync(int idx)
{
	modify(_whichPersonaToSync, idx);
}

} // namespace fso::fred::dialogs

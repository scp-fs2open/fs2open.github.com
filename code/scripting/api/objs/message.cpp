//
//

#include "message.h"
#include "sound.h"

#include "mission/missionmessage.h"
#include "parse/sexp.h"
#include "parse/sexp_container.h"

extern int add_wave( const char *wave_name );

namespace scripting {
namespace api {

//**********HANDLE: Persona
ADE_OBJ(l_Persona, int, "persona", "Persona handle");

ADE_VIRTVAR(Name, l_Persona, "string", "The name of the persona", "string", "The name or empty string on error")
{
	int idx = -1;

	if (!ade_get_args(L, "o", l_Persona.Get(&idx)))
		return ade_set_error(L, "s", "");

	if (Personas == NULL)
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= Num_personas)
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Personas[idx].name);
}

ADE_FUNC(isValid, l_Persona, NULL, "Detect if the handle is valid", "boolean", "true if valid, false otherwise")
{
	int idx = -1;

	if (!ade_get_args(L, "o", l_Persona.Get(&idx)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", idx >= 0 && idx < Num_personas);
}

//**********HANDLE: Message
ADE_OBJ(l_Message, int, "message", "Handle to a mission message");

ADE_VIRTVAR(Name, l_Message, "string", "The name of the message as specified in the mission file", "string", "The name or an empty string if handle is invalid")
{
	int idx = -1;
	if (!ade_get_args(L, "o", l_Message.Get(&idx)))
		return ade_set_error(L, "s", "");

	if (idx < 0 && idx >= (int) Messages.size())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Messages[idx].name);
}

ADE_VIRTVAR(Message, l_Message, "string", "The unaltered text of the message, see getMessage() for options to replace variables<br>"
	"<b>NOTE:</b> Changing the text will also change the text for messages not yet played but already in the message queue!",
			"string", "The message or an empty string if handle is invalid")
{
	int idx = -1;
	const char* newText = nullptr;
	if (!ade_get_args(L, "o|s", l_Message.Get(&idx), &newText))
		return ade_set_error(L, "s", "");

	if (idx < 0 && idx >= (int) Messages.size())
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR && newText != NULL)
	{
		if (strlen(newText) > MESSAGE_LENGTH)
			LuaError(L, "New message text is too long, maximum is %d!", MESSAGE_LENGTH);
		else
			strcpy_s(Messages[idx].message, newText);
	}

	return ade_set_args(L, "s", Messages[idx].message);
}

// from mission/missionmessage.cpp
ADE_VIRTVAR(VoiceFile, l_Message, "soundfile", "The voice file of the message", "soundfile", "The voice file handle or invalid handle when not present")
{
	int idx = -1;
	soundfile_h* sndIdx = nullptr;

	if (!ade_get_args(L, "o|o", l_Message.Get(&idx), l_Soundfile.GetPtr(&sndIdx)))
		return ade_set_error(L, "o", l_Soundfile.Set(soundfile_h()));

	if (idx < 0 && idx >= (int) Messages.size())
		return ade_set_error(L, "o", l_Soundfile.Set(soundfile_h()));

	MissionMessage* msg = &Messages[idx];

	if (ADE_SETTING_VAR)
	{
		if (sndIdx->idx.isValid()) {
			const char* newFilename = snd_get_filename(sndIdx->idx);

			msg->wave_info.index = add_wave(newFilename);
		}
		else
		{
			msg->wave_info.index = -1;
		}
	}

	if (msg->wave_info.index < 0)
	{
		return ade_set_args(L, "o", l_Soundfile.Set(soundfile_h()));
	}
	else
	{
		int index = msg->wave_info.index;
		// Load the sound before using it
		message_load_wave(index, Message_waves[index].name);

		return ade_set_args(L, "o", l_Soundfile.Set(soundfile_h(Message_waves[index].num)));
	}
}

ADE_VIRTVAR(Persona, l_Message, "persona", "The persona of the message", "persona", "The persona handle or invalid handle if not present")
{
	int idx = -1;
	int newPersona = -1;

	if (!ade_get_args(L, "o|o", l_Message.Get(&idx), l_Persona.Get(&newPersona)))
		return ade_set_error(L, "o", l_Soundfile.Set(soundfile_h()));

	if (idx < 0 && idx >= (int) Messages.size())
		return ade_set_error(L, "o", l_Soundfile.Set(soundfile_h()));

	if (ADE_SETTING_VAR && newPersona >= 0 && newPersona < Num_personas)
	{
		Messages[idx].persona_index = newPersona;
	}

	return ade_set_args(L, "o", l_Persona.Set(Messages[idx].persona_index));
}

ADE_FUNC(getMessage, l_Message, "[boolean replaceVars = true]", "Gets the text of the message and optionally replaces SEXP variables with their respective values.", "string", "The message or an empty string if handle is invalid")
{
	int idx = -1;
	bool replace = true;
	if (!ade_get_args(L, "o|b", l_Message.Get(&idx), &replace))
		return ade_set_error(L, "s", "");

	if (idx < 0 && idx >= (int) Messages.size())
		return ade_set_error(L, "s", "");

	if (!replace)
		return ade_set_args(L, "s", Messages[idx].message);
	else
	{
		char temp_buf[MESSAGE_LENGTH];
		strcpy_s(temp_buf, Messages[idx].message);

		sexp_replace_variable_names_with_values(temp_buf, MESSAGE_LENGTH - 1);
		sexp_container_replace_refs_with_values(temp_buf, MESSAGE_LENGTH - 1);

		return ade_set_args(L, "s", temp_buf);
	}
}

ADE_FUNC(isValid, l_Message, NULL, "Checks if the message handle is valid", "boolean", "true if valid, false otherwise")
{
	int idx = -1;
	if (!ade_get_args(L, "o", l_Message.Get(&idx)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", idx >= 0 && idx < (int) Messages.size());
}


}
}

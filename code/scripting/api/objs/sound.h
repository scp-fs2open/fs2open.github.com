#pragma once

#include <sound/ds.h>
#include "scripting/ade_api.h"
#include "gamesnd/gamesnd.h"
#include "sound/sound.h"

namespace scripting {
namespace api {


struct sound_entry_h
{
	gamesnd_id idx;

	sound_entry_h();

	explicit sound_entry_h(gamesnd_id n_idx);

	game_snd *Get() const;

	bool isValid() const;
};

//**********HANDLE: SoundEntry
DECLARE_ADE_OBJ(l_SoundEntry, sound_entry_h);

struct sound_h
{
	sound_entry_h entryh;
	sound_handle sig;

	sound_h();

	sound_h(gamesnd_id n_gs_idx, sound_handle n_sig);

	sound_handle getSignature() const;

	bool isValid() const;			// This is the function that is checked by ade_set_args
	bool isValidWithEntry() const;
};

//**********HANDLE: Sound
DECLARE_ADE_OBJ(l_Sound, sound_h);


DECLARE_ADE_OBJ(l_Sound3D, sound_h);


//**********HANDLE: Soundfile

struct soundfile_h {
	sound_load_id idx;

	explicit soundfile_h(sound_load_id id = sound_load_id::invalid());
};

DECLARE_ADE_OBJ(l_Soundfile, soundfile_h);
}
}


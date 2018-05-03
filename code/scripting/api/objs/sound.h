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

	game_snd *Get();

	bool IsValid();
};

//**********HANDLE: SoundEntry
DECLARE_ADE_OBJ(l_SoundEntry, sound_entry_h);

struct sound_h : public sound_entry_h
{
	int sig;

	sound_h();

	sound_h(gamesnd_id n_gs_idx, int n_sig);

	int getSignature();

	bool IsSoundValid();

	bool IsValid();
};

//**********HANDLE: Sound
DECLARE_ADE_OBJ(l_Sound, sound_h);


DECLARE_ADE_OBJ(l_Sound3D, sound_h);


//**********HANDLE: Soundfile
DECLARE_ADE_OBJ(l_Soundfile, int);

}
}


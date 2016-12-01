#pragma once
#ifndef FS2_OPEN_GAMESTATE_H
#define FS2_OPEN_GAMESTATE_H

#include "scripting/ade_api.h"
#include "gamesequence/gamesequence.h"

namespace scripting {
namespace api {

class gamestate_h {
 private:
	int sdx;
 public:
	gamestate_h();
	gamestate_h(int n_state);

	bool IsValid();

	int Get();
};

DECLARE_ADE_OBJ(l_GameState, gamestate_h);


}
}

#endif // FS2_OPEN_GAMESTATE_H

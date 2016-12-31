#pragma once
#ifndef FS2_OPEN_GAMEEVENT_H
#define FS2_OPEN_GAMEEVENT_H

#include "scripting/ade_api.h"
#include "gamesequence/gamesequence.h"

namespace scripting {
namespace api {

class gameevent_h
{
private:
	int edx;
public:
	gameevent_h();

	explicit gameevent_h(int n_event);

	bool IsValid();

	int Get();
};

DECLARE_ADE_OBJ(l_GameEvent, gameevent_h);

}
}


#endif

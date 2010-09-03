/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// The code in here is just for bookeeping, allocating
// ai slots and linking them to ships.
// See AiCode.cpp for the actual AI code.

#include "ai/ai.h"
#include "object/object.h"
#include "ship/ship.h"



int Total_goal_ship_names = 0;
char Goal_ship_names[MAX_GOAL_SHIP_NAMES][NAME_LENGTH];
ai_info Ai_info[MAX_AI_INFO];
ai_info *Player_ai;

// Return index of free AI slot.
// Return -1 if no free slot.
int ai_get_slot(int shipnum)
{
	int	i;

	for (i=0; i<MAX_AI_INFO ; i++)
		if (Ai_info[i].shipnum == -1)	{
			Ai_info[i].shipnum = shipnum;
			return i;
		}

	Warning( LOCATION, "Couldn't get AI slot" );
	Int3();

	return -1;
}

// Releases an AI slot to be used by someone else.
// Only modifies in Ai_info struct.  Does not modify hook in ship.
void ai_free_slot(int ai_index)
{
	Assert( (ai_index >= 0) && (ai_index < MAX_AI_INFO) );

	Ai_info[ai_index].shipnum = -1;
}

int get_wingnum(int objnum)
{
	int	shipnum, ai_index;

	shipnum = Objects[objnum].instance;

	ai_index = Ships[shipnum].ai_index;

	return Ai_info[ai_index].wing;
}

void set_wingnum(int objnum, int wingnum)
{
	int	shipnum, ai_index;

	Assert(Objects[objnum].type == OBJ_SHIP);

	shipnum = Objects[objnum].instance;

	Assert((shipnum >= 0) && (shipnum < MAX_SHIPS));

	ai_index = Ships[shipnum].ai_index;

	Assert( (ai_index >= 0) && (ai_index < MAX_AI_INFO) );

	Ai_info[ai_index].wing = wingnum;
}

char *ai_get_goal_ship_name(char *name, int *index)
{
	int i;

	for (i=0; i < Total_goal_ship_names; i++)
		if (!stricmp(name, Goal_ship_names[i])) {
			*index = i;
			return Goal_ship_names[i];
		}

	Assert(Total_goal_ship_names < MAX_GOAL_SHIP_NAMES);
	Assertion(strlen(name) < NAME_LENGTH - 1, "Ship name %s is too long. Needs to be 31 characters or less.", name);
	i = Total_goal_ship_names++;
	strcpy_s(Goal_ship_names[i], name);
	*index = i;
	return Goal_ship_names[i];
}

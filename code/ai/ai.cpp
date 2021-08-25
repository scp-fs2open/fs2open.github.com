/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/**
 * @file 
 * The code in here is just for bookeeping, allocating AI slots and linking them to ships.
 * See AiCode.cpp for the actual AI code.
 */

#include "ai/ai.h"
#include "object/object.h"
#include "ship/ship.h"


SCP_vector<char *> Goal_target_names;
ai_info Ai_info[MAX_AI_INFO];
ai_info *Player_ai;

/**
 * @brief Returns index of free AI slot.
 * @return Return -1 if no free slot.
 */
int ai_get_slot(int shipnum)
{
	int	i;

	for (i=0; i<MAX_AI_INFO ; i++)
		if (Ai_info[i].shipnum == -1)	{
			Ai_info[i].shipnum = shipnum;
			return i;
		}

	Error( LOCATION, "Couldn't get AI slot, please tell a coder!");

	return -1;
}

/**
 * @brief Frees a currently used AI slot.
 * @details Only modifies in ::Ai_info struct. Does not modify hook in ship.
 */
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

const char *ai_get_goal_target_name(const char *name, int *index)
{
	Assertion(name != nullptr && index != nullptr, "Arguments cannot be null!");

	for (int i = 0; i < static_cast<int>(Goal_target_names.size()); ++i) {
		if (!stricmp(name, Goal_target_names[i])) {
			*index = i;
			return Goal_target_names[i];
		}
	}

	*index = static_cast<int>(Goal_target_names.size());
	Goal_target_names.push_back(vm_strdup(name));
	return Goal_target_names[*index];
}

void ai_clear_goal_target_names()
{
	for (auto ptr : Goal_target_names)
		vm_free(ptr);
	Goal_target_names.clear();
}

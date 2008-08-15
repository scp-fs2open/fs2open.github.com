#include "ai/aibig.h"
#include "ai/aigoals.h"
#include "ai/aiinternal.h"

struct aicode_call_table AICodeNewAITable;

#include "iff_defs/iff_defs.h"

#define DEPARTURE_HULL_PERCENT 0.3 // Departure if 15% to 30% hull strength left

void newai_ai_maybe_depart(object* objp_ship)
{
	ship *shipp = &Ships[objp_ship->instance];

	// Coward mode:
	//
	// Depart if:
	//   * not player and
	//   * same team as player and player not traitor and
	//   * fighter or bomber
	//   * hull multiplied with random value below critical
	
	if (objp_ship->flags & OF_PLAYER_SHIP)
		return;

	// make sure player is on their team and not a traitor
	if ((Player_ship->team != shipp->team) || (Player_ship->team == Iff_traitor))
		return;

	if ((Ship_info[shipp->ship_info_index].flags & (SIF_FIGHTER|SIF_BOMBER)) == 0)
		return;

	if (objp_ship->hull_strength*(frand()+1) < DEPARTURE_HULL_PERCENT * shipp->ship_max_hull_strength)
		mission_do_departure(objp_ship);
}

//      Object *objp_ship was hit by either weapon *objp_weapon or collided into by ship hit_objp at point *hitpos.
void    newai_aicode_ai_ship_hit (object *objp_ship, object *hit_objp, vec3d *hitpos, int shield_quadrant, vec3d *hit_normal)
{
	// Call the default implementation
	AICodeDefaultTable.ai_ship_hit(objp_ship, hit_objp, hitpos, shield_quadrant, hit_normal);
	
	if (hit_objp->parent >= 0)
	{
		object *objp_hitter = &Objects[hit_objp->parent];
		
		if ( objp_hitter->type == OBJ_SHIP )
		{
			ship *shipp_hitter = &Ships[objp_hitter->instance];
			ship *shipp_hit = &Ships[objp_ship->instance];
						
			fprintf(stderr, "Ship %s hit %s\n", shipp_hitter->ship_name, shipp_hit->ship_name);
		}

	}

	newai_ai_maybe_depart(objp_ship);
}


void newai_aicode_maybe_process_friendly_hit(object *objp_hitter, object *objp_hit, object *objp_weapon)
{
	AICodeDefaultTable.maybe_process_friendly_hit(objp_hitter, objp_hit, objp_weapon);
	
	if (obj_team(objp_hitter) == obj_team(objp_hit))
	{
		if ( objp_hitter->type != OBJ_SHIP )
			return;

		ship *shipp_hitter = &Ships[objp_hitter->instance];
		ship *shipp_hit = &Ships[objp_hit->instance];
						
		fprintf(stderr, "Ship %s friendly hit %s\n", shipp_hitter->ship_name, shipp_hit->ship_name);
	}
}

int newai_aicode_ai_fire_primary_weapon(object *objp)
{
	int fired = AICodeDefaultTable.ai_fire_primary_weapon(objp);

	if (fired)
	{
		ship *shipp = &Ships[objp->instance];
		fprintf(stderr, "Ship %s fired primary weapon\n", shipp->ship_name);
	}
}

void newai_aicode_init_ship_info()
{
	static int called=0;
	if (called == 0)
	{
	        fprintf(stderr, "Init ship info was called\n");
		called=1;
	}
        AICodeDefaultTable.init_ship_info();
}

extern "C" {
	void newai_ai_module_init();
}

void newai_ai_module_init()
{

#ifdef PACKAGE_NAME
	fprintf(stderr, "ai_module_init() called\n");
#else
	fprintf(stderr, "ai_module_init() modular called\n");
#endif
        
	// First make a copy
        memcpy(&AICodeNewAITable, &AICodeDefaultTable, sizeof(AICodeNewAITable));

        // Now set up our overridden functions  

        aicode_table = &AICodeNewAITable;
        aicode_table->init_ship_info=newai_aicode_init_ship_info;
        aicode_table->ai_ship_hit=newai_aicode_ai_ship_hit;
        //aicode_table->ai_fire_primary_weapon=newai_aicode_ai_fire_primary_weapon;
	//aicode_table->maybe_process_friendly_hit=newai_aicode_maybe_process_friendly_hit;
}

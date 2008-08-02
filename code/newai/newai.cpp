#include "ai/aibig.h"
#include "ai/aigoals.h"
#include "ai/aiinternal.h"

struct aicode_call_table AICodeNewAITable;

//      Object *objp_ship was hit by either weapon *objp_weapon or collided into by ship hit_objp at point *hitpos.
void    newai_aicode_ai_ship_hit (object *objp_ship, object *hit_objp, vec3d *hitpos, int shield_quadrant, vec3d *hit_normal)
{
	AICodeDefaultTable.ai_ship_hit(objp_ship, hit_objp, hitpos, shield_quadrant, hit_normal);

	if (hit_objp->parent >= 0)
	{
		object *objp_hitter = &Objects[hit_objp->parent];
		object *objp_hit = objp_ship;
		
		if ( objp_hitter->type != OBJ_SHIP )
			return;

		ship *shipp_hitter = &Ships[objp_hitter->instance];
		ship *shipp_hit = &Ships[objp_hit->instance];
						
		fprintf(stderr, "Ship %s hit %s\n", shipp_hitter->ship_name, shipp_hit->ship_name);

	}

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
						
		fprintf(stderr, "Ship %s hit %s\n", shipp_hitter->ship_name, shipp_hit->ship_name);
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
	
	fprintf(stderr, "ai_module_init() called\n");
        
	// First make a copy
        memcpy(&AICodeNewAITable, &AICodeDefaultTable, sizeof(AICodeNewAITable));

        // Now set up our overridden functions  

        aicode_table = &AICodeNewAITable;
        aicode_table->init_ship_info=newai_aicode_init_ship_info;
        aicode_table->ai_ship_hit=newai_aicode_ai_ship_hit;
        //aicode_table->ai_fire_primary_weapon=newai_aicode_ai_fire_primary_weapon;
	//aicode_table->maybe_process_friendly_hit=newai_aicode_maybe_process_friendly_hit;
}

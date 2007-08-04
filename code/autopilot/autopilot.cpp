// Autopilot.cpp
// Derek Meek
// 4-30-2004

/*
 * $Logfile: /Freespace2/code/Autopilot/Autopilot.cpp $
 * $Revision: 1.39 $
 * $Date: 2007-08-04 22:30:01 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.38  2007/08/03 01:35:48  Goober5000
 * fix warning
 *
 * Revision 1.37  2007/07/25 20:44:49  Kazan
 * I <3 making stupid bugs (return camera to normal when not using cutscene bars)
 *
 * Revision 1.36  2007/07/25 14:50:44  Kazan
 * two-part autopilot warp, add +No_Cutscene_Bars optional flag to autopilot.tbl
 *
 * Revision 1.35  2007/07/24 20:30:36  Kazan
 * oops
 *
 * Revision 1.34  2007/07/24 20:17:36  Kazan
 * Make asteroid/debris fields interrupt autopilot, add "hazards near" message to autopilot.tbl, add use-nav-cinematics sexp, fix mantis #1441
 *
 * Revision 1.33  2007/07/24 13:04:10  Kazan
 * Resolve Mantis 1281
 *
 * Revision 1.32  2007/07/23 15:35:40  Kazan
 * typo correction in autopilot.tbl
 *
 * Revision 1.31  2007/07/23 15:16:47  Kazan
 * Autopilot upgrades as described, MSVC2005 project fixes
 *
 * Revision 1.30  2007/03/22 22:14:55  taylor
 * get rid of non-standard itoa(), make use of the proper sprintf() instead
 *
 * Revision 1.29  2006/12/28 00:59:19  wmcoolmon
 * WMC codebase commit. See pre-commit build thread for details on changes.
 *
 * Revision 1.28  2006/11/03 18:45:11  Kazan
 * prevent autopilot initialization while player is gliding
 *
 * Revision 1.27  2006/09/04 09:21:55  Backslash
 * Turn off gliding upon activating autopilot
 *
 * Revision 1.26  2006/09/04 06:06:38  wmcoolmon
 * Cover-up a bloody annoying warning
 *
 * Revision 1.25  2006/07/27 19:17:59  Kazan
 * Lock player primaries during autopilot
 *
 * Revision 1.24  2006/07/26 03:46:04  Kazan
 * Resolve Mantis #1011
 *
 * Revision 1.23  2006/02/25 21:42:31  Goober5000
 * fixes from last commit
 *
 * Revision 1.22  2005/08/23 09:18:08  Goober5000
 * ensure init/reset of goals works cleanly
 * --Goober5000
 *
 * Revision 1.21  2005/07/23 16:25:32  Goober5000
 * remove an extraneous extern
 * --Goober5000
 *
 * Revision 1.20  2005/07/13 02:30:52  Goober5000
 * removed autopilot #define
 * --Goober5000
 *
 * Revision 1.19  2005/04/19 23:03:42  wmcoolmon
 * Lock time compression when autopilot is engaged
 *
 * Revision 1.18  2005/04/05 05:53:14  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 1.17  2005/03/25 06:57:32  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 1.16  2005/03/03 06:05:26  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 1.15  2005/01/31 23:27:51  taylor
 * merge with Linux/OSX tree - p0131-2
 *
 * Revision 1.14  2004/10/03 21:41:10  Kazan
 * Autopilot convergence collision fix for ai_fly_to_ship() and ai_waypoints() -- mathematically expensive, only usable by autopilot
 *
 * Revision 1.13  2004/09/28 22:51:41  Kazan
 * fix autopilot formation bug
 *
 * Revision 1.12  2004/09/28 19:54:31  Kazan
 * | is binary or, || is boolean or - please use the right one
 * autopilot velocity ramping biasing
 * made debugged+k kill guardianed ships
 *
 * Revision 1.11  2004/07/29 23:41:21  Kazan
 * bugfixes
 *
 * Revision 1.10  2004/07/27 18:52:10  Kazan
 * squished another
 *
 * Revision 1.9  2004/07/27 18:04:09  Kazan
 * i love it when bugs go crunch (autopilot ai fixup)
 *
 * Revision 1.8  2004/07/26 20:47:24  Kazan
 * remove MCD complete
 *
 * Revision 1.7  2004/07/26 17:54:04  Kazan
 * Autopilot system completed -- i am dropping plans for GUI nav map
 * Fixed FPS counter during time compression
 *
 * Revision 1.6  2004/07/25 19:27:51  Kazan
 * only disable afterburning during AIM_WAYPOINTS and AIM_FLY_TO_SHIP while AutoPilotEngaged
 *
 * Revision 1.5  2004/07/25 18:46:28  Kazan
 * -fred_no_warn has become -no_warn and applies to both fred and fs2
 * added new ai directive (last commit) and disabled afterburners while performing AIM_WAYPOINTS or AIM_FLY_TO_SHIP
 * fixed player ship speed bug w/ player-use-ai, now stays in formation correctly and manages speed
 * made -radar_reduce ignore itself if no parameter is given (ignoring launcher bug)
 *
 * Revision 1.4  2004/07/25 00:31:27  Kazan
 * i have absolutely nothing to say about that subject
 *
 * Revision 1.3  2004/07/12 16:32:42  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 1.2  2004/07/01 16:38:18  Kazan
 * working on autonav
 *
 * Revision 1.1  2004/05/07 23:50:14  Kazan
 * Sorry Guys!
 *
 *
 *
 *
 */


#include "autopilot/autopilot.h"
#include "ai/ai.h"
#include "ai/aigoals.h"
#include "ship/ship.h"
#include "object/object.h"
#include "object/objcollide.h"
#include "parse/sexp.h"
#include "freespace2/freespace.h"
#include "globalincs/linklist.h"
#include "iff_defs/iff_defs.h"
#include "object/waypoint/waypoint.h"
#include "sound/audiostr.h"
#include "mission/missiontraining.h"
#include "mission/missionmessage.h"
#include "io/timer.h"
#include "gamesnd/eventmusic.h"
#include "cfile/cfile.h"
#include "parse/parselo.h"
#include "globalincs/def_files.h"
#include "localization/localize.h"
#include "camera/camera.h"
#include "asteroid/asteroid.h"

// Extern functions/variables
extern int		Player_use_ai;
extern int get_wing_index(object *objp, int wingnum);
extern object * get_wing_leader(int wingnum);

// Module variables
bool AutoPilotEngaged;
bool UseCutsceneBars;
int CurrentNav;
float ramp_bias;
NavPoint Navs[MAX_NAVPOINTS];
NavMessage NavMsgs[NP_NUM_MESSAGES];
int audio_handle;
int NavLinkDistance;

// time offsets for autonav events
int LockAPConv;
int EndAPCinematic;
bool CinematicStarted;
vec3d cameraPos;

// used for ramping time compression;
int start_dist;

// ********************************************************************************************

void autopilot_ai_waypoint_goal_fixup(ai_goal* aigp)
{
	// this function sets wp_index properly;
	for (int i = 0; i < Num_waypoint_lists; i++)
	{
		if (!stricmp(aigp->ship_name, Waypoint_lists[i].name))
		{
			aigp->wp_index = i;
			return;
		}
	}
}


// ********************************************************************************************
bool Sel_NextNav()
{
	if (AutoPilotEngaged)
		return false;

	int i;
	if (CurrentNav == -1)
	{
		for (i = 0; i < MAX_NAVPOINTS; i++)
		{
			if (Navs[i].flags & NP_VALIDTYPE && !(Navs[i].flags & NP_NOSELECT))
			{
				CurrentNav=i;
				return true;
			}
		}		
	}
	else
	{
		for (i = CurrentNav+1; i < MAX_NAVPOINTS+CurrentNav; i++)
		{
			if (Navs[i%MAX_NAVPOINTS].flags & NP_VALIDTYPE && !(Navs[i%MAX_NAVPOINTS].flags & NP_NOSELECT))
			{
				if (i != CurrentNav)
				{
					CurrentNav=i%MAX_NAVPOINTS;
					return true;
				}
			}
		}
	}
	return false;
}


// ********************************************************************************************
vec3d *NavPoint::GetPosition()
{
	if (flags & NP_WAYPOINT)
	{
		return &((waypoint_list*) target_obj)->waypoints[waypoint_num-1];
	}
	else
	{
		return &Objects[((ship*) target_obj)->objnum].pos;
	}
}

char* NavPoint::GetInteralName()
{
	char *NavName;
	char strtmp[33];

	if (flags & NP_WAYPOINT)
	{
		NavName = new char[strlen(((waypoint_list*)target_obj)->name)+5];
		memset(NavName, 0, strlen(((waypoint_list*)target_obj)->name)+5);
		strcpy(NavName, ((waypoint_list*)target_obj)->name);

		strcat(NavName, ":");
		sprintf(strtmp, "%d", waypoint_num);
		strcat(NavName, strtmp);
	}
	else
	{		
		NavName = new char[strlen(((ship*)target_obj)->ship_name)+1];
		memset(NavName, 0, strlen(((ship*)target_obj)->ship_name)+1);
		strcpy(NavName, ((ship*)target_obj)->ship_name);
	}

	return NavName;
}

// ********************************************************************************************
// Tell us if autopilot is allowed
// This needs:
//        * Nav point selected
//        * No enemies within X distance
//        * Destination > 1,000 meters away
bool CanAutopilot(bool send_msg)
{
	if (CurrentNav == -1)
	{
		if (send_msg)
					send_autopilot_msgID(NP_MSG_FAIL_NOSEL);
		return false;
	}
		
	if (object_get_gliding(Player_obj))
	{
		if (send_msg)
					send_autopilot_msgID(NP_MSG_FAIL_GLIDING);
		return false;
	}
	// You cannot autopilot if you're within 1000 meters of your destination nav point
	if (vm_vec_dist_quick(&Player_obj->pos, Navs[CurrentNav].GetPosition()) < 1000)
	{
		if (send_msg)
					send_autopilot_msgID(NP_MSG_FAIL_TOCLOSE);
		return false;
	}

	// see if any hostiles are nearby
	for (ship_obj *so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so))
	{
		object *other_objp = &Objects[so->objnum];

		// attacks player?
		if (iff_x_attacks_y(obj_team(other_objp), obj_team(Player_obj)))
		{
			// Cannot autopilot if enemy within 5,000 meters
			if (vm_vec_dist_quick(&Player_obj->pos, &other_objp->pos) < 5000)
			{
				if (send_msg)
					send_autopilot_msgID(NP_MSG_FAIL_HOSTILES);
				return false;
			}
		}
	}

	//check for asteroids	
	for (int n=0; n<MAX_ASTEROIDS; n++) 
	{
		// asteroid
		if (Asteroids[n].flags & AF_USED)
		{
			// Cannot autopilot if asteroid within 1,000 meters
			if (vm_vec_dist_quick(&Player_obj->pos, &Objects[Asteroids[n].objnum].pos) < 1000)
			{
				if (send_msg)
					send_autopilot_msgID(NP_MSG_FAIL_HAZARD);
				return false;
			}
		}
	}

	return true;
}

// ********************************************************************************************
// Tell us if autopilot is allowed at a certain position (only performs checks based on that position)
// This needs:
//        * Nav point selected
//        * No enemies within X distance
//        * Destination > 1,000 meters away
bool CanAutopilotPos(vec3d targetPos)
{
	// You cannot autopilot if you're within 1000 meters of your destination nav point
	if (vm_vec_dist_quick(&targetPos, Navs[CurrentNav].GetPosition()) < 1000)
		return false;
	// see if any hostiles are nearby
	for (ship_obj *so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so))
	{
		object *other_objp = &Objects[so->objnum];
		// attacks player?
		if (iff_x_attacks_y(obj_team(other_objp), obj_team(Player_obj)))
		{
			// Cannot autopilot if enemy within 5,000 meters
			if (vm_vec_dist_quick(&targetPos, &other_objp->pos) < 5000)
				return false;
		}
	}
	
	//check for asteroids	
	for (int n=0; n<MAX_ASTEROIDS; n++) 
	{
		// asteroid
		if (Asteroids[n].flags & AF_USED)
		{
			// Cannot autopilot if asteroid within 1,000 meters
			if (vm_vec_dist_quick(&targetPos, &Objects[Asteroids[n].objnum].pos) < 1000)
				return false;
		}
	}

	return true;
}

// ********************************************************************************************
// Engages autopilot
// This does:
//        * Control switched from player to AI
//        * Time compression to 32x
//        * Lock time compression -WMC
//        * Turn off gliding -Backslash
//        * Lock primary/secondary weapons
//        * Tell AI to fly to targetted Nav Point (for all nav-status wings/ships)
//		  * Sets max waypoint speed to the best-speed of the slowest ship tagged
void StartAutopilot()
{
	if (!CanAutopilot())
		return;

	AutoPilotEngaged = true;

	if (The_mission.flags & MISSION_FLAG_USE_AP_CINEMATICS)
		LockAPConv = timestamp(); // lock convergence instantly
	else
		LockAPConv = timestamp(3000); // 3 seconds before we lock convergence
	Player_use_ai = 1;
	set_time_compression(1);
	lock_time_compression(true);

	// determine speed cap
	int i,j, wcount=1;
	float speed_cap = 1000000.0; // 1m is a safe starting point
	float radius = Player_obj->radius;
	
	// vars for usage w/ cinematic
	vec3d pos, norm1, perp, tpos, rpos = Player_obj->pos, zero;
	memset(&zero, 0, sizeof(vec3d));


	// instantly turn player toward tpos
	if (The_mission.flags & MISSION_FLAG_USE_AP_CINEMATICS)
	{
		vm_vec_sub(&norm1, Navs[CurrentNav].GetPosition(), &Player_obj->pos);
		vm_vector_2_matrix(&Player_obj->orient, &norm1, NULL, NULL);
	}

	for (i = 0; i < MAX_SHIPS; i++)
	{
		if (Ships[i].objnum != -1 && 
				(Ships[i].flags2 & SF2_NAVPOINT_CARRY || 
					(Ships[i].wingnum != -1 && Wings[Ships[i].wingnum].flags & WF_NAV_CARRY)
				)
			)
		{
			if (speed_cap > vm_vec_mag(&Ship_info[Ships[i].ship_info_index].max_vel))
				speed_cap = vm_vec_mag(&Ship_info[Ships[i].ship_info_index].max_vel);
		}
	}


	// damp speed_cap to 90% of actual -- to make sure ships stay in formation
	if (The_mission.flags & MISSION_FLAG_USE_AP_CINEMATICS)
		speed_cap = 0.90f * speed_cap;
	ramp_bias = speed_cap/50.0f;

	// assign ship goals
	// when assigning goals to individual ships only do so if Ships[shipnum].wingnum != -1 
	// we will assign wing goals below

	for (i = 0; i < MAX_SHIPS; i++)
	{
		if (Ships[i].objnum != -1 && 
				(Ships[i].flags2 & SF2_NAVPOINT_CARRY || 
					(Ships[i].wingnum != -1 && Wings[Ships[i].wingnum].flags & WF_NAV_CARRY)
				)
			)
		{
			// turn off glide
			if (object_get_gliding(&Objects[Ships[i].objnum]))
				object_set_gliding(&Objects[Ships[i].objnum], false);

			// check for bigger radius for usage later
			/*if (!vm_vec_cmp(&rpos, &Player_obj->pos)) 
				// want to make sure rpos isn't player pos - we can worry about it being largest object's later
			{
				rpos = Objects[Ships[i].objnum].pos;
			}*/

			if (Objects[Ships[i].objnum].radius > radius)
			{
				rpos = Objects[Ships[i].objnum].pos;
				radius = Objects[Ships[i].objnum].radius;
			}

			if (The_mission.flags & MISSION_FLAG_USE_AP_CINEMATICS)
			{// instantly turn the ship to match the direction player is looking
				//vm_vec_sub(&norm1, Navs[CurrentNav].GetPosition(), &Player_obj->pos);
				vm_vector_2_matrix(&Objects[Ships[i].objnum].orient, &norm1, NULL, NULL);
			}

			// snap wings into formation them into formation
			if (The_mission.flags & MISSION_FLAG_USE_AP_CINEMATICS &&  // only if using cinematics 
				(Ships[i].wingnum != -1 && Wings[Ships[i].wingnum].flags & WF_NAV_CARRY) // only if in a wing
				&& Player_obj != &Objects[Ships[i].objnum]) //only if not player object
			{	
				ai_info	*aip = aip = &Ai_info[Ships[i].ai_index];
				int wingnum = aip->wing, wing_index = get_wing_index(&Objects[Ships[i].objnum], wingnum);
				vec3d goal_point;
				object *leader_objp = get_wing_leader(wingnum);

				if (leader_objp != &Objects[Ships[i].objnum])
				{
					// not leader.. get our position relative to leader
					get_absolute_wing_pos(&goal_point, leader_objp, wing_index, aip->ai_flags & AIF_FORMATION_OBJECT);
				}
				else
				{
					ai_clear_wing_goals(wingnum);
					switch (wcount % 2)
					{
						case 1: // back-left
							vm_vec_sub(&perp, &zero, &Player_obj->orient.vec.rvec);
							vm_vec_sub(&perp, &perp, &Player_obj->orient.vec.fvec);
							vm_vec_normalize(&perp);
							vm_vec_scale(&perp, 166.0f*float((wcount+1)/2)); // 166m is supposedly the optimal range according to tolwyn
							vm_vec_add(&goal_point, &Player_obj->pos, &perp);
							break;

						default: //back-right
						case 0:
							vm_vec_add(&perp, &zero, &Player_obj->orient.vec.rvec);
							vm_vec_sub(&perp, &perp, &Player_obj->orient.vec.fvec);
							vm_vec_normalize(&perp);
							vm_vec_scale(&perp, 166.0f*float(wcount/2));
							vm_vec_add(&goal_point, &Player_obj->pos, &perp);
							break;
					}
					wcount++;

				}
				Objects[Ships[i].objnum].pos = goal_point;
			}
			// lock primary and secondary weapons
			Ships[i].flags2 |= (SF2_PRIMARIES_LOCKED | SF2_SECONDARIES_LOCKED);

			// clear the ship goals and cap the waypoint speed
			ai_clear_ship_goals(&Ai_info[Ships[i].ai_index]);
			Ai_info[Ships[i].ai_index].waypoint_speed_cap = (int)speed_cap;

			
			// if they're not part of a wing set their goal
			if (Ships[i].wingnum == -1 || The_mission.flags & MISSION_FLAG_USE_AP_CINEMATICS)
			{ 
				if (Navs[CurrentNav].flags & NP_WAYPOINT)
				{
					ai_add_ship_goal_player( AIG_TYPE_PLAYER_WING, AI_GOAL_WAYPOINTS_ONCE, 0, ((waypoint_list*)Navs[CurrentNav].target_obj)->name, &Ai_info[Ships[i].ai_index] );
					//fixup has to wait until after wing goals
				}
				else
				{
					ai_add_ship_goal_player( AIG_TYPE_PLAYER_WING, AI_GOAL_FLY_TO_SHIP, 0, ((ship*)Navs[CurrentNav].target_obj)->ship_name, &Ai_info[Ships[i].ai_index] );
				}

			}
		}
	}

	// assign wing goals
	if (!(The_mission.flags & MISSION_FLAG_USE_AP_CINEMATICS))
	{
		for (i = 0; i < MAX_WINGS; i++)
		{
			if (Wings[i].flags & WF_NAV_CARRY )
			{	
				//ai_add_ship_goal_player( int type, int mode, int submode, char *shipname, ai_info *aip );

				//ai_add_wing_goal_player( AIG_TYPE_PLAYER_WING, AI_GOAL_STAY_NEAR_SHIP, 0, target_shipname, wingnum );
				//ai_add_wing_goal_player( AIG_TYPE_PLAYER_WING, AI_GOAL_WAYPOINTS_ONCE, 0, target_shipname, wingnum );
				//ai_clear_ship_goals( &(Ai_info[Ships[num].ai_index]) );
				
				ai_clear_wing_goals( i );
				if (Navs[CurrentNav].flags & NP_WAYPOINT)
				{
					
					ai_add_wing_goal_player( AIG_TYPE_PLAYER_WING, AI_GOAL_WAYPOINTS_ONCE, 0, ((waypoint_list*)Navs[CurrentNav].target_obj)->name, i );

					// "fix up" the goal
					for (j = 0; j < MAX_AI_GOALS; j++)
					{
						if (Wings[i].ai_goals[j].ai_mode == AI_GOAL_WAYPOINTS_ONCE ||
							Wings[i].ai_goals[j].ai_mode == AIM_WAYPOINTS )
						{
							autopilot_ai_waypoint_goal_fixup(&(Wings[i].ai_goals[j]));
						}
					}
				}
				else
				{
					ai_add_wing_goal_player( AIG_TYPE_PLAYER_WING, AI_GOAL_FLY_TO_SHIP, 0, ((ship*)Navs[CurrentNav].target_obj)->ship_name, i );

				}
			}
		}
	}

	// fixup has to go down here because ships are assigned goals during wing goals as well
	for (i = 0; i < MAX_SHIPS; i++)
	{
		if (Ships[i].objnum != -1)
		{
			if (Ships[i].flags2 & SF2_NAVPOINT_CARRY || 
				(Ships[i].wingnum != -1 && Wings[Ships[i].wingnum].flags & WF_NAV_CARRY))
				for (j = 0; j < MAX_AI_GOALS; j++)
				{
					if (Ai_info[Ships[i].ai_index].goals[j].ai_mode == AI_GOAL_WAYPOINTS_ONCE ||
						Ai_info[Ships[i].ai_index].goals[j].ai_mode == AIM_WAYPOINTS)
					{
						autopilot_ai_waypoint_goal_fixup( &(Ai_info[Ships[i].ai_index].goals[j]) );

						
						// formation fixup
						//ai_form_on_wing(&Objects[Ships[i].objnum], &Objects[Player_ship->objnum]);
					}
				}
		}
	}
	start_dist = DistanceTo(CurrentNav);

	// ----------------------------- setup cinematic -----------------------------
	if (The_mission.flags & MISSION_FLAG_USE_AP_CINEMATICS)
	{
		
		tpos = *Navs[CurrentNav].GetPosition();
		// determine distance toward nav at which camera will be
		vm_vec_sub(&pos, &tpos, &Player_obj->pos);
		vm_vec_normalize(&pos);
		norm1 = pos;
		vm_vec_scale(&pos, 5.0f*speed_cap);
		vm_vec_add(&pos, &pos, &Player_obj->pos);

		switch (myrand()%16) 
		// 8 different ways of getting perp points
		{

			case 1: // down
			case 9:
				vm_vec_sub(&perp, &zero, &Player_obj->orient.vec.uvec);
				break;

			case 2: // up
			case 10:
				perp = Player_obj->orient.vec.uvec;
				break;

			case 3: // left
			case 11:
				vm_vec_sub(&perp, &zero, &Player_obj->orient.vec.rvec);
				break;

			case 4: // up-left
			case 12:
				vm_vec_sub(&perp, &zero, &Player_obj->orient.vec.rvec);
				vm_vec_add(&perp, &perp, &Player_obj->orient.vec.uvec);
				break;

			case 5: // up-right
			case 13:
				vm_vec_add(&perp, &zero, &Player_obj->orient.vec.rvec);
				vm_vec_add(&perp, &perp, &Player_obj->orient.vec.uvec);
				break;

			case 6: // down-left
			case 14:
				vm_vec_sub(&perp, &zero, &Player_obj->orient.vec.rvec);
				vm_vec_sub(&perp, &perp, &Player_obj->orient.vec.uvec);
				break;

			case 7: // down-right
			case 15:
				vm_vec_add(&perp, &zero, &Player_obj->orient.vec.rvec);
				vm_vec_sub(&perp, &perp, &Player_obj->orient.vec.uvec);
				break;

			default:
			case 0: // right
			case 8:
				perp = Player_obj->orient.vec.rvec;
				break;

		}
		vm_vec_normalize(&perp);
		vm_vec_scale(&perp, 2*radius);
		vm_vec_add(&cameraPos, &pos, &perp);

	}
}

// ********************************************************************************************
// Checks if autopilot should automatically die
// Returns true if:
//         * Targetted waypoint < 10,000 meters away
//         * Enemy < 10,000 meters
bool Autopilot_AutoDiable()
{
	return !CanAutopilot();
}

// ********************************************************************************************
// Disengages autopilot
// this does:
//         * Time compression to 1x
//         * Delete AI nav goal
//         * Control switched from AI to player
void EndAutoPilot()
{
	AutoPilotEngaged = false;

	set_time_compression(1);
	lock_time_compression(false);
	Player_use_ai = 0;
	//Clear AI Goals

	if (CinematicStarted) // clear cinematic if we need to
	{
		if (UseCutsceneBars)
		{
			Cutscene_bar_flags &= ~CUB_CUTSCENE;
		}
		Viewer_mode &= ~VM_FREECAMERA;
		hud_set_draw(1);
		CinematicStarted = false;
	}

	// assign ship goals
	// when assigning goals to individual ships only do so if Ships[shipnum].wingnum != -1 
	// we will assign wing goals below
	int i,j;

	for (i = 0; i < MAX_SHIPS; i++)

	{
		if (Ships[i].objnum != -1 && 
			(
				Ships[i].flags2 & SF2_NAVPOINT_CARRY || 
				(Ships[i].wingnum != -1 && Wings[Ships[i].wingnum].flags & WF_NAV_CARRY )
			 )
		   )
		{
			//unlock their weaponry
			Ships[i].flags2 &= ~(SF2_PRIMARIES_LOCKED | SF2_SECONDARIES_LOCKED);
			Ai_info[Ships[i].ai_index].waypoint_speed_cap = -1; // uncap their speed
			
			ai_clear_ship_goals( &(Ai_info[Ships[i].ai_index]) );
			if (Ships[i].wingnum != -1)
				ai_clear_wing_goals( Ships[i].wingnum );

		}
	}

	// assign wing goals
	
	if (!(The_mission.flags & MISSION_FLAG_USE_AP_CINEMATICS))
	{
		for (i = 0; i < MAX_WINGS; i++)
		{
			if (Wings[i].flags & WF_NAV_CARRY )
			{
				// old "dumb" routine
				//ai_clear_wing_goals( i );
				for (j = 0; j < MAX_AI_GOALS; j++)
				{
					if (Wings[i].ai_goals[j].ai_mode == AI_GOAL_WAYPOINTS_ONCE ||
						Wings[i].ai_goals[j].ai_mode == AI_GOAL_FLY_TO_SHIP ||
						Wings[i].ai_goals[j].ai_mode == AIM_WAYPOINTS ||
						Wings[i].ai_goals[j].ai_mode == AIM_FLY_TO_SHIP)
					{
						Wings[i].ai_goals[j].ai_mode = AI_GOAL_NONE;
						Wings[i].ai_goals[j].signature = -1;
						Wings[i].ai_goals[j].priority = -1;
						Wings[i].ai_goals[j].flags = 0;
					}
				}
			}
		}
	}
}


// ********************************************************************************************

void camera_face(vec3d &loc)
{
	vec3d destvec;
	matrix cam_orient;
	vm_vec_sub(&destvec, &loc, Free_camera->get_position());
	vm_vector_2_matrix(&cam_orient, &destvec, NULL, NULL);
	Free_camera->set_rotation(&cam_orient, 0.0f, 0.0f);
}

void nav_warp(bool prewarp=false)
{
	// ok... find our end distance - norm1 is still a unit vector in the direction from the player to the navpoint
	vec3d targetPos, tpos=Player_obj->pos, pos;
	
	vm_vec_sub(&pos, Navs[CurrentNav].GetPosition(), &Player_obj->pos);
	vm_vec_normalize(&pos);
	vm_vec_scale(&pos, 250.0f); // we move by increments of 250
	
	while (CanAutopilotPos(tpos))
	{
		vm_vec_add(&tpos, &tpos, &pos);
	}
	vm_vec_sub(&targetPos, &tpos, &Player_obj->pos); //targetPos is actually a projection in a the direction toward the nav

	if (prewarp)
	{
		vm_vec_scale(&targetPos, 0.5);
		vm_vec_add(&cameraPos, &cameraPos, &targetPos);
	}

	for (int i = 0; i < MAX_SHIPS; i++)
	{
		if (Ships[i].objnum != -1 && 
				(Ships[i].flags2 & SF2_NAVPOINT_CARRY || 
					(Ships[i].wingnum != -1 && Wings[Ships[i].wingnum].flags & WF_NAV_CARRY)
				)
			)
		{
				vm_vec_add(&Objects[Ships[i].objnum].pos, &Objects[Ships[i].objnum].pos, &targetPos);
		}
	}

	// retime all collision pairs
	obj_all_collisions_retime();

}

// ********************************************************************************************
// Checks for changes every NPS_TICKRATE milliseconds
// Checks:
//			* if we've gotten close enough to a nav point for it to be counted as "Visited"
//			* If we're current AutoNavigating it checks if we need to autodisengage
void NavSystem_Do()
{
	static unsigned int last_update = 0;
	if (clock () - last_update > NPS_TICKRATE)
	{
		if (AutoPilotEngaged)
		{
			if (The_mission.flags & MISSION_FLAG_USE_AP_CINEMATICS)
			{
				if (CinematicStarted)
				{
					// update our cinematic and possibly perform warp
					camera_face(Player_obj->pos);

					if (timestamp() >= EndAPCinematic || Autopilot_AutoDiable())
					{
						nav_warp();
						EndAutoPilot();
					}
				}
				else
				{
					// start cinematic
					if (UseCutsceneBars)
					{
						Cutscene_bar_flags |= CUB_CUTSCENE;
						Cutscene_bar_flags &= ~CUB_GRADUAL;
					}
					Viewer_mode |= VM_FREECAMERA;
					hud_set_draw(0);
					nav_warp(true);

					Free_camera->set_position(&cameraPos);
					camera_face(Player_obj->pos);
					EndAPCinematic = timestamp(10000); // 10 seconds before end of cinematic
					CinematicStarted = true;
				}
			}
			else
			{
				if (Autopilot_AutoDiable())
					EndAutoPilot();

			}

		}
		// check if a NavPoints target has left, delete it if so
		int i;

		for (i = 0; i < MAX_NAVPOINTS; i++)
		{
			if ((Navs[i].flags & NP_SHIP) && (Navs[i].target_obj != NULL))
			{
				if (((ship*)Navs[i].target_obj)->objnum == -1)
					DelNavPoint(i);
			}
		}
		
		// check if we're reached a Node
		for (i = 0; i < MAX_NAVPOINTS; i++)
		{
			if (Navs[i].target_obj != NULL)
			{
				if (Navs[i].flags & NP_VALIDTYPE && DistanceTo(i) < 1000)
					Navs[i].flags |= NP_VISITED;
			}
		}
	}

	// ramp time compression - only if not using cinematics
	if (AutoPilotEngaged && !(The_mission.flags & MISSION_FLAG_USE_AP_CINEMATICS))
	{
		int dstfrm_start = start_dist - DistanceTo(CurrentNav);

		// Ramp UP time compression
		if (dstfrm_start < (3500*ramp_bias))
		{

			if (dstfrm_start >= (3000*ramp_bias) && DistanceTo(CurrentNav) > 30000)
				set_time_compression(64);
			else if (dstfrm_start >= (2000*ramp_bias))
				set_time_compression(32);
			else if (dstfrm_start >= (1600*ramp_bias))
				set_time_compression(16);
			else if (dstfrm_start >= (1200*ramp_bias))
				set_time_compression(8);
			else if (dstfrm_start >= (800*ramp_bias))
				set_time_compression(4);
			else if (dstfrm_start >= (400*ramp_bias))
				set_time_compression(2);
		}

		// Ramp DOWN time compression
		if (DistanceTo(CurrentNav) <= (7000*ramp_bias))
		{
			int dist = DistanceTo(CurrentNav);
			if (dist >= (5000*ramp_bias))
				set_time_compression(32);
			else if (dist >= (4000*ramp_bias))
				set_time_compression(16);
			else if (dist >= (3000*ramp_bias))
				set_time_compression(8);
			else if (dist >= (2000*ramp_bias))
				set_time_compression(4);
			else if (dist >= (1000*ramp_bias))
				set_time_compression(2);
		}
	}

	// autopilot linking
	for (int i = 0; i < MAX_SHIPS; i++)
	{
		if (Ships[i].objnum != -1 && Ships[i].flags2 & SF2_NAVPOINT_NEEDSLINK)
		{
			object *other_objp = &Objects[Ships[i].objnum];

			if (vm_vec_dist_quick(&Player_obj->pos, &other_objp->pos) < (NavLinkDistance + other_objp->radius))
			{
				Ships[i].flags2 &= ~SF2_NAVPOINT_NEEDSLINK;
				Ships[i].flags2 |= SF2_NAVPOINT_CARRY;
				
				send_autopilot_msgID(NP_MSG_MISC_LINKED);
			}
		
		}
	}
}
// ********************************************************************************************

void send_autopilot_msgID(int msgid)
{
	if (msgid < 0 || msgid > NP_NUM_MESSAGES)
		return;

	send_autopilot_msg(NavMsgs[msgid].message, NavMsgs[msgid].filename);
}
// ********************************************************************************************

void send_autopilot_msg(char *msg, char *snd)
{
	// setup
	if (audio_handle != -1)
	{
		audiostream_close_file(audio_handle, 0);
		audio_handle = -1;
	}

	if (strlen(msg) != 0 && strcmp(msg, "none"))
		change_message("autopilot builtin message", msg, -1, 0);

	// load sound
	if (snd != NULL || strlen(snd) == 0 || !strcmp(snd, "none"))
	{
		audio_handle = audiostream_open(snd, ASF_MENUMUSIC );
	}

	// send/play
	if (audio_handle != -1)
	{
		audiostream_play(audio_handle, Master_event_music_volume, 0);
	}

	if (strlen(msg) != 0 && strcmp(msg, "none"))
		message_training_queue("autopilot builtin message", timestamp(0), 5); // display message for five seconds
}

// ********************************************************************************************
// Inits the Nav System
void NavSystem_Init()
{
	memset((char *)&Navs, 0, sizeof(Navs));
	AutoPilotEngaged = false;
	CurrentNav = -1;
	audio_handle = -1;
	CinematicStarted = false;
	UseCutsceneBars = true;

	// defaults... can be tabled or bound to mission later
	if (cf_exists_full("autopilot.tbl", CF_TYPE_TABLES))
		parse_autopilot_table("autopilot.tbl");
	else
		parse_autopilot_table(NULL);
}

// ********************************************************************************************

void parse_autopilot_table(char *longname)
{
	int rval;

	// open localization
	lcl_ext_open();

	if ((rval = setjmp(parse_abort)) != 0)
	{
		mprintf(("TABLES: Unable to parse '%s'.  Code = %i.\n", rval, (longname) ? longname : NOX("<default>")));
		return;
	}
	else
	{
		if (longname == NULL)
		{
			read_file_text_from_array(defaults_get_file("autopilot.tbl"));
		}
		else
		{
			read_file_text(longname);
		}

		reset_parse();		
	}
	std::vector<std::string> lines;
	
	required_string("#Autopilot");

	// autopilot link distance
	required_string("$Link Distance:");
	stuff_int(&NavLinkDistance);

	// optional no cutscene bars
	if (optional_string("+No_Cutscene_Bars"))
		UseCutsceneBars = false;

	// No Nav selected message
	char *msg_tags[] = { "$No Nav Selected:", "$Gliding:", "$Too Close:", "$Hostiles:", "$Linked:", "$Hazard:" };
	for (int i = 0; i < NP_NUM_MESSAGES; i++)
	{
		required_string(msg_tags[i]);
		
		required_string("+Msg:");
		stuff_string(NavMsgs[i].message, F_MESSAGE, 256);

		required_string("+Snd File:");
		stuff_string(NavMsgs[i].filename, F_NAME, 256);
	}


	required_string("#END");
}


// ********************************************************************************************
// Finds a Nav point by name
int FindNav(char *Nav)
{
	for (int i = 0; i < MAX_NAVPOINTS; i++)
	{
		if (!stricmp(Navs[i].NavName, Nav))
			return i;
	}

	return -1;
}

// ********************************************************************************************
// Set A Nav point to "ZERO"
void ZeroNav(int i)
{
	memset((char *)&Navs[i], 0, sizeof(NavPoint));
}

// ********************************************************************************************
// Removes a Nav
bool DelNavPoint(char *Nav)
{
	int n = FindNav(Nav);

	return DelNavPoint(n);

}


bool DelNavPoint(int nav)
{	
	if (nav != -1)
	{
		if (nav != MAX_NAVPOINTS-1)
		{
			for (int i = nav; i < MAX_NAVPOINTS-1; i++)
			{
				Navs[nav] = Navs[nav+1];
			}

		}
		ZeroNav(MAX_NAVPOINTS-1);
		return true;
	}

	return false;
}

// ********************************************************************************************
// adds a Nav
bool AddNav_Ship(char *Nav, char *TargetName, int flags)
{
	// find an empty nav - should be the end

	int empty = -1;
	int i;

	for (i = 0; i < MAX_NAVPOINTS && empty == -1; i++)
	{
		if (Navs[i].flags == 0)
			empty = i;
	}

	if (empty == -1) // no empty navpoint slots
		return false;

	// Create the NavPoint struct
	NavPoint tnav;

	strncpy(tnav.NavName, Nav, 32);
	tnav.flags = NP_SHIP | flags;

	Assert(!(tnav.flags & NP_WAYPOINT));


	for (i = 0; i < MAX_SHIPS; i++)
	{
		if (Ships[i].objnum != -1 && !stricmp(TargetName, Ships[i].ship_name))
		{
			tnav.target_obj = (void *)&Ships[i];		
		}
	}

	tnav.waypoint_num = 0; // unused for NP_SHIP


	// copy it into it's location
	Navs[empty] = tnav;

	return true;
}


bool AddNav_Waypoint(char *Nav, char *WP_Path, int node, int flags)
{
	// find an empty nav - should be the end

	int empty = -1;
	int i;

	if (node == 0)
		node = 1;

	for (i = 0; i < MAX_NAVPOINTS && empty == -1; i++)
	{
		if (Navs[i].flags == 0)
			empty = i;
	}

	if (empty == -1) // no empty navpoint slots
		return false;

	// Create the NavPoint struct
	NavPoint tnav;

	strncpy(tnav.NavName, Nav, 32);
	tnav.flags = NP_WAYPOINT | flags;

	Assert(!(tnav.flags & NP_SHIP));


	for (i = 0; i < Num_waypoint_lists; i++)
	{
		if (!stricmp(WP_Path, Waypoint_lists[i].name))
		{
			tnav.target_obj = (void *)&Waypoint_lists[i];		
		}
	}

	tnav.waypoint_num = node;

	// copy it into it's location
	Navs[empty] = tnav;

	return true;
}

// ********************************************************************************************
//Change Flags
bool Nav_Alt_Flags(char *Nav, int flags)
{
	flags &= ~NP_VALIDTYPE; //clear the NP_SHIP and NP_WAYPOINT bits, make sure they haven't been set

	int nav = FindNav(Nav);

	if (nav != -1)
	{
		Navs[nav].flags &= NP_VALIDTYPE; // Clear all bits BUT NP_SHIP or NO_WAYPOINT
		Navs[nav].flags |= flags; // merge
	}

	return (nav != -1);
}

//Get Flags
int Nav_Get_Flags(char *Nav)
{
	int flags = 0;

	int nav = FindNav(Nav);

	if (nav != -1)
		flags = Navs[nav].flags;

	return flags;
}

// ********************************************************************************************
// Sexp Accessors

//Generic
bool Nav_Set_Flag(char *Nav, int flag)
{
	Assert(!(flag & NP_VALIDTYPE));
	int nav = FindNav(Nav);

	if (nav != -1)
	{
		Navs[nav].flags |= flag;
		return true;
	}

	return false;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

bool Nav_UnSet_Flag(char *Nav, int flag)
{
	Assert(!(flag & NP_VALIDTYPE));

	int nav = FindNav(Nav);

	if (nav != -1)
	{
		Navs[nav].flags &= ~flag;
		return true;
	}

	return false;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//Named
bool Nav_Set_Hidden(char *Nav)
{
	return Nav_Set_Flag(Nav, NP_HIDDEN);

}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

bool Nav_Set_NoAccess(char *Nav)
{
	return Nav_Set_Flag(Nav, NP_NOACCESS);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

bool Nav_Set_Visited(char *Nav)
{
	
	return Nav_Set_Flag(Nav, NP_VISITED);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

bool Nav_UnSet_Hidden(char *Nav)
{
	return Nav_UnSet_Flag(Nav, NP_HIDDEN);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

bool Nav_UnSet_NoAccess(char *Nav)
{
	return Nav_UnSet_Flag(Nav, NP_NOACCESS);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

bool Nav_UnSet_Visited(char *Nav)
{
	return Nav_UnSet_Flag(Nav, NP_VISITED);
}



//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


unsigned int DistanceTo(char *nav)
{
	int n = FindNav(nav);

	return DistanceTo(n);
}

unsigned int DistanceTo(int nav)
{
	if (nav > MAX_NAVPOINTS && nav < 0)
		return 0xFFFFFFFF;

	//WMC - Get rid of this bloody warning. Yes, we're casting to a uint.
	//Deal with it.
	return (uint)vm_vec_dist_quick(&Player_obj->pos, Navs[nav].GetPosition());
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

bool IsVisited(char *nav)
{
	int n = FindNav(nav);

	return IsVisited(n);
}

bool IsVisited(int nav)
{
	if (nav > MAX_NAVPOINTS && nav < 0)
		return 0;

	if (Navs[nav].flags & NP_VISITED)
		return 1;
	return 0;
}


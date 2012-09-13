#include "object/waypoint.h"
#include "object/object.h"
#include "globalincs/linklist.h"

//********************GLOBALS********************
SCP_list<waypoint_list> Waypoint_lists;

SCP_list<waypoint> dummy_waypoint;
const SCP_list<waypoint>::iterator INVALID_WAYPOINT_POSITION = dummy_waypoint.end();

//********************CLASS MEMBERS********************
waypoint::waypoint()
{
	this->m_position.xyz.x = 0.0f;
	this->m_position.xyz.y = 0.0f;
	this->m_position.xyz.z = 0.0f;

	this->objnum = -1;
	this->m_parent_list = NULL;
}

waypoint::waypoint(vec3d *position, waypoint_list *parent_list)
{
	Assert(position != NULL);

	this->m_position.xyz.x = position->xyz.x;
	this->m_position.xyz.y = position->xyz.y;
	this->m_position.xyz.z = position->xyz.z;

	this->objnum = -1;
	this->m_parent_list = parent_list;
}

waypoint::~waypoint()
{
	// nothing to do
}

vec3d *waypoint::get_pos()
{
	return &m_position;
}

int waypoint::get_objnum()
{
	return objnum;
}

waypoint_list *waypoint::get_parent_list()
{
	return m_parent_list;
}

void waypoint::set_pos(vec3d *pos)
{
	Assert(pos != NULL);
	this->m_position = *pos;
}

waypoint_list::waypoint_list()
{
	this->m_name[0] = '\0';
}

waypoint_list::waypoint_list(const char *name)
{
	Assert(name != NULL);
	Assert(find_matching_waypoint_list(name) == NULL);
	strcpy_s(this->m_name, name);
}

waypoint_list::~waypoint_list()
{
	// nothing to do
}

char *waypoint_list::get_name()
{
	return m_name;
}

SCP_list<waypoint> &waypoint_list::get_waypoints()
{
	return waypoints;
}

void waypoint_list::set_name(const char *name)
{
	Assert(name != NULL);
	strcpy_s(this->m_name, name);
}

//********************FUNCTIONS********************
void waypoint_parse_init()
{
	Waypoint_lists.clear();
}

void waypoint_level_close()
{
	Waypoint_lists.clear();
}

int calc_waypoint_instance(int waypoint_list_index, int waypoint_index)
{
	Assert(waypoint_list_index >= 0);
	Assert(waypoint_index >= 0);
	return waypoint_list_index * 0x10000 + waypoint_index;
}

void calc_waypoint_indexes(int waypoint_instance, int &waypoint_list_index, int &waypoint_index)
{
	Assert(waypoint_instance >= 0);
	waypoint_list_index = calc_waypoint_list_index(waypoint_instance);
	waypoint_index = calc_waypoint_index(waypoint_instance);
}

int calc_waypoint_list_index(int waypoint_instance)
{
	Assert(waypoint_instance >= 0);
	return waypoint_instance / 0x10000;
}

int calc_waypoint_index(int waypoint_instance)
{
	Assert(waypoint_instance >= 0);
	return waypoint_instance & 0xffff;
}

void waypoint_create_game_object(waypoint *wpt, int list_index, int wpt_index)
{
	Assert(wpt != NULL);
	Assert(list_index >= 0);
	Assert(wpt_index >= 0);
	wpt->objnum = obj_create(OBJ_WAYPOINT, -1, calc_waypoint_instance(list_index, wpt_index), NULL, wpt->get_pos(), 0.0f, OF_RENDERS);
}

// done immediately after mission load; originally found in aicode.cpp
void waypoint_create_game_objects()
{
	SCP_list<waypoint_list>::iterator ii;
	SCP_list<waypoint>::iterator jj;

	int list = 0;
	for (ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++ii)
	{
		int wpt = 0;
		for (jj = ii->get_waypoints().begin(); jj != ii->get_waypoints().end(); ++jj)
		{
			waypoint_create_game_object(&(*jj), list, wpt);
			wpt++;
		}
		list++;
	}
}

waypoint_list *find_matching_waypoint_list(const char *name)
{
	Assert(name != NULL);
	SCP_list<waypoint_list>::iterator ii;

	for (ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++ii)
	{
		if (!stricmp(ii->get_name(), name))
			return &(*ii);
	}

	return NULL;
}

// NOTE: waypoint names are always in the format Name:index
waypoint *find_matching_waypoint(const char *name)
{
	Assert(name != NULL);
	SCP_list<waypoint_list>::iterator ii;

	for (ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++ii)
	{
		uint len = strlen(ii->get_name());

		// the first half (before the :) matches
		if (!strnicmp(ii->get_name(), name, len))
		{
			// this is ok because it could be "Waypoint path 1" vs. "Waypoint path 10"
			if (*(name + len) != ':')
				continue;

			// skip over the : to inspect a new string holding only the index
			const char *index_str = name + len + 1;
			if (*index_str == '\0')
			{
				nprintf(("waypoints", "possible error with waypoint name '%s': no waypoint number after the colon\n", name));
				continue;
			}

			// make sure it's actually a number
			bool valid = true;
			for (const char *ch = index_str; *ch != '\0'; ch++)
			{
				if (!isdigit(*ch))
				{
					valid = false;
					break;
				}
			}
			if (!valid)
			{
				nprintf(("waypoints", "possible error with waypoint name '%s': string after the colon is not a number\n", name));
				continue;
			}

			// get the number and make sure it's in range
			uint index = atoi(index_str);
			if (index < 1 || index > ii->get_waypoints().size())
			{
				nprintf(("waypoints", "possible error with waypoint name '%s': waypoint number is out of range\n", name));
				continue;
			}

			return find_waypoint_at_index(&(*ii), index - 1);
		}
	}

	return NULL;
}

waypoint *find_waypoint_with_objnum(int objnum)
{
	if (objnum < 0 || Objects[objnum].type != OBJ_WAYPOINT)
		return NULL;

	SCP_list<waypoint_list>::iterator ii;
	SCP_list<waypoint>::iterator jj;

	for (ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++ii)
	{
		for (jj = ii->get_waypoints().begin(); jj != ii->get_waypoints().end(); ++jj)
		{
			if (jj->get_objnum() == objnum)
				return &(*jj);
		}
	}

	return NULL;
}

waypoint_list *find_waypoint_list_with_instance(int waypoint_instance, int *waypoint_index)
{
	if (waypoint_instance < 0)
	{
		if (waypoint_index != NULL)
			*waypoint_index = -1;
		return NULL;
	}

	waypoint_list *wp_list = find_waypoint_list_at_index(calc_waypoint_list_index(waypoint_instance));
	if (wp_list == NULL)
	{
		if (waypoint_index != NULL)
			*waypoint_index = -1;
		return NULL;
	}

	if (waypoint_index != NULL)
	{
		*waypoint_index = calc_waypoint_index(waypoint_instance);
		Assert(*waypoint_index >= 0 && (uint) *waypoint_index < wp_list->get_waypoints().size());
	}
	return wp_list;
}

waypoint *find_waypoint_with_instance(int waypoint_instance)
{
	if (waypoint_instance < 0)
		return NULL;

	waypoint_list *wp_list = find_waypoint_list_at_index(calc_waypoint_list_index(waypoint_instance));
	if (wp_list == NULL)
		return NULL;

	return find_waypoint_at_index(wp_list, calc_waypoint_index(waypoint_instance));
}

waypoint_list *find_waypoint_list_at_index(int index)
{
	Assert(index >= 0);

	int i = 0;
	SCP_list<waypoint_list>::iterator ii;

	for (ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++i, ++ii)
	{
		if (i == index)
			return &(*ii);
	}

	return NULL;
}

waypoint *find_waypoint_at_index(waypoint_list *list, int index)
{
	Assert(list != NULL);
	Assert(index >= 0);

	int i = 0;
	SCP_list<waypoint>::iterator ii;

	for (ii = list->get_waypoints().begin(); ii != list->get_waypoints().end(); ++i, ++ii)
	{
		if (i == index)
			return &(*ii);
	}

	return NULL;
}

int find_index_of_waypoint_list(waypoint_list *wp_list)
{
	Assert(wp_list != NULL);
	SCP_list<waypoint_list>::iterator ii;

	int index = 0;
	for (ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++ii)
	{
		if (&(*ii) == wp_list)
			return index;
		index++;
	}

	return -1;
}

int find_index_of_waypoint(waypoint_list *wp_list, waypoint *wpt)
{
	Assert(wp_list != NULL);
	Assert(wpt != NULL);
	SCP_list<waypoint>::iterator ii;

	int index = 0;
	for (ii = wp_list->get_waypoints().begin(); ii != wp_list->get_waypoints().end(); ++ii)
	{
		if (&(*ii) == wpt)
			return index;
		index++;
	}

	return -1;
}

void waypoint_find_unique_name(char *dest_name, int start_index)
{
	Assert(dest_name != NULL);
	Assert(start_index >= 0);

	int index = start_index;
	waypoint_list *collision;

	do {
		sprintf(dest_name, "Waypoint path %d", index);
		index++;

		// valid name if no collision
		collision = find_matching_waypoint_list(dest_name);
	} while (collision != NULL);
}

void waypoint_add_list(const char *name, SCP_vector<vec3d> &vec_list)
{
	Assert(name != NULL);

	if (find_matching_waypoint_list(name) != NULL)
	{
		Warning(LOCATION, "Waypoint list '%s' already exists in this mission!  Not adding the new list...");
		return;
	}

	waypoint_list new_list(name);
	Waypoint_lists.push_back(new_list);
	waypoint_list *wp_list = &Waypoint_lists.back();

	SCP_vector<vec3d>::iterator ii;
	for (ii = vec_list.begin(); ii != vec_list.end(); ++ii)
	{
		waypoint new_waypoint(&(*ii), wp_list);
		wp_list->get_waypoints().push_back(new_waypoint);
	}

	// so that masking in the other function works
	// though if you actually hit this Assert, you have other problems
	Assert(wp_list->get_waypoints().size() <= 0xffff);
}

int waypoint_add(vec3d *pos, int waypoint_instance)
{
	Assert(pos != NULL);
	waypoint_list *wp_list;
	waypoint *wpt;
	int i, wp_list_index, wp_index;

	// find a new list to start
	if (waypoint_instance < 0)
	{
		// get a name for it
		char buf[NAME_LENGTH];
		waypoint_find_unique_name(buf, Waypoint_lists.size() + 1);

		// add new list with that name
		waypoint_list new_list(buf);
		Waypoint_lists.push_back(new_list);
		wp_list = &Waypoint_lists.back();

		// set up references
		wp_list_index = Waypoint_lists.size() - 1;
		wp_index = wp_list->get_waypoints().size();
	}
	// create the waypoint on the same list as, and immediately after, waypoint_instance
	else
	{
		calc_waypoint_indexes(waypoint_instance, wp_list_index, wp_index);
		wp_list = find_waypoint_list_at_index(wp_list_index);

		// theoretically waypoint_instance points to a current waypoint, so advance past it
		wp_index++;

		// it has to be on, or at the end of, an existing list
		Assert(wp_list != NULL);
		Assert((uint) wp_index <= wp_list->get_waypoints().size());

		// iterate through all waypoints that are at this index or later,
		// and edit their instances so that they point to a waypoint one place higher
		for (object *objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp))
		{
			if ((objp->type == OBJ_WAYPOINT) && (calc_waypoint_list_index(objp->instance) == wp_list_index) && (calc_waypoint_index(objp->instance) >= wp_index))
				objp->instance++;
		}
	}

	// so that masking in the other function works
	// (though if you actually hit this Assert, you have other problems)
	Assert(wp_index < 0x10000);

	// create the waypoint object
	waypoint new_waypoint(pos, wp_list);

	// add it at its appropriate spot, which may be the end of the list
	SCP_list<waypoint>::iterator ii = wp_list->get_waypoints().begin();
	for (i = 0; i < wp_index; i++)
		++ii;
	wp_list->get_waypoints().insert(ii, new_waypoint);
	wpt = find_waypoint_at_index(wp_list, wp_index);

	// apparently we create it in-game too; this is called by both scripting and FRED
	waypoint_create_game_object(wpt, wp_list_index, wp_index);

	return wpt->get_objnum();
}

void waypoint_remove(waypoint *wpt)
{
	int objnum = wpt->get_objnum();
	waypoint_list *wp_list = wpt->get_parent_list();

	int this_list = calc_waypoint_list_index(Objects[objnum].instance);
	int this_index = calc_waypoint_index(Objects[objnum].instance);

	// special case... this is the only waypoint on its list
	if (wp_list->get_waypoints().size() == 1)
	{
		wp_list->get_waypoints().clear();

		// special special case... this is the only waypoint list!
		if (Waypoint_lists.size() == 1)
		{
			Waypoint_lists.clear();
		}
		// shift the other waypoint lists down
		else
		{
			// remove this particular waypoint list
			SCP_list<waypoint_list>::iterator ii;
			int i;
			for (i = 0, ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++i, ++ii)
			{
				if (i == this_list)
				{
					Waypoint_lists.erase(ii);
					break;
				}
			}
			Assert(ii != Waypoint_lists.end());

			// iterate through all waypoints that are in lists later than this one,
			// and edit their instances so that they point to a list one place lower
			for (object *objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp))
			{
				if ((objp->type == OBJ_WAYPOINT) && (calc_waypoint_list_index(objp->instance) > this_list))
					objp->instance -= 0x10000;
			}
		}
	}
	// shift the other waypoints down
	else
	{
		// remove this particular waypoint
		SCP_list<waypoint>::iterator ii;
		int i;
		for (i = 0, ii = wp_list->get_waypoints().begin(); ii != wp_list->get_waypoints().end(); ++i, ++ii)
		{
			if (i == this_index)
			{
				wp_list->get_waypoints().erase(ii);
				break;
			}
		}
		Assert(ii != wp_list->get_waypoints().end());

		// iterate through all waypoints that are later than this one,
		// and edit their instances so that they point to a waypoint one place lower
		for (object *objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp))
		{
			if ((objp->type == OBJ_WAYPOINT) && (calc_waypoint_list_index(objp->instance) == this_list) && (calc_waypoint_index(objp->instance) > this_index))
				objp->instance--;
		}
	}

	// FRED has its own object removal logic
	if (!Fred_running)
		obj_delete(objnum);
}

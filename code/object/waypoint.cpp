
#include <cctype>

#include "globalincs/linklist.h"
#include "object/object.h"
#include "object/waypoint.h"
#include "network/multiutil.h"
#include <limits.h>

//********************GLOBALS********************
SCP_vector<waypoint_list> Waypoint_lists;

// In order to restore ai_info to a plain-old-data struct, ai_info->wp_index
// now uses size_t rather than iterator to index into the list.  If ai_info
// eventually becomes an actual class, we ought to go back to using iterators.
// 2023 addendum - Perhaps not; iterators don't gain us much compared to
// indexes.  See commit b4cc5d72f4 for the historical context.
const int INVALID_WAYPOINT_POSITION = INT_MAX;

//********************CLASS MEMBERS********************
waypoint::waypoint()
{
	this->m_position.xyz.x = 0.0f;
	this->m_position.xyz.y = 0.0f;
	this->m_position.xyz.z = 0.0f;

	this->m_objnum = -1;
}

waypoint::waypoint(const vec3d *position)
{
	Assert(position != NULL);

	this->m_position.xyz.x = position->xyz.x;
	this->m_position.xyz.y = position->xyz.y;
	this->m_position.xyz.z = position->xyz.z;

	this->m_objnum = -1;
}

waypoint::~waypoint()
{
	// nothing to do
}

const vec3d *waypoint::get_pos() const
{
	return &m_position;
}

int waypoint::get_objnum() const
{
	return m_objnum;
}

const waypoint_list *waypoint::get_parent_list() const
{
	int list_index = get_parent_list_index();
	if (list_index < 0 || list_index >= static_cast<int>(Waypoint_lists.size()))
		return nullptr;

	return &Waypoint_lists[list_index];
}

waypoint_list *waypoint::get_parent_list()
{
	int list_index = get_parent_list_index();
	if (list_index < 0 || list_index >= static_cast<int>(Waypoint_lists.size()))
		return nullptr;

	return &Waypoint_lists[list_index];
}

int waypoint::get_parent_list_index() const
{
	if (m_objnum < 0)
		return -1;

	return calc_waypoint_list_index(Objects[m_objnum].instance);
}

int waypoint::get_index() const
{
	if (m_objnum < 0)
		return -1;

	return calc_waypoint_index(Objects[m_objnum].instance);
}

void waypoint::set_pos(const vec3d *pos)
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

const char *waypoint_list::get_name() const
{
	return m_name;
}

const SCP_vector<waypoint> &waypoint_list::get_waypoints() const
{
	return m_waypoints;
}

SCP_vector<waypoint> &waypoint_list::get_waypoints()
{
	return m_waypoints;
}

void waypoint_list::set_name(const char *name)
{
	Assert(name != NULL);
	strcpy_s(this->m_name, name);
}

//********************FUNCTIONS********************
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
	if (waypoint_instance < 0)
		return -1;

	return waypoint_instance / 0x10000;
}

int calc_waypoint_index(int waypoint_instance)
{
	if (waypoint_instance < 0)
		return -1;

	return waypoint_instance & 0xffff;
}

void waypoint_create_game_object(waypoint *wpt, int list_index, int wpt_index)
{
	Assert(wpt != NULL);
	Assert(list_index >= 0);
	Assert(wpt_index >= 0);
    flagset<Object::Object_Flags> default_flags;
    default_flags.set(Object::Object_Flags::Renders);
	wpt->m_objnum = obj_create(OBJ_WAYPOINT, -1, calc_waypoint_instance(list_index, wpt_index), NULL, wpt->get_pos(), 0.0f, default_flags);

	Assert(wpt->m_objnum > -1);
	if (wpt->m_objnum < 0) {
		return;
	}

	Objects[wpt->m_objnum].net_signature = multi_assign_network_signature(MULTI_SIG_WAYPOINT);
}

// done immediately after mission load; originally found in aicode.cpp
void waypoint_create_game_objects()
{
	int list = 0;
	for (auto &ii: Waypoint_lists)
	{
		int wpt = 0;
		for (auto &jj: ii.get_waypoints())
		{
			waypoint_create_game_object(&jj, list, wpt);
			wpt++;
		}
		list++;
	}
}

waypoint_list *find_matching_waypoint_list(const char *name)
{
	Assert(name != nullptr);

	for (auto &ii: Waypoint_lists)
	{
		if (!stricmp(ii.get_name(), name))
			return &ii;
	}

	return nullptr;
}

int find_matching_waypoint_list_index(const char *name)
{
	Assert(name != nullptr);

	int i = 0;
	for (auto &ii: Waypoint_lists)
	{
		if (!stricmp(ii.get_name(), name))
			return i;
		i++;
	}

	return -1;
}

// NOTE: waypoint names are always in the format Name:index
waypoint *find_matching_waypoint(const char *name)
{
	Assert(name != NULL);

	for (auto &ii: Waypoint_lists)
	{
		auto len = strlen(ii.get_name());

		// the first half (before the :) matches
		if (!strnicmp(ii.get_name(), name, len))
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
			if (index < 1 || index > ii.get_waypoints().size())
			{
				nprintf(("waypoints", "possible error with waypoint name '%s': waypoint number is out of range\n", name));
				continue;
			}

			return find_waypoint_at_index(&ii, index - 1);
		}
	}

	return NULL;
}

waypoint *find_waypoint_with_objnum(int objnum)
{
	if (objnum < 0 || objnum >= MAX_OBJECTS || Objects[objnum].type != OBJ_WAYPOINT)
		return nullptr;

	return find_waypoint_with_instance(Objects[objnum].instance);
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
		return nullptr;

	return find_waypoint_at_indexes(calc_waypoint_list_index(waypoint_instance), calc_waypoint_index(waypoint_instance));
}

waypoint_list *find_waypoint_list_at_index(int index)
{
	if (SCP_vector_inbounds(Waypoint_lists, index))
		return &Waypoint_lists[index];

	return nullptr;
}

waypoint *find_waypoint_at_index(waypoint_list *list, int index)
{
	if (list && SCP_vector_inbounds(list->get_waypoints(), index))
		return &list->get_waypoints()[index];

	return nullptr;
}

waypoint *find_waypoint_at_indexes(int list_index, int index)
{
	return find_waypoint_at_index(find_waypoint_list_at_index(list_index), index);
}

int find_index_of_waypoint_list(const waypoint_list *wp_list)
{
	Assert(wp_list != NULL);

	ptrdiff_t index = wp_list - Waypoint_lists.data();

	if (index >= 0 && index < (ptrdiff_t)Waypoint_lists.size())
		return static_cast<int>(index);

	return -1;
}

int find_index_of_waypoint(const waypoint_list *wp_list, const waypoint *wpt)
{
	Assert(wp_list != NULL);
	Assert(wpt != NULL);

	ptrdiff_t index = wpt - wp_list->get_waypoints().data();

	if (index >= 0 && index < (ptrdiff_t)wp_list->get_waypoints().size())
		return (int)index;

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

void waypoint_add_list(const char *name, const SCP_vector<vec3d> &vec_list)
{
	Assert(name != NULL);

	if (find_matching_waypoint_list(name) != NULL)
	{
		Warning(LOCATION, "Waypoint list '%s' already exists in this mission!  Not adding the new list...", name);
		return;
	}

	Waypoint_lists.emplace_back(name);
	auto& wp_list = Waypoint_lists.back();

	wp_list.get_waypoints().reserve(vec_list.size());
	for (const auto &ii: vec_list)
	{
		wp_list.get_waypoints().emplace_back(&ii);
	}

	// so that masking in the other function works
	// though if you actually hit this Assert, you have other problems
	Assert(wp_list.get_waypoints().size() <= 0xffff);
}

int waypoint_add(const vec3d *pos, int waypoint_instance, bool first_waypoint_in_list)
{
	Assert(pos != NULL);
	waypoint_list *wp_list;
	waypoint *wpt;
	int wp_list_index, wp_index;

	// find a new list to start
	if (waypoint_instance < 0)
	{
		// get a name for it
		char buf[NAME_LENGTH];
		waypoint_find_unique_name(buf, static_cast<int>(Waypoint_lists.size()) + 1);

		// add new list with that name
		Waypoint_lists.emplace_back(buf);
		wp_list = &Waypoint_lists.back();

		// set up references
		wp_list_index = static_cast<int>(Waypoint_lists.size()) - 1;
		wp_index = static_cast<int>(wp_list->get_waypoints().size());
	}
	// create the waypoint on the same list as, and immediately after, waypoint_instance (unless it's the first waypoint)
	else
	{
		calc_waypoint_indexes(waypoint_instance, wp_list_index, wp_index);
		wp_list = find_waypoint_list_at_index(wp_list_index);

		if (first_waypoint_in_list)
		{
			wp_index = 0;
		}
		else
		{
			// theoretically waypoint_instance points to a current waypoint, so advance past it
			wp_index++;
		}

		// it has to be on, or at the end of, an existing list
		Assert(wp_list != NULL);
		Assert(wp_index <= static_cast<int>(wp_list->get_waypoints().size()));

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
	waypoint new_waypoint(pos);

	// add it at its appropriate spot, which may be the end of the list
	wp_list->get_waypoints().insert(wp_list->get_waypoints().begin() + wp_index, new_waypoint);
	wpt = find_waypoint_at_index(wp_list, wp_index);

	// apparently we create it in-game too; this is called by both scripting and FRED
	waypoint_create_game_object(wpt, wp_list_index, wp_index);

	return wpt->get_objnum();
}

void waypoint_remove(const waypoint *wpt)
{
	int objnum = wpt->get_objnum();
	int this_list = calc_waypoint_list_index(Objects[objnum].instance);
	int this_index = calc_waypoint_index(Objects[objnum].instance);

	waypoint_list *wp_list = find_waypoint_list_at_index(this_list);
	Assert(wp_list == wpt->get_parent_list());

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
			Waypoint_lists.erase(Waypoint_lists.begin() + this_list);

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
		wp_list->get_waypoints().erase(wp_list->get_waypoints().begin() + this_index);

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

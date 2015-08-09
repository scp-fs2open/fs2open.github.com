#ifndef _WXFRED2_H
#define _WXFRED2_H
/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "docfred.h"

#include <mission/missionparse.h>
#include <object/object.h>
#include <globalincs/globals.h>
#include <globalincs/pstypes.h>

#include <wx/wx.h>

#include <exception>
#include <string>
#include <vector>

// Directories and File Handling stuff
SCP_string Fred_base_dir;

// The following typedef's use negative values as invalid states. Most functions only check if it's negative or positive, not a specific value
typedef int MTIN_t; // Model Type Index Number, used to create objects of the assoc. model type
typedef int OIN_t;  // Object Index Number, used to keep track of all object instances (except waypoints)
typedef int WIN_t;  // Waypoint Index Number, used to keep track of all waypoint instances/lists

static MTIN_t MTIN_player_default;  // MTIN of the player's default ship

// The following MTIN's are effectively appended to the end of the ship type selection list.
static MTIN_t MTIN_jump_node;       // MTIN of jump nodes, equal to Num_ship_classes + 2
static MTIN_t MTIN_player_start;    // MTIN of player starts, equal to Num_ship_classes + 1
static MTIN_t MTIN_waypoint;        // MTIN of waypoints, equal to Num_ship_classes

const WIN_t WIN_new_list = -1; // Waypoints created with this WIN also create a new wp list (with them as the first wp)

// a.k.a. z64's stupid exception placeholder
class Fred_exception : std::exception
{
public:
	Fred_exception(const char* msg) : msg(msg) {};
	const char* what() const;

private:
	const char* msg;
};

class wxFRED2 : public wxApp
{
public:
	virtual bool OnInit();

	// Mission management
	bool Mission_load(const wxString infile);
	void Mission_new(void);
	void Mission_save(void);

	// Object management

	/**
	* @brief Copies the indexed object
	*
	* @param[in] oin index of the object to copy
	*
	* @returns index of the copy
	*/
	OIN_t Copy_Object(OIN_t oin);

	/**
	* @brief Copies the indexed objects within the selection queue
	*
	* @param[in] selq A queue of indexed objects selected for copying
	*
	* @returns index of the copy
	*/
	void  Copy_Objects(SCP_vector<OIN_t> &selq);

	/**
	* @brief Deletes the indexed object
	*
	* @param[in] oin Index of the object to delete
	*
	* @returns 0 If successful
	* @returns 1 If unsuccessful
	* @returns 2 If aborted
	*/
	int   Delete_Object(OIN_t oin);

	/**
	* @brief Deletes the indexed objects within the selection queue
	*
	* @param[in] selq A queue of indexed objects selected for deletion
	*
	* @returns 0 If successful
	* @returns 1 If unsuccessful
	* @returns 2 If aborted
	*/
	int   Delete_Objects(SCP_vector<OIN_t> &selq);

	/**
	* @brief Creates a jump node at the given position
	*
	* @param pos Position to create a new jump node
	*/
	OIN_t Create_jumpnode(vec3d *pos);

	/**
	* @brief Creates a player spawn point at the designated position.
	*
	* @param[in] pos    Position to create a new spawn point
	* @param[in] orient Direction the player faces (if spawned here)
	* @param[in] type   Type of craft this player defaults to
	*
	*/
	OIN_t Create_player(vec3d *pos, matrix *orient, MTIN_t type);

	/**
	* @brief Creates a ship at the designated position
	*
	* @param[in] pos    Position to create a new ship
	* @param[in] orient Direction the player faces (if spawned here)
	* @param[in] type   Type of craft to create
	*/
	OIN_t Create_ship(vec3d *pos, matrix *orient, MTIN_t type);

	/**
	* @brief Creates a waypoint at the designated position
	*
	* @param[in] pos   Position to create a new waypoint
	*/
	OIN_t Create_waypoint(vec3d *pos, WIN_t win);

	char** Get_ship_types(void);	// Retrieves the available ship types as a char[][]

	// Shield Systems (temporarily here) probably should be bool
	int Shield_sys_teams[MAX_IFFS];
	int Shield_sys_types[MAX_SHIP_CLASSES];

protected:
	// Mission management
	void Mission_reset(void);	// Init's/ Resets the mission to initial conditions
	void Mission_clear(void);	// Wipes out everything, leaving a clean slate to start with


private:
	std::vector<std::string> ship_types;

	docFRED2 Document;
};

DECLARE_APP(wxFRED2)

#endif
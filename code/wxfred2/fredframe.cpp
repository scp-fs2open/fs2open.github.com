/*
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */ 

/*
 * $Logfile: /Freespace2/code/wxFRED2/FREDFrame.cpp $
 * $Revision: 1.1 $
 * $Date: 2005-03-29 13:35:53 $
 * $Author: Goober5000 $
 *
 * FRED app frame
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.0  2005/03/29 07:55:19  Goober5000
 * Addition to CVS repository
 *
 */

// precompiled header for compilers that support it
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "fredframe.h"


FREDFrame::FREDFrame(const wxChar *title, int xpos, int ypos, int width, int height)
	: wxFrame(NULL, -1, title, wxPoint(xpos, ypos), wxSize(width, height))
{
	myMenuBar = new wxMenuBar();

	// file menu
	myFileMenu = new wxMenu();
	myFileMenu->Append(MENU_FILE_NEW, "&New\tCtrl+N", "Create a new mission");
	myFileMenu->Append(MENU_FILE_OPEN, "&Open...\tCtrl+O", "Open an existing mission");
	myFileMenu->Append(MENU_FILE_SAVE, "&Save\tCtrl+S", "Save the current mission");
	myFileMenu->Append(MENU_FILE_SAVE_AS, "Save &As...", "Save the current mission with a new name");
	myFileMenu->Append(MENU_FILE_REVERT, "&Revert", "Revert to the saved version of the mission");
	myFileMenu->AppendSeparator();

	myFileSaveFormatMenu = new wxMenu();
	myFileSaveFormatMenu->AppendRadioItem(MENU_FILE_SAVE_FORMAT_FS2_OPEN, "FS2 open", "Change to the Freespace 2 Open save format");
	myFileSaveFormatMenu->AppendRadioItem(MENU_FILE_SAVE_FORMAT_FS2_RETAIL, "FS2 retail", "Change to the Freespace 2 Retail save format");
	myFileSaveFormatMenu->Check(MENU_FILE_SAVE_FORMAT_FS2_OPEN, TRUE);
	myFileMenu->Append(MENU_FILE_SAVE_FORMAT, "Save &Format", myFileSaveFormatMenu);

	myFileImportMenu = new wxMenu();
	myFileImportMenu->Append(MENU_FILE_IMPORT_FREESPACE_1_MISSION, "FS1 mission...", "Import a Freespace 1 mission into Freespace 2 format");
	myFileImportMenu->Append(MENU_FILE_IMPORT_WEAPON_LOADOUTS, "FS1 weapon loadouts...", "Import Freespace 1 weapon loadouts");
	myFileMenu->Append(MENU_FILE_IMPORT, "&Import", myFileImportMenu);

	myFileMenu->AppendSeparator();
	myFileMenu->Append(MENU_FILE_RUN_FREESPACE, "&Run Freespace\tAlt+R", "Run Freespace");
	myFileMenu->AppendSeparator();
	myFileMenu->Append(MENU_FILE_RECENT_FILE_LIST, "Recent File List", "Open this mission");
	myFileMenu->AppendSeparator();
	myFileMenu->Append(MENU_FILE_EXIT, "E&xit", "Exit the mission editor");
	myMenuBar->Append(myFileMenu, "&File");

	// edit menu
	myEditMenu = new wxMenu();
	myEditMenu->Append(MENU_EDIT_UNDO, "&Undo\tCtrl+Z", "Undo the last action");
	myEditMenu->Append(MENU_EDIT_DELETE, "&Delete\tDel", "Delete the selected object(s)");
	myEditMenu->Append(MENU_EDIT_DELETE_WING, "Delete &Wing\tCtrl+Del", "Delete the selected wing");
	myEditMenu->AppendSeparator();
	myEditMenu->AppendCheckItem(MENU_EDIT_DISABLE_UNDO, "Disable Undo", "Disable or enable the Undo command");
	myMenuBar->Append(myEditMenu, "&Edit");

	// view menu
	myViewMenu = new wxMenu();
	myViewMenu->AppendCheckItem(MENU_VIEW_TOOLBAR, "Toolbar", "Show the toolbar");
	myViewMenu->AppendCheckItem(MENU_VIEW_STATUS_BAR, "Status Bar", "Show the status bar");
	myViewMenu->AppendSeparator();

	myViewDisplayFilterMenu = new wxMenu();
	myViewDisplayFilterMenu->AppendCheckItem(MENU_VIEW_DISPLAY_FILTER_SHOW_SHIPS, "Show Ships", "Display ships");
	myViewDisplayFilterMenu->AppendCheckItem(MENU_VIEW_DISPLAY_FILTER_SHOW_PLAYER_STARTS, "Show Player Starts", "Display player start positions");
	myViewDisplayFilterMenu->AppendCheckItem(MENU_VIEW_DISPLAY_FILTER_SHOW_WAYPOINTS, "Show Waypoints", "Display waypoints");
	myViewDisplayFilterMenu->AppendSeparator();
	myViewDisplayFilterMenu->AppendCheckItem(MENU_VIEW_DISPLAY_FILTER_SHOW_FRIENDLY, "Show Friendly", "Display friendly objects");
	myViewDisplayFilterMenu->AppendCheckItem(MENU_VIEW_DISPLAY_FILTER_SHOW_HOSTILE, "Show Hostile", "Display hostile objects");
	myViewDisplayFilterMenu->Check(MENU_VIEW_DISPLAY_FILTER_SHOW_SHIPS, TRUE);
	myViewDisplayFilterMenu->Check(MENU_VIEW_DISPLAY_FILTER_SHOW_PLAYER_STARTS, TRUE);
	myViewDisplayFilterMenu->Check(MENU_VIEW_DISPLAY_FILTER_SHOW_WAYPOINTS, TRUE);
	myViewDisplayFilterMenu->Check(MENU_VIEW_DISPLAY_FILTER_SHOW_FRIENDLY, TRUE);
	myViewDisplayFilterMenu->Check(MENU_VIEW_DISPLAY_FILTER_SHOW_HOSTILE, TRUE);
	myViewMenu->Append(MENU_VIEW_DISPLAY_FITER, "Display Filter", myViewDisplayFilterMenu);

	myViewMenu->AppendSeparator();
	myViewMenu->Append(MENU_VIEW_HIDE_MARKED_OBJECTS, "Hide Marked Objects", "Hide the selected objects");
	myViewMenu->Append(MENU_VIEW_SHOW_HIDDEN_OBJECTS, "Show Hidden Objects", "Show all hidden objects");
	myViewMenu->AppendSeparator();
	myViewMenu->AppendCheckItem(MENU_VIEW_SHOW_SHIP_MODELS, "Show Ship Models\tShift+Alt+M", "Show actual polygon models for visible ships");
	myViewMenu->AppendCheckItem(MENU_VIEW_SHOW_OUTLINES, "Show Outlines\tShift+Alt+O", "Show outlines for visible ships");
	myViewMenu->AppendCheckItem(MENU_VIEW_SHOW_SHIP_INFO, "Show Ship Info\tShift+Alt+I", "Display ship names and ship types for visible ships");
	myViewMenu->AppendCheckItem(MENU_VIEW_SHOW_COORDINATES, "Show Coordinates\tShift+Alt+C", "Display the coordinates of objects");
	myViewMenu->AppendCheckItem(MENU_VIEW_SHOW_GRID_POSITIONS, "Show Grid Positions\tShift+Alt+P", "Show grid positions for visible objects");
	myViewMenu->AppendCheckItem(MENU_VIEW_SHOW_DISTANCES, "Show Distances\tD", "Show distances between marked objects");
	myViewMenu->AppendCheckItem(MENU_VIEW_SHOW_MODEL_PATHS, "Show Model Paths", "Draw paths for visible objects");
	myViewMenu->AppendCheckItem(MENU_VIEW_SHOW_MODEL_DOCK_POINTS, "Show Model Dock Points", "Draw dock points for visible objects");
	myViewMenu->AppendSeparator();
	myViewMenu->AppendCheckItem(MENU_VIEW_SHOW_GRID, "Show Grid\tShift+Alt+G", "Show grid");
	myViewMenu->AppendCheckItem(MENU_VIEW_SHOW_HORIZON, "Show Horizon\tShift+Alt+H", "Show the horizon line");
	myViewMenu->AppendCheckItem(MENU_VIEW_DOUBLE_FINE_GRIDLINES, "Double Fine Gridlines", "Show double fine gridlines");
	myViewMenu->AppendCheckItem(MENU_VIEW_ANTI_ALIASED_GRIDLINES, "Anti-Aliased Gridlines", "Show anti-aliased gridlines");
	myViewMenu->AppendCheckItem(MENU_VIEW_SHOW_3D_COMPASS, "Show 3-D Compass\tShift+Alt+3", "Display the three-dimensional compass");
	myViewMenu->AppendCheckItem(MENU_VIEW_SHOW_BACKGROUND, "Show Background\tShift+Alt+B", "Display background images");
	myViewMenu->AppendSeparator();

	myViewViewpointMenu = new wxMenu();
	myViewViewpointMenu->AppendRadioItem(MENU_VIEW_VIEWPOINT_CAMERA, "Camera", "Change viewpoint to camera");
	myViewViewpointMenu->AppendRadioItem(MENU_VIEW_VIEWPOINT_CURRENT_SHIP, "Current Ship", "Change viewpoint to current ship");
	myViewViewpointMenu->Check(MENU_VIEW_VIEWPOINT_CAMERA, TRUE);
	myViewMenu->Append(MENU_VIEW_VIEWPOINT, "Viewpoint\tShift+V", myViewViewpointMenu);

	myViewMenu->Append(MENU_VIEW_SAVE_CAMERA_POS, "Save Camera Pos\tCtrl+P", "Save current camera position");
	myViewMenu->Append(MENU_VIEW_RESTORE_CAMERA_POS, "Restore Camera Pos\tCtrl+R", "Restore saved camera position");
	myViewMenu->AppendSeparator();
	myViewMenu->AppendCheckItem(MENU_VIEW_LIGHTING_FROM_SUNS, "Lighting From Suns", "Toggle lighting effects from suns");
	myViewMenu->Check(MENU_VIEW_TOOLBAR, TRUE);
	myViewMenu->Check(MENU_VIEW_STATUS_BAR, TRUE);
	myViewMenu->Check(MENU_VIEW_SHOW_SHIP_MODELS, TRUE);
	myViewMenu->Check(MENU_VIEW_SHOW_SHIP_INFO, TRUE);
	myViewMenu->Check(MENU_VIEW_SHOW_COORDINATES, TRUE);
	myViewMenu->Check(MENU_VIEW_SHOW_GRID_POSITIONS, TRUE);
	myViewMenu->Check(MENU_VIEW_SHOW_GRID, TRUE);
	myViewMenu->Check(MENU_VIEW_SHOW_3D_COMPASS, TRUE);
	myViewMenu->Check(MENU_VIEW_SHOW_BACKGROUND, TRUE);
	myViewMenu->Check(MENU_VIEW_LIGHTING_FROM_SUNS, TRUE);
	myMenuBar->Append(myViewMenu, "&View");

	// speed menu
	mySpeedMenu = new wxMenu();
	mySpeedMovementMenu = new wxMenu();
	mySpeedMovementMenu->AppendRadioItem(MENU_SPEED_MOVEMENT_X1, "x1\t1", "Change movement speed to 1x normal");
	mySpeedMovementMenu->AppendRadioItem(MENU_SPEED_MOVEMENT_X2, "x2\t2", "Change movement speed to 2x normal");
	mySpeedMovementMenu->AppendRadioItem(MENU_SPEED_MOVEMENT_X3, "x3\t3", "Change movement speed to 3x normal");
	mySpeedMovementMenu->AppendRadioItem(MENU_SPEED_MOVEMENT_X5, "x5\t4", "Change movement speed to 5x normal");
	mySpeedMovementMenu->AppendRadioItem(MENU_SPEED_MOVEMENT_X8, "x8\t5", "Change movement speed to 8x normal");
	mySpeedMovementMenu->AppendRadioItem(MENU_SPEED_MOVEMENT_X10, "x10\t6", "Change movement speed to 10x normal");
	mySpeedMovementMenu->AppendRadioItem(MENU_SPEED_MOVEMENT_X50, "x50\t7", "Change movement speed to 50x normal");
	mySpeedMovementMenu->AppendRadioItem(MENU_SPEED_MOVEMENT_X100, "x100\t8", "Change movement speed to 100x normal");
	mySpeedMovementMenu->Check(MENU_SPEED_MOVEMENT_X1, TRUE);
	mySpeedMenu->Append(MENU_SPEED_MOVEMENT, "Movement", mySpeedMovementMenu);
	mySpeedRotationMenu = new wxMenu();
	mySpeedRotationMenu->AppendRadioItem(MENU_SPEED_ROTATION_X1, "x1\tShift+1", "Change rotation speed to 1x normal");
	mySpeedRotationMenu->AppendRadioItem(MENU_SPEED_ROTATION_X5, "x5\tShift+2", "Change rotation speed to 5x normal");
	mySpeedRotationMenu->AppendRadioItem(MENU_SPEED_ROTATION_X12, "x12\tShift+3", "Change rotation speed to 12x normal");
	mySpeedRotationMenu->AppendRadioItem(MENU_SPEED_ROTATION_X25, "x25\tShift+4", "Change rotation speed to 25x normal");
	mySpeedRotationMenu->AppendRadioItem(MENU_SPEED_ROTATION_X50, "x50\tShift+5", "Change rotation speed to 50x normal");
	mySpeedRotationMenu->Check(MENU_SPEED_ROTATION_X1, TRUE);
	mySpeedMenu->Append(MENU_SPEED_ROTATION, "Rotation", mySpeedRotationMenu);
	myMenuBar->Append(mySpeedMenu, "&Speed");

	// editors menu
	myEditorsMenu = new wxMenu();
	myEditorsMenu->Append(MENU_EDITORS_SHIPS, "Ships\tShift+S", "Display ship editor");
	myEditorsMenu->Append(MENU_EDITORS_WINGS, "Wings\tShift+W", "Display wing editor");
	myEditorsMenu->Append(MENU_EDITORS_OBJECTS, "Objects\tShift+O", "Display object editor");
	myEditorsMenu->Append(MENU_EDITORS_WAYPOINT_PATHS, "Waypoint Paths\tShift+Y", "Display waypoint path editor");
	myEditorsMenu->Append(MENU_EDITORS_MISSION_OBJECTIVES, "Mission Objectives\tShift+G", "Display mission objectives editor");
	myEditorsMenu->Append(MENU_EDITORS_EVENTS, "Events\tShift+E", "Display event editor");
	myEditorsMenu->Append(MENU_EDITORS_TEAM_LOADOUT, "Team Loadout\tShift+P", "Display team loadout editor");
	myEditorsMenu->Append(MENU_EDITORS_BACKGROUND, "Background\tShift+I", "Display background editor");
	myEditorsMenu->Append(MENU_EDITORS_REINFORCEMENTS, "Reinforcements\tShift+R", "Display reinforcements editor");
	myEditorsMenu->Append(MENU_EDITORS_ASTEROID_FIELD, "Asteroid Field\tShift+A", "Display asteroid field editor");
	myEditorsMenu->Append(MENU_EDITORS_MISSION_SPECS, "Mission Specs\tShift+N", "Display mission specs editor");
	myEditorsMenu->Append(MENU_EDITORS_BRIEFING, "Briefing\tShift+B", "Display briefing editor");
	myEditorsMenu->Append(MENU_EDITORS_DEBRIEFING, "Debriefing\tShift+D", "Display debriefing editor");
	myEditorsMenu->Append(MENU_EDITORS_SHIELD_SYSTEM, "Shield System", "Display shield system editor");
	myEditorsMenu->Append(MENU_EDITORS_COMMAND_BRIEFING, "Command Briefing", "Display command briefing editor");
	myEditorsMenu->Append(MENU_EDITORS_SET_GLOBAL_SHIP_FLAGS, "Set Global Ship Flags", "Display global flags editor");
	myEditorsMenu->Append(MENU_EDITORS_VOICE_ACTING_MANAGER, "Voice Manager", "Display voice manager editor");
	myEditorsMenu->AppendSeparator();
	myEditorsMenu->Append(MENU_EDITORS_CAMPAIGN, "Campaign\tShift+C", "Display campaign editor");
	myMenuBar->Append(myEditorsMenu, "&Editors");

	// groups menu
	myGroupsMenu = new wxMenu();
	myGroupsMenu->Append(MENU_GROUPS_GROUP_1, "Group 1\tCtrl+1", "Display group 1");
	myGroupsMenu->Append(MENU_GROUPS_GROUP_2, "Group 2\tCtrl+2", "Display group 2");
	myGroupsMenu->Append(MENU_GROUPS_GROUP_3, "Group 3\tCtrl+3", "Display group 3");
	myGroupsMenu->Append(MENU_GROUPS_GROUP_4, "Group 4\tCtrl+4", "Display group 4");
	myGroupsMenu->Append(MENU_GROUPS_GROUP_5, "Group 5\tCtrl+5", "Display group 5");
	myGroupsMenu->Append(MENU_GROUPS_GROUP_6, "Group 6\tCtrl+6", "Display group 6");
	myGroupsMenu->Append(MENU_GROUPS_GROUP_7, "Group 7\tCtrl+7", "Display group 7");
	myGroupsMenu->Append(MENU_GROUPS_GROUP_8, "Group 8\tCtrl+8", "Display group 8");
	myGroupsMenu->Append(MENU_GROUPS_GROUP_9, "Group 9\tCtrl+9", "Display group 9");
	myGroupsSetGroupMenu = new wxMenu();
	myGroupsSetGroupMenu->Append(MENU_GROUPS_SET_GROUP_GROUP_1, "Group 1", "Assign marked objects to group 1");
	myGroupsSetGroupMenu->Append(MENU_GROUPS_SET_GROUP_GROUP_2, "Group 2", "Assign marked objects to group 2");
	myGroupsSetGroupMenu->Append(MENU_GROUPS_SET_GROUP_GROUP_3, "Group 3", "Assign marked objects to group 3");
	myGroupsSetGroupMenu->Append(MENU_GROUPS_SET_GROUP_GROUP_4, "Group 4", "Assign marked objects to group 4");
	myGroupsSetGroupMenu->Append(MENU_GROUPS_SET_GROUP_GROUP_5, "Group 5", "Assign marked objects to group 5");
	myGroupsSetGroupMenu->Append(MENU_GROUPS_SET_GROUP_GROUP_6, "Group 6", "Assign marked objects to group 6");
	myGroupsSetGroupMenu->Append(MENU_GROUPS_SET_GROUP_GROUP_7, "Group 7", "Assign marked objects to group 7");
	myGroupsSetGroupMenu->Append(MENU_GROUPS_SET_GROUP_GROUP_8, "Group 8", "Assign marked objects to group 8");
	myGroupsSetGroupMenu->Append(MENU_GROUPS_SET_GROUP_GROUP_9, "Group 9", "Assign marked objects to group 9");
	myGroupsMenu->Append(MENU_GROUPS_SET_GROUP, "Set Group", myGroupsSetGroupMenu);
	myMenuBar->Append(myGroupsMenu, "&Groups");

	// misc menu
	myMiscMenu = new wxMenu();
	myMiscMenu->Append(MENU_MISC_LEVEL_OBJECT, "Level Object\tL", "Align object's (or camera's, if no object is marked) x-axis with global x-axis");
	myMiscMenu->Append(MENU_MISC_ALIGN_OBJECT, "Align Object\tCtrl+L", "Align object's (or camera's, if no object is marked) y-axis with global y-axis");
	myMiscMenu->Append(MENU_MISC_MARK_WING, "Mark Wing\tW", "Mark selected object's entire wing");
	myMiscMenu->Append(MENU_MISC_CONTROL_OBJECT, "Control Object\tT", "Toggle movement keys controlling marked object or camera");
	myMiscMenu->Append(MENU_MISC_NEXT_OBJECT, "Next Object\tTab", "Select the next object");
	myMiscMenu->Append(MENU_MISC_PREVIOUS_OBJECT, "Prev Object\tCtrl+Tab", "Select the previous object");
	myMiscMenu->Append(MENU_MISC_ADJUST_GRID, "Adjust Grid", "Adjust orientaition and level of grid");
	myMiscMenu->Append(MENU_MISC_NEXT_SUBSYSTEM, "Next Subsystem\tK", "Select the next subsystem on the selected object");
	myMiscMenu->Append(MENU_MISC_PREV_SUBSYSTEM, "Prev Subsystem\tShift+K", "Select the previous subsystem on the selected object");
	myMiscMenu->Append(MENU_MISC_CANCEL_SUBSYSTEM, "Cancel Subsystem\tAlt+K", "Deselect any subsystems on the selected object");
	myMiscMenu->Append(MENU_MISC_MISSION_STATISTICS, "Mission Statistics\tCtrl+Shift+D", "Display mission statistics");
	myMiscMenu->AppendSeparator();
	myMiscMenu->Append(MENU_MISC_ERROR_CHECKER, "Error Checker\tShift+H", "Check mission for errors");
	myMenuBar->Append(myMiscMenu, "&Misc");

	// help menu
	myHelpMenu = new wxMenu();
	myHelpMenu->Append(MENU_HELP_HELP_TOPICS, "Help Topics\tF1", "List Help topics");
	myHelpMenu->AppendSeparator();
	myHelpMenu->Append(MENU_HELP_ABOUT_FRED2, "About FRED2", "Display program information and copyright");
	myHelpMenu->Append(MENU_HELP_SHOW_SEXP_HELP, "Show SEXP Help", "Toggle display of SEXP help");
	myMenuBar->Append(myHelpMenu, "&Help");

	SetMenuBar(myMenuBar);

	CreateStatusBar(5);
	SetStatusText("For Help, press F1", 0);
}

FREDFrame::~FREDFrame()
{}

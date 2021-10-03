/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */


#include "globalincs/pstypes.h"
#include "physics/physics.h"

#define BRIEFING_LOOKAT_POINT_ID	99999

extern int Aa_gridlines;    //!< Bool. If nonzero, draw anti-aliased gridlines
extern int player_start1;   //!< Object index of the player start
extern int Editing_mode;    //!< Enum. The edit mode. 0 = Select; 1 = Select and Move; 2 = Select and Rotate
extern int Control_mode;    //!< Bool. Control mode. 0 = Camera/Viewpoint control. 1 = Object control
extern int Show_grid;       //!< Bool. If nonzero, draw the grid
extern int Show_grid_positions;     //!< Bool. If nonzero, draw an elevation line from each object to the grid.
extern int Show_coordinates;        //!< Bool. If nonzero, draw the coordinates of each object on their label
extern int Show_outlines;           //!< Bool. If nonzero, draw each object's mesh. If models are shown, highlight them in white.
extern bool Draw_outlines_on_selected_ships;	// If a ship is selected, don't draw the mesh lines that would normally be drawn
extern int Show_stars;              //!< Bool. If nonzero, draw the starfield, nebulas, and suns. Might also handle skyboxes
extern int Single_axis_constraint;  //!< Bool. If nonzero, constrain movement to one axis
extern int Show_distances;          //!< Bool. If nonzero, draw lines between each object and display their distance on the middle of each line
extern int Universal_heading;       //!< Bool. Unknown.
extern int Flying_controls_mode;    //!< Bool. Unknown.
extern int Group_rotate;            //!< Bool. If nonzero, each object rotates around the leader. If zero. rotate
extern int Show_horizon;            //!< Bool. If nonzero, draw a line representing the horizon (XZ plane)
extern int Lookat_mode;             //!< Bool. Unknown.
extern int True_rw;                 //!< Unsigned. The width of the render area
extern int True_rh;                 //!< Unsigned. The height of the render area
extern int Fixed_briefing_size;     //!< Bool. If nonzero then expand the briefing preview as much as we can, maintaining the aspect ratio.
extern vec3d Constraint;            //!< Which axis (or axes) we can move/rotate on
extern vec3d Anticonstraint;        //!< Which axis (or axes) we can't move/rotate on
extern physics_info view_physics;   //!< Physics info of the camera/controlled object
extern vec3d view_pos;
extern vec3d eye_pos;
extern matrix view_orient;
extern matrix eye_orient;

/**
 * @brief Initializes the renderer.
 *
 * @details Called every time a new mission is created (and erasing old mission from memory). New mission should be
 * blank at this point.
 */
void fred_render_init();

/**
 * @brief Handler for Mouse movement
 *
 * @param[in] btn Bitfield of mousebuttons
 * @param[in] mdx X delta
 * @param[in] mdy Y delta
 */
void move_mouse(int btn, int mdx, int mdy);

/**
 * @brief Handler for OnIdle. Does a game frame within the editor
 */
void game_do_frame();

/**
 * @brief Handler for OnPaint. Renders the game frame
 */
void render_frame();

/**
 * @brief Reset the angles of the controlled object to be "wings level" with the grid, either the camera or the selection
 */
void level_controlled();

/**
 * @brief Similar to level_controlled, aligns the controlled object to the closest axis
 */
void verticalize_controlled();

/**
 * @brief Finds the closest object or waypoint under the mouse cursor and returns its index.
 *
 * @param[in] cx X coordinate on the viewport
 * @param[in] cy Y coordinate on the viewport
 *
 * @return Object index number of the object, if any. or
 * @return -1 if no object found
 */
int select_object(int cx, int cy);

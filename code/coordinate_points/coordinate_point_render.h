#pragma once

#include "coordinate_points/coordinate_point.h"

struct vec3d;
struct matrix;

// Distance-scaled size constants: matches the waypoint sphere sizing in qtFRED so coord points
// read at similar scale to existing markers.
constexpr float COORDINATE_POINT_LOLLIPOP_SIZE = 2.0f;
constexpr float COORDINATE_POINT_DIST_DIVISOR  = 20.0f;

// Draw a single coordinate point as a camera-facing line-loop polygon at its world position.
// Uses the supplied camera orientation for right/up vectors. The camera eye is needed for
// distance-based sizing. Color, shape, and size_scale come from the coord point itself.
void draw_coordinate_point_shape(const mission_coordinate_point& cp,
                                 const vec3d* camera_eye,
                                 const matrix* camera_orient);

// Returns the world-space radius the shape will be drawn at given the camera eye position.
// Exposed for HUD target-bracket sizing so the bracket fits the rendered shape.
float get_coordinate_point_world_radius(const mission_coordinate_point& cp, const vec3d& camera_eye);

// In-game render pass. Iterates Coordinate_points, filters by the Visible_in_mission flag, and
// draws each visible coord point using the global Eye_position / Eye_matrix.
void coordinate_points_render_all_in_mission();

// Editor render pass. Iterates Coordinate_points and draws each one regardless of the
// Visible_in_mission flag, using the supplied camera. Caller is responsible for any
// view-level "show coordinate points" toggle.
void coordinate_points_render_all_in_editor(const vec3d* camera_eye, const matrix* camera_orient);

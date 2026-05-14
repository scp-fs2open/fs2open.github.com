#pragma once

// Helpers for editor-side object duplication (Ctrl-drag, Clone Marked Objects).
// Shared between FRED and QtFRED so both editors stay in sync.

// Copy every per-instance ship field that the FRED/QtFRED ship-editor
// dialogs expose, from src_shipnum -> dest_shipnum. dest_shipnum must
// already be a freshly-created ship of the same class as the source.
// Pure data copy: no UI, no popups, no editor-specific side effects.
//
// CANONICAL FIELD LIST - when you add a new editable ship field, update
// all of:
//   Fred_mission_save::save_objects() / save_common_object_data() in missionsave.cpp
//   parse_create_object_sub() in missionparse.cpp
//   clone_ship_instance_data() in missioneditor/objectduplication.cpp
void clone_ship_instance_data(int src_shipnum, int dest_shipnum);

// Copy every per-instance prop field that the FRED/QtFRED prop editor
// exposes, from src_prop_id -> dest_prop_id. dest_prop_id must already
// be a freshly-created prop of the same prop class as the source.
//
// CANONICAL FIELD LIST - when you add a new editable prop field, update
// all of:
//   Fred_mission_save::save_props() in missionsave.cpp
//   parse_prop() in missionparse.cpp
//   clone_prop_instance_data() in missioneditor/objectduplication.cpp
void clone_prop_instance_data(int src_prop_id, int dest_prop_id);

// Copy every per-instance jump node field that the FRED/QtFRED jump node
// editor exposes, from src to dest. The destination must already be a
// freshly-constructed CJumpNode at its desired position.
//
// CANONICAL FIELD LIST - when you add a new editable jump node field,
// update all of:
//   Fred_mission_save::save_waypoints() (jump-node section) in missionsave.cpp
//   jump node parse in missionparse.cpp (around the "$Jump Node:" handling)
//   clone_jump_node_instance_data() in missioneditor/objectduplication.cpp
class CJumpNode;
void clone_jump_node_instance_data(const CJumpNode& src, CJumpNode& dest);

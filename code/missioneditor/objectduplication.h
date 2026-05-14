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

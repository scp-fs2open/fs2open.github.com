#pragma once

// Checkpoint files.  Each pilot has one file per mission, holding all checkpoints saved in that mission under their
// index: data/players/<callsign>.<mission>.chk.

#include "checkpoint/codec.h"

namespace checkpoint {

// Stores a checkpoint under an index, replacing what was stored there.  Returns false if the file couldn't be written.
bool store_save(int index, const Buffer& buffer);

// Reads the checkpoint stored under an index.  Returns false if there is none.
bool store_load(int index, Buffer& buffer);

// Reads all checkpoints of the current mission, by index.
void store_load_all(SCP_map<int, Buffer>& slots);

// Whether a checkpoint is stored under an index, or under any index if index is negative.  The mission defaults to the
// current one; it may be given with or without its extension.
bool store_has(int index, const char* mission = nullptr);

// The lowest index from 1 up that has no checkpoint in the current mission's file.
int store_next_free_index();

// Removes the checkpoint stored under an index, or the whole file if index is negative.
void store_clear(int index);

// Removes all checkpoint files of a pilot, e.g. when the pilot is deleted.
void store_delete_pilot_files(const char* callsign);

} // namespace checkpoint

#pragma once

// An index into a list that is built up while the game runs, such as a list that grows as missions need entries.
// The same entry can sit at a different index in another session, so such an index is not stable data.
//
// A type deriving from session_index converts to and from int like the index it replaces.  It also says how it is
// identified independently of the session, which checkpoints use to save and restore it:
//
//     Key checkpoint_key() const;                             // a stable key for the entry
//     static Derived from_checkpoint_key(const Key& key);     // finds or adds the entry for a key
struct session_index {
	int index = -1;

	constexpr session_index() = default;
	constexpr session_index(int i) : index(i) {}
	constexpr operator int() const { return index; }
};

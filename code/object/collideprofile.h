#pragma once

#include <cstdint>

/** @file
 *  Lightweight instrumentation for the collision subsystem.
 *
 *  The narrowphase runs on the collision worker threads, and the tracing system cannot be used
 *  there: tracing::complete::start/end bump a plain (non-atomic) `current_id` and hand events to
 *  FrameProfiler, which latches a main thread id from whichever event reaches it first.  So the
 *  counters here live in thread-local storage instead, are incremented without atomics, and are
 *  folded into a shared per-frame total once per thread per frame.
 *
 *  Counts, not timings, are the primary signal.  A BSP node visit count is deterministic and
 *  machine independent, which makes it a far better regression test for a traversal optimization
 *  than a wall clock number taken from a mission that is never reproducible frame for frame.
 *
 *  Two levels of detail:
 *    - the per-frame phase timers and the -collision_bench driver are always compiled in; they
 *      cost a handful of clock reads per frame, which is not measurable.
 *    - the per-pair and per-BSP-node counters are hot enough to perturb what they measure, so
 *      they are gated on COLLISION_PROFILING.  Measured cost on bp2-massivebattle: 4.29 vs
 *      4.07 ms/frame, i.e. about 5% of the collision phase, nearly all of it the per-pair
 *      narrowphase timer.  Set this to 0 when you want timings rather than counts.
 */

#define COLLISION_PROFILING 1

namespace collision_profiling {

struct counters {
	// broadphase
	std::uint64_t pair_calls;		// every obj_collide_pair call, including the ones rejected up front
	std::uint64_t pairs_considered;		// obj_collide_pair calls that got as far as the type dispatch
	std::uint64_t cache_size;		// Collision_cached_pairs entry count, sampled once per frame
	std::uint64_t pairs_cache_skipped;	// rejected by the Collision_cached_pairs timestamp
	std::uint64_t pairs_enqueued;		// handed to a worker thread
	std::uint64_t pairs_checked_inline;	// narrowphased on the main thread

	// breakdown of pairs_checked_inline, so it is visible which types are still costing us
	std::uint64_t inline_beam;
	std::uint64_t inline_weapon_weapon;
	std::uint64_t inline_debris_ship;
	std::uint64_t inline_asteroid_ship;
	std::uint64_t inline_prop;
	std::uint64_t inline_other;

	// narrowphase
	std::uint64_t model_collide_calls;
	std::uint64_t bsp_node_visits;		// model_collide_bsp entries
	std::uint64_t bsp_leaf_tests;		// polys pulled out of a leaf chain
	std::uint64_t sphereline_edge_tests;	// fvi_polyedge_sphereline calls, the expensive branch

	// threading
	std::uint64_t worker_idle_spins;	// worker loop iterations with nothing to do
	std::uint64_t drain_spins;		// post_process_threaded_collisions loop iterations

	// main thread wall time, nanoseconds
	std::uint64_t collision_ns;		// all of obj_sort_and_collide
	std::uint64_t sort_ns;			// the three quicksort passes
	std::uint64_t overlap_ns;		// the three sweep passes (includes obj_collide_pair + inline narrowphase)
	std::uint64_t narrowphase_inline_ns;	// the check_collision calls made on the main thread
	std::uint64_t cache_lookup_ns;		// just the Collision_cached_pairs hash lookup
	std::uint64_t drain_ns;			// post_process_threaded_collisions

	counters& operator+=(const counters& other);
};

extern thread_local counters Local;

//! Fold this thread's counters into the frame total and reset them.  Called once per thread per frame.
void flush_local();

/**
 * @brief Per-frame benchmark driver, hooked into game_do_frame().
 *
 * Does nothing unless -collision_bench was passed.  Skips a warmup window, accumulates the
 * requested number of frames, writes a summary to stdout and quits the game.
 */
void benchmark_frame();

//! Once-per-frame accounting; always compiled in.
#define COLLISION_PROF_FRAME_ADD(field, n) (::collision_profiling::Local.field += (n))

#if COLLISION_PROFILING
//! Per-pair / per-node accounting; hot enough to perturb the measurement, so it is opt-in.
#define COLLISION_PROF_INC(field) (++::collision_profiling::Local.field)
#define COLLISION_PROF_ADD(field, n) (::collision_profiling::Local.field += (n))
#else
#define COLLISION_PROF_INC(field) ((void)0)
#define COLLISION_PROF_ADD(field, n) ((void)0)
#endif

}

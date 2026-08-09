#include "object/collideprofile.h"

#include "cmdline/cmdline.h"
#include "gamesequence/gamesequence.h"
#include "utils/threading.h"

#include <cinttypes>
#include <cstdio>
#include <mutex>

namespace collision_profiling {

counters& counters::operator+=(const counters& other)
{
	pair_calls += other.pair_calls;
	pairs_considered += other.pairs_considered;
	cache_size += other.cache_size;
	pairs_cache_skipped += other.pairs_cache_skipped;
	pairs_enqueued += other.pairs_enqueued;
	pairs_checked_inline += other.pairs_checked_inline;
	inline_beam += other.inline_beam;
	inline_weapon_weapon += other.inline_weapon_weapon;
	inline_debris_ship += other.inline_debris_ship;
	inline_asteroid_ship += other.inline_asteroid_ship;
	inline_prop += other.inline_prop;
	inline_other += other.inline_other;

	model_collide_calls += other.model_collide_calls;
	bsp_node_visits += other.bsp_node_visits;
	bsp_leaf_tests += other.bsp_leaf_tests;
	sphereline_edge_tests += other.sphereline_edge_tests;

	worker_idle_spins += other.worker_idle_spins;
	drain_spins += other.drain_spins;

	collision_ns += other.collision_ns;
	sort_ns += other.sort_ns;
	overlap_ns += other.overlap_ns;
	narrowphase_inline_ns += other.narrowphase_inline_ns;
	cache_lookup_ns += other.cache_lookup_ns;
	drain_ns += other.drain_ns;

	return *this;
}

thread_local counters Local = {};

static std::mutex Frame_mutex;
static counters Frame_total = {};

// accumulated across the measured window of a -collision_bench run
static counters Bench_total = {};
static int Bench_frames_seen = 0;
static bool Bench_finished = false;

// Mission load and page-in leave the first frames wildly unrepresentative, so throw some away
// before we start accumulating.
static const int BENCH_WARMUP_FRAMES = 120;

void flush_local()
{
	std::scoped_lock lock(Frame_mutex);
	Frame_total += Local;
	Local = counters{};
}

static void reset_frame()
{
	std::scoped_lock lock(Frame_mutex);
	Frame_total = counters{};
}

static void dump(const counters& c, int frames)
{
	if (frames < 1) {
		frames = 1;
	}
	const auto per_frame = [frames](std::uint64_t total) { return static_cast<double>(total) / frames; };

	// Deliberately not mprintf: LoggingEnabled is false in NDEBUG builds without SCP_RELEASE_LOGGING
	// (pstypes.h), and a release build is exactly where we need these numbers.
	FILE* out = stdout;
	fprintf(out, "\n=== collision benchmark: %d frames ===\n", frames);
	fprintf(out, "  threads              : %d worker(s)\n", static_cast<int>(threading::get_num_workers()));
	fprintf(out, "  collision phase      : %.3f ms/frame (main thread, obj_sort_and_collide)\n",
		per_frame(c.collision_ns) / 1000000.0);
	fprintf(out, "    sort passes        : %.3f ms/frame\n", per_frame(c.sort_ns) / 1000000.0);
	fprintf(out, "    sweep passes       : %.3f ms/frame (incl. inline narrowphase)\n", per_frame(c.overlap_ns) / 1000000.0);
	fprintf(out, "      inline narrowph. : %.3f ms/frame\n", per_frame(c.narrowphase_inline_ns) / 1000000.0);
	fprintf(out, "      pair gen/cull    : %.3f ms/frame\n",
		(per_frame(c.overlap_ns) - per_frame(c.narrowphase_inline_ns)) / 1000000.0);
	fprintf(out, "        pair cache hash: %.3f ms/frame\n", per_frame(c.cache_lookup_ns) / 1000000.0);
	fprintf(out, "    worker drain       : %.3f ms/frame\n", per_frame(c.drain_ns) / 1000000.0);
	fprintf(out, "  -- broadphase --\n");
	fprintf(out, "  obj_collide_pair call: %.1f /frame\n", per_frame(c.pair_calls));
	fprintf(out, "  pairs considered     : %.1f /frame (%" PRIu64 " total)\n", per_frame(c.pairs_considered), c.pairs_considered);
	fprintf(out, "  pair cache entries   : %.0f (mean over the window)\n", per_frame(c.cache_size));
	fprintf(out, "  cache skipped        : %.1f /frame\n", per_frame(c.pairs_cache_skipped));
	fprintf(out, "  enqueued to workers  : %.1f /frame\n", per_frame(c.pairs_enqueued));
	fprintf(out, "  checked inline       : %.1f /frame\n", per_frame(c.pairs_checked_inline));
	fprintf(out, "    beam               : %.1f /frame\n", per_frame(c.inline_beam));
	fprintf(out, "    weapon<->weapon    : %.1f /frame\n", per_frame(c.inline_weapon_weapon));
	fprintf(out, "    debris<->ship      : %.1f /frame\n", per_frame(c.inline_debris_ship));
	fprintf(out, "    asteroid<->ship    : %.1f /frame\n", per_frame(c.inline_asteroid_ship));
	fprintf(out, "    prop               : %.1f /frame\n", per_frame(c.inline_prop));
	fprintf(out, "    other              : %.1f /frame\n", per_frame(c.inline_other));
	fprintf(out, "  -- narrowphase --\n");
	fprintf(out, "  model_collide calls  : %.1f /frame\n", per_frame(c.model_collide_calls));
	fprintf(out, "  BSP node visits      : %.1f /frame (%" PRIu64 " total)\n", per_frame(c.bsp_node_visits), c.bsp_node_visits);
	fprintf(out, "  BSP leaf poly tests  : %.1f /frame\n", per_frame(c.bsp_leaf_tests));
	fprintf(out, "  sphereline edge tests: %.1f /frame\n", per_frame(c.sphereline_edge_tests));
	fprintf(out, "  -- threading --\n");
	fprintf(out, "  worker idle spins    : %.1f /frame (%" PRIu64 " total)\n", per_frame(c.worker_idle_spins), c.worker_idle_spins);
	fprintf(out, "  drain spins          : %.1f /frame (%" PRIu64 " total)\n", per_frame(c.drain_spins), c.drain_spins);
	fprintf(out, "=== end collision benchmark ===\n\n");
	fflush(out);
}

void benchmark_frame()
{
	if (Cmdline_collision_bench <= 0 || Bench_finished) {
		reset_frame();
		return;
	}

	++Bench_frames_seen;

	if (Bench_frames_seen > BENCH_WARMUP_FRAMES) {
		Bench_total += Frame_total;

		if (Bench_frames_seen - BENCH_WARMUP_FRAMES >= Cmdline_collision_bench) {
			dump(Bench_total, Cmdline_collision_bench);
			Bench_finished = true;
			gameseq_post_event(GS_EVENT_QUIT_GAME);
		}
	}

	reset_frame();
}

}

#pragma once

/** @file
 *  @ingroup tracing
 */

namespace tracing {

/**
 * Per-frame entry point for the ImGui frame profiler overlay, called once from gr_flip().
 *
 * Drains the frame profiler (see frame_profile_process_frame()), folds the frame's total traced
 * time into the rolling history behind the graph/avg/median, and contributes the overlay window
 * -- avg/median frametime, a scrolling frametime graph, a stacked frame-budget bar of the top
 * traced categories by self-time, and the -gr_debug graphics stats -- to this frame's ImGui pass,
 * opening one via gr_imgui_begin_frame() if no other consumer already has.
 *
 * No-op when frame profiling is disabled. Being the single per-frame driver is what keeps the
 * profiler's event buffer bounded in every game state, so it must stay on the common flip path
 * rather than being called per game state.
 */
void profiler_overlay_frame();

}

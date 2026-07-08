#pragma once

/** @file
 *  @ingroup tracing
 */

namespace tracing {

/**
 * Records the current frame's total traced time (see get_frame_profiler_overlay_snapshot()) into
 * the rolling history buffer used by profiler_overlay_draw()'s graph/avg/median. Call once per
 * frame, right after frame_profile_process_frame(), while profiling is active.
 */
void profiler_overlay_record_frame();

/**
 * Draws the ImGui/ImPlot frame profiler overlay window: avg/median frametime, a scrolling
 * frametime graph, and a pie chart of the top-5 traced categories by self-time. Self-contained --
 * does its own ImGui::NewFrame()/Render() and submits the draw data, since mission gameplay
 * doesn't otherwise touch ImGui. Must be called before the current frame is flipped.
 */
void profiler_overlay_draw();

}

//
//

#include "ProfilerOverlay.h"

#include "tracing.h"

#include "graphics/2d.h"
#include "io/timer.h"
#include "model/model.h"
#include "object/object.h"

// ImPlot's Plot*() functions are templates that get instantiated here (unlike ImGui's ordinary
// function API), and their header-inline ImPool<ImPlotItem>::Add() uses memcpy on the
// non-trivially-copyable ImPlotItem -- which trips pstypes.h's memcpy-safety macro. That macro
// exists to catch accidental memcpy of non-POD engine types; ImPlotItem is third-party-internal
// and already safe, so suppress it for just these headers.
#pragma push_macro("memcpy")
#undef memcpy
#include "imgui.h"
#include "implot.h"
#pragma pop_macro("memcpy")

#include <algorithm>
#include <numeric>

namespace tracing {

namespace {

constexpr size_t HISTORY_SIZE = 300; // ~5s at 60 FPS
constexpr double NANOSEC_PER_MS = 1'000'000.0;

SCP_vector<float> History_ms;

void push_history(float frame_ms) {
	History_ms.push_back(frame_ms);
	if (History_ms.size() > HISTORY_SIZE) {
		History_ms.erase(History_ms.begin());
	}
}

float history_average() {
	if (History_ms.empty()) {
		return 0.0f;
	}
	float sum = std::accumulate(History_ms.begin(), History_ms.end(), 0.0f);
	return sum / static_cast<float>(History_ms.size());
}

float history_median() {
	if (History_ms.empty()) {
		return 0.0f;
	}

	SCP_vector<float> sorted_copy(History_ms.begin(), History_ms.end());
	size_t mid = sorted_copy.size() / 2;
	std::nth_element(sorted_copy.begin(), sorted_copy.begin() + mid, sorted_copy.end());
	float median = sorted_copy[mid];

	if (sorted_copy.size() % 2 == 0) {
		std::nth_element(sorted_copy.begin(), sorted_copy.begin() + mid - 1, sorted_copy.end());
		median = (median + sorted_copy[mid - 1]) * 0.5f;
	}

	return median;
}

void draw_frametime_graph() {
	if (History_ms.empty()) {
		return;
	}

	if (ImPlot::BeginPlot("##frametime_ms",
			ImVec2(-1, 90),
			ImPlotFlags_NoMouseText | ImPlotFlags_NoLegend | ImPlotFlags_NoTitle)) {
		ImPlot::SetupAxes(nullptr,
			"ms",
			ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoGridLines,
			ImPlotAxisFlags_AutoFit);
		ImPlot::SetupAxisLimits(ImAxis_X1, 0, static_cast<double>(History_ms.size()), ImPlotCond_Always);

		ImPlot::PlotLine("frametime", History_ms.data(), static_cast<int>(History_ms.size()));

		ImPlot::EndPlot();
	}
}

/**
 * Maps a category name to a stable color, so a given category keeps the same color across frames
 * regardless of how its rank shifts in the breakdown. Uses an FNV-1a hash of the name to pick a
 * hue; "Other" gets a fixed neutral grey (see draw_frame_budget_bar).
 */
ImU32 color_for_name(const char* name) {
	uint32_t hash = 2166136261u; // FNV-1a
	for (const char* p = name; *p != '\0'; ++p) {
		hash ^= static_cast<uint8_t>(*p);
		hash *= 16777619u;
	}
	const float hue = static_cast<float>(hash % 360) / 360.0f;
	return ImGui::ColorConvertFloat4ToU32(static_cast<ImVec4>(ImColor::HSV(hue, 0.55f, 0.95f)));
}

/**
 * Draws a single 100%-stacked horizontal "frame budget" bar: one row whose width is split into
 * segments proportional to each category's share of the frame's traced time (the snapshot's
 * top contributors, see FRAME_OVERLAY_MAX_CONTRIBUTORS, + "Other"), followed by a legend with
 * matching swatches, absolute ms and percentage. Cheaper than a pie chart -- it's a handful of
 * ImDrawList rectangles with no ImPlot plot setup.
 */
void draw_frame_budget_bar(const frame_overlay_snapshot& snapshot) {
	if (snapshot.total_nanosec == 0) {
		return;
	}

	// +1 for "Other": snapshot.top_contributors holds at most FRAME_OVERLAY_MAX_CONTRIBUTORS
	// entries (see tracing.h), so that's the real bound on how many segments can ever exist here.
	constexpr int MAX_SEGMENTS = static_cast<int>(FRAME_OVERLAY_MAX_CONTRIBUTORS) + 1;
	constexpr ImU32 OTHER_COLOR = IM_COL32(130, 130, 130, 255);

	struct segment {
		const char* name;
		double pct;
		float ms;
		ImU32 color;
	};

	segment segments[MAX_SEGMENTS];
	int count = 0;

	const auto total = static_cast<double>(snapshot.total_nanosec);

	// Self-defending against MAX_SEGMENTS regardless of caller bookkeeping, since this writes into
	// a fixed-size array: silently drops anything past capacity rather than overflowing it.
	auto add_segment = [&](const char* name, uint64_t self_nanosec, ImU32 color) {
		if (count >= MAX_SEGMENTS) {
			return;
		}
		segments[count].name = name;
		segments[count].pct = 100.0 * static_cast<double>(self_nanosec) / total;
		segments[count].ms = static_cast<float>(static_cast<double>(self_nanosec) / NANOSEC_PER_MS);
		segments[count].color = color;
		count++;
	};

	for (const auto& contributor : snapshot.top_contributors) {
		add_segment(contributor.name.c_str(), contributor.self_nanosec, color_for_name(contributor.name.c_str()));
	}
	if (snapshot.other_nanosec > 0) {
		add_segment("Other", snapshot.other_nanosec, OTHER_COLOR);
	}

	if (count == 0) {
		return;
	}

	// The stacked bar.
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	const float width = ImGui::GetContentRegionAvail().x;
	constexpr float height = 22.0f;

	draw_list->AddRectFilled(origin, ImVec2(origin.x + width, origin.y + height), IM_COL32(35, 35, 35, 255), 3.0f);

	float x = origin.x;
	for (int i = 0; i < count; i++) {
		float seg_w = width * static_cast<float>(segments[i].pct / 100.0);
		// Make sure the final segment reaches the right edge despite float rounding.
		const float x_end = (i == count - 1) ? (origin.x + width) : (x + seg_w);
		draw_list->AddRectFilled(ImVec2(x, origin.y), ImVec2(x_end, origin.y + height), segments[i].color);
		x = x_end;
	}
	draw_list->AddRect(origin, ImVec2(origin.x + width, origin.y + height), IM_COL32(80, 80, 80, 255), 3.0f);

	ImGui::Dummy(ImVec2(width, height));

	// The legend.
	for (int i = 0; i < count; i++) {
		ImGui::ColorButton(segments[i].name,
			ImGui::ColorConvertU32ToFloat4(segments[i].color),
			ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder,
			ImVec2(12, 12));
		ImGui::SameLine();
		ImGui::Text("%s: %.2f ms (%.1f%%)", segments[i].name, segments[i].ms, segments[i].pct);
	}
}

/**
 * Draws whichever -gr_debug stat groups the active graphics backend collects (see
 * gr_get_debug_stats()). Silently omits a group if its "valid" flag is unset, so this stays a
 * no-op for backends (or builds) that don't populate a given group.
 */
void draw_graphics_debug_stats() {
	gr_debug_stats stats = gr_get_debug_stats();

	if (!stats.uniform_buffer_valid && !stats.draw_stats_valid) {
		return;
	}

	ImGui::Separator();
	ImGui::TextUnformatted("Graphics API stats (-gr_debug)");

	if (stats.uniform_buffer_valid) {
		ImGui::Text("Uniform buffer: " SIZE_T_ARG " / " SIZE_T_ARG " bytes used",
			stats.uniform_buffer_used,
			stats.uniform_buffer_size);
	}

	if (stats.draw_stats_valid) {
		ImGui::Text("Draw calls: %d (%d indexed)", stats.draw_calls, stats.draw_indexed_calls);
		ImGui::Text("Vertices: %d   Indices: %d", stats.total_vertices, stats.total_indices);
		ImGui::Text("Material applies: %d (%d failed, %d no pipeline)",
			stats.apply_material_calls,
			stats.apply_material_failures,
			stats.no_pipeline_skips);
		ImGui::Text("Descriptor sets: %d   writes: %d   pipelines: " SIZE_T_ARG,
			stats.descriptor_sets_allocated,
			stats.descriptor_writes,
			stats.pipeline_count);
		if (stats.on_demand_texture_uploads > 0) {
			ImGui::Text("On-demand texture uploads: %d", stats.on_demand_texture_uploads);
		}
	}
}

// Memory stats describe level-scope state (loaded models, live object pools, GPU heaps), not
// per-frame state, so they're refreshed on an interval rather than walked every frame like the
// timing data above -- that avoids perturbing the very frame cost this overlay measures.
constexpr int MEMORY_STATS_REFRESH_INTERVAL_MS = 1000;

bool Memory_stats_initialized = false;
int Last_memory_stats_refresh_ms = 0;
object_memory_stats Cached_object_memory_stats;
model_memory_stats Cached_model_memory_stats;
gr_memory_stats Cached_gr_memory_stats;

void refresh_memory_stats_if_needed() {
	int now_ms = timer_get_milliseconds();
	if (Memory_stats_initialized && (now_ms - Last_memory_stats_refresh_ms) < MEMORY_STATS_REFRESH_INTERVAL_MS) {
		return;
	}

	Cached_object_memory_stats = obj_get_memory_stats();
	Cached_model_memory_stats = model_get_memory_stats();
	Cached_gr_memory_stats = gr_get_memory_stats();

	Last_memory_stats_refresh_ms = now_ms;
	Memory_stats_initialized = true;
}

/**
 * Formats a byte count as a human-readable string using the largest unit that keeps at least one
 * digit before the decimal point (e.g. "12.3 MB"). Returned by value: several call sites below pass
 * multiple format_bytes() results as arguments to the same ImGui::Text() call, and every temporary
 * in a function call's argument list lives until the end of that call, so this needs no buffer
 * management to stay safe -- unlike returning a pointer into any kind of shared/reused storage.
 */
SCP_string format_bytes(size_t bytes) {
	constexpr double KB = 1024.0;
	constexpr double MB = KB * 1024.0;
	constexpr double GB = MB * 1024.0;

	char buf[32];
	if (auto b = static_cast<double>(bytes); b >= GB) {
		snprintf(buf, sizeof(buf), "%.2f GB", b / GB);
	} else if (b >= MB) {
		snprintf(buf, sizeof(buf), "%.2f MB", b / MB);
	} else if (b >= KB) {
		snprintf(buf, sizeof(buf), "%.2f KB", b / KB);
	} else {
		snprintf(buf, sizeof(buf), SIZE_T_ARG " B", bytes);
	}

	return SCP_string(buf);
}

/**
 * Draws pool occupancy (used / max) for the object, ship, and weapon pools. These are fixed-size
 * static arrays, so their byte footprint is a compile-time constant -- occupancy is the only
 * signal worth showing.
 */
void draw_object_memory_stats() {
	const object_memory_stats& stats = Cached_object_memory_stats;

	ImGui::Separator();
	ImGui::TextUnformatted("Object Management");
	ImGui::Text("Objects: %d / %d (peak %d)", stats.objects_used, stats.objects_max, stats.objects_peak);
	ImGui::Text("Ships:   %d / %d", stats.ships_used, stats.ships_max);
	ImGui::Text("Weapons: %d / %d", stats.weapons_used, stats.weapons_max);
}

/**
 * Draws model and texture memory usage. The GPU-heap numbers (live, reflects frees as models
 * unload) and the per-model summed numbers (from currently loaded models) are two different views
 * of related-but-not-identical data and are deliberately not added together into a single total.
 */
void draw_asset_memory_stats() {
	const model_memory_stats& model_stats = Cached_model_memory_stats;
	const gr_memory_stats& gr_stats = Cached_gr_memory_stats;

	if (!model_stats.valid && !gr_stats.model_heap_valid && !gr_stats.locked_bitmap_ram_valid) {
		return;
	}

	ImGui::Separator();
	ImGui::TextUnformatted("Assets");

	if (model_stats.valid) {
		ImGui::Text("Models loaded: %d", model_stats.model_count);
		ImGui::Text("  Vertex data: %s   Index data: %s   BSP/collision: %s",
			format_bytes(model_stats.vertex_bytes).c_str(),
			format_bytes(model_stats.index_bytes).c_str(),
			format_bytes(model_stats.bsp_data_bytes).c_str());
	}

	if (gr_stats.locked_bitmap_ram_valid) {
		ImGui::Text("Bitmaps (locked): %s", format_bytes(gr_stats.locked_bitmap_ram_bytes).c_str());
	}

	if (gr_stats.model_heap_valid) {
		ImGui::Text("GPU model vertex heap: %s / %s",
			format_bytes(gr_stats.model_vertex_heap_used).c_str(),
			format_bytes(gr_stats.model_vertex_heap_size).c_str());
		ImGui::Text("GPU model index heap:  %s / %s",
			format_bytes(gr_stats.model_index_heap_used).c_str(),
			format_bytes(gr_stats.model_index_heap_size).c_str());
	}

	if (gr_stats.gpu_purpose_valid) {
		ImGui::Text("GPU textures: %s   geometry: %s   render targets: %s",
			format_bytes(gr_stats.gpu_texture_bytes).c_str(),
			format_bytes(gr_stats.gpu_geometry_bytes).c_str(),
			format_bytes(gr_stats.gpu_render_target_bytes).c_str());
	}
}

} // namespace

void profiler_overlay_frame() {
	if (!frame_profiling_active()) {
		History_ms.clear();
		return;
	}

	frame_profile_process_frame();

	const frame_overlay_snapshot& snapshot = get_frame_profiler_overlay_snapshot();
	if (snapshot.valid) {
		push_history(static_cast<float>(static_cast<double>(snapshot.total_nanosec) / NANOSEC_PER_MS));
	}

	gr_imgui_begin_frame();
	if (!gr_imgui_frame_active()) {
		// No ImGui backend on this renderer (standalone server) — nothing to draw into.
		return;
	}

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(380, 480), ImGuiCond_FirstUseEver);
	ImGui::Begin("Frame Profiler", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);

	if (History_ms.empty()) {
		ImGui::TextUnformatted("Collecting data...");
	} else {
		float avg_ms = history_average();
		float median_ms = history_median();

		ImGui::Text("Avg: %.2f ms (%.0f FPS)   Median: %.2f ms",
			avg_ms,
			avg_ms > 0.0f ? 1000.0f / avg_ms : 0.0f,
			median_ms);

		draw_frametime_graph();

		ImGui::Separator();

		draw_frame_budget_bar(snapshot);
	}

	draw_graphics_debug_stats();

	refresh_memory_stats_if_needed();
	if (ImGui::CollapsingHeader("Memory", ImGuiTreeNodeFlags_DefaultOpen)) {
		draw_object_memory_stats();
		draw_asset_memory_stats();
	}

	ImGui::End();
}

} // namespace tracing

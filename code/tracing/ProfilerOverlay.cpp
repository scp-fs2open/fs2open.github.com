//
//

#include "ProfilerOverlay.h"

#include "tracing.h"

#include "graphics/2d.h"

// ImPlot's Plot*() functions are templates that get instantiated here (unlike ImGui's ordinary
// function API), and their header-inline ImPool<ImPlotItem>::Add() uses memcpy on the
// non-trivially-copyable ImPlotItem -- which trips pstypes.h's memcpy-safety macro. That macro
// exists to catch accidental memcpy of non-POD engine types; ImPlotItem is third-party-internal
// and already safe, so suppress it for just these headers.
#pragma push_macro("memcpy")
#undef memcpy
#include "imgui.h"
#include "implot.h"
#include "implot_internal.h"
#pragma pop_macro("memcpy")
#include "backends/imgui_impl_sdl3.h"

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
 * Draws the pie chart (left) and a text legend with matching swatch colors (right). Slice colors
 * come from ImPlot's own per-item color assignment (keyed by category name), which stays stable
 * across frames as long as the same names keep appearing -- no manual palette bookkeeping needed.
 */
void draw_pie_chart(const frame_overlay_snapshot& snapshot) {
	if (snapshot.total_nanosec == 0) {
		return;
	}

	constexpr int MAX_SLICES = 6; // top 5 contributors + "Other"

	static SCP_string label_storage[MAX_SLICES];
	static const char* labels[MAX_SLICES];
	double pct_values[MAX_SLICES];
	int count = 0;

	auto add_slice = [&](const SCP_string& name, uint64_t self_nanosec) {
		label_storage[count] = name;
		labels[count] = label_storage[count].c_str();
		pct_values[count] = 100.0 * static_cast<double>(self_nanosec) / static_cast<double>(snapshot.total_nanosec);
		count++;
	};

	for (auto& contributor : snapshot.top_contributors) {
		if (count >= MAX_SLICES) {
			break;
		}
		add_slice(contributor.name, contributor.self_nanosec);
	}
	if (snapshot.other_nanosec > 0 && count < MAX_SLICES) {
		add_slice("Other", snapshot.other_nanosec);
	}

	if (count == 0) {
		return;
	}

	ImVec4 slice_colors[MAX_SLICES];

	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnWidth(0, 190);

	if (ImPlot::BeginPlot("##frametime_pie",
			ImVec2(180, 180),
			ImPlotFlags_Equal | ImPlotFlags_NoMouseText | ImPlotFlags_NoLegend)) {
		ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
		ImPlot::SetupAxesLimits(0, 1, 0, 1, ImPlotCond_Always);

		ImPlot::PlotPieChart(labels, pct_values, count, 0.5, 0.5, 0.4, "%.1f%%");

		for (int i = 0; i < count; i++) {
			ImPlotItem* item = ImPlot::GetItem(labels[i]);
			slice_colors[i] = item ? ImGui::ColorConvertU32ToFloat4(item->Color) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		ImPlot::EndPlot();
	}

	ImGui::NextColumn();

	for (int i = 0; i < count; i++) {
		ImGui::ColorButton(labels[i], slice_colors[i], ImGuiColorEditFlags_NoTooltip, ImVec2(12, 12));
		ImGui::SameLine();
		ImGui::Text("%s: %.1f%%", labels[i], pct_values[i]);
	}

	ImGui::Columns(1);
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

} // namespace

void profiler_overlay_record_frame() {
	const frame_overlay_snapshot& snapshot = get_frame_profiler_overlay_snapshot();
	if (!snapshot.valid) {
		return;
	}

	push_history(static_cast<float>(static_cast<double>(snapshot.total_nanosec) / NANOSEC_PER_MS));
}

void profiler_overlay_draw() {
	gr_imgui_new_frame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(380, 480), ImGuiCond_FirstUseEver);
	ImGui::Begin("Frame Profiler", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);

	if (!frame_profiling_active() || History_ms.empty()) {
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

		draw_pie_chart(get_frame_profiler_overlay_snapshot());
	}

	draw_graphics_debug_stats();

	ImGui::End();

	ImGui::Render();
	gr_imgui_render_draw_data();
}

} // namespace tracing

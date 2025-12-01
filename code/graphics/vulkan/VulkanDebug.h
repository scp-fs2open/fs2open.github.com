#pragma once

#include "globalincs/pstypes.h"

#include <cstdarg>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace graphics {
namespace vulkan {

inline void vk_write_debug_line(const char* tag, const char* message, bool mirror_to_mprintf)
{
	if (!message) {
		return;
	}

	std::string line;
	if (tag && *tag) {
		line.append(tag).append(": ");
	}
	line.append(message);

	const bool has_newline = !line.empty() && line.back() == '\n';
	if (!has_newline) {
		line.push_back('\n');
	}

	FILE* f = fopen("vulkan_debug.log", "a");
	if (f) {
		fwrite(line.c_str(), 1, line.size(), f);
		fflush(f);
		fclose(f);
	} else {
		fprintf(stderr, "Vulkan: Failed to write debug log to 'vulkan_debug.log': %s\n", strerror(errno));
	}

	if (mirror_to_mprintf) {
		mprintf(("%s", line.c_str()));
	}
}

// File-only log entry with VulkanRenderer prefix (matches existing initialization log style)
inline void vk_debug(const char* msg)
{
	vk_write_debug_line("VulkanRenderer", msg ? msg : "<null>", false);
}

// Log to both mprintf and vulkan_debug.log with a caller-provided tag
inline void vk_log(const char* tag, const char* msg)
{
	vk_write_debug_line(tag, msg ? msg : "<null>", true);
}

// Formatted log to both mprintf and vulkan_debug.log with a caller-provided tag
inline void vk_logf(const char* tag, const char* fmt, ...)
{
	if (!fmt) {
		return;
	}

	va_list args;
	va_start(args, fmt);
	const int needed = std::vsnprintf(nullptr, 0, fmt, args);
	va_end(args);

	if (needed <= 0) {
		vk_write_debug_line(tag, "vk_logf: formatting error", true);
		return;
	}

	std::vector<char> buffer(static_cast<size_t>(needed) + 1);
	va_start(args, fmt);
	std::vsnprintf(buffer.data(), buffer.size(), fmt, args);
	va_end(args);

	vk_write_debug_line(tag, buffer.data(), true);
}

} // namespace vulkan
} // namespace graphics

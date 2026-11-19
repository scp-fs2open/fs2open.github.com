#include "cfile/mod_bootstrap.h"

#include "libs/jansson.h"
#include "osapi/osapi.h"

#include <SDL3/SDL.h>
#include <filesystem>
#include <optional>

namespace {

namespace fs = std::filesystem;
std::optional<ModBootstrap> Active_bootstrap;

SCP_string resolve_directory(fs::path path, const fs::path& base, const SCP_string& field)
{
	if (!path.is_absolute()) {
		if (base.empty() || path.has_root_path()) {
			throw std::runtime_error(field + " requires a fully qualified path: " + path.u8string());
		}
		path = base / path;
	}
	path = path.lexically_normal().make_preferred();

	std::error_code error;
	fs::directory_iterator entries(path, error);
	if (error) {
		throw std::runtime_error(field + " cannot be opened as a directory: " + path.u8string() +
			" (" + error.message() + ")");
	}

	auto result = path.string();
	// CFILE appends a separator when it installs the root.
	if (result.size() + 1 >= CF_MAX_PATHNAME_LENGTH) {
		throw std::runtime_error(field + " exceeds CFILE's path limit: " + path.u8string());
	}
	return result;
}

ModBootstrap read_bootstrap(const char* filename)
{
	if (!fs::u8path(filename).is_absolute()) {
		throw std::runtime_error("-bootstrap requires an absolute path");
	}

	size_t size = 0;
	std::unique_ptr<void, decltype(&SDL_free)> bytes(SDL_LoadFile(filename, &size), SDL_free);
	if (!bytes) {
		throw std::runtime_error(SDL_GetError());
	}

	json_error_t error;
	std::unique_ptr<json_t> document(json_loadb(static_cast<const char*>(bytes.get()), size, JSON_REJECT_DUPLICATES, &error));
	if (!document) {
		throw json_exception(error);
	}
	if (!json_is_object(document.get())) {
		throw std::runtime_error("the top-level value must be a JSON object");
	}
	const auto version = json_object_get(document.get(), "version");
	const auto format_version = json_integer_value(version);
	if (!json_is_integer(version) || format_version != 1) {
		throw std::runtime_error("expected bootstrap version 1");
	}
	const auto roots = json_object_get(document.get(), "roots");
	if (!json_is_array(roots)) {
		throw std::runtime_error("roots must be an array of directory paths");
	}

	ModBootstrap result;
	result.version = static_cast<int>(format_version);
	const auto mod_id = json_object_get(document.get(), "mod_id");
	if (mod_id && !json_is_null(mod_id)) {
		if (!json_is_string(mod_id)) {
			throw std::runtime_error("mod_id must be a string or null");
		}
		result.mod_id = json_string_value(mod_id);
		if (result.mod_id.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.") != SCP_string::npos ||
			result.mod_id == "." || result.mod_id == "..") {
			throw std::runtime_error("mod_id contains characters that cannot be used in a settings filename");
		}
	}
	if (json_array_size(roots) != 0 && result.mod_id.empty()) {
		throw std::runtime_error("mod_id is required when roots contains mods");
	}

	const auto workfolder = json_object_get(document.get(), "workfolder");
	if (workfolder && !json_is_null(workfolder) && !json_is_string(workfolder)) {
		throw std::runtime_error("workfolder must be an absolute path or null");
	}
	fs::path base;
	if (json_is_string(workfolder) && json_string_length(workfolder) != 0) {
		base = fs::u8path(json_string_value(workfolder));
	} else {
#ifdef __ANDROID__
		base = fs::u8path(os_android_get_working_folder_path());
#else
		base = fs::current_path();
#endif
	}
	result.workfolder = resolve_directory(base, {}, "workfolder");

	size_t index;
	json_t* root;
	json_array_foreach(roots, index, root) {
		const auto field = "roots[" + std::to_string(index) + "]";
		if (!json_is_string(root) || json_string_length(root) == 0) {
			throw std::runtime_error(field + " must be a nonempty directory path");
		}
		result.roots.push_back(resolve_directory(fs::u8path(json_string_value(root)), base, field));
	}
	return result;
}

} // namespace

void mod_bootstrap_init(const char* filename)
{
	Active_bootstrap.reset();
	if (filename) {
		Active_bootstrap = read_bootstrap(filename);
	}
}

const ModBootstrap* mod_bootstrap_get()
{
	return Active_bootstrap ? &*Active_bootstrap : nullptr;
}

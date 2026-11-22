#include "cfile/mod_bootstrap.h"

#include "libs/jansson.h"
#include "osapi/dialogs.h"
#include "osapi/osapi.h"

#include <SDL3/SDL.h>
#include <algorithm>
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

ModBootstrap read_bootstrap(const char* filename, const char* launch_directory)
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
	if (json_typeof(document.get()) != JSON_OBJECT) {
		throw std::runtime_error("the top-level value must be a JSON object");
	}
	const auto format_version = json_integer_value(json_object_get(document.get(), "version"));
	if (format_version > MOD_BOOTSTRAP_VERSION) {
		throw std::runtime_error("expected bootstrap version up to " + std::to_string(MOD_BOOTSTRAP_VERSION));
	}
	if (format_version < 1) {
		throw std::runtime_error("invalid bootstrap version: " + std::to_string(format_version));
	}
	const auto roots = json_object_get(document.get(), "roots");
	if (!roots || json_typeof(roots) != JSON_ARRAY) {
		throw std::runtime_error("roots must be an array of directory paths");
	}

	ModBootstrap result;
	result.version = static_cast<int>(format_version);
	result.filename = filename;
	if (launch_directory) {
		result.launch_directory = launch_directory;
	}
	const auto mod_id = json_object_get(document.get(), "mod_id");
	if (mod_id && json_typeof(mod_id) != JSON_NULL) {
		if (json_typeof(mod_id) != JSON_STRING) {
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
	const auto workfolder_type = workfolder ? json_typeof(workfolder) : JSON_NULL;
	if (workfolder_type != JSON_NULL && workfolder_type != JSON_STRING) {
		throw std::runtime_error("workfolder must be an absolute path or null");
	}
	fs::path base;
	if (workfolder_type == JSON_STRING && json_string_length(workfolder) != 0) {
		base = fs::u8path(json_string_value(workfolder));
	} else if (launch_directory) {
		base = fs::path(launch_directory);
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
		if (!root || json_typeof(root) != JSON_STRING || json_string_length(root) == 0) {
			throw std::runtime_error(field + " must be a nonempty directory path");
		}
		result.roots.push_back(resolve_directory(fs::u8path(json_string_value(root)), base, field));
	}
	return result;
}

} // namespace

void mod_bootstrap_init(const char* filename, const char* launch_directory)
{
	Active_bootstrap.reset();
	if (filename) {
		Active_bootstrap = read_bootstrap(filename, launch_directory);
	}
}

bool mod_bootstrap_init_from_cmdline(const char* launch_directory)
{
	const char* filename = nullptr;
	try {
		filename = cmdline_bootstrap_path();
		mod_bootstrap_init(filename, launch_directory);
		return true;
	} catch (const std::exception& error) {
		const auto message = SCP_string("Could not load -bootstrap") +
			(filename ? SCP_string(" '") + filename + "'" : "") + ": " + error.what();
		fprintf(stderr, "%s\n", message.c_str());
		os::dialogs::Message(os::dialogs::MESSAGEBOX_ERROR, message.c_str());
		return false;
	}
}

void mod_bootstrap_log()
{
	if (const auto bootstrap = mod_bootstrap_get()) {
		mprintf(("Bootstrap: '%s' (version %d, mod '%s', workfolder '%s')\n", bootstrap->filename.c_str(),
			bootstrap->version, bootstrap->mod_id.c_str(), bootstrap->workfolder.c_str()));
		for (size_t i = 0; i < bootstrap->roots.size(); ++i) {
			mprintf(("  roots[%zu]: '%s'\n", i, bootstrap->roots[i].c_str()));
		}
	}
}

const ModBootstrap* mod_bootstrap_get()
{
	return Active_bootstrap ? &*Active_bootstrap : nullptr;
}

bool mod_bootstrap_contains_path(const SCP_string& filename)
{
	const auto bootstrap = mod_bootstrap_get();
	if (!bootstrap || filename.empty()) {
		return false;
	}

	const auto normalize = [](const SCP_string& path) {
		std::error_code error;
		auto normalized = fs::weakly_canonical(fs::path(path), error);
		return error ? fs::path(path).lexically_normal() : normalized;
	};
	const auto file = normalize(filename);
	const auto in_directory = [&](const SCP_string& directory) {
		auto root = normalize(directory);
		if (root.has_relative_path() && !root.has_filename()) {
			root = root.parent_path();
		}
		auto matches = std::mismatch(root.begin(), root.end(), file.begin(), file.end(),
			[](const fs::path& first, const fs::path& second) {
#ifdef _WIN32
				return _wcsicmp(first.c_str(), second.c_str()) == 0;
#else
				return first == second;
#endif
			});
		return matches.first == root.end();
	};

	return in_directory(bootstrap->workfolder) ||
		std::any_of(bootstrap->roots.begin(), bootstrap->roots.end(), in_directory);
}

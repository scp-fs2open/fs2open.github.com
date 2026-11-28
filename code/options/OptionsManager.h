#pragma once

#include "globalincs/pstypes.h"
#include "libs/jansson.h"
#include <functional>
#include <memory>
#include <optional>

namespace options {

// Forward declaration
class OptionBase;

enum class OverrideSource {
	CommandLine,
	Mod
};
// Whether this machine can meet an option's requirement; Unknown when nothing could find out
enum class RequirementSupport {
	Unknown,
	Supported,
	Unsupported
};
using PostPersistHook = std::function<SCP_vector<const OptionBase*>(const SCP_vector<const OptionBase*>& changed)>;

// A JSON string for text that may not be UTF-8: such text is read as Latin-1 instead of being dropped. Caller owns it.
json_t* json_text(const SCP_string& text);

// Writes root to out as one line of JSON ending in a newline; false if any of it could not be written
bool json_write_line(const json_t* root, FILE* out, size_t flags);

class OptionsManager {
	OptionsManager();

	// A value forced by something outside of the options system itself: a command line flag, or a mod's own tables.
	// This takes priority over the persisted config value and over any in-session edit. The options UI shows
	// an overridden option as a disabled control with the reason in its tooltip.
	struct OverrideEntry {
		std::unique_ptr<json_t> value;
		SCP_string reason; // human-readable source, e.g. "-no_vsync", shown in the options UI
		OverrideSource source;
	};
	SCP_unordered_map<SCP_string, OverrideEntry> _config_overrides;

	SCP_unordered_map<SCP_string, std::unique_ptr<json_t>> _changed_values;

	SCP_unordered_map<SCP_string, const OptionBase*> _optionsMapping;

	// Enforced options are hidden from the player and do not load values from user settings
	SCP_unordered_set<SCP_string> _enforcedOptions;

	SCP_vector<std::shared_ptr<const OptionBase>> _options;

	// what loadInitialValues() found: enforced and renderer-pruned options leave _options afterwards
	SCP_vector<std::shared_ptr<const OptionBase>> _describedAtLoad;

	// options the current renderer drops, for a launcher: what the player needs to use them, and a sentence saying so
	SCP_unordered_map<SCP_string, std::pair<SCP_string, SCP_string>> _requirements;

	// keyed by requirement ("vulkan", "raytracing"), as the windowless hardware query answered it
	SCP_unordered_map<SCP_string, RequirementSupport> _requirementSupport;
	bool _optionsSorted = false;

	SCP_map<int, PostPersistHook> _post_persist_hooks;
	int _next_post_persist_hook = 0;

	SCP_vector<const OptionBase*> runPostPersistHooks(const SCP_vector<const OptionBase*>& changed) const;

  public:
	static OptionsManager* instance();

	~OptionsManager();

	std::optional<std::unique_ptr<json_t>> getValueFromConfig(const SCP_string& key) const;

	// The value in the settings file only, ignoring overrides and pending edits; nullopt if the file has none
	std::optional<std::unique_ptr<json_t>> getSavedValue(const SCP_string& key) const;

	void setConfigValue(const SCP_string& key, std::unique_ptr<json_t>&& value);

	/**
	 * @brief Checks whether an option has an edit that no persist has written yet
	 *
	 * @param key The option's config key
	 * @return true if setConfigValue() stored a value for this key that is still pending
	 */
	bool hasPendingEdit(const SCP_string& key) const;

	void setOverride(const SCP_string& key,
		const SCP_string& json,
		const SCP_string& reason,
		OverrideSource source = OverrideSource::CommandLine);

	// Overload that takes an already-built JSON value. Prefer this one: it lets a caller reuse the option's
	// own serializer instead of formatting the JSON by hand, which can produce a value the option's
	// deserializer then rejects.
	void setOverride(const SCP_string& key,
		std::unique_ptr<json_t>&& value,
		const SCP_string& reason,
		OverrideSource source = OverrideSource::CommandLine);

	// Returns true if this key currently has an override.
	bool isOverridden(const SCP_string& key) const;

	// Returns the reason string passed to setOverride() for this key, if it is currently overridden.
	std::optional<SCP_string> getOverrideReason(const SCP_string& key) const;

	// Returns where this key's override came from, if it is currently overridden.
	std::optional<OverrideSource> getOverrideSource(const SCP_string& key) const;

	// Removes all overrides. For tests, so that one test's overrides do not leak into the next.
	void clearOverrides();

	const OptionBase* addOption(std::shared_ptr<const OptionBase>&& option);

	void removeOption(const std::shared_ptr<const OptionBase>& option);

	const OptionBase* getOptionByKey(SCP_string name);

	void enforceOption(const SCP_string& key);

	void unenforceOption(const SCP_string& key);

	bool isOptionEnforced(const SCP_string& key) const;

	const SCP_vector<std::shared_ptr<const options::OptionBase>>& getOptions();

	bool persistOptionChanges(const options::OptionBase* option);

	/**
	 * @brief Persists changes to the disk
	 *
	 * This writes all the outstanding changes to the config file and returns a vector of all the options that indicated
	 * that they could not change the value. If the return value is empty then all changes were applied but if it is not
	 * empty then a restart is required for all the options to take effect.
	 *
	 * @return A vector of options that did not support changing the value
	 */
	SCP_vector<const options::OptionBase*> persistChanges();

	/**
	 * @brief Adds a function that runs after every persist, once all changed options have been notified
	 *
	 * Lets several options that act on one thing apply together, whatever order their listeners ran in.
	 *
	 * @param hook Receives the options whose listener this persist notified, and returns the options whose new value
	 * it could not apply; those are reported like options whose listener returned false
	 * @return An id to pass to removePostPersistHook()
	 */
	int addPostPersistHook(PostPersistHook hook);

	/**
	 * @brief Removes a function added with addPostPersistHook()
	 *
	 * @note Must not be called from inside a hook while the hooks run; a hook cannot remove itself.
	 *
	 * @param id The id addPostPersistHook() returned
	 */
	void removePostPersistHook(int id);

	void discardChanges();

	void loadInitialValues();

	void printValues();

	/**
	 * @brief Describes every option for a launcher: key, title, description, category, type, allowed values or range,
	 * default, the value saved in the settings file (null if none), the value this launch uses (an enforced table
	 * value or an override wins over the saved one), override source and reason, whether a mod enforces it, whether
	 * a change needs a restart, and what the option needs (the Vulkan renderer, ray tracing) with whether this machine
	 * has it.
	 * @note The restart flag is approximate, for information only: it is false only for options bound straight to a
	 * variable, so an option whose own listener applies a change at once (HUD.BallisticGauge) still reports true.
	 * @return A new JSON array the caller owns
	 */
	json_t* describeOptions();

	// Notes what a player needs to use an option ("vulkan", "raytracing"), for describeOptions()
	void setRequirement(const SCP_string& key, const SCP_string& requirement, const SCP_string& explanation);

	// Records whether this machine meets a requirement, for describeOptions(); requirements never recorded are Unknown
	void setRequirementSupport(const SCP_string& requirement, RequirementSupport support);

	// Puts back what loadInitialValues() removed and forgets its record. For tests, which share this singleton.
	void forgetLoadedOptions();

	void set_ingame_binary_option(SCP_string key, bool value);
	void set_ingame_multi_option(SCP_string key, int value);
	void set_ingame_range_option(SCP_string key, int value);
	void set_ingame_range_option(SCP_string key, float value);
};

}

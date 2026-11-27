//
//

#include "option.h"
#include "enums.h"

#include "graphics/2d.h"

namespace scripting {
namespace api {

ADE_OBJ(l_ValueDescription, options::ValueDescription, "ValueDescription",
        "An option value that contains a displayable string and the serialized value.");

ADE_FUNC(__tostring, l_ValueDescription, nullptr, "Value display string", "string",
         "The display string or nil on error")
{
	options::ValueDescription* desc;
	if (!ade_get_args(L, "o", l_ValueDescription.GetPtr(&desc))) {
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "s", desc->display.c_str());
}
ADE_FUNC(__eq, l_ValueDescription, "ValueDescription other", "Compares two value descriptions", "string",
         "True if equal, false otherwise")
{
	options::ValueDescription* desc;
	options::ValueDescription* other;
	if (!ade_get_args(L, "oo", l_ValueDescription.GetPtr(&desc), l_ValueDescription.GetPtr(&other))) {
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "b", desc->serialized == other->serialized);
}
ADE_VIRTVAR(Display, l_ValueDescription, nullptr, "Value display string", "string",
            "The display string or nil on error")
{
	options::ValueDescription* desc;
	if (!ade_get_args(L, "o", l_ValueDescription.GetPtr(&desc))) {
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "s", desc->display.c_str());
}
ADE_VIRTVAR(Serialized, l_ValueDescription, nullptr, "Serialized string value of the contained value", "string",
            "The serialized string or nil on error")
{
	options::ValueDescription* desc;
	if (!ade_get_args(L, "o", l_ValueDescription.GetPtr(&desc))) {
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "s", desc->serialized.c_str());
}

option_h::option_h(const options::OptionBase* opt) : _opt(opt) {}
bool option_h::isValid() const { return _opt != nullptr; }
const options::OptionBase* option_h::get() { return _opt; }

//**********HANDLE: Object
ADE_OBJ(l_Option, option_h, "option", "Option handle");

ADE_VIRTVAR(Title, l_Option, nullptr, "The title of this option (read-only)", "string", "The title or nil on error")
{
	option_h* opt;
	if (!ade_get_args(L, "o", l_Option.GetPtr(&opt))) {
		return ADE_RETURN_NIL;
	}

	if (!opt->isValid()) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "s", opt->get()->getTitle().c_str());
}
ADE_VIRTVAR(Description, l_Option, nullptr, "The description of this option (read-only)", "string",
            "The description or nil on error")
{
	option_h* opt;
	if (!ade_get_args(L, "o", l_Option.GetPtr(&opt))) {
		return ADE_RETURN_NIL;
	}

	if (!opt->isValid()) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "s", opt->get()->getDescription().c_str());
}
ADE_VIRTVAR(Key, l_Option, nullptr, "The configuration key of this option. This will be a unique string. (read-only)",
            "string", "The key or nil on error")
{
	option_h* opt;
	if (!ade_get_args(L, "o", l_Option.GetPtr(&opt))) {
		return ADE_RETURN_NIL;
	}

	if (!opt->isValid()) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "s", opt->get()->getConfigKey().c_str());
}
ADE_VIRTVAR(Category, l_Option, nullptr, "The category of this option. (read-only)", "string",
            "The category or nil on error")
{
	option_h* opt;
	if (!ade_get_args(L, "o", l_Option.GetPtr(&opt))) {
		return ADE_RETURN_NIL;
	}

	if (!opt->isValid()) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "s", opt->get()->getCategory().c_str());
}
ADE_VIRTVAR(Type, l_Option, nullptr, "The type of this option. One of the OPTION_TYPE_* values. (read-only)",
            "enumeration", "An OPTION_TYPE_* enumeration, or nil on error")
{
	option_h* opt;
	if (!ade_get_args(L, "o", l_Option.GetPtr(&opt))) {
		return ADE_RETURN_NIL;
	}

	if (!opt->isValid()) {
		return ADE_RETURN_NIL;
	}

	lua_enum enum_val = ENUM_INVALID;
	switch (opt->get()->getType()) {
	case options::OptionType::Selection:
		enum_val = LE_OPTION_TYPE_SELECTION;
		break;
	case options::OptionType::Range:
		enum_val = LE_OPTION_TYPE_RANGE;
		break;
	}

	return ade_set_args(L, "o", l_Enum.Set(enum_h(enum_val)));
}
ADE_VIRTVAR(Value, l_Option, "ValueDescription", "The current value of this option.", "ValueDescription",
            "The current value or nil on error")
{
	option_h* opt;
	options::ValueDescription* new_val = nullptr;
	if (!ade_get_args(L, "o|o", l_Option.GetPtr(&opt), l_ValueDescription.GetPtr(&new_val))) {
		return ADE_RETURN_NIL;
	}
	if (!opt->isValid()) {
		return ADE_RETURN_NIL;
	}
	if (ADE_SETTING_VAR && new_val != nullptr) {
		opt->get()->setValueDescription(*new_val);
	}
	return ade_set_args(L, "o", l_ValueDescription.Set(opt->get()->getCurrentValueDescription()));
}
ADE_VIRTVAR(Flags,
	l_Option,
	nullptr,
	"Contains a list mapping a flag name to its value. Possible names are:"
	"<ul>"
	"<li><b>ForceMultiValueSelection:</b> If true, a selection option with two values should be displayed the "
	"same as an option with more possible values</li>"
	"<li><b>RetailBuiltinOption:</b> If true, the option is one of the original retail options</li>"
	"<li><b>RangeTypeInteger:</b> If true, this range option requires an integer for the range value rather than a float</li>"
	"</ul>",
	"{ string => boolean ... }",
	"The table of flags values.")
{
	option_h* opt;
	options::ValueDescription* new_val = nullptr;
	if (!ade_get_args(L, "o|o", l_Option.GetPtr(&opt), l_ValueDescription.GetPtr(&new_val))) {
		return ADE_RETURN_NIL;
	}
	if (!opt->isValid()) {
		return ADE_RETURN_NIL;
	}

	luacpp::LuaTable t = luacpp::LuaTable::create(L);

	t.addValue("ForceMultiValueSelection", opt->get()->getFlags()[options::OptionFlags::ForceMultiValueSelection]);
	t.addValue("RetailBuiltinOption", opt->get()->getFlags()[options::OptionFlags::RetailBuiltinOption]);
	t.addValue("RangeTypeInteger", opt->get()->getFlags()[options::OptionFlags::RangeTypeInteger]);

	return ade_set_args(L, "t", &t);
}
ADE_FUNC(getValueFromRange, l_Option, "number interpolant",
         "Gets a value from an option range. The specified value must be between 0 and 1.", "ValueDescription",
         "The value at the specifiedposition")
{
	option_h* opt;
	float f;
	if (!ade_get_args(L, "of", l_Option.GetPtr(&opt), &f)) {
		return ADE_RETURN_NIL;
	}
	if (!opt->isValid()) {
		return ADE_RETURN_NIL;
	}
	if (opt->get()->getType() != options::OptionType::Range) {
		LuaError(L, "This option is not a range option!");
		return ADE_RETURN_NIL;
	}
	if (f < 0.0f || f > 1.0f) {
		LuaError(L, "Invalid interpolant value %f!", f);
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "o", l_ValueDescription.Set(opt->get()->getValueFromRange(f)));
}
ADE_FUNC(getInterpolantFromValue, l_Option, "ValueDescription value",
         "From a value description of this option, determines the range value.", "number",
         "The range value or 0 on error.")
{
	option_h* opt;
	options::ValueDescription* value = nullptr;
	if (!ade_get_args(L, "oo", l_Option.GetPtr(&opt), l_ValueDescription.GetPtr(&value))) {
		return ADE_RETURN_NIL;
	}
	if (!opt->isValid()) {
		return ADE_RETURN_NIL;
	}
	if (opt->get()->getType() != options::OptionType::Range) {
		LuaError(L, "This option is not a range option!");
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "f", opt->get()->getInterpolantFromValue(*value));
}
ADE_FUNC(getValidValues,
	l_Option,
	nullptr,
	"Gets the valid values of this option. The order or the returned values must be maintained in the UI. This is "
	"only valid for selection or boolean options.",
	"ValueDescription[]",
	"A table containing the possible values or nil on error.")
{
	option_h* opt;
	if (!ade_get_args(L, "o", l_Option.GetPtr(&opt))) {
		return ADE_RETURN_NIL;
	}
	if (!opt->isValid()) {
		return ADE_RETURN_NIL;
	}
	if (opt->get()->getType() != options::OptionType::Selection) {
		LuaError(L, "This option is not a selection option!");
		return ADE_RETURN_NIL;
	}
	auto values = opt->get()->getValidValues();

	auto table = luacpp::LuaTable::create(L);
	auto i     = 1;
	for (auto& value : values) {
		table.addValue(i, l_ValueDescription.Set(value));
		++i;
	}

	return ade_set_args(L, "t", &table);
}
ADE_VIRTVAR(OverrideReason,
	l_Option,
	nullptr,
	"If this option's value is currently forced by something outside the options system, this is a human-readable "
	"description of what forces it: the flag for a command line override (e.g. \"-no_vsync\"), or \"an active mod\". "
	"See OverrideSource to tell the two apart. nil if the option is not currently overridden.",
	"string",
	"The override reason, or nil if not overridden or on error")
{
	option_h* opt;
	if (!ade_get_args(L, "o", l_Option.GetPtr(&opt))) {
		return ADE_RETURN_NIL;
	}
	if (!opt->isValid()) {
		return ADE_RETURN_NIL;
	}

	auto reason = options::OptionsManager::instance()->getOverrideReason(opt->get()->getConfigKey());
	if (!reason.has_value()) {
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "s", reason->c_str());
}
ADE_VIRTVAR(OverrideSource,
	l_Option,
	nullptr,
	"What is forcing this option's value: \"CommandLine\" for a command line flag, \"Mod\" for an active mod's tables. "
	"nil if the option is not currently overridden.",
	"string",
	"The override source, or nil if not overridden or on error")
{
	option_h* opt;
	if (!ade_get_args(L, "o", l_Option.GetPtr(&opt))) {
		return ADE_RETURN_NIL;
	}
	if (!opt->isValid()) {
		return ADE_RETURN_NIL;
	}

	auto source = options::OptionsManager::instance()->getOverrideSource(opt->get()->getConfigKey());
	if (!source.has_value()) {
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "s", (*source == options::OverrideSource::Mod) ? "Mod" : "CommandLine");
}

static std::optional<std::pair<SCP_string, SCP_string>> option_requirement(lua_State* L)
{
	option_h* opt;
	if (!ade_get_args(L, "o", l_Option.GetPtr(&opt)) || !opt->isValid()) {
		return std::nullopt;
	}
	return options::OptionsManager::instance()->getRequirement(opt->get()->getConfigKey());
}
ADE_VIRTVAR(Requirement,
	l_Option,
	nullptr,
	"What this option needs in order to work: \"opengl\", \"vulkan\" or \"raytracing\" (Vulkan with ray tracing "
	"hardware). Other values can appear in later versions, and they count as met. The value can be set also when the "
	"running renderer meets it, so use RequirementMet to decide whether to grey out the option. nil if the option has "
	"no requirement. (read-only)",
	"string",
	"The requirement, or nil if there is none or on error")
{
	auto requirement = option_requirement(L);
	if (!requirement.has_value()) {
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "s", requirement->first.c_str());
}
ADE_VIRTVAR(RequirementText,
	l_Option,
	nullptr,
	"A human-readable explanation of Requirement, for example \"Needs the Vulkan renderer\". nil if the option has "
	"no requirement. (read-only)",
	"string",
	"The explanation, or nil if there is none or on error")
{
	auto requirement = option_requirement(L);
	if (!requirement.has_value()) {
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "s", requirement->second.c_str());
}
ADE_VIRTVAR(RequirementMet,
	l_Option,
	nullptr,
	"Whether the running renderer and hardware meet Requirement. true if the option has no requirement. The in-game "
	"options dialog greys out an option when this is false, and shows RequirementText in its tooltip. (read-only)",
	"boolean",
	"true if the requirement is met or there is none, false if it is not met, nil on error")
{
	option_h* opt;
	if (!ade_get_args(L, "o", l_Option.GetPtr(&opt)) || !opt->isValid()) {
		return ADE_RETURN_NIL;
	}
	auto requirement = options::OptionsManager::instance()->getRequirement(opt->get()->getConfigKey());
	return ade_set_args(L, "b", !requirement.has_value() || gr_option_requirement_met_now(requirement->first));
}
ADE_FUNC(persistChanges, l_Option, nullptr,
         "Immediately persists any changes made to this specific option.", "boolean",
         "true if the change was applied successfully, false otherwise. nil on error.")
{
	option_h* opt;
	if (!ade_get_args(L, "o", l_Option.GetPtr(&opt))) {
		return ADE_RETURN_NIL;
	}
	if (!opt->isValid()) {
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "b", opt->get()->persistChanges());
}

} // namespace api
} // namespace scripting

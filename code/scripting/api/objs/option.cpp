//
//

#include "option.h"
#include "enums.h"

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
            "enumeration", "The enum or nil on error")
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

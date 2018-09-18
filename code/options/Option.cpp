//
//

#include "Option.h"
#include "OptionsManager.h"
#include "parse/parselo.h"
#include <utility>

namespace {


} // namespace

namespace options {

namespace internal {

json_t* string_serializer(const SCP_string& value) { return json_pack("s", value.c_str()); }
SCP_string string_deserializer(const json_t* value) {
	const char* s;

	json_error_t err;
	if (json_unpack_ex((json_t*)value, &err, 0, "s", &s) != 0) {
		throw json_exception(err);
	}

	return SCP_string(s);
}
template<>
void set_defaults<SCP_string>(Option<SCP_string>& opt) {
	opt.setDeserializer(string_deserializer);
	opt.setSerializer(string_serializer);
}

float float_deserializer(const json_t* value)
{
	double f;

	json_error_t err;
	if (json_unpack_ex((json_t*)value, &err, 0, "f", &f) != 0) {
		throw json_exception(err);
	}

	return (float)f;
}
json_t* float_serializer(float value) { return json_pack("f", value); }
SCP_string float_display(float value)
{
	SCP_string out;
	sprintf(out, "%.1f", value);
	return out;
}
template<>
void set_defaults<float>(Option<float>& opt) {
	opt.setDeserializer(float_deserializer);
	opt.setSerializer(float_serializer);
	opt.setDisplayFunc(float_display);

	opt.setType(OptionType::Range);
}

bool boolean_deserializer(const json_t* value)
{
	int b;

	json_error_t err;
	if (json_unpack_ex((json_t*)value, &err, 0, "b", &b) != 0) {
		throw json_exception(err);
	}

	return b != 0;
}
json_t* boolean_serializer(bool value) { return json_pack("b", value ? 1 : 0); }
SCP_string boolean_display(bool value) { return value ? "On" : "Off"; }
template<>
void set_defaults<bool>(Option<bool>& opt) {
	opt.setDeserializer(boolean_deserializer);
	opt.setSerializer(boolean_serializer);
	opt.setDisplayFunc(boolean_display);
	opt.setValueEnumerator(VectorEnumerator<bool>({false, true}));
}

} // namespace internal

ValueDescription::ValueDescription(SCP_string _display, SCP_string _serialized)
    : display(std::move(_display)), serialized(std::move(_serialized))
{
}

OptionBase::~OptionBase() = default;
OptionBase::OptionBase(SCP_string config_key, SCP_string title, SCP_string description)
    : _config_key(std::move(config_key)), _title(std::move(title)), _description(std::move(description))
{
	_parent = OptionsManager::instance();
}
std::unique_ptr<json_t> OptionBase::getConfigValue() const { return _parent->getValueFromConfig(_config_key); }

ExpertLevel OptionBase::getExpertLevel() const { return _expert_level; }
void OptionBase::setExpertLevel(ExpertLevel expert_level) { _expert_level = expert_level; }

void OptionBase::setPreset(PresetKind preset, const SCP_string& value) { _preset_values.emplace(preset, value); }

const SCP_string& OptionBase::getCategory() const { return _category; }
void OptionBase::setCategory(const SCP_string& category) { _category = category; }

const SCP_string& OptionBase::getConfigKey() const {
	return _config_key;
}
const SCP_string& OptionBase::getTitle() const {
	return _title;
}
const SCP_string& OptionBase::getDescription() const {
	return _description;
}
int OptionBase::getImportance() const {
	return _importance;
}
void OptionBase::setImportance(int importance) {
	_importance = importance;
}
bool operator<(const OptionBase& lhs, const OptionBase& rhs) {
	if (lhs._category < rhs._category)
		return true;
	if (rhs._category < lhs._category)
		return false;
	return lhs._importance > rhs._importance; // Importance is sorted from highest to lowest
}
bool operator>(const OptionBase& lhs, const OptionBase& rhs) {
	return rhs < lhs;
}
bool operator<=(const OptionBase& lhs, const OptionBase& rhs) {
	return !(rhs < lhs);
}
bool operator>=(const OptionBase& lhs, const OptionBase& rhs) {
	return !(lhs < rhs);
}

} // namespace options

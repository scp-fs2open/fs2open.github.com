#pragma once

#include "globalincs/flagset.h"
#include "globalincs/pstypes.h"
#include "libs/jansson.h"
#include "options/OptionsManager.h"

#include <functional>
#include <utility>

namespace options {

enum class OptionType { Range, Selection };

enum class ExpertLevel { Beginner, Advanced, Expert };

enum class PresetKind { Basic, Low, Medium, High, Ultra };

// clang-format off
FLAG_LIST(OptionFlags) {
	/**
	 * @brief Force the option menu to display this as a multi value selection.
	 *
	 * This can be useful if an option typically has multiple values but it is also possible for
	 * this option to have exactly two values which may cause the option menu to display it as a
	 * boolean option even though the option is not designed for that.
	 */
	ForceMultiValueSelection = 0,

	NUM_VALUES
};
// clang-format on

struct ValueDescription {
	const SCP_string display;
	const SCP_string serialized;

	ValueDescription(SCP_string _display, SCP_string _serialized);
};

class OptionBase {
  protected:
	OptionsManager* _parent   = nullptr;
	ExpertLevel _expert_level = ExpertLevel::Expert;

	SCP_string _config_key;

	SCP_string _category = "Other";

	SCP_string _title;
	SCP_string _description;

	int _importance = 0;

	SCP_unordered_map<PresetKind, SCP_string> _preset_values;

	flagset<OptionFlags> _flags;

	std::unique_ptr<json_t> getConfigValue() const;

	OptionBase(SCP_string config_key, SCP_string title, SCP_string description);

  public:
	virtual ~OptionBase() = 0;

	ExpertLevel getExpertLevel() const;
	void setExpertLevel(ExpertLevel expert_level);

	const SCP_string& getCategory() const;
	void setCategory(const SCP_string& category);

	int getImportance() const;
	void setImportance(int importance);

	const flagset<OptionFlags>& getFlags() const;
	void setFlags(const flagset<OptionFlags>& flags);

	const SCP_string& getConfigKey() const;
	const SCP_string& getTitle() const;
	const SCP_string& getDescription() const;

	void setPreset(PresetKind preset, const SCP_string& value);

	bool persistChanges() const;

	virtual bool valueChanged(const json_t* val) const = 0;

	virtual ValueDescription getCurrentValueDescription() const = 0;

	virtual void setValueDescription(const ValueDescription& desc) const = 0;

	virtual OptionType getType() const = 0;

	virtual SCP_vector<ValueDescription> getValidValues() const = 0;

	virtual ValueDescription getValueFromRange(float interpolant) const = 0;

	virtual float getInterpolantFromValue(const ValueDescription& val) const = 0;

	virtual void loadInitial() const = 0;

	friend bool operator<(const OptionBase& lhs, const OptionBase& rhs);
	friend bool operator>(const OptionBase& lhs, const OptionBase& rhs);
	friend bool operator<=(const OptionBase& lhs, const OptionBase& rhs);
	friend bool operator>=(const OptionBase& lhs, const OptionBase& rhs);
};

template <typename T>
using ValueEnumerator = std::function<SCP_vector<T>()>;
template <typename T>
using ValueDeserializer = std::function<T(const json_t*)>;
template <typename T>
using ValueSerializer = std::function<json_t*(const T&)>;
template <typename T>
using ValueDisplay = std::function<SCP_string(const T&)>;
template <typename T>
using DefaultValueFunctor = std::function<T()>;
template <typename T>
using ValueChangeListener = std::function<bool(const T&, bool initial)>;
template <typename T>
using ValueInterpolator = std::function<T(float)>;
template <typename T>
using ValueDeinterpolator = std::function<float(const T&)>;

template <typename T>
class ValueFunctor {
	T _value;

  public:
	ValueFunctor() = default;
	explicit ValueFunctor(T value) : _value(value) {}

	T operator()() { return _value; }
};

template <typename T>
class Option : public OptionBase {
  protected:
	ValueEnumerator<T> _valueEnumerator;
	ValueDeserializer<T> _deserializer;
	ValueSerializer<T> _serializer;
	ValueDisplay<T> _displayFunc;
	DefaultValueFunctor<T> _defaultValueFunc = ValueFunctor<T>();
	ValueChangeListener<T> _changeListener;
	ValueInterpolator<T> _interpolator;
	ValueDeinterpolator<T> _deinterpolator;

	OptionType _type = OptionType::Selection; // By default the generic type is always a selection

	ValueDescription toDescription(const T& val) const
	{
		auto json     = _serializer(val);
		auto json_str = json_dump_string_new(json, JSON_COMPACT | JSON_ENSURE_ASCII | JSON_ENCODE_ANY);
		if (_displayFunc) {
			return ValueDescription(_displayFunc(val), json_str);
		} else {
			return ValueDescription(json_str, json_str);
		}
	}
	std::unique_ptr<json_t> fromDescription(const ValueDescription& desc) const
	{
		json_error_t err;
		std::unique_ptr<json_t> el(json_loads(desc.serialized.c_str(), JSON_DECODE_ANY, &err));
		if (el == nullptr) {
			// Shouldn't really happen...
			throw json_exception(err);
		}
		return el;
	}

  public:
	Option(const SCP_string& config_key, const SCP_string& title, const SCP_string& description)
	    : OptionBase(config_key, title, description)
	{
	}
	~Option() override = default;

	OptionType getType() const override { return _type; }
	void setType(OptionType type) { _type = type; }

	SCP_vector<ValueDescription> getValidValues() const override
	{
		Assertion(_valueEnumerator, "Invalid value enumerator detected!");
		Assertion(_type == OptionType::Selection, "getValidValues may only be called on selection option!");

		auto values = _valueEnumerator();
		SCP_vector<ValueDescription> descriptions;
		descriptions.reserve(values.size());

		for (const auto& val : values) {
			descriptions.emplace_back(toDescription(val));
		}

		return descriptions;
	}

	ValueDescription getValueFromRange(float interpolant) const override
	{
		Assertion(_interpolator, "Need a valid interpolator!");
		Assertion(_type == OptionType::Range, "getValidValues may only be called on range option!");
		try {
			return toDescription(_interpolator(interpolant));
		} catch (const std::exception&) {
			return toDescription(_defaultValueFunc());
		}
	}
	float getInterpolantFromValue(const ValueDescription& val) const override
	{
		Assertion(_deinterpolator, "Need a valid deinterpolator!");
		Assertion(_type == OptionType::Range, "getInterpolantFromValue may only be called on range option!");
		try {
			auto el = fromDescription(val);
			return _deinterpolator(_deserializer(el.get()));
		} catch (const std::exception&) {
			return 0.0f;
		}
	}
	T getValue() const
	{
		try {
			return _deserializer(getConfigValue().get());
		} catch (const std::exception&) {
			// deserializers are allowed to throw on error
			return _defaultValueFunc();
		}
	}
	ValueDescription getCurrentValueDescription() const override
	{
		auto val = getValue();
		return toDescription(val);
	}
	void setValueDescription(const ValueDescription& desc) const override
	{
		std::unique_ptr<json_t> el;
		try {
			el = fromDescription(desc);
		} catch (const std::exception&) {
			// Functions are allowed to throw
		}
		if (el) {
			_parent->setConfigValue(_config_key, std::move(el));
		}
	}
	bool valueChanged(const json_t* val) const override
	{
		try {
			if (_changeListener) {
				return _changeListener(_deserializer(val), false);
			}
			return false; // Value not changed, requires restart
		} catch (const std::exception&) {
			// Functions are allowed to throw
			return false;
		}
	}

	void loadInitial() const override
	{
		if (!_changeListener) {
			// This requires a change listener
			return;
		}

		try {
			_changeListener(getValue(), true);
		} catch (const std::exception&) {
		}
	}

	const DefaultValueFunctor<T>& getDefaultValueFunc() const { return _defaultValueFunc; }
	void setDefaultValueFunc(const DefaultValueFunctor<T>& defaultValueFunc) { _defaultValueFunc = defaultValueFunc; }

	const ValueDeserializer<T>& getDeserializer() const { return _deserializer; }
	void setDeserializer(const ValueDeserializer<T>& converter) { _deserializer = converter; }

	const ValueSerializer<T>& getSerializer() const { return _serializer; }
	void setSerializer(const ValueSerializer<T>& serializer) { _serializer = serializer; }

	const ValueDisplay<T>& getDisplayFunc() const { return _displayFunc; }
	void setDisplayFunc(const ValueDisplay<T>& displayFunc) { _displayFunc = displayFunc; }

	const ValueEnumerator<T>& getValueEnumerator() const { return _valueEnumerator; }
	void setValueEnumerator(const ValueEnumerator<T>& valueEnumerator)
	{
		_valueEnumerator = valueEnumerator;
		_type            = OptionType::Selection;
	}

	const ValueChangeListener<T>& getChangeListener() const { return _changeListener; }
	void setChangeListener(const ValueChangeListener<T>& changeListener) { _changeListener = changeListener; }

	const ValueInterpolator<T>& getInterpolator() const { return _interpolator; }
	const ValueDeinterpolator<T>& getDeinterpolator() const { return _deinterpolator; }
	void setInterpolator(const ValueInterpolator<T>& interpolator, const ValueDeinterpolator<T>& deinterpolator)
	{
		_interpolator   = interpolator;
		_deinterpolator = deinterpolator;
		_type           = OptionType::Range;
	}
};

template <typename T>
class VectorEnumerator {
	SCP_vector<T> _values;

  public:
	explicit VectorEnumerator(SCP_vector<T> values) : _values(std::move(values)) {}

	SCP_vector<T> operator()() { return _values; }
};

template <typename T>
class MapValueDisplay {
	SCP_unordered_map<T, SCP_string> _mapping;

  public:
	explicit MapValueDisplay(SCP_unordered_map<T, SCP_string> mapping)
	    : _mapping(std::move(mapping))
	{
	}

	SCP_string operator()(const T& value) const
	{
		auto iter = _mapping.find(value);
		if (iter == _mapping.end()) {
			throw std::runtime_error("Display called with invalid value!");
		}
		return iter->second;
	}
};

namespace internal {
// This would be so much easier with Concepts...
template <typename T>
typename std::enable_if<!std::is_enum<T>::value, void>::type set_defaults(Option<T>& /*opt*/)
{
	// No defaults for the generic case since we don't know anything about this
}

template <>
void set_defaults<SCP_string>(Option<SCP_string>& opt);

template <>
void set_defaults<float>(Option<float>& opt);

template <>
void set_defaults<int>(Option<int>& opt);

template <>
void set_defaults<bool>(Option<bool>& opt);

template <typename T>
typename std::enable_if<std::is_enum<T>::value, void>::type set_defaults(Option<T>& opt)
{
	opt.setSerializer([](T val) { return json_pack("I", static_cast<json_int_t>(val)); });
	opt.setDeserializer([](const json_t* value) {
		json_int_t i;

		json_error_t err;
		if (json_unpack_ex((json_t*)value, &err, 0, "I", &i) != 0) {
			throw json_exception(err);
		}

		return static_cast<T>(i);
	});
}

template <typename T, size_t N>
void set_defaults(Option<flagset<T, N>>& opt)
{
	opt.setSerializer([](const flagset<T, N>& val) { return json_pack("I", static_cast<json_int_t>(val.to_u64())); });
	opt.setDeserializer([](const json_t* value) {
		json_int_t i;

		json_error_t err;
		if (json_unpack_ex((json_t*)value, &err, 0, "I", &i) != 0) {
			throw json_exception(err);
		}

		flagset<T, N> out;
		out.from_u64((std::uint64_t)i);
		return out;
	});
}

} // namespace internal

template <typename T>
class OptionBuilder {
	Option<T> _instance;

	SCP_unordered_map<PresetKind, T> _preset_values;

  public:
	OptionBuilder(const SCP_string& config_key, const SCP_string& title, const SCP_string& description)
	    : _instance(config_key, title, description)
	{
		internal::set_defaults(_instance);
	}

	OptionBuilder(const OptionBuilder&) = delete;
	OptionBuilder& operator=(const OptionBuilder&) = delete;

	OptionBuilder(OptionBuilder&&) noexcept = delete;
	OptionBuilder& operator=(OptionBuilder&&) noexcept = delete;
	//Set the category of the option
	OptionBuilder& category(const SCP_string& category)
	{
		_instance.setCategory(category);
		return *this;
	}
	//Set the level value of the option. Can be "Beginniner", "Advanced", or "Expert"
	OptionBuilder& level(ExpertLevel level)
	{
		_instance.setExpertLevel(level);
		return *this;
	}
	//Directly set the default value of an option
	OptionBuilder& default_val(const T& val)
	{
		_instance.setDefaultValueFunc(ValueFunctor<T>(val));
		return *this;
	}
	//Function to find the default value of the option
	OptionBuilder& default_func(const DefaultValueFunctor<T>& func)
	{
		_instance.setDefaultValueFunc(func);
		return *this;
	}
	//Deserialize an option for persisting changes
	OptionBuilder& deserializer(const ValueDeserializer<T>& converter)
	{
		_instance.setDeserializer(converter);
		return *this;
	}
	//Serialize an option value for display
	OptionBuilder& serializer(const ValueSerializer<T>& serializer)
	{
		_instance.setSerializer(serializer);
		return *this;
	}
	//Builds an enum list of values for the optoin
	OptionBuilder& enumerator(const ValueEnumerator<T>& enumerator)
	{
		_instance.setValueEnumerator(enumerator);
		return *this;
	}
	//The valid values for the option
	OptionBuilder& values(const SCP_vector<std::pair<T, SCP_string>>& value_display_pairs)
	{
		SCP_vector<T> values;
		SCP_unordered_map<T, SCP_string> display_mapping;

		for (auto& p : value_display_pairs) {
			values.push_back(p.first);
			display_mapping.emplace(p.first, p.second);
		}

		_instance.setValueEnumerator(VectorEnumerator<T>(values));
		_instance.setDisplayFunc(MapValueDisplay<T>(display_mapping));
		return *this;
	}
	//The string to display for the option values, usually a function that returns a string
	OptionBuilder& display(const ValueDisplay<T>& display)
	{
		_instance.setDisplayFunc(display);
		return *this;
	}
	//The code to run if the option value is changed
	OptionBuilder& change_listener(const ValueChangeListener<T>& listener)
	{
		_instance.setChangeListener(listener);
		return *this;
	}
	//The global variable to bind the option to immediately.
	OptionBuilder& bind_to(T* dest)
	{
		return change_listener([dest](const T& val, bool) {
			*dest = val;
			return true;
		});
	}
	//The global variable to bind the option to once. Will require game restart to persist changes.
	OptionBuilder& bind_to_once(T* dest)
	{
		return change_listener([dest](const T& val, bool initial) {
			if (initial) {
				*dest = val;
			}
			return initial;
		});
	}
	//Set the minimum and maximum range for a "slider" type option
	OptionBuilder& range(T min, T max)
	{
		Assertion(min <= max, "Invalid number range!");
		_instance.setInterpolator(
		    [min, max](float f) { return min + static_cast<T>((max - min) * f); },
		    [min, max](T f) { return static_cast<float>(f - min) / static_cast<float>(max - min); });
		return *this;
	}
	//Set the preset value for the option
	OptionBuilder& preset(PresetKind kind, const T& value)
	{
		_preset_values.emplace(kind, value);
		return *this;
	}
	//Set the option importance, used for sorting
	OptionBuilder& importance(int value)
	{
		_instance.setImportance(value);
		return *this;
	}
	//Set the option flags
	OptionBuilder& flags(const flagset<OptionFlags>& flags)
	{
		_instance.setFlags(flags);
		return *this;
	}
	//Finishes building the option and returns a pointer to it
	const Option<T>* finish()
	{
		for (auto& val : _preset_values) {
			_instance.setPreset(val.first, json_dump_string_new(_instance.getSerializer()(val.second),
			                                                    JSON_COMPACT | JSON_ENSURE_ASCII | JSON_ENCODE_ANY));
		}
		std::unique_ptr<Option<T>> opt_ptr(new Option<T>(_instance));
		auto ptr = opt_ptr.get(); // We need to get the pointer now since we loose the type information otherwise
		OptionsManager::instance()->addOption(std::unique_ptr<OptionBase>(opt_ptr.release()));
		return ptr;
	}
};

} // namespace options

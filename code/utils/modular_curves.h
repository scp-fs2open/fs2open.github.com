#pragma once

#include "utils/RandomRange.h"
#include "math/curve.h"

#include <optional>
#include <type_traits>
#include <utility>

template<typename>   constexpr bool is_optional = false;
template<typename T> constexpr bool is_optional<std::optional<T>> = true;
template<> 			 constexpr bool is_optional<std::nullopt_t> = true;

template<auto... grabbers>
struct modular_curves_submember_input {
  private:
	template<typename input_type, auto grabber>
	inline auto grab_part(const input_type& input) const {
		//Pointer to member type, i.e. for submember access
		if constexpr (std::is_member_object_pointer_v<decltype(grabber)>) {
			if constexpr (std::is_pointer_v<std::remove_reference_t<input_type>>)
				return std::cref(input->*grabber);
			else
				return std::cref(input.*grabber);
		}
		//Pointer to static variable, i.e. used to index into things.
		else if constexpr (std::is_pointer_v<decltype(grabber)>) {
			static_assert(std::is_integral_v<std::remove_cv_t<std::remove_reference_t<input_type>>>, "Can only index into array from an integral input");
			using indexing_type = std::decay_t<decltype((*grabber)[input])>;
			if (input >= 0)
				return std::optional<std::reference_wrapper<const indexing_type>>{ std::cref((*grabber)[input]) };
			else
				return std::optional<std::reference_wrapper<const indexing_type>>(std::nullopt);
		}
		//Integer, used to index into tuples. Should be rarely used by actual users, but is required to do child-types.
		else if constexpr (std::is_integral_v<decltype(grabber)>) {
			if constexpr (std::is_pointer_v<std::remove_reference_t<input_type>>)
				return std::cref(std::get<grabber>(*input));
			else
				return std::cref(std::get<grabber>(input));
		}
		else {
			static_assert(!std::is_same_v<input_type, input_type>, "Unknown grabber type in modular curves");
		}
	}

	template<typename input_type, auto grabber, auto... other_grabbers>
	inline auto grab_internal(std::reference_wrapper<const input_type> input) const {
		//If we're down to one grabber only, return it
		if constexpr (sizeof...(other_grabbers) == 0){
			return grab_part<input_type, grabber>(input.get());
		}
		//Otherwise, use the current grabber, and forward the result to the next grabber
		else {
			const auto& current_access = grab_part<input_type, grabber>(input.get());
			//If the current grabber isn't guaranteed to succeed, check for completion first
			if constexpr (is_optional<std::decay_t<decltype(current_access)>>) {
				using lower_return_type = decltype(grab_internal<std::decay_t<decltype((*current_access).get())>, other_grabbers...>(*current_access));
				using return_type = typename std::conditional_t<is_optional<std::decay_t<lower_return_type>>, lower_return_type, std::optional<lower_return_type>>;
				//If we're already nullopt (i.e. this array access failed) return early
				if (current_access.has_value()) {
					//Now, it's possible that lower acceses _also_ produce optionals. In this case, we need to forward the lower result, not re-wrap it.
					if constexpr (is_optional<std::decay_t<lower_return_type>>) {
						return grab_internal<std::decay_t<decltype((*current_access).get())>, other_grabbers...>(*current_access);
					}
					else{
						return return_type(grab_internal<std::decay_t<decltype((*current_access).get())>, other_grabbers...>(*current_access));
					}
				}
				else
				    return return_type(std::nullopt);
			}
			//Otherwise just send it on to the next grabber
			else {
				return grab_internal<std::decay_t<decltype(current_access.get())>, other_grabbers...>(current_access.get());
			}
		}
	}

  public:
	template<typename input_type>
	inline float grab(const input_type& input) const {
		const auto& result = grab_internal<input_type, grabbers...>(std::cref(input));
		if constexpr (is_optional<typename std::decay_t<decltype(result)>>) {
			if (result.has_value())
				return result->get();
			else
				return 1.0f;
		}
		else {
			return result.get();
		}
	}
};

template<typename grabber_fnc>
struct modular_curves_functional_input {
  private:
	grabber_fnc grabber;

  public:
	modular_curves_functional_input(grabber_fnc grabber_) : grabber(std::move(grabber_)) {}

	template<typename input_type>
	inline float grab(const input_type& input) const {
		if constexpr (std::is_pointer_v<std::remove_reference_t<input_type>>){
			return grabber(*input);
		}
		else {
			return grabber(input);
		}
	}
};

struct modular_curves_entry {
	int curve_idx = -1;
	//TODO update to parsed
	::util::UniformFloatRange scaling_factor = ::util::UniformFloatRange(1.f);
	::util::UniformFloatRange translation = ::util::UniformFloatRange(0.f);
	bool wraparound = true;
};

struct modular_curves_entry_instance {
	float scaling_factor;
	float translation;

};

/*
 * output_enum must be a contiguous enum with a NUM_VALUES end
 * */
template<typename input_type, typename output_enum, typename... input_grabbers>
struct modular_curves {
  private:
	SCP_unordered_map<SCP_string, output_enum> outputs;
	std::tuple<std::pair<const char*, input_grabbers>...> inputs;
	std::array<SCP_vector<std::pair<size_t, modular_curves_entry>>, static_cast<std::underlying_type_t<output_enum>>(output_enum::NUM_VALUES)> curves; //Output -> List<(Input, curve_entry)>

  public:
	modular_curves(SCP_unordered_map<SCP_string, output_enum> outputs_, std::tuple<std::pair<const char*, input_grabbers>...> inputs_) : outputs(std::move(outputs_)), inputs(std::move(inputs_)), curves() {}

	using instance_data = SCP_unordered_map<output_enum, SCP_unordered_map<size_t, modular_curves_entry_instance>>;
	instance_data create_instance() const {
		instance_data instance;
		for (const auto& [output, curve_pair] : curves) {
			auto& instance_output_map = instance[output];
			for (const auto& [input, curve_entry] : curve_pair) {
				instance_output_map.emplace(input, modular_curves_entry_instance{ curve_entry.scaling_factor.next(), curve_entry.translation.next() });
			}
		}
		return instance;
	}

  private:
	inline std::pair<float, float> get_instance_data(output_enum output, size_t input, const modular_curves_entry& curve_entry, const instance_data* instance) const {
		std::pair<float, float> data;
		bool has_data = false;
		if (instance != nullptr) {
			auto current_instance = instance->find(output);
			if (current_instance != instance->end()) {
				auto current_curve = current_instance->second.find(input);
				if (current_curve != current_instance->second.end()) {
					data.first = current_curve->second.scaling_factor;
					data.second = current_curve->second.translation;
					has_data = true;
				}
			}
		}
		if (!has_data) {
			data.first = curve_entry.scaling_factor.next();
			data.second = curve_entry.translation.next();
		}
		return data;
	}

	template<size_t... idx>
	inline float get_individual_input(size_t input_index, const input_type& input, std::index_sequence<idx...>) const {
		float result = 1.f;
		//GCC11+ and Clang will properly unroll this fold expression into a switch-case jumptable
		bool matched_case = ((idx == input_index ? (result = std::get<idx>(inputs).second.grab(input)), true : false) || ...);
		if (!matched_case) {
			UNREACHABLE("Modular Curves requested Input %zu which has no grabber!", input_index);
		}
		return result;
	}

  public:
	float get_output(output_enum output, const input_type& input, const instance_data* instance = nullptr) const {
		float result = 1.f;

		for (const auto& [input_idx, curve_entry] : curves[static_cast<std::underlying_type_t<output_enum>>(output)]){
			const auto& [scaling_factor, translation] = get_instance_data(output, input_idx, curve_entry, instance);
			const auto& curve = Curves[curve_entry.curve_idx];

			float input_value = (get_individual_input(input_idx, input, std::index_sequence_for<input_grabbers...>{}) / scaling_factor) + translation;

			if (curve_entry.wraparound) {
				float final_x = curve.keyframes.back().pos.x;
				input_value = std::fmod(input_value, final_x);
			}

			float output_value = curve.GetValue(input_value);
			if (output_value < 0.f) {
				output_value = 0.f;
			}

			result *= output_value;
		}

		return result;
	}
};

template<typename input_type, typename output_enum, typename... input_grabbers>
constexpr auto make_modular_curves(SCP_unordered_map<SCP_string, output_enum> outputs, std::pair<const char*, input_grabbers>... inputs) {
	return modular_curves<input_type, output_enum, input_grabbers...>(
		std::move(outputs), std::make_tuple(std::move(inputs)...)
	);
}

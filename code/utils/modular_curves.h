#pragma once

#include "utils/RandomRange.h"
#include "math/curve.h"
#include "parse/encrypt.h"

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
	::util::ParsedRandomFloatRange scaling_factor = ::util::UniformFloatRange(1.f);
	::util::ParsedRandomFloatRange translation = ::util::UniformFloatRange(0.f);
	bool wraparound = true;
};

struct modular_curves_entry_instance {
	int seed_scaling_factor;
	int seed_translation;
};

/*
 * output_enum must be a contiguous enum with a NUM_VALUES end
 * */
template<typename input_type, typename output_enum, typename... input_grabbers>
struct modular_curves {
  private:
	static constexpr size_t num_inputs = sizeof...(input_grabbers);
	static constexpr size_t num_outputs = static_cast<size_t>(output_enum::NUM_VALUES);

	SCP_unordered_map<SCP_string, output_enum, SCP_string_lcase_hash, SCP_string_lcase_equal_to> outputs;
	std::tuple<std::pair<const char*, input_grabbers>...> inputs;
	std::array<SCP_vector<std::pair<size_t, modular_curves_entry>>, num_outputs> curves; //Output -> List<(Input, curve_entry)>
  public:
	modular_curves(SCP_unordered_map<SCP_string, output_enum, SCP_string_lcase_hash, SCP_string_lcase_equal_to> outputs_, std::tuple<std::pair<const char*, input_grabbers>...> inputs_) : outputs(std::move(outputs_)), inputs(std::move(inputs_)), curves() {}

	modular_curves_entry_instance create_instance() const {
		return modular_curves_entry_instance{util::Random::next(), util::Random::next()};
	}

  private:
	inline std::pair<float, float> get_maybe_instanced_randoms(output_enum output, size_t input, const modular_curves_entry& curve_entry, const modular_curves_entry_instance* instance) const {
		if (instance != nullptr) {
			static constexpr std::array<std::array<uint32_t, num_outputs>, num_inputs> inout_seeds = []() constexpr {
				std::array<std::array<uint32_t, num_outputs>, num_inputs> temp{};
				for(size_t in = 0; in < num_inputs; in++) {
					for(size_t out = 0; out < num_outputs; out++) {
						//This isn't perfect, but absolutely sufficient to give each input-output combination different random values.
						temp[in][out] = hash_fnv1a(hash_fnv1a(in) ^ out);
					}
				}
				return temp;
			}();

			uint32_t seed = inout_seeds[input][static_cast<std::underlying_type_t<output_enum>>(output)] ^ curve_entry.curve_idx;

			curve_entry.scaling_factor.seed(seed ^ instance->seed_scaling_factor);
			curve_entry.translation.seed(seed ^ instance->seed_translation);

			//This will yield consistent seeds (and thus random values) for the same tuples of input_idx-output_idx-curve_idx-instance_seed.
			//if any of these four changes, the resulting value should be random with regard to the previous value.
			//Furthermore, this seed generation is not commutative, so input 0 and output 1 will result in a different seed to input 1 and output 0
		}

		return {curve_entry.scaling_factor.next(), curve_entry.translation.next()};
	}

	template<size_t... idx>
	inline size_t get_input_idx_by_name(const char* input, std::index_sequence<idx...>) const {
		size_t result = -1;
		bool matched_case = ((!stricmp(input, std::get<idx>(inputs).first) ? (result = idx), true : false) || ...);
		if (!matched_case) {
			error_display(1, "Unexpected modular curve input %s!", input);
		}
		return result;
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
	float get_output(output_enum output, const input_type& input, const modular_curves_entry_instance* instance = nullptr) const {
		float result = 1.f;

		for (const auto& [input_idx, curve_entry] : curves[static_cast<std::underlying_type_t<output_enum>>(output)]){
			const auto& [scaling_factor, translation] = get_maybe_instanced_randoms(output, input_idx, curve_entry, instance);
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

	void parse(SCP_string curve_type) {
		while(optional_string(curve_type.c_str())) {
			SCP_string input;
			required_string("+Input:");
			stuff_string(input, F_NAME);
			size_t input_idx = get_input_idx_by_name(input.c_str(), std::index_sequence_for<input_grabbers...>{});

			SCP_string output;
			required_string("+Output:");
			stuff_string(output, F_NAME);
			auto output_it = outputs.find(output);
			if (output_it == outputs.end()){
				error_display(1, "Unexpected modular curve output %s!", output.c_str());
			}
			output_enum output_idx = output_it->second;

			modular_curves_entry curve_entry;

			required_string("+Curve Name:");
			SCP_string curve;
			stuff_string(curve, F_NAME);
			curve_entry.curve_idx = curve_get_by_name(curve);
			if (curve_entry.curve_idx < 0){
				error_display(1, "Unknown curve %s requested for modular curves!", curve.c_str());
			}

			if (optional_string("+Random Scaling Factor:")) {
				curve_entry.scaling_factor = ::util::ParsedRandomFloatRange::parseRandomRange();
			} else {
				curve_entry.scaling_factor = ::util::UniformFloatRange(1.0f);
			}
			if (optional_string("+Random Translation:")) {
				curve_entry.translation = ::util::ParsedRandomFloatRange::parseRandomRange();
			} else {
				curve_entry.translation = ::util::UniformFloatRange(0.0f);
			}

			curve_entry.wraparound = true;
			parse_optional_bool_into("+Wraparound:", &curve_entry.wraparound);

			curves[static_cast<std::underlying_type_t<output_enum>>(output_idx)].emplace_back(input_idx, std::move(curve_entry));
		}
	}
};

template<typename input_type, typename output_enum, typename... input_grabbers>
constexpr auto make_modular_curves(SCP_unordered_map<SCP_string, output_enum, SCP_string_lcase_hash, SCP_string_lcase_equal_to> outputs, std::pair<const char*, input_grabbers>... inputs) {
	return modular_curves<input_type, output_enum, input_grabbers...>(
		std::move(outputs), std::make_tuple(std::move(inputs)...)
	);
}

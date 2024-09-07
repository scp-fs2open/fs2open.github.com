#pragma once

#include "utils/RandomRange.h"
#include "math/curve.h"
#include "parse/encrypt.h"
#include "globalincs/type_traits.h"

#include <optional>
#include <type_traits>
#include <utility>

//
// Modular Curve Input Grabbers
//
// These structs help at building accessors to potential inputs for modular curve definitions
// modular_curves_submember_input -> provides a fully inlineable and compiletime-defined way to access class members, members-of-members, and index into global arrays
// modular_curves_functional_input -> provides an interface for grabbing floats from arbitrary functions. Not as well optimizable as the above, and not declarable inline (until C++20)
//

template<auto... grabbers>
struct modular_curves_submember_input {
  private:
	template<typename input_type, auto grabber>
	static inline auto grab_part(const input_type& input) {
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
	static inline auto grab_internal(std::reference_wrapper<const input_type> input) {
		//If we're down to one grabber only, return it
		if constexpr (sizeof...(other_grabbers) == 0){
			return grab_part<input_type, grabber>(input.get());
		}
		//Otherwise, use the current grabber, and forward the result to the next grabber
		else {
			const auto& current_access = grab_part<input_type, grabber>(input.get());
			//If the current grabber isn't guaranteed to succeed, check for completion first
			if constexpr (is_optional_v<std::decay_t<decltype(current_access)>>) {
				using lower_return_type = decltype(grab_internal<std::decay_t<decltype((*current_access).get())>, other_grabbers...>(*current_access));
				using return_type = typename std::conditional_t<is_optional_v<std::decay_t<lower_return_type>>, lower_return_type, std::optional<lower_return_type>>;
				//If we're already nullopt (i.e. this array access failed) return early
				if (current_access.has_value()) {
					//Now, it's possible that lower acceses _also_ produce optionals. In this case, we need to forward the lower result, not re-wrap it.
					if constexpr (is_optional_v<std::decay_t<lower_return_type>>) {
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

	template<int tuple_idx, typename input_type>
	static inline auto grab_from_tuple(const input_type& input) {
		if constexpr(tuple_idx < 0)
			return std::cref(input);
		else
			return std::cref(std::get<tuple_idx>(input));
	}

  public:
	template<int tuple_idx, typename input_type>
	static inline float grab(const input_type& input) {
		const auto& result = grab_internal<std::decay_t<decltype(grab_from_tuple<tuple_idx, input_type>(input).get())>, grabbers...>(grab_from_tuple<tuple_idx, input_type>(input));
		if constexpr (is_optional_v<typename std::decay_t<decltype(result)>>) {
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

template<auto grabber_fnc>
struct modular_curves_functional_input {
  private:
	template<int tuple_idx, typename input_type>
	static inline auto grab_from_tuple(const input_type& input) {
		if constexpr(tuple_idx < 0)
			return std::cref(input);
		else
			return std::cref(std::get<tuple_idx>(input));
	}

  public:
	template<int tuple_idx, typename input_type>
	static inline float grab(const input_type& input) {
		if constexpr (std::is_pointer_v<std::remove_reference_t<input_type>>){
			return grabber_fnc(*grab_from_tuple<tuple_idx, input_type>(input));
		}
		else {
			return grabber_fnc(grab_from_tuple<tuple_idx, input_type>(input));
		}
	}
};

//
// Non-template modular curve helper structs, carrying per-curve data as well as data per-instance affected by modular curves.
//

// Do not instantiate this manually. The modular_curve_set struct will manage every use of this struct for you automatically.
struct modular_curves_entry {
	int curve_idx = -1;
	::util::ParsedRandomFloatRange scaling_factor = ::util::UniformFloatRange(1.f);
	::util::ParsedRandomFloatRange translation = ::util::UniformFloatRange(0.f);
	bool wraparound = true;
};

//
// This modular_curves_entry_instance contains data for any instances affected by modular curves.
// On the example of Weapon Curves, each weapon instance would have one modular_curves_entry_instance.
// Do not instance this struct manually, instead use modular_curves_set::create_instance()
//
struct modular_curves_entry_instance {
	int seed_scaling_factor;
	int seed_translation;
};

// Forward declaration of modular_curves_set, see explanation below
template<const auto& definitions, typename input_type, typename output_enum, typename input_tuple_index, typename... input_grabbers>
struct modular_curves_set;

//
// The modular_curves_definition contains the definition of one type of modular curves.
// I.e., something like "Weapon Curves" should have ONE global constexpr instance of a modular_curves_definition struct.
// It is in fact required that this instance is constexpr to be able to instance sets from it.
// Do not instance this by hand, but use the make_modular_curve_definition helper template function,
// or modular_curves_definition::derive_modular_curves_subset for derived defitions.
// output_enum must be a contiguous enum with a NUM_VALUES end
//
template<typename input_type, typename output_enum, size_t output_names, typename input_tuple_index, typename... input_grabbers>
struct modular_curves_definition {
  private:
	// Friends for instantiation and access from the set.
	template<typename, typename, size_t, typename, typename...>
	friend struct modular_curves_definition;

	template<const auto&, typename, typename, typename, typename...>
	friend struct modular_curves_set;

	template<const auto&>
	friend inline auto make_modular_curve_set();

	template<typename, typename output_enum_, size_t output_names_, typename... input_grabbers_>
	friend constexpr auto make_modular_curve_definition(std::array<std::pair<const char*, output_enum_>, output_names_> outputs, std::pair<const char*, input_grabbers_>... inputs);

	// Internal data
	using this_definition_type = modular_curves_definition<input_type, output_enum, output_names, input_tuple_index, input_grabbers...>;
	static constexpr size_t num_outputs = static_cast<size_t>(output_enum::NUM_VALUES);

	std::array<std::pair<const char*, output_enum>, output_names> outputs;
	std::tuple<std::pair<const char*, input_grabbers>...> inputs;

	// Constructors and creation helpers
	constexpr modular_curves_definition(std::array<std::pair<const char*, output_enum>, output_names> outputs_, std::tuple<std::pair<const char*, input_grabbers>...> inputs_) : outputs(std::move(outputs_)), inputs(std::move(inputs_)) {}

	template<const this_definition_type& curve_definition>
	static inline auto make_modular_curve_set() {
		return modular_curves_set<curve_definition, input_type, output_enum, input_tuple_index, input_grabbers...>{};
	}

	//Helper function to for parsing
	template<bool parsing, size_t... idx>
	inline size_t get_input_idx_by_name(const char* input, std::index_sequence<idx...>) const {
		size_t result = -1;
		bool matched_case = ((!stricmp(input, std::get<idx>(inputs).first) ? (result = idx), true : false) || ...);
		if (!matched_case) {
			if constexpr (parsing)
				error_display(1, "Unexpected modular curve input %s!", input);
			else
				UNREACHABLE("Unexpected modular curve input %s!", input);
		}
		return result;
	}

	// Parsing
	void parse(std::array<SCP_vector<std::pair<size_t, modular_curves_entry>>, num_outputs>& curves, const SCP_string& curve_type) const {
		while(optional_string(curve_type.c_str())) {
			SCP_string input;
			required_string("+Input:");
			stuff_string(input, F_NAME);
			size_t input_idx = get_input_idx_by_name<true>(input.c_str(), std::index_sequence_for<input_grabbers...>{});

			SCP_string output;
			required_string("+Output:");
			stuff_string(output, F_NAME);

			bool found_output = false;
			output_enum output_idx;
			for (const auto& output_pair : outputs){
				if (!stricmp(output_pair.first, output.c_str())){
					found_output = true;
					output_idx = output_pair.second;
					break;
				}
			}
			if (!found_output){
				error_display(1, "Unexpected modular curve output %s!", output.c_str());
			}

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

	void add_curve(std::array<SCP_vector<std::pair<size_t, modular_curves_entry>>, num_outputs>& curves, const SCP_string& input, output_enum output, modular_curves_entry curve_entry) const {
		size_t input_idx = get_input_idx_by_name<false>(input.c_str(), std::index_sequence_for<input_grabbers...>{});
		curves[static_cast<std::underlying_type_t<output_enum>>(output)].emplace_back(input_idx, std::move(curve_entry));
	}

	// Helper functions to compute the correct type for derived modular curved sets. Meant for unevaluated context (i.e. within a decltype) ONLY!
	template<typename maybe_tuple, typename... tuple_additions>
	static auto unevaluated_maybe_tuple_cat(const maybe_tuple& in, const tuple_additions&... adds) {
		if constexpr(is_tuple_v<maybe_tuple>)
			return std::tuple_cat(in, std::tuple<const tuple_additions&...>(adds...));
		else
			return std::tuple<const maybe_tuple&, const tuple_additions&...>(in, adds...);
	}

	template<size_t... idx>
	static auto unevaluated_tuple_of_input_idx_least_0(std::index_sequence<idx...>) {
		return std::tuple<std::integral_constant<int, std::tuple_element_t<idx, input_tuple_index>::value < 0 ? 0 : std::tuple_element_t<idx, input_tuple_index>::value>...>();
	}

	template<typename tuple_of_integrals, size_t... idx>
	static constexpr int find_lowest_tuple_integral_constant(std::index_sequence<idx...>) {
		//This explicitly "finds" 0 for a tuple of only -1's
		int result = 0;
		((result = (result < std::tuple_element_t<idx, tuple_of_integrals>::value ? std::tuple_element_t<idx, tuple_of_integrals>::value : result)), ...);
		return result;
	}

public:
	template<typename additional_input_type, typename new_output_enum, size_t new_output_size, typename... additional_input_grabbers>
	constexpr auto derive_modular_curves_subset(std::array<std::pair<const char*, new_output_enum>, new_output_size> new_outputs, std::pair<const char*, additional_input_grabbers>... additional_inputs) const {
		using new_input_type = decltype(unevaluated_maybe_tuple_cat(std::declval<input_type>(), std::declval<additional_input_type>()));
		using new_input_tuple_index = decltype(std::tuple_cat(
				//Old tuple accessors, but if they were -1 (i.e. no tuple) set them to 0 (i.e. first element)
				unevaluated_tuple_of_input_idx_least_0(std::index_sequence_for<input_grabbers...>()),
				//New tuple accessors, highest observed one + 1
				std::tuple<std::integral_constant<std::conditional_t<true, int, additional_input_grabbers>, find_lowest_tuple_integral_constant<input_tuple_index>(std::index_sequence_for<input_grabbers...>()) + 1>...>())); //This "seemingly unnecessary" conditional is required to be able to unpack the parameter pack over the additional_input_grabbers and get a tuple type of the identical length
		return modular_curves_definition<
				new_input_type,
				new_output_enum,
				new_output_size,
				new_input_tuple_index,
				input_grabbers..., additional_input_grabbers...>(
				std::move(new_outputs), std::tuple_cat(inputs, std::make_tuple(std::move(additional_inputs)...))
		);
	}
};

//
// A modular_curves_set contains all data that is required at runtime to store the parsed table.
// It itself contains no static parsing data, only a compiletime reference to the curve definitions (which have this data)
// On the example of weapon curves, one instance of a modular_curves_set should exist for every weapon class.
// Do not manually instance this struct, instead use the make_modular_curve_set<definition>() helper.
//
template<const auto& definitions, typename input_type, typename output_enum, typename input_tuple_index, typename... input_grabbers>
struct modular_curves_set {
private:
	// Friends to allow creation from curve definitions
	template<typename, typename, size_t, typename, typename...>
	friend struct modular_curves_definition;

	// Internal data
	static constexpr size_t num_inputs = sizeof...(input_grabbers);
	static constexpr size_t num_outputs = static_cast<size_t>(output_enum::NUM_VALUES);

	using input_grabber_tuple = std::tuple<input_grabbers...>;
	std::array<SCP_vector<std::pair<size_t, modular_curves_entry>>, num_outputs> curves; //Output -> List<(Input, curve_entry)>

	constexpr modular_curves_set() : curves() {}
public:
	// Used to create an instance for any single thing affected by modular curves. Note that having an instance is purely optional
	modular_curves_entry_instance create_instance() const {
		return modular_curves_entry_instance{util::Random::next(), util::Random::next()};
	}

private:
	// Internal methods for computing the curve result
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
	inline float get_individual_input(size_t input_index, const input_type& input, std::index_sequence<idx...>) const {
		float result = 1.f;
		//GCC11+ and Clang will properly unroll this fold expression into a switch-case jumptable
		bool matched_case = ((idx == input_index ? (result = std::tuple_element_t<idx, input_grabber_tuple>::template grab<std::tuple_element_t<idx, input_tuple_index>::value>(input)), true : false) || ...);
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

			result *= std::max(curve.GetValue(input_value), 0.0f);
		}

		return result;
	}

	inline void parse(const SCP_string& curve_type) {
		definitions.parse(curves, curve_type);
	}

	// Use add_curve to programmatically add a curve as if it had been tabled.
	inline void add_curve(const SCP_string& input, output_enum output, modular_curves_entry curve_entry) {
		definitions.add_curve(curves, input, output, std::move(curve_entry));
	}
};

//
// Helper functions to instance the modular curve structs. Helper functions are used because function template argument deduction is slightly more robust than CTAD, resulting in less verbose calls
//

template<typename input_type, typename output_enum, size_t output_names, typename... input_grabbers>
constexpr auto make_modular_curve_definition(std::array<std::pair<const char*, output_enum>, output_names> outputs, std::pair<const char*, input_grabbers>... inputs) {
	return modular_curves_definition<
			input_type,
			output_enum,
			output_names,
			std::tuple<std::integral_constant<std::conditional_t<true, int, input_grabbers>, -1>...>, //This "seemingly unnecessary" conditional is required to be able to unpack the parameter pack over the input_grabbers and get a tuple type of the identical length
			input_grabbers...>(
			std::move(outputs), std::make_tuple(std::move(inputs)...)
	);
}

template<const auto& curve_definition>
inline auto make_modular_curve_set() {
	using modular_curves_definition = std::decay_t<decltype(curve_definition)>;
	return modular_curves_definition::template make_modular_curve_set<curve_definition>();
}

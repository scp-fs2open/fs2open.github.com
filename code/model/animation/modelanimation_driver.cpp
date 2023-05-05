#include "modelanimation_driver.h"

#include "model/animation/modelanimation.h"

namespace animation {
	std::function<float(polymodel_instance*)> parse_generic_property_driver_source() {
		switch(optional_string_one_of(1,
				"Random"
				)){
			case 0: {
				float min, max;
				stuff_float(&min);
				stuff_float(&max);
				const int seed = util::Random::next();
				return [min, max, seed](polymodel_instance *pmi) -> float {
					//We want something that is as random as possible, however, if the same driver
					// is called from the same object, the same result should be produced.
					//But since driver objects are shared between animation sets (i.e. ship classes),
					// just assigning a random to each driver object is not sufficient.
					return static_cast<float>(std::mt19937(seed ^ pmi->objnum)()) / static_cast<float>(std::mt19937::max()) * (max - min) + min;
				};
			}
			default:
				error_display(0, "Unknown driver specifier encountered! Driver will be disabled!");
				return {};
		}
	}

	template<typename property, property object::* property_ptr, float property::* subproperty_ptr>
	static float get_object_subproperty_float(polymodel_instance* pmi){
		Assertion(pmi->objnum > 0, "Invalid object used in animation property driver!");
		return Objects[pmi->objnum].*property_ptr.*subproperty_ptr;
	}

	std::function<float(polymodel_instance*)> parse_object_property_driver_source() {
		switch(optional_string_one_of(5,
				"Speed",
				"SpeedForward",
				"Pitch",
				"Bank",
				"Heading"
				)){
			case 0:
				return get_object_subproperty_float<physics_info, &object::phys_info, &physics_info::speed>;
			case 1:
				return get_object_subproperty_float<physics_info, &object::phys_info, &physics_info::fspeed>;
			default:
				return {};
		}
	}

	std::function<float(polymodel_instance*)> parse_ship_property_driver_source() {
		switch(optional_string_one_of(1,
				""
				)){
			case 0:
				break;
			default:
				break;
		}
	}

	//std::function<void(ModelAnimation&, ModelAnimation::instance_data&, polymodel_instance*)> property_driver = parse_generic_property_driver();
}
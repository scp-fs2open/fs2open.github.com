#include "modelanimation_driver.h"

#include "hud/hudets.h"
#include "model/animation/modelanimation.h"
#include "playerman/player.h"
#include "ship/ship.h"

namespace animation {
	template<float control_info::* ci_property>
	static float get_player_ci_property(polymodel_instance* /*pmi*/){
		return Player->ci.*ci_property;
	}

	std::function<float(polymodel_instance*)> parse_generic_property_driver_source() {
		switch(optional_string_one_of(7,
				"Random",
				"ControlAccelForward",
				"ControlAccelLateral",
				"ControlAccelVertical",
				"ControlRotationPitch",
				"ControlRotationBank",
				"ControlRotationHeading"
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
			case 1:
				return get_player_ci_property<&control_info::forward>;
			case 2:
				return get_player_ci_property<&control_info::sideways>;
			case 3:
				return get_player_ci_property<&control_info::vertical>;
			case 4:
				return get_player_ci_property<&control_info::pitch>;
			case 5:
				return get_player_ci_property<&control_info::bank>;
			case 6:
				return get_player_ci_property<&control_info::heading>;
			default:
				return {};
		}
	}

	static int get_pmi_objnum(const polymodel_instance* pmi) {
		return pmi->objnum == model_objnum_special::OBJNUM_COCKPIT ? Player->objnum : pmi->objnum;
	}

	template<typename property, property object::* property_ptr, float property::* subproperty_ptr>
	static float get_object_subproperty_float(polymodel_instance* pmi){
		int objnum = get_pmi_objnum(pmi);
		Assertion(objnum >= 0, "Invalid object used in animation property driver!");
		return Objects[objnum].*property_ptr.*subproperty_ptr;
	}

	template<matrix object::* property_ptr, float angles::* angle>
	static float get_object_matrix_angle(polymodel_instance* pmi){
		int objnum = get_pmi_objnum(pmi);
		Assertion(objnum >= 0, "Invalid object used in animation property driver!");
		angles a;
		vm_extract_angles_matrix(&a, &(Objects[objnum].*property_ptr));
		return a.*angle;
	}

	std::function<float(polymodel_instance*)> parse_object_property_driver_source() {
		auto precedence = parse_generic_property_driver_source();
		if (precedence)
			return precedence;

		switch(optional_string_one_of(5,
				"SpeedForward",
				"Speed",
				"Pitch",
				"Bank",
				"Heading"
				)){
			case 0:
				return get_object_subproperty_float<physics_info, &object::phys_info, &physics_info::fspeed>;
			case 1:
				return get_object_subproperty_float<physics_info, &object::phys_info, &physics_info::speed>;
			case 2:
				return get_object_matrix_angle<&object::orient, &angles::p>;
			case 3:
				return get_object_matrix_angle<&object::orient, &angles::b>;
			case 4:
				return get_object_matrix_angle<&object::orient, &angles::h>;
			default:
				return {};
		}
	}

	template<typename property, property ship::* property_ptr, float property::* subproperty_ptr>
	static float get_ship_subproperty_float(polymodel_instance* pmi){
		int objnum = get_pmi_objnum(pmi);
		Assertion(objnum >= 0, "Invalid object used in animation property driver!");
		Assertion(Objects[objnum].type == OBJ_SHIP, "Non-ship object used in ship animation property driver!");
		return Ships[Objects[objnum].instance].*property_ptr.*subproperty_ptr;
	}

	template<int ship::* ets_property>
	static float get_ship_ets_property(polymodel_instance* pmi){
		int objnum = get_pmi_objnum(pmi);
		Assertion(objnum >= 0, "Invalid object used in animation property driver!");
		Assertion(Objects[objnum].type == OBJ_SHIP, "Non-ship object used in ship animation property driver!");
		return ets_power_factor(&Objects[objnum], false) * Energy_levels[Ships[Objects[objnum].instance].*ets_property];
	}

	std::function<float(polymodel_instance*)> parse_ship_property_driver_source() {
		auto precedence = parse_object_property_driver_source();
		if (precedence)
			return precedence;

		switch(optional_string_one_of(3,
				"ETSShield",
				"ETSEngine",
				"ETSWeapons"
				)){
			case 0:
				return get_ship_ets_property<&ship::shield_recharge_index>;
			case 1:
				return get_ship_ets_property<&ship::engine_recharge_index>;
			case 2:
				return get_ship_ets_property<&ship::weapon_recharge_index>;
			default:
				return {};
		}
	}

	ModelAnimationPropertyDriverTarget parse_property_driver_target() {
		switch(optional_string_one_of(2,
				"Speed",
				"Time" //Is dangerous as a property driver target, but makes sense as a startup driver
				)){
			case 0:
				return {&ModelAnimation::instance_data::speed, std::nullopt};
			case 1:
				return {&ModelAnimation::instance_data::time, &ModelAnimation::instance_data::duration};
			default:
				return {};
		}
	}
}
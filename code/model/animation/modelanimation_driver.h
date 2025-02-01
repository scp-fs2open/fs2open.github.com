#pragma once

#include "model/animation/modelanimation.h"

namespace animation {

	struct ModelAnimationPropertyDriverTarget {
		float ModelAnimation::instance_data::* target;
		std::optional<float ModelAnimation::instance_data::*> clamp;
	};

	//Drivers for any PMI
	std::function<float(polymodel_instance*)> parse_generic_property_driver_source();
	std::function<float(polymodel_instance*)> parse_object_property_driver_source();
	std::function<float(polymodel_instance*)> parse_ship_property_driver_source();

	ModelAnimationPropertyDriverTarget parse_property_driver_target();
}
#include "volumetric.h"

#include "graphics/2d.h"
#include "mission/missionparse.h"
#include "network/multimsgs.h"
#include "network/psnet2.h"
#include "scripting/api/objs/color.h"
#include "scripting/api/objs/vecmath.h"

namespace scripting::api {

ADE_OBJ(l_Volumetric, volumetric_h, "volumetric_nebula", "Mission volumetric nebula handle");

volumetric_h::volumetric_h(int idx) : index(idx) {}

bool volumetric_h::isValid() const
{
	return index == 1 && The_mission.volumetrics.has_value();
}

volumetric_nebula* volumetric_h::get() const
{
	if (!isValid()) {
		return nullptr;
	}

	return &(*The_mission.volumetrics);
}

void volumetric_h::serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& value, ubyte* data, int& packet_size) {
	volumetric_h event;
	value.getValue(l_Volumetric.Get(&event));
	ADD_INT(event.index);
}
void volumetric_h::deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset) { // NOLINT(readability-non-const-parameter)
	int index;
	GET_INT(index);
	new(data_ptr) volumetric_h(index);
}

ADE_FUNC(isValid,
	l_Volumetric,
	nullptr,
	"Determines if this handle is valid",
	"boolean",
	"true if valid, false otherwise")
{
	volumetric_h* vh = nullptr;
	if (!ade_get_args(L, "o", l_Volumetric.GetPtr(&vh))) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", vh != nullptr && vh->isValid());
}

ADE_VIRTVAR(POF,
	l_Volumetric,
	nullptr,
	"Read only hull POF file name.",
	"string",
	"The POF file name, or empty string if invalid")
{
	volumetric_h* vh = nullptr;
	if (!ade_get_args(L, "o", l_Volumetric.GetPtr(&vh)) || vh == nullptr || !vh->isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting the volumetric POF is not allowed!");
	}

	return ade_set_args(L, "s", vh->get()->getHullPof().c_str());
}

ADE_VIRTVAR(Position,
	l_Volumetric,
	"vector",
	"Volumetric nebula position.",
	"vector",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	vec3d* value = nullptr;
	if (!ade_get_args(L, "o|o", l_Volumetric.GetPtr(&vh), l_Vector.GetPtr(&value)) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR && value != nullptr) {
		vh->get()->set_runtime_params(*value);
	}

	return ade_set_args(L, "o", l_Vector.Set(vh->get()->getPos()));
}

// Deliberately not exposed but left here for posterity
/*ADE_VIRTVAR(EdgeSmoothing,
	l_Volumetric,
	"boolean",
	"Whether edge smoothing is enabled.",
	"boolean",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	bool value = false;
	if (!ade_get_args(L, "o|b", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		const auto smoothing_value = value ? std::max(vh->get()->getSmoothing(), 0.01f) : 0.0f;
		vh->get()->set_runtime_params(
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			smoothing_value);
	}

	return ade_set_args(L, "b", vh->get()->getSmoothing() > 0.0f);
}*/

ADE_VIRTVAR(Steps,
	l_Volumetric,
	"number",
	"Configured volumetric render steps.",
	"number",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	int value = 0;
	if (!ade_get_args(L, "o|i", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(std::nullopt, value);
	}

	return ade_set_args(L, "i", vh->get()->getConfiguredSteps());
}

ADE_VIRTVAR(SunSteps,
	l_Volumetric,
	"number",
	"Configured volumetric global light steps.",
	"number",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	int value = 0;
	if (!ade_get_args(L, "o|i", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(std::nullopt, std::nullopt, value);
	}

	return ade_set_args(L, "i", vh->get()->getConfiguredGlobalLightSteps());
}

// Not currently configurable during runtime but left here for posterity.
/*ADE_VIRTVAR(Resolution,
	l_Volumetric,
	"number",
	"Configured volumetric render resolution.",
	"number",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	int value = 0;
	if (!ade_get_args(L, "o|i", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			value);
	}

	return ade_set_args(L, "i", vh->get()->getResolution());
}*/

// Not currently configurable during runtime but left here for posterity.
/*ADE_VIRTVAR(ResolutionOversampling,
	l_Volumetric,
	"number",
	"Configured volumetric resolution oversampling.",
	"number",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	int value = 0;
	if (!ade_get_args(L, "o|i", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			value);
	}

	return ade_set_args(L, "i", vh->get()->getOversampling());
}*/

// Not currently configurable during runtime but left here for posterity.
/*ADE_VIRTVAR(Smoothing,
	l_Volumetric,
	"number",
	"Configured edge smoothing amount.",
	"number",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	float value = 0.0f;
	if (!ade_get_args(L, "o|f", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			value);
	}

	return ade_set_args(L, "f", vh->get()->getSmoothing());
}*/

ADE_VIRTVAR(VisibilityDistance,
	l_Volumetric,
	"number",
	"Opacity distance in meters.",
	"number",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	float value = 0.0f;
	if (!ade_get_args(L, "o|f", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(std::nullopt, std::nullopt, std::nullopt, value);
	}

	return ade_set_args(L, "f", vh->get()->getOpacityDistance());
}

ADE_VIRTVAR(VisibilityOpacity,
	l_Volumetric,
	"number",
	"Target opacity limit.",
	"number",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	float value = 0.0f;
	if (!ade_get_args(L, "o|f", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(std::nullopt, std::nullopt, std::nullopt, std::nullopt, value);
	}

	return ade_set_args(L, "f", vh->get()->getAlphaLim());
}

ADE_VIRTVAR(EmissiveSpread,
	l_Volumetric,
	"number",
	"Emissive spread factor.",
	"number",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	float value = 0.0f;
	if (!ade_get_args(L, "o|f", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, value);
	}

	return ade_set_args(L, "f", vh->get()->getEmissiveSpread());
}

ADE_VIRTVAR(EmissiveIntensity,
	l_Volumetric,
	"number",
	"Emissive intensity factor.",
	"number",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	float value = 0.0f;
	if (!ade_get_args(L, "o|f", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			value);
	}

	return ade_set_args(L, "f", vh->get()->getEmissiveIntensity());
}

ADE_VIRTVAR(EmissiveFalloff,
	l_Volumetric,
	"number",
	"Emissive falloff correction.",
	"number",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	float value = 0.0f;
	if (!ade_get_args(L, "o|f", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			value);
	}

	return ade_set_args(L, "f", vh->get()->getEmissiveFalloff());
}

ADE_VIRTVAR(HenyeyGreenstein,
	l_Volumetric,
	"number",
	"Henyey-Greenstein coefficient.",
	"number",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	float value = 0.0f;
	if (!ade_get_args(L, "o|f", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			value);
	}

	return ade_set_args(L, "f", vh->get()->getHenyeyGreensteinCoeff());
}

ADE_VIRTVAR(SunFalloffFactor,
	l_Volumetric,
	"number",
	"Global light distance factor.",
	"number",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	float value = 0.0f;
	if (!ade_get_args(L, "o|f", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			value);
	}

	return ade_set_args(L, "f", vh->get()->getGlobalLightDistanceFactor());
}

// Not currently configurable during runtime but left here for posterity.
/*ADE_VIRTVAR(NoiseActive,
	l_Volumetric,
	"boolean",
	"Whether edge noise is active.",
	"boolean",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	bool value = false;
	if (!ade_get_args(L, "o|b", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			value);
	}

	return ade_set_args(L, "b", vh->get()->getNoiseActive());
}*/

ADE_VIRTVAR(NoiseScaleBase, l_Volumetric, "number", "Base noise scale.", "number", "Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	float value = 0.0f;
	if (!ade_get_args(L, "o|f", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			value);
	}

	return ade_set_args(L, "f", std::get<0>(vh->get()->getNoiseColorScale()));
}

ADE_VIRTVAR(NoiseScaleSub, l_Volumetric, "number", "Sub noise scale.", "number", "Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	float value = 0.0f;
	if (!ade_get_args(L, "o|f", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			value);
	}

	return ade_set_args(L, "f", std::get<1>(vh->get()->getNoiseColorScale()));
}

ADE_VIRTVAR(NoiseIntensity,
	l_Volumetric,
	"number",
	"Noise color intensity.",
	"number",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	float value = 0.0f;
	if (!ade_get_args(L, "o|f", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			value);
	}

	return ade_set_args(L, "f", vh->get()->getNoiseColorIntensity());
}

// Not currently configurable during runtime but left here for posterity.
/*ADE_VIRTVAR(NoiseResolution,
	l_Volumetric,
	"number",
	"Configured noise texture resolution (2^n).",
	"number",
	"Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	int value = 0;
	if (!ade_get_args(L, "o|i", l_Volumetric.GetPtr(&vh), &value) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		vh->get()->set_runtime_params(
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			value);
	}

	return ade_set_args(L, "i", vh->get()->getNoiseResolution());
}*/

ADE_VIRTVAR(MainColor, l_Volumetric, "color", "Main color.", "color", "Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	color* value = nullptr;
	if (!ade_get_args(L, "o|o", l_Volumetric.GetPtr(&vh), l_Color.GetPtr(&value)) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR && value != nullptr) {
		vh->get()->set_runtime_params(
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			static_cast<float>(value->red) / 255.0f,
			static_cast<float>(value->green) / 255.0f,
			static_cast<float>(value->blue) / 255.0f);
	}

	color current;
	gr_init_color(&current,
		static_cast<int>(std::get<0>(vh->get()->getNebulaColor()) * 255.0f),
		static_cast<int>(std::get<1>(vh->get()->getNebulaColor()) * 255.0f),
		static_cast<int>(std::get<2>(vh->get()->getNebulaColor()) * 255.0f));
	return ade_set_args(L, "o", l_Color.Set(current));
}

ADE_VIRTVAR(NoiseColor, l_Volumetric, "color", "Noise color.", "color", "Current value, or nil if invalid")
{
	volumetric_h* vh = nullptr;
	color* value = nullptr;
	if (!ade_get_args(L, "o|o", l_Volumetric.GetPtr(&vh), l_Color.GetPtr(&value)) || vh == nullptr || !vh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR && value != nullptr) {
		vh->get()->set_runtime_params(
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			static_cast<float>(value->red) / 255.0f,
			static_cast<float>(value->green) / 255.0f,
			static_cast<float>(value->blue) / 255.0f);
	}

	color current;
	gr_init_color(&current,
		static_cast<int>(std::get<0>(vh->get()->getNoiseColor()) * 255.0f),
		static_cast<int>(std::get<1>(vh->get()->getNoiseColor()) * 255.0f),
		static_cast<int>(std::get<2>(vh->get()->getNoiseColor()) * 255.0f));
	return ade_set_args(L, "o", l_Color.Set(current));
}

} // namespace scripting::api
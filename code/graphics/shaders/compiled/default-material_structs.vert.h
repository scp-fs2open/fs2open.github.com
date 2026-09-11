
#pragma once

#include <cstdint>
#include <array>

struct genericData_default_material_v_sdr {
	SPIRV_FLOAT_MAT_4x4 modelMatrix;
	SPIRV_FLOAT_VEC4 color;
	SPIRV_FLOAT_VEC4 clipEquation;
	std::int32_t baseMapIndex;
	std::int32_t alphaTexture;
	std::int32_t noTexturing;
	std::int32_t srgb;
	float intensity;
	float alphaThreshold;
	std::uint32_t clipEnabled;
};
static_assert(sizeof(genericData_default_material_v_sdr) == 124, "Size of struct genericData_default_material_v_sdr does not match what is expected for the uniform block!");
static_assert(offsetof(genericData_default_material_v_sdr, modelMatrix) == 0, "Offset of member modelMatrix does not match the uniform buffer offset!");
static_assert(offsetof(genericData_default_material_v_sdr, color) == 64, "Offset of member color does not match the uniform buffer offset!");
static_assert(offsetof(genericData_default_material_v_sdr, clipEquation) == 80, "Offset of member clipEquation does not match the uniform buffer offset!");
static_assert(offsetof(genericData_default_material_v_sdr, baseMapIndex) == 96, "Offset of member baseMapIndex does not match the uniform buffer offset!");
static_assert(offsetof(genericData_default_material_v_sdr, alphaTexture) == 100, "Offset of member alphaTexture does not match the uniform buffer offset!");
static_assert(offsetof(genericData_default_material_v_sdr, noTexturing) == 104, "Offset of member noTexturing does not match the uniform buffer offset!");
static_assert(offsetof(genericData_default_material_v_sdr, srgb) == 108, "Offset of member srgb does not match the uniform buffer offset!");
static_assert(offsetof(genericData_default_material_v_sdr, intensity) == 112, "Offset of member intensity does not match the uniform buffer offset!");
static_assert(offsetof(genericData_default_material_v_sdr, alphaThreshold) == 116, "Offset of member alphaThreshold does not match the uniform buffer offset!");
static_assert(offsetof(genericData_default_material_v_sdr, clipEnabled) == 120, "Offset of member clipEnabled does not match the uniform buffer offset!");
struct matrixData_default_material_v_sdr {
	SPIRV_FLOAT_MAT_4x4 modelViewMatrix;
	SPIRV_FLOAT_MAT_4x4 projMatrix;
};
static_assert(sizeof(matrixData_default_material_v_sdr) == 128, "Size of struct matrixData_default_material_v_sdr does not match what is expected for the uniform block!");
static_assert(offsetof(matrixData_default_material_v_sdr, modelViewMatrix) == 0, "Offset of member modelViewMatrix does not match the uniform buffer offset!");
static_assert(offsetof(matrixData_default_material_v_sdr, projMatrix) == 64, "Offset of member projMatrix does not match the uniform buffer offset!");

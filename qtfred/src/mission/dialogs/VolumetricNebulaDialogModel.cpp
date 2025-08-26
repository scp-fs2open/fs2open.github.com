#include "mission/dialogs/VolumetricNebulaDialogModel.h"

namespace fso::fred::dialogs {

VolumetricNebulaDialogModel::VolumetricNebulaDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport),
	_bypass_errors(false)
{
	initializeData();
}

bool VolumetricNebulaDialogModel::apply()
{
	if (!VolumetricNebulaDialogModel::validate_data()) {
		return false;
	}
	if (!_volumetrics.enabled) {
		The_mission.volumetrics.reset();
	} else {
		if (!The_mission.volumetrics) {
			The_mission.volumetrics.emplace();
		}
		makeVolumetricsCopy(*The_mission.volumetrics, _volumetrics);
	}
	return true;
}

void VolumetricNebulaDialogModel::reject()
{
	//do nothing - only here because parent class reject() function is virtual
}

void VolumetricNebulaDialogModel::initializeData()
{
	if (The_mission.volumetrics) {
		// Copy authoring fields into our working copy
		makeVolumetricsCopy(_volumetrics, *The_mission.volumetrics);
	} else {
		// Start from engine defaults
		makeVolumetricsCopy(_volumetrics, volumetric_nebula{});
		_volumetrics.enabled = false;
	}
}

bool VolumetricNebulaDialogModel::validate_data()
{
	if (!_volumetrics.enabled) {
		return true;
	}
	else {
		// be helpful to the FREDer; try to advise precisely what the problem is
		// more general checks 1st, followed by more specific ones
		_bypass_errors = false;

		if (_volumetrics.hullPof.empty()) {
			showErrorDialogNoCancel("You must select a hull model for the volumetric nebula.");
			return false;
		}

	}

	return true;
}

void VolumetricNebulaDialogModel::showErrorDialogNoCancel(const SCP_string& message)
{
	if (_bypass_errors) {
		return;
	}

	_bypass_errors = true;
	_viewport->dialogProvider->showButtonDialog(DialogType::Error,
												"Error",
												message,
												{ DialogButton::Ok });
}

void VolumetricNebulaDialogModel::makeVolumetricsCopy(volumetric_nebula& dest, const volumetric_nebula& src)
{
	// Instance / placement / look
	dest.hullPof = src.hullPof;
	dest.pos = src.pos;
	dest.nebulaColor = src.nebulaColor;

	// Quality
	dest.doEdgeSmoothing = src.doEdgeSmoothing;
	dest.steps = src.steps;
	dest.globalLightSteps = src.globalLightSteps;
	dest.resolution = src.resolution;
	dest.oversampling = src.oversampling;
	dest.smoothing = src.smoothing;
	dest.noiseResolution = src.noiseResolution;

	// Visibility
	dest.opacityDistance = src.opacityDistance;
	dest.alphaLim = src.alphaLim;

	// Emissive
	dest.emissiveSpread = src.emissiveSpread;
	dest.emissiveIntensity = src.emissiveIntensity;
	dest.emissiveFalloff = src.emissiveFalloff;

	// Lighting (sun/global)
	dest.henyeyGreensteinCoeff = src.henyeyGreensteinCoeff;
	dest.globalLightDistanceFactor = src.globalLightDistanceFactor;

	// Noise authoring
	dest.noiseActive = src.noiseActive;
	dest.noiseScale = src.noiseScale;
	dest.noiseColorFunc1 = src.noiseColorFunc1;
	dest.noiseColorFunc2 = src.noiseColorFunc2;
	dest.noiseColor = src.noiseColor;
	dest.noiseColorIntensity = src.noiseColorIntensity;

	// Enabled flag
	dest.enabled = src.enabled;
}

bool VolumetricNebulaDialogModel::getEnabled() const
{
	return _volumetrics.enabled;
}

void VolumetricNebulaDialogModel::setEnabled(bool e)
{
	modify(_volumetrics.enabled, e);
}

const SCP_string& VolumetricNebulaDialogModel::getHullPof() const
{
	return _volumetrics.hullPof;
}

void VolumetricNebulaDialogModel::setHullPof(const SCP_string& pofPath)
{
	modify(_volumetrics.hullPof, pofPath);
}

float VolumetricNebulaDialogModel::getPosX() const
{
	return _volumetrics.pos.xyz.x;
}

void VolumetricNebulaDialogModel::setPosX(float x)
{
	modify(_volumetrics.pos.xyz.x, x);
}

float VolumetricNebulaDialogModel::getPosY() const
{
	return _volumetrics.pos.xyz.y;
}

void VolumetricNebulaDialogModel::setPosY(float y)
{
	modify(_volumetrics.pos.xyz.y, y);
}

float VolumetricNebulaDialogModel::getPosZ() const
{
	return _volumetrics.pos.xyz.z;
}

void VolumetricNebulaDialogModel::setPosZ(float z)
{
	modify(_volumetrics.pos.xyz.z, z);
}

int VolumetricNebulaDialogModel::getColorR() const
{
	const auto& c = _volumetrics.nebulaColor;
	const int r = static_cast<int>(std::get<0>(c) * 255.0f + 0.5f);
	return std::clamp(r, 0, 255);
}

void VolumetricNebulaDialogModel::setColorR(int r)
{
	CLAMP(r, 0 , 255);
	auto t = _volumetrics.nebulaColor;
	std::get<0>(t) = r / 255.0f;
	modify(_volumetrics.nebulaColor, t);
}

int VolumetricNebulaDialogModel::getColorG() const
{
	const auto& c = _volumetrics.nebulaColor;
	const int g = static_cast<int>(std::get<1>(c) * 255.0f + 0.5f);
	return std::clamp(g, 0, 255);
}

void VolumetricNebulaDialogModel::setColorG(int g)
{
	CLAMP(g, 0, 255);
	auto t = _volumetrics.nebulaColor;
	std::get<1>(t) = g / 255.0f;
	modify(_volumetrics.nebulaColor, t);
}

int VolumetricNebulaDialogModel::getColorB() const
{
	const auto& c = _volumetrics.nebulaColor;
	const int b = static_cast<int>(std::get<2>(c) * 255.0f + 0.5f);
	return std::clamp(b, 0, 255);
}

void VolumetricNebulaDialogModel::setColorB(int b)
{
	CLAMP(b, 0, 255);
	auto t = _volumetrics.nebulaColor;
	std::get<2>(t) = b / 255.0f;
	modify(_volumetrics.nebulaColor, t);
}

float VolumetricNebulaDialogModel::getOpacity() const
{
	return _volumetrics.alphaLim;
}

void VolumetricNebulaDialogModel::setOpacity(float v)
{
	CLAMP(v, getOpacityLimit().first, getOpacityLimit().second);
	modify(_volumetrics.alphaLim, v);
}

float VolumetricNebulaDialogModel::getOpacityDistance() const
{
	return _volumetrics.opacityDistance;
}

void VolumetricNebulaDialogModel::setOpacityDistance(float v)
{
	CLAMP(v, getOpacityDistanceLimit().first, getOpacityDistanceLimit().second);
	modify(_volumetrics.opacityDistance, v);
}

int VolumetricNebulaDialogModel::getSteps() const
{
	return _volumetrics.steps;
}

void VolumetricNebulaDialogModel::setSteps(int v)
{
	CLAMP(v, getStepsLimit().first, getStepsLimit().second);
	modify(_volumetrics.steps, v);
}

int VolumetricNebulaDialogModel::getResolution() const
{
	return _volumetrics.resolution;
}

void VolumetricNebulaDialogModel::setResolution(int v)
{
	CLAMP(v, getResolutionLimit().first, getResolutionLimit().second);
	modify(_volumetrics.resolution, v);
}

int VolumetricNebulaDialogModel::getOversampling() const
{
	return _volumetrics.oversampling;
}

void VolumetricNebulaDialogModel::setOversampling(int v)
{
	CLAMP(v, getOversamplingLimit().first, getOversamplingLimit().second);
	modify(_volumetrics.oversampling, v);
}

float VolumetricNebulaDialogModel::getSmoothing() const
{
	return _volumetrics.smoothing;
}

void VolumetricNebulaDialogModel::setSmoothing(float v)
{
	CLAMP(v, getSmoothingLimit().first, getSmoothingLimit().second);
	modify(_volumetrics.smoothing, v);
}

float VolumetricNebulaDialogModel::getHenyeyGreenstein() const
{
	return _volumetrics.henyeyGreensteinCoeff;
}

void VolumetricNebulaDialogModel::setHenyeyGreenstein(float v)
{
	CLAMP(v, getHenyeyGreensteinLimit().first, getHenyeyGreensteinLimit().second);
	modify(_volumetrics.henyeyGreensteinCoeff, v);
}

float VolumetricNebulaDialogModel::getSunFalloffFactor() const
{
	return _volumetrics.globalLightDistanceFactor;
}

void VolumetricNebulaDialogModel::setSunFalloffFactor(float v)
{
	CLAMP(v, getSunFalloffFactorLimit().first, getSunFalloffFactorLimit().second);
	modify(_volumetrics.globalLightDistanceFactor, v);
}

int VolumetricNebulaDialogModel::getSunSteps() const
{
	return _volumetrics.globalLightSteps;
}

void VolumetricNebulaDialogModel::setSunSteps(int v)
{
	CLAMP(v, getSunStepsLimit().first, getSunStepsLimit().second);
	modify(_volumetrics.globalLightSteps, v);
}

float VolumetricNebulaDialogModel::getEmissiveSpread() const
{
	return _volumetrics.emissiveSpread;
}

void VolumetricNebulaDialogModel::setEmissiveSpread(float v)
{
	CLAMP(v, getEmissiveSpreadLimit().first, getEmissiveSpreadLimit().second);
	modify(_volumetrics.emissiveSpread, v);
}

float VolumetricNebulaDialogModel::getEmissiveIntensity() const
{
	return _volumetrics.emissiveIntensity;
}

void VolumetricNebulaDialogModel::setEmissiveIntensity(float v)
{
	CLAMP(v, getEmissiveIntensityLimit().first, getEmissiveIntensityLimit().second);
	modify(_volumetrics.emissiveIntensity, v);
}

float VolumetricNebulaDialogModel::getEmissiveFalloff() const
{
	return _volumetrics.emissiveFalloff;
}

void VolumetricNebulaDialogModel::setEmissiveFalloff(float v)
{
	CLAMP(v, getEmissiveFalloffLimit().first, getEmissiveFalloffLimit().second);
	modify(_volumetrics.emissiveFalloff, v);
}

bool VolumetricNebulaDialogModel::getNoiseEnabled() const
{
	return _volumetrics.noiseActive;
}

void VolumetricNebulaDialogModel::setNoiseEnabled(bool on)
{
	modify(_volumetrics.noiseActive, on);
}

int VolumetricNebulaDialogModel::getNoiseColorR() const
{
	const auto& c = _volumetrics.noiseColor;
	const int r = static_cast<int>(std::get<0>(c) * 255.0f + 0.5f);
	return std::clamp(r, 0, 255);
}
void VolumetricNebulaDialogModel::setNoiseColorR(int r)
{
	CLAMP(r, 0, 255);
	auto t = _volumetrics.noiseColor;
	std::get<0>(t) = r / 255.0f;
	modify(_volumetrics.noiseColor, t);
}

int VolumetricNebulaDialogModel::getNoiseColorG() const
{
	const auto& c = _volumetrics.noiseColor;
	const int g = static_cast<int>(std::get<1>(c) * 255.0f + 0.5f);
	return std::clamp(g, 0, 255);
}
void VolumetricNebulaDialogModel::setNoiseColorG(int g)
{
	CLAMP(g, 0, 255);
	auto t = _volumetrics.noiseColor;
	std::get<1>(t) = g / 255.0f;
	modify(_volumetrics.noiseColor, t);
}

int VolumetricNebulaDialogModel::getNoiseColorB() const
{
	const auto& c = _volumetrics.noiseColor;
	const int b = static_cast<int>(std::get<2>(c) * 255.0f + 0.5f);
	return std::clamp(b, 0, 255);
}
void VolumetricNebulaDialogModel::setNoiseColorB(int b)
{
	CLAMP(b, 0, 255);
	auto t = _volumetrics.noiseColor;
	std::get<2>(t) = b / 255.0f;
	modify(_volumetrics.noiseColor, t);
}

float VolumetricNebulaDialogModel::getNoiseScaleBase() const
{
	return std::get<0>(_volumetrics.noiseScale);
}

void VolumetricNebulaDialogModel::setNoiseScaleBase(float v)
{
	CLAMP(v, getNoiseScaleBaseLimit().first, getNoiseScaleBaseLimit().second);
	auto t = _volumetrics.noiseScale;
	std::get<0>(t) = v;
	modify(_volumetrics.noiseScale, t);
}

float VolumetricNebulaDialogModel::getNoiseScaleSub() const
{
	return std::get<1>(_volumetrics.noiseScale);
}

void VolumetricNebulaDialogModel::setNoiseScaleSub(float v)
{
	CLAMP(v, getNoiseScaleSubLimit().first, getNoiseScaleSubLimit().second);
	auto t = _volumetrics.noiseScale;
	std::get<1>(t) = v;
	modify(_volumetrics.noiseScale, t);
}

float VolumetricNebulaDialogModel::getNoiseIntensity() const
{
	return _volumetrics.noiseColorIntensity;
}

void VolumetricNebulaDialogModel::setNoiseIntensity(float v)
{
	CLAMP(v, getNoiseIntensityLimit().first, getNoiseIntensityLimit().second);
	modify(_volumetrics.noiseColorIntensity, v);
}

int VolumetricNebulaDialogModel::getNoiseResolution() const
{
	return _volumetrics.noiseResolution;
}

void VolumetricNebulaDialogModel::setNoiseResolution(int v)
{
	CLAMP(v, getNoiseResolutionLimit().first, getNoiseResolutionLimit().second);
	modify(_volumetrics.noiseResolution, v);
}

} // namespace fso::fred::dialogs
#include "SoundEnvironmentDialogModel.h"

namespace fso::fred::dialogs {

SoundEnvironmentDialogModel::SoundEnvironmentDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
}

bool SoundEnvironmentDialogModel::apply()
{
	// No direct application; this model is used to collect custom strings
	// and the actual application is handled by the MissionSpecDialogModel.
	return true;
}

void SoundEnvironmentDialogModel::reject()
{
	// No direct rejection; this model is used to collect custom strings
	// and the actual rejection is handled by the MissionSpecDialogModel.
}

void SoundEnvironmentDialogModel::setInitial(const sound_env& env)
{
	_working = env;
}

sound_env SoundEnvironmentDialogModel::params() const
{
	return _working;
}

bool SoundEnvironmentDialogModel::validateVolume(float vol, SCP_string* errorOut)
{
	if (vol < 0.0f || vol > 1.0f) {
		if (errorOut)
			*errorOut = "Volume must be between 0.0 and 1.0.";
		return false;
	}
	return true;
}

bool SoundEnvironmentDialogModel::validateDamping(float d, SCP_string* errorOut)
{
	if (d < 0.0f || d > 1.0f) {
		if (errorOut)
			*errorOut = "Damping must be between 0.0 and 1.0.";
		return false;
	}
	return true;
}

bool SoundEnvironmentDialogModel::validateDecay(float decay, SCP_string* errorOut)
{
	if (decay <= 0.0f) {
		if (errorOut)
			*errorOut = "Decay must be greater than 0.";
		return false;
	}
	return true;
}

bool SoundEnvironmentDialogModel::setId(int id, SCP_string* errorOut)
{
	if (id < -1 || id >= static_cast<int>(EFX_presets.size())) {
		if (errorOut)
			*errorOut = "Invalid environment ID.";
		return false;
	}
	
	if (_working.id == id)
		return true;

	modify(_working.id, id);

	if (id == -1) {
		// No environment selected; clear fields to defaults
		modify(_working.volume, 0.0f);
		modify(_working.damping, 0.1f);
		modify(_working.decay, 0.1f);
		return true;
	}

	_working.volume = EFX_presets[id].flGain;
	_working.damping = EFX_presets[id].flDecayHFRatio;
	_working.decay = EFX_presets[id].flDecayTime;
	return true;
}

int SoundEnvironmentDialogModel::getId() const
{
	return _working.id;
}

bool SoundEnvironmentDialogModel::setVolume(float vol, SCP_string* errorOut)
{
	if (!validateVolume(vol, errorOut))
		return false;
	if (_working.volume == vol)
		return true;

	modify(_working.volume, vol);
	return true;
}

bool SoundEnvironmentDialogModel::setDamping(float d, SCP_string* errorOut)
{
	if (!validateDamping(d, errorOut))
		return false;
	if (_working.damping == d)
		return true;
	
	modify(_working.damping, d);
	return true;
}

bool SoundEnvironmentDialogModel::setDecay(float decay, SCP_string* errorOut)
{
	if (!validateDecay(decay, errorOut))
		return false;
	if (_working.decay == decay)
		return true;
	
	modify(_working.decay, decay);
	return true;
}

} // namespace fso::fred::dialogs

#pragma once

#include "globalincs/pstypes.h" // for SCP_string
#include "sound/sound.h"

#include "mission/dialogs/AbstractDialogModel.h"

namespace fso::fred::dialogs {

class SoundEnvironmentDialogModel final : public AbstractDialogModel {
  public:
	SoundEnvironmentDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	void setInitial(const sound_env& env);
	sound_env params() const;

	bool setId(int id, SCP_string* errorOut = nullptr);
	int getId() const;
	bool setVolume(float vol, SCP_string* errorOut = nullptr);
	bool setDamping(float damping, SCP_string* errorOut = nullptr);
	bool setDecay(float decay, SCP_string* errorOut = nullptr);

  private:
	sound_env _working = {};

	static bool validateVolume(float vol, SCP_string* errorOut);
	static bool validateDamping(float d, SCP_string* errorOut);
	static bool validateDecay(float decay, SCP_string* errorOut);
};

} // namespace fso::fred::dialogs

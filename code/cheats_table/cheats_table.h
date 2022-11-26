#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "io/key.h"
#include "hud/hudmessage.h"
#include "hud/hudsquadmsg.h"
#include "scripting/global_hooks.h"
#include "scripting/hook_api.h"
#include "ship/ship.h"
#include "ship/shipfx.h"

#define CHEAT_BUFFER_LEN	17

class CustomCheat {
	public:
		SCP_string cheatCode;
		SCP_string cheatMsg;
		bool requireCheatsEnabled;

		CustomCheat(SCP_string cheat_code, SCP_string cheat_msg, bool require_cheats_enabled) : 
			cheatCode(std::move(cheat_code)),
			cheatMsg(std::move(cheat_msg)),
			requireCheatsEnabled(require_cheats_enabled) { }

		virtual ~CustomCheat() = default;

		virtual void runCheat();

		bool canUseCheat();
};

class SpawnShipCheat : public CustomCheat {
	protected:
	SCP_string shipClassName;
	SCP_string shipName;
	public:

	SpawnShipCheat(SCP_string cheat_code, SCP_string cheat_msg,  bool require_cheats_enabled, SCP_string class_name, SCP_string ship_name) : CustomCheat(cheat_code, cheat_msg, require_cheats_enabled),
		shipClassName(std::move(class_name)),
		shipName(std::move(ship_name)) { }

	void runCheat() override;
};

static SCP_map<SCP_string, std::unique_ptr<CustomCheat>> customCheats;

void cheat_table_init();
void parse_cheat_table(const char* filename);
bool checkForCustomCheats(char converted_buffer[], int buffer_length);
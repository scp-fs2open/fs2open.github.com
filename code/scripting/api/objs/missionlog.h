#pragma once

#include "mission/missionlog.h"
#include "hud/hudmessage.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct log_entry_h {
	int section;
	log_entry_h();
	explicit log_entry_h(int l_section);
	const log_line_complete* getSection() const;
	bool isValid() const;
};

struct message_entry_h {
	int section;
	message_entry_h();
	explicit message_entry_h(int l_section);
	const line_node* getSection() const;
	bool isValid() const;
};

DECLARE_ADE_OBJ(l_Log_Entry, log_entry_h);
DECLARE_ADE_OBJ(l_Message_Entry, message_entry_h);

} // namespace api
} // namespace scripting
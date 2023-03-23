#pragma once

#include "gamehelp/gameplayhelp.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct help_section_h {
	int section;
	help_section_h();
	explicit help_section_h(int l_section);
	gameplay_help_section* getSection() const;
};

DECLARE_ADE_OBJ(l_Help_Section, help_section_h);

} // namespace api
} // namespace scripting
#pragma once

#include "globalincs/pstypes.h"

#include "ade_doc.h"

namespace scripting {

struct argument_def {
	ade_type_info type;
	SCP_string name;
	SCP_string def_val;
	bool optional = false;
};

class argument_list_parser {
  public:
	bool parse(const SCP_string& argumentList);

	const SCP_vector<scripting::argument_def>& getArgList() const;

  private:
	SCP_vector<argument_def> _argList;
};

} // namespace scripting

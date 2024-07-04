#pragma once

#include "globalincs/pstypes.h"

#include "ade_doc.h"

namespace scripting {

struct argument_def {
	ade_type_info type;
	SCP_string name;
	SCP_string def_val;
	bool optional = false;
	SCP_string comment;
};

class argument_list_parser {
  public:
	explicit argument_list_parser(const SCP_vector<SCP_string>& validTypeNames);

	bool parse(const SCP_string& argumentList);

	const SCP_vector<scripting::argument_def>& getArgList() const;

	const SCP_string& getErrorMessage() const;

  private:
	SCP_unordered_set<SCP_string> _validTypeNames;
	SCP_vector<argument_def> _argList;

	SCP_string _errorMessage;
};

class type_parser {
  public:
	explicit type_parser(const SCP_vector<SCP_string>& validTypeNames);

	bool parse(const SCP_string& type);

	const ade_type_info& getType() const;

	const SCP_string& getErrorMessage() const;

  private:
	SCP_unordered_set<SCP_string> _validTypeNames;
	ade_type_info _parsedType;

	SCP_string _errorMessage;
};

} // namespace scripting

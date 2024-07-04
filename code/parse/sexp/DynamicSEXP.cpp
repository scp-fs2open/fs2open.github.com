//
//

#include "DynamicSEXP.h"

namespace sexp {

DynamicSEXP::DynamicSEXP(const SCP_string& name) : _name(name) {
}
const SCP_string& DynamicSEXP::getName() const {
	return _name;
}
const SCP_string& DynamicSEXP::getHelpText() const {
	return _help_text;
}

}


#pragma once

#include "options/Option.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

class option_h {
	const options::OptionBase* _opt = nullptr;

  public:
	explicit option_h(const options::OptionBase* opt = nullptr);

	bool isValid() const;

	const options::OptionBase* get();
};

DECLARE_ADE_OBJ(l_Option, option_h);

} // namespace api
} // namespace scripting

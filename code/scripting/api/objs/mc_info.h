#pragma once

#include "scripting/ade_api.h"
#include "model/model.h"

namespace scripting {
namespace api {

class mc_info_h
{
 protected:
	mc_info info;
	bool valid = false;

  public:
	explicit mc_info_h(const mc_info& val);
	mc_info_h();

	mc_info *Get();

	bool IsValid();
};

DECLARE_ADE_OBJ(l_ColInfo, mc_info_h);
}
}

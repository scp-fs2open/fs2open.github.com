#pragma once
#ifndef FS2_OPEN_CONTROLBINDING_H
#define FS2_OPEN_CONTROLBINDING_H

#include "scripting/ade_api.h"
#include "controlconfig/controlsconfig.h"

namespace scripting {
namespace api {

/**
 * @brief A class wrapping the Control_config's ActionID to Lua
 * @details A class wrapping the Control_Config's ActionID's to Lua. Lua can request a control-binding object by name and will get this class.
 * This class, only holding the IoActionId as an identifier, can then be used to access and modify the control binding with this id.
 */
class cci_h {
 private:
	IoActionId idx; //!< The ActionId this lua object references.
 public:
	cci_h();
	cci_h(int n_id);

	/*
	* @returns true if this object holds a valid IoActionId, false otherwise
	*/
	bool IsValid();

	IoActionId Get();
};

DECLARE_ADE_OBJ(l_ControlBinding, cci_h);


}
}

#endif // FS2_OPEN_CONTROLBINDING_H

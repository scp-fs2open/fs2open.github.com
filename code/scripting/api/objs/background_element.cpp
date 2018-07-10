//
//

#include "background_element.h"

namespace scripting {
namespace api {

ADE_OBJ(l_BackgroundElement, background_el_h, "background_element", "Background element handle");

background_el_h::background_el_h(BackgroundType in_type, int in_id) : type(in_type), id(in_id) {}
bool background_el_h::isValid() const { return id >= 0 && type != BackgroundType::Invalid; }

ADE_FUNC(isValid, l_BackgroundElement, nullptr, "Determines if this handle is valid", "boolean",
         "true if valid, false if not.")
{
	background_el_h* el = nullptr;
	if (!ade_get_args(L, "o", l_BackgroundElement.GetPtr(&el))) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", el->isValid());
}

} // namespace api
} // namespace scripting

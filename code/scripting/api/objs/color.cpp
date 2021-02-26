
#include "color.h"

namespace scripting {
namespace api {

ADE_OBJ(l_Color, color, "color", "A color value");

// That member syntax is pretty weird to be honest...
static int colorVarHelper(lua_State* L, ubyte color::*member)
{
	color* clr = nullptr;
	float newVal = -1.0f;

	if (!ade_get_args(L, "o|f", l_Color.GetPtr(&clr), &newVal)) {
		return ade_set_args(L, "f", -1.0f);
	}

	if (ADE_SETTING_VAR) {
		if (newVal < 0.0f || newVal > 255.0f) {
			LuaError(L, "Invalid color value %f!", newVal);
			CLAMP(newVal, 0.0f, 255.0f);
		}

		clr->*member = static_cast<ubyte>(newVal);
	}

	return ade_set_args(L, "i", clr->*member);
}

ADE_VIRTVAR(Red,
	l_Color,
	"number",
	"The 'red' value of the color in the range from 0 to 255",
	"number",
	"The 'red' value")
{
	return colorVarHelper(L, &color::red);
}

ADE_VIRTVAR(Green,
	l_Color,
	"number",
	"The 'green' value of the color in the range from 0 to 255",
	"number",
	"The 'green' value")
{
	return colorVarHelper(L, &color::green);
}

ADE_VIRTVAR(Blue,
	l_Color,
	"number",
	"The 'blue' value of the color in the range from 0 to 255",
	"number",
	"The 'blue' value")
{
	return colorVarHelper(L, &color::blue);
}

ADE_VIRTVAR(Alpha,
	l_Color,
	"number",
	"The 'alpha' or opacity value of the color in the range from 0 to 255. 0 is totally transparent, 255 is completely "
	"opaque.",
	"number",
	"The 'alpha' value")
{
	return colorVarHelper(L, &color::alpha);
}

} // namespace api
} // namespace scripting

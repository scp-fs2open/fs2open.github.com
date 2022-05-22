
#include "font.h"

namespace scripting {
namespace api {

//**********HANDLE: Font
font_h::font_h(font::FSFont* fontIn): font(fontIn) {
}

font_h::font_h(): font(NULL) {
}

font::FSFont* font_h::Get() {
	if (!isValid())
		return NULL;

	return font;
}

bool font_h::isValid() {
	return font != NULL;
}

ADE_OBJ(l_Font, font_h, "font", "font handle");

ADE_FUNC(__tostring, l_Font, NULL, "Name of font", "string", "Font filename, or an empty string if the handle is invalid")
{
	font_h *fh = NULL;
	if (!ade_get_args(L, "o", l_Font.GetPtr(&fh)))
		return ade_set_error(L, "s", "");

	if (fh != nullptr && !fh->isValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", fh->Get()->getName().c_str());
}

ADE_FUNC(__eq, l_Font, "font, font", "Checks if the two fonts are equal", "boolean", "true if equal, false otherwise")
{
	font_h *fh1, *fh2;
	if (!ade_get_args(L, "oo", l_Font.GetPtr(&fh1), l_Font.GetPtr(&fh2)))
		return ade_set_error(L, "b", false);

	if (!fh1->isValid() || !fh2->isValid())
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", fh1->Get()->getName() == fh2->Get()->getName());
}

ADE_VIRTVAR(Filename, l_Font, "string", "Name of font (including extension)<br><b>Important:</b>This variable is deprecated. Use <i>Name</i> instead.", "string", NULL)
{
	font_h *fh = NULL;
	const char* newname = nullptr;
	if (!ade_get_args(L, "o|s", l_Font.GetPtr(&fh), &newname))
		return ade_set_error(L, "s", "");

	if (fh != nullptr && !fh->isValid())
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR)
	{
		fh->Get()->setName(newname);
	}

	return ade_set_args(L, "s", fh->Get()->getName().c_str());
}

ADE_VIRTVAR(Name, l_Font, "string", "Name of font (including extension)", "string", NULL)
{
	font_h *fh = NULL;
	const char* newname = nullptr;
	if (!ade_get_args(L, "o|s", l_Font.GetPtr(&fh), &newname))
		return ade_set_error(L, "s", "");

	if (fh != nullptr && !fh->isValid())
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR)
	{
		fh->Get()->setName(newname);
	}

	return ade_set_args(L, "s", fh->Get()->getName().c_str());
}

ADE_VIRTVAR(Height, l_Font, "number", "Height of font (in pixels)", "number", "Font height, or 0 if the handle is invalid")
{
	font_h *fh = NULL;
	int newheight = -1;
	if (!ade_get_args(L, "o|i", l_Font.GetPtr(&fh), &newheight))
		return ade_set_error(L, "i", 0);

	if (fh != nullptr && !fh->isValid())
		return ade_set_error(L, "i", 0);

	if (ADE_SETTING_VAR && newheight > 0)
	{
		LuaError(L, "Height setting isn't available anymore!");
	}

	return ade_set_args(L, "f", fh->Get()->getHeight());
}

ADE_VIRTVAR(TopOffset, l_Font, "number", "The offset this font has from the baseline of textdrawing downwards. (in pixels)", "number", "Font top offset, or 0 if the handle is invalid")
{
	font_h *fh = NULL;
	float newOffset = -1;
	if (!ade_get_args(L, "o|f", l_Font.GetPtr(&fh), &newOffset))
		return ade_set_error(L, "f", 0.0f);

	if (fh != nullptr && !fh->isValid())
		return ade_set_error(L, "f", 0.0f);

	if (ADE_SETTING_VAR && newOffset > 0)
	{
		fh->Get()->setTopOffset(newOffset);
	}

	return ade_set_args(L, "f", fh->Get()->getTopOffset());
}

ADE_VIRTVAR(BottomOffset, l_Font, "number", "The space (in pixels) this font skips downwards after drawing a line of text", "number", "Font bottom offset, or 0 if the handle is invalid")
{
	font_h *fh = NULL;
	float newOffset = -1;
	if (!ade_get_args(L, "o|f", l_Font.GetPtr(&fh), &newOffset))
		return ade_set_error(L, "f", 0.0f);

	if (fh != nullptr && !fh->isValid())
		return ade_set_error(L, "f", 0.0f);

	if (ADE_SETTING_VAR && newOffset > 0)
	{
		fh->Get()->setBottomOffset(newOffset);
	}

	return ade_set_args(L, "f", fh->Get()->getBottomOffset());
}

ADE_FUNC(isValid, l_Font, NULL, "True if valid, false or nil if not", "boolean", "Detects whether handle is valid")
{
	font_h *fh = nullptr;
	if (!ade_get_args(L, "o", l_Font.GetPtr(&fh)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", fh != nullptr && fh->isValid());
}

}
} // namespace scripting

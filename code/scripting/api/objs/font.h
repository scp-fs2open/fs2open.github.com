
#pragma once
#ifndef FS2_OPEN_FONT_H
#define FS2_OPEN_FONT_H

#include "scripting/ade_api.h"
#include "graphics/font.h"

namespace scripting {
namespace api {

class font_h
{
	font::FSFont *font;

public:
	explicit font_h(font::FSFont* fontIn);

	font_h();

	font::FSFont* Get();

	bool isValid();
};

DECLARE_ADE_OBJ(l_Font, font_h);

}
}

#endif
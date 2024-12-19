
#pragma once
#ifndef FS2_OPEN_FONT_H
#define FS2_OPEN_FONT_H

#include "scripting/ade_api.h"
#include "graphics/font.h"

namespace scripting {
namespace api {

class font_h
{
	int _fontIndex;

public:
	font_h(int fontIndex);
	font_h();

	font::FSFont* Get() const;
	int GetIndex() const;

	bool isValid() const;
};

DECLARE_ADE_OBJ(l_Font, font_h);

}
}

#endif
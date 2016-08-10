#include "graphics/software/FSFont.h"

namespace font
{
	FSFont::FSFont() : name(SCP_string("<Invalid>")), offsetTop(0.0f), offsetBottom(0.0f)
	{
	}

	FSFont::~FSFont()
	{
	}

	void FSFont::setBottomOffset(float offset)
	{
		this->offsetBottom = offset;
	}

	void FSFont::setTopOffset(float offset)
	{
		this->offsetTop = offset;
	}

	void FSFont::setName(const SCP_string &newName)
	{
		this->name = newName;
	}

	float FSFont::getBottomOffset() const
	{
		return this->offsetBottom;
	}

	float FSFont::getTopOffset() const
	{
		return this->offsetTop;
	}

	float FSFont::getHeight() const
	{
		return this->getTextHeight() + this->offsetTop + this->offsetBottom;
	}

	const SCP_string &FSFont::getName() const
	{
		return this->name;
	}

	void FSFont::checkHeight() {
		float height = this->getTextHeight() + this->offsetTop + this->offsetBottom;

		if (height <= 1.0f)
		{
			Warning(LOCATION, "The height of font %s has an invalid height of %f, must be greater than one!",
					getName().c_str(), height);
		}
	}
}

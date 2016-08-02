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
		Assertion(offset >= 0.0f, "Bottom offset for font %s has to be larger than zero but it is %f",
				  this->getName().c_str(), offset);

		this->offsetBottom = offset;
	}

	void FSFont::setTopOffset(float offset)
	{
		Assertion(offset >= 0.0f, "Top offset for font %s has to be larger than zero but it is %f",
				  this->getName().c_str(), offset);

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
#ifndef NDEBUG
		if (!heightChecked)
		{
			float height = this->getTextHeight() + this->offsetTop + this->offsetBottom;

			if (height <= 0.0f)
			{
				Warning(LOCATION, "The height of font %s has an invalid height of %f, must be greater than zero!",
						getName().c_str(), height);
			}

			// HACK: This function is const but to disable the warning in the future we need to change a variable
			const_cast<FSFont *>(this)->heightChecked = true;
		}
#endif
		return this->getTextHeight() + this->offsetTop + this->offsetBottom;
	}

	const SCP_string &FSFont::getName() const
	{
		return this->name;
	}
}

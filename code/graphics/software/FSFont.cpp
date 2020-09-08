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

	void FSFont::setFilename(const SCP_string& newName) 
	{
		this->filename = newName;
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
		return _height;
	}

	const SCP_string &FSFont::getName() const
	{
		return this->name;
	}

	const SCP_string &FSFont::getFilename() const
	{
		return this->filename;
	}

	void FSFont::computeFontMetrics() {
		_height = this->getTextHeight() + this->offsetTop + this->offsetBottom;

		// By default the base line of the font is also the lowest point of the font
		_ascender = _height;
		_descender = 0.0f;

		checkFontMetrics();
	}
	void FSFont::checkFontMetrics() {
		if (_height <= 1.0f)
		{
			Warning(LOCATION, "The height of font %s has an invalid height of %f, must be greater than one!",
					getName().c_str(), _height);
		}
	}
	float FSFont::getAscender() {
		return _ascender;
	}
	float FSFont::getDescender() {
		return _descender;
	}
}

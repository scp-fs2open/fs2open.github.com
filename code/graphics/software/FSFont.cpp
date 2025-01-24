#include "graphics/software/FSFont.h"
#include "options/Option.h"

float Font_Scale_Factor = 1.0;

static auto FontScaleFactor __UNUSED = options::OptionBuilder<float>("Game.FontScaleFactor",
	std::pair<const char*, int>{"Font Scale Factor", 1862},
	std::pair<const char*, int>{
		"Sets a multipler to scale fonts by. Only works on fonts the mod has explicitely allowed",
		1863})
										   .category(std::make_pair("Game", 1824))
										   .range(0.2f, 4.0f) // Upper limit is somewhat arbitrary
										   .level(options::ExpertLevel::Advanced)
										   .default_val(1.0)
										   .bind_to(&Font_Scale_Factor)
										   .importance(55)
										   .finish();

void removeFontMultiplierOption()
{
	options::OptionsManager::instance()->removeOption(FontScaleFactor);
}

float get_font_scale_factor()
{
	return FontScaleFactor->getValue();
}

namespace font
{

	void FSFont::setAutoScaleBehavior(bool auto)
	{
		this->autoScale = auto;
	}

	void FSFont::setScaleBehavior(bool scale)
	{
		this->canScale = scale;
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

	[[nodiscard]] bool FSFont::getAutoScaleBehavior() const
	{
		return this->autoScale;
	}

	[[nodiscard]] bool FSFont::getScaleBehavior() const
	{
		return this->canScale;
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

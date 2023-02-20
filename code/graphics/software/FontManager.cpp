

#include "graphics/software/FontManager.h"
#include "graphics/software/VFNTFont.h"
#include "graphics/software/NVGFont.h"
#include "graphics/software/font.h"
#include "graphics/software/font_internal.h"

#include "graphics/paths/PathRenderer.h"

#include "bmpman/bmpman.h"
#include "cfile/cfile.h"
#include "localization/localize.h"

namespace font
{

	SCP_map<SCP_string, TrueTypeFontData> FontManager::allocatedData;
	SCP_map<SCP_string, std::unique_ptr<font>> FontManager::vfntFontData;
	SCP_vector<std::unique_ptr<FSFont>> FontManager::fonts;

	FSFont* FontManager::currentFont = NULL;

	FSFont* FontManager::getFont(const SCP_string& name)
	{
		for (SCP_vector<std::unique_ptr<FSFont>>::iterator iter = fonts.begin(); iter != fonts.end(); iter++)
		{
			if ((*iter)->getName() == name)
				return iter->get();
		}

		return NULL;
	}

	FSFont* FontManager::getFontByFilename(const SCP_string& filename) 
	{
		for (auto & iter : fonts) 
		{
			if (iter->getFilename() == filename)
				return iter.get();
		}
		return nullptr;
	}

	FSFont *FontManager::getCurrentFont()
	{
		return currentFont;
	}

	int FontManager::getCurrentFontIndex()
	{
		if (!FontManager::isReady())
			return -1;

		return FontManager::getFontIndex(currentFont);
	}

	int FontManager::getFontIndex(const SCP_string& name)
	{
		int index = 0;

		for (SCP_vector<std::unique_ptr<FSFont>>::iterator iter = fonts.begin(); iter != fonts.end(); iter++, index++)
		{
			if ((*iter)->getName() == name)
				return index;
		}

		return -1;
	}

	int FontManager::getFontIndex(FSFont *font)
	{
		if (font == NULL)
			return -1;

		int index = 0;

		for (SCP_vector<std::unique_ptr<FSFont>>::iterator iter = fonts.begin(); iter != fonts.end(); iter++, index++)
		{
			if (iter->get() == font)
				return index;
		}

		return -1;
	}

	int FontManager::numberOfFonts()
	{
		return (int)fonts.size();
	}

	bool FontManager::isReady()
	{
		return currentFont != NULL;
	}

	bool FontManager::isFontNumberValid(int id)
	{
		return id >= 0 && id < (int)fonts.size();
	}

	void FontManager::setCurrentFont(FSFont *font)
	{
		Assertion(font != NULL, "New font pointer may not be NULL!");
		currentFont = font;
	}

	font* FontManager::loadFontOld(const SCP_string& typeface)
	{
		if (vfntFontData.find(typeface) != vfntFontData.end())
		{
			font* data = vfntFontData[typeface].get();

			Assert(data != NULL);

			return data;
		}

		// try localized version first
		CFILE* fp = nullptr;
		SCP_string typeface_lcl = typeface;

		lcl_add_dir_to_path_with_filename(typeface_lcl);

		fp = cfopen(typeface_lcl.c_str(), "rb", CFILE_NORMAL, CF_TYPE_ANY);

		// fallback if not found
		if ( !fp )
		{
			fp = cfopen(typeface.c_str(), "rb", CFILE_NORMAL, CF_TYPE_ANY);
		}

		if (fp == NULL)
		{
			mprintf(("Unable to find font file \"%s\"\n", typeface.c_str()));
			return NULL;
		}

		std::unique_ptr<font> fnt(new font());
		if (!fnt)
		{
			mprintf(("Unable to allocate memory for \"%s\"\n", typeface.c_str()));
			return NULL;
		}

		strcpy_s(fnt->filename, typeface.c_str());
		cfread(&fnt->id, 4, 1, fp);
		cfread(&fnt->version, sizeof(int), 1, fp);
		cfread(&fnt->num_chars, sizeof(int), 1, fp);
		cfread(&fnt->first_ascii, sizeof(int), 1, fp);
		cfread(&fnt->w, sizeof(int), 1, fp);
		cfread(&fnt->h, sizeof(int), 1, fp);
		cfread(&fnt->num_kern_pairs, sizeof(int), 1, fp);
		cfread(&fnt->kern_data_size, sizeof(int), 1, fp);
		cfread(&fnt->char_data_size, sizeof(int), 1, fp);
		cfread(&fnt->pixel_data_size, sizeof(int), 1, fp);

		fnt->id = INTEL_SHORT(fnt->id); //-V570
		fnt->version = INTEL_INT(fnt->version); //-V570
		fnt->num_chars = INTEL_INT(fnt->num_chars); //-V570
		fnt->first_ascii = INTEL_INT(fnt->first_ascii); //-V570
		fnt->w = INTEL_INT(fnt->w); //-V570
		fnt->h = INTEL_INT(fnt->h); //-V570
		fnt->num_kern_pairs = INTEL_INT(fnt->num_kern_pairs); //-V570
		fnt->kern_data_size = INTEL_INT(fnt->kern_data_size); //-V570
		fnt->char_data_size = INTEL_INT(fnt->char_data_size); //-V570
		fnt->pixel_data_size = INTEL_INT(fnt->pixel_data_size); //-V570

		if (fnt->kern_data_size)	{
			fnt->kern_data = (font_kernpair *)vm_malloc(fnt->kern_data_size);
			Assert(fnt->kern_data != NULL);
			cfread(fnt->kern_data, fnt->kern_data_size, 1, fp);
		}
		else {
			fnt->kern_data = NULL;
		}
		if (fnt->char_data_size)	{
			fnt->char_data = (font_char *)vm_malloc(fnt->char_data_size);
			Assert(fnt->char_data != NULL);
			cfread(fnt->char_data, fnt->char_data_size, 1, fp);

			for (int i = 0; i<fnt->num_chars; i++) {
				fnt->char_data[i].spacing = INTEL_INT(fnt->char_data[i].spacing); //-V570
				fnt->char_data[i].byte_width = INTEL_INT(fnt->char_data[i].byte_width); //-V570
				fnt->char_data[i].offset = INTEL_INT(fnt->char_data[i].offset); //-V570
				fnt->char_data[i].kerning_entry = INTEL_INT(fnt->char_data[i].kerning_entry); //-V570
				fnt->char_data[i].user_data = INTEL_SHORT(fnt->char_data[i].user_data); //-V570
			}
		}
		else {
			fnt->char_data = NULL;
		}
		if (fnt->pixel_data_size)	{
			fnt->pixel_data = (ubyte *)vm_malloc(fnt->pixel_data_size);
			Assert(fnt->pixel_data != NULL);
			cfread(fnt->pixel_data, fnt->pixel_data_size, 1, fp);
		}
		else {
			fnt->pixel_data = NULL;
		}
		cfclose(fp);

		// Create a bitmap for hardware cards.
		// JAS:  Try to squeeze this into the smallest square power of two texture.
		// This should probably be done at font generation time, not here.
		int w, h;
		if (fnt->pixel_data_size * 4 < 64 * 64) {
			w = h = 64;
		}
		else if (fnt->pixel_data_size * 4 < 128 * 128) {
			w = h = 128;
		}
		else if (fnt->pixel_data_size * 4 < 256 * 256) {
			w = h = 256;
		}
		else if (fnt->pixel_data_size * 4 < 512 * 512) {
			w = h = 512;
		}
		else {
			w = h = 1024;
		}

		fnt->bm_w = w;
		fnt->bm_h = h;
		fnt->bm_data = (ubyte *)vm_malloc(fnt->bm_w*fnt->bm_h);
		fnt->bm_u = (int *)vm_malloc(sizeof(int)*fnt->num_chars);
		fnt->bm_v = (int *)vm_malloc(sizeof(int)*fnt->num_chars);

		memset(fnt->bm_data, 0, fnt->bm_w * fnt->bm_h);

		int i, x, y;
		x = y = 0;
		for (i = 0; i<fnt->num_chars; i++)	{
			ubyte * ubp;
			int x1, y1;
			ubp = &fnt->pixel_data[fnt->char_data[i].offset];
			if (x + fnt->char_data[i].byte_width >= fnt->bm_w)	{
				x = 0;
				y += fnt->h + 2;
				if (y + fnt->h > fnt->bm_h) {
					Error(LOCATION, "Font too big!\n");
				}
			}
			fnt->bm_u[i] = x;
			fnt->bm_v[i] = y;

			for (y1 = 0; y1<fnt->h; y1++)	{
				for (x1 = 0; x1<fnt->char_data[i].byte_width; x1++)	{
					uint c = *ubp++;
					if (c > 14) c = 14;
					// The font pixels only have ~4 bits of information in them (since the value is at maximum 14) but
					// the bmpman code expects 8 bits of pixel information. To fix that we simply rescale this value to
					// fit into the [0, 255] range (15 * 17 is 255). This was adapted from the previous version where
					// the graphics code used an internal array for converting these values
					fnt->bm_data[(x + x1) + (y + y1)*fnt->bm_w] = (unsigned char)(c * 17);
				}
			}
			x += fnt->char_data[i].byte_width + 2;
		}

		fnt->bitmap_id = bm_create(8, fnt->bm_w, fnt->bm_h, fnt->bm_data, BMP_AABITMAP);

		auto ptr = fnt.get();

		vfntFontData[typeface] = std::move(fnt);

		return ptr;
	}

	VFNTFont *FontManager::loadVFNTFont(const SCP_string& name)
	{
		font* font = FontManager::loadFontOld(name);

		if (font == NULL)
		{
			return NULL;
		}
		else
		{
			std::unique_ptr<VFNTFont> vfnt(new VFNTFont(font));

			auto ptr = vfnt.get();

			fonts.push_back(std::move(vfnt));

			return ptr;
		}
	}

	NVGFont *FontManager::loadNVGFont(const SCP_string& fileName, float fontSize)
	{
		if (allocatedData.find(fileName) == allocatedData.end())
		{
			CFILE *fontFile = cfopen(const_cast<char*>(fileName.c_str()), "rb", CFILE_NORMAL, CF_TYPE_ANY);

			if (fontFile == NULL)
			{
				mprintf(("Couldn't open font file \"%s\"\n", fileName.c_str()));
				return NULL;
			}

			size_t size = static_cast<size_t>(cfilelength(fontFile));

			std::unique_ptr<ubyte[]> fontData(new ubyte[size]);

			if (!fontData)
			{
				mprintf(("Couldn't allocate " SIZE_T_ARG " bytes for reading font file \"%s\"!\n", size, fileName.c_str()));
				cfclose(fontFile);
				return NULL;
			}

			if (!cfread(fontData.get(), (int)size, 1, fontFile))
			{
				mprintf(("Error while reading font data from \"%s\"\n", fileName.c_str()));
				cfclose(fontFile);
				return NULL;
			}

			cfclose(fontFile);

			TrueTypeFontData newData;

			newData.size = size;
			std::swap(newData.data, fontData);

			allocatedData.insert(std::make_pair(fileName, std::move(newData)));
		}

		auto data = &allocatedData.find(fileName)->second;

		auto path = graphics::paths::PathRenderer::instance();

		int handle = path->createFontMem(fileName.c_str(), data->data.get(), (int)data->size, 0);
		
		if (handle < 0)
		{
			mprintf(("Couldn't couldn't create font for file \"%s\"\n", fileName.c_str()));
			return NULL;
		}

		std::unique_ptr<NVGFont> nvgFont(new NVGFont());
		nvgFont->setHandle(handle);
		nvgFont->setSize(fontSize);

		auto ptr = nvgFont.get();

		fonts.push_back(std::move(nvgFont));

		return ptr;
	}

	void FontManager::init()
	{
	}

	void FontManager::close()
	{
		allocatedData.clear();
		vfntFontData.clear();
		fonts.clear();

		currentFont = NULL;
	}
}

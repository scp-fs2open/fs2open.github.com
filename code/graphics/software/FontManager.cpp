

#include "graphics/software/FontManager.h"
#include "graphics/software/VFNTFont.h"
#include "graphics/software/NVGFont.h"
#include "graphics/software/font.h"
#include "graphics/software/font_internal.h"

#include "graphics/paths/PathRenderer.h"

#include "bmpman/bmpman.h"
#include "cfile/cfile.h"
#include "localization/localize.h"
#include "options/Option.h"

namespace font
{
	SCP_map<SCP_string, TrueTypeFontData> FontManager::allocatedData;
	SCP_map<SCP_string, std::unique_ptr<font>> FontManager::vfntFontData;
	SCP_vector<std::unique_ptr<FSFont>> FontManager::fonts;

	int FontManager::currentFontIndex = -1;

	FSFont* FontManager::getFont(const SCP_string& name)
	{
		for (auto &iter: fonts)
		{
			if (lcase_equal(iter->getName(), name))
				return iter.get();
		}

		return nullptr;
	}

	FSFont* FontManager::getFontByFilename(const SCP_string& filename)
	{
		for (auto &iter: fonts)
		{
			if (lcase_equal(iter->getFilename(), filename))
				return iter.get();
		}

		return nullptr;
	}

	FSFont* FontManager::getFont(int index)
	{
		if (!isFontNumberValid(index))
			return nullptr;

		return fonts[index].get();
	}

	FSFont *FontManager::getCurrentFont()
	{
		int id = gr_lua_context_active() ? gr_lua_screen.current_font_index : currentFontIndex;
		return getFont(id);
	}

	int FontManager::getCurrentFontIndex()
	{
		int id = gr_lua_context_active() ? gr_lua_screen.current_font_index : currentFontIndex;
		if (!isFontNumberValid(id))
			return -1;

		return id;
	}

	int FontManager::getFontIndex(const SCP_string& name)
	{
		int index = 0;

		for (const auto &iter: fonts)
		{
			if (lcase_equal(iter->getName(), name))
				return index;
			index++;
		}

		return -1;
	}

	int FontManager::getFontIndexByFilename(const SCP_string& filename)
	{
		int index = 0;

		for (const auto &iter: fonts)
		{
			if (lcase_equal(iter->getFilename(), filename))
				return index;
			index++;
		}

		return -1;
	}

	bool FontManager::hasScalingFonts()
	{
		return std::any_of(fonts.begin(), fonts.end(), [](const std::unique_ptr<FSFont>& font) {
			const auto& thisFont = font.get();
			return thisFont->getScaleBehavior();
		});
	}

	int FontManager::numberOfFonts()
	{
		return static_cast<int>(fonts.size());
	}

	bool FontManager::isReady()
	{
		return isFontNumberValid(currentFontIndex);
	}

	bool FontManager::isFontNumberValid(int id)
	{
		return SCP_vector_inbounds(fonts, id);
	}

	void FontManager::setCurrentFontIndex(int id)
	{
		Assertion(isFontNumberValid(id), "New font index must be valid!");
		if (gr_lua_context_active()) {
			gr_lua_screen.current_font_index = id;
		} else {
			currentFontIndex = id;
		}
	}

	font* FontManager::loadFontOld(const SCP_string& typeface)
	{
		auto iter = vfntFontData.find(typeface);
		if (iter != vfntFontData.end())
		{
			font* data = iter->second.get();

			Assert(data != nullptr);

			return data;
		}

		// try localized version first
		CFILE* fp = nullptr;
		SCP_string typeface_lcl = typeface;

		lcl_add_dir_to_path_with_filename(typeface_lcl);

		fp = cfopen(typeface_lcl.c_str(), "rb", CF_TYPE_ANY);

		// fallback if not found
		if ( !fp )
		{
			fp = cfopen(typeface.c_str(), "rb", CF_TYPE_ANY);
		}

		if ( !fp )
		{
			mprintf(("Unable to find font file \"%s\"\n", typeface.c_str()));
			return nullptr;
		}

		std::unique_ptr<font> fnt(new font());

		if (typeface.size() <= MAX_FILENAME_LEN - 1)
			strcpy_s(fnt->filename, typeface.c_str());
		else
		{
			strncpy_s(fnt->filename, typeface.c_str(), MAX_FILENAME_LEN - 1);
			fnt->filename[MAX_FILENAME_LEN - 1] = '\0';
		}

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
			fnt->kern_data = new font_kernpair[fnt->kern_data_size];
			cfread(fnt->kern_data, fnt->kern_data_size, 1, fp);
		}
		else {
			fnt->kern_data = nullptr;
		}
		if (fnt->char_data_size)	{
			fnt->char_data = new font_char[fnt->char_data_size];
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
			fnt->char_data = nullptr;
		}
		if (fnt->pixel_data_size)	{
			fnt->pixel_data = new ubyte[fnt->pixel_data_size];
			cfread(fnt->pixel_data, fnt->pixel_data_size, 1, fp);
		}
		else {
			fnt->pixel_data = nullptr;
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
		fnt->bm_data = new ubyte[fnt->bm_w * fnt->bm_h];
		fnt->bm_u = new int[fnt->num_chars];
		fnt->bm_v = new int[fnt->num_chars];

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
					Warning(LOCATION, "Font %s too big!\n", fnt->filename);
					return nullptr;
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

		vfntFontData.emplace(typeface, std::move(fnt));

		return ptr;
	}

	std::pair<VFNTFont*, int> FontManager::loadVFNTFont(const SCP_string& name)
	{
		font* font = FontManager::loadFontOld(name);

		if (font == nullptr)
		{
			return { nullptr, -1 };
		}
		else
		{
			std::unique_ptr<VFNTFont> vfnt(new VFNTFont(font));

			auto ptr = vfnt.get();

			fonts.emplace_back(std::move(vfnt));

			return { ptr, static_cast<int>(fonts.size() - 1) };
		}
	}

	// This function will extract the font family name (Name ID 1) from TrueType font data.
	// It handles UCS-2 (UTF-16BE) to UTF-8 conversion.
	static SCP_string extractFamilyNameFromTTF(const TrueTypeFontData& fontData)
	{
		try {
			// TTF/OTF fonts start the table directory at byte 12.
			constexpr size_t table_offset = 12;

			const ubyte* data = fontData.data.get();
			const size_t size = fontData.size;

			if (size < table_offset) {
				throw std::runtime_error("Font data too small for table offset");
			}

			// Offset to the start of the table directory (after SFNT header)
			const uint8_t* tableDir = data + table_offset;

			// Read numTables (ushort at offset 4 in SFNT header)
			// The SFNT header bytes are big-endian
			uint16_t numTables = (static_cast<uint16_t>(data[4]) << 8) | static_cast<uint16_t>(data[5]);

			const uint8_t* nameTable = nullptr;

			// Each table entry is 16 bytes
			constexpr size_t tableEntrySize = 16;
			const size_t tableDirSize = static_cast<size_t>(numTables) * tableEntrySize;
			if (size < table_offset + tableDirSize) {
				throw std::runtime_error("Table directory extends past file size");
			}

			// Iterate through table directory entries to find the 'name' table
			for (int i = 0; i < numTables; ++i) {
				const uint8_t* entry = tableDir + i * tableEntrySize;

				// Ensure entry access is within range
				if (entry + 12 >= data + size) {
					throw std::runtime_error("Table entry access out of bounds");
				}

				// Read table tag (4 bytes)
				uint32_t tag = (static_cast<uint32_t>(entry[0]) << 24) | (static_cast<uint32_t>(entry[1]) << 16) |
							   (static_cast<uint32_t>(entry[2]) << 8) | static_cast<uint32_t>(entry[3]);

				// Check if it's the 'name' table (tag 0x6E616D65)
				if (tag == 0x6E616D65) {
					// Read offset to the 'name' table (4 bytes, big-endian)
					uint32_t offset = (static_cast<uint32_t>(entry[8]) << 24) |
									  (static_cast<uint32_t>(entry[9]) << 16) |
									  (static_cast<uint32_t>(entry[10]) << 8) | static_cast<uint32_t>(entry[11]);

					if (offset >= static_cast<uint32_t>(size)) {
						throw std::runtime_error("Name table offset beyond file size");
					}

					nameTable = data + offset;
					break;
				}
			}

			if (!nameTable || nameTable + 6 > data + size) {
				throw std::runtime_error("Name table header is missing or truncated");
			}

			// Name table header - All values are big-endian
			uint16_t count = (static_cast<uint16_t>(nameTable[2]) << 8) | static_cast<uint16_t>(nameTable[3]);
			uint16_t stringOffset = (static_cast<uint16_t>(nameTable[4]) << 8) | static_cast<uint16_t>(nameTable[5]);

			constexpr size_t recordSize = 12;
			const uint8_t* recordBase = nameTable + 6;

			if (recordBase + count * recordSize > data + size) {
				throw std::runtime_error("Name records extend beyond font data");
			}

			// Iterate through name records
			for (int i = 0; i < count; ++i) {
				const uint8_t* record =
					recordBase + i * recordSize; // 6 bytes for name table header, 12 bytes per record

				// Read metadata for the record
				uint16_t platformID = (static_cast<uint16_t>(record[0]) << 8) | static_cast<uint16_t>(record[1]);
				uint16_t encodingID = (static_cast<uint16_t>(record[2]) << 8) | static_cast<uint16_t>(record[3]);
				uint16_t nameID = (static_cast<uint16_t>(record[6]) << 8) | static_cast<uint16_t>(record[7]);
				uint16_t length = (static_cast<uint16_t>(record[8]) << 8) | static_cast<uint16_t>(record[9]);
				uint16_t offset = (static_cast<uint16_t>(record[10]) << 8) | static_cast<uint16_t>(record[11]);

				// We are looking for Name ID 1 (Font Family Name)
				// Prefer Unicode (Platform ID 0, 3) or Apple Roman (Platform ID 1) if available
				if (nameID == 1) {
					// Check for Unicode (Platform ID 0, Encoding ID 3 or 4) or (Platform ID 3, Encoding ID 1)
					// or Mac Roman (Platform ID 1, Encoding ID 0)
					bool isUnicode = (platformID == 0 && (encodingID == 3 || encodingID == 4)) ||
									 (platformID == 3 && encodingID == 1);
					bool isMacRoman = (platformID == 1 && encodingID == 0);

					if (isUnicode || isMacRoman) {
						const uint8_t* nameString = nameTable + stringOffset + offset;
						SCP_string familyName;

						// Bounds check to prevent reading past the end of the font metadata
						if (nameString + length > data + size) {
							throw std::runtime_error("Name string extends past font data");
						}

						if (isUnicode) {
							// UCS-2 (UTF-16BE) to UTF-8 conversion
							// This assumes standard UCS-2, which is UTF-16BE for basic multilingual plane.
							for (int j = 0; j < length; j += 2) {
								uint16_t unicodeChar = (static_cast<uint16_t>(nameString[j]) << 8) |
													   static_cast<uint16_t>(nameString[j + 1]);

								if (unicodeChar <= 0x7F) { // 1-byte UTF-8
									familyName += static_cast<char>(unicodeChar);
								} else if (unicodeChar <= 0x7FF) { // 2-byte UTF-8
									familyName += static_cast<char>(0xC0 | (unicodeChar >> 6));
									familyName += static_cast<char>(0x80 | (unicodeChar & 0x3F));
								} else { // 3-byte UTF-8
									familyName += static_cast<char>(0xE0 | (unicodeChar >> 12));
									familyName += static_cast<char>(0x80 | ((unicodeChar >> 6) & 0x3F));
									familyName += static_cast<char>(0x80 | (unicodeChar & 0x3F));
								}
								// For supplementary planes (4-byte UTF-8, characters > 0xFFFF),
								// this simple conversion is not sufficient and would require surrogate pair handling.
								// Most common font names will be in BMP (Basic Multilingual Plane) according to my
								// research - Mjn
							}
						} else { // Mac Roman (single byte) - this encoding is less common but supported easily enough
							for (int j = 0; j < length; ++j) {
								familyName += static_cast<char>(nameString[j]);
							}
						}
						return familyName;
					}
				}
			}

			throw std::runtime_error("No suitable family name found");
		} catch (const std::exception& e) {
			mprintf(("Failed to extract font name: %s\n", e.what()));
			return "";
		} catch (...) {
			mprintf(("Failed to extract font name: Unknown exception\n"));
			return "";
		}
	}

	std::pair<NVGFont*, int> FontManager::loadNVGFont(const SCP_string& fileName, float fontSize)
	{
		auto iter = allocatedData.find(fileName);
		if (iter == allocatedData.end())
		{
			CFILE *fontFile = cfopen(fileName.c_str(), "rb", CF_TYPE_ANY);

			if (fontFile == nullptr)
			{
				mprintf(("Couldn't open font file \"%s\"\n", fileName.c_str()));
				return { nullptr, -1 };
			}

			int size = cfilelength(fontFile);

			std::unique_ptr<ubyte[]> fontData(new ubyte[size]);

			if (!cfread(fontData.get(), size, 1, fontFile))
			{
				mprintf(("Error while reading font data from \"%s\"\n", fileName.c_str()));
				cfclose(fontFile);
				return { nullptr, -1 };
			}

			cfclose(fontFile);

			TrueTypeFontData newData;

			newData.size = static_cast<size_t>(size);
			std::swap(newData.data, fontData);

			auto pair = allocatedData.emplace(fileName, std::move(newData));
			iter = pair.first;
		}

		auto data = &iter->second;

		auto path = graphics::paths::PathRenderer::instance();

		int handle = path->createFontMem(fileName.c_str(), data->data.get(), (int)data->size, 0);
		
		if (handle < 0)
		{
			mprintf(("Couldn't create font for file \"%s\"\n", fileName.c_str()));
			return { nullptr, -1 };
		}

		std::unique_ptr<NVGFont> nvgFont(new NVGFont());
		nvgFont->setHandle(handle);
		nvgFont->setSize(fontSize);
		nvgFont->setFamilyName(extractFamilyNameFromTTF(*data));

		auto ptr = nvgFont.get();

		fonts.emplace_back(std::move(nvgFont));

		return { ptr, static_cast<int>(fonts.size() - 1) };
	}

	void FontManager::init()
	{
	}

	void FontManager::close()
	{
		allocatedData.clear();
		vfntFontData.clear();
		fonts.clear();

		currentFontIndex = -1;
	}
}

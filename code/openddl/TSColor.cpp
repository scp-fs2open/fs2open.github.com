//
// This file is part of the Terathon Common Library, by Eric Lengyel.
// Copyright 1999-2021, Terathon Software LLC
//
// This software is licensed under the GNU General Public License version 3.
// Separate proprietary licenses are available from Terathon Software.
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
// EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. 
//


#include "TSColor.h"
#include "TSMath.h"


using namespace Terathon;


namespace
{
	alignas(16) const char hexDigit[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
}


const ConstColorRGBA Color::black = {0.0F, 0.0F, 0.0F, 1.0F};
const ConstColorRGBA Color::white = {1.0F, 1.0F, 1.0F, 1.0F};
const ConstColorRGBA Color::transparent = {0.0F, 0.0F, 0.0F, 0.0F};
const ConstColorRGBA Color::red = {1.0F, 0.0F, 0.0F, 1.0F};
const ConstColorRGBA Color::green = {0.0F, 1.0F, 0.0F, 1.0F};
const ConstColorRGBA Color::blue = {0.0F, 0.0F, 1.0F, 1.0F};
const ConstColorRGBA Color::yellow = {1.0F, 1.0F, 0.0F, 1.0F};
const ConstColorRGBA Color::cyan = {0.0F, 1.0F, 1.0F, 1.0F};
const ConstColorRGBA Color::magenta = {1.0F, 0.0F, 1.0F, 1.0F};


alignas(64) const uint8 Color::srgbLinearizationTable[256] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x0A, 0x0A, 0x0A, 0x0B, 0x0B, 0x0C, 0x0C, 0x0C, 0x0D,
	0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F, 0x10, 0x10, 0x11, 0x11, 0x11, 0x12, 0x12, 0x13, 0x13, 0x14,
	0x14, 0x15, 0x16, 0x16, 0x17, 0x17, 0x18, 0x18, 0x19, 0x19, 0x1A, 0x1B, 0x1B, 0x1C, 0x1D, 0x1D,
	0x1E, 0x1E, 0x1F, 0x20, 0x20, 0x21, 0x22, 0x23, 0x23, 0x24, 0x25, 0x25, 0x26, 0x27, 0x28, 0x29,
	0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x33, 0x34, 0x35, 0x36,
	0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
	0x47, 0x48, 0x49, 0x4A, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x54, 0x55, 0x56, 0x57, 0x58,
	0x5A, 0x5B, 0x5C, 0x5D, 0x5F, 0x60, 0x61, 0x63, 0x64, 0x65, 0x67, 0x68, 0x69, 0x6B, 0x6C, 0x6D,
	0x6F, 0x70, 0x72, 0x73, 0x74, 0x76, 0x77, 0x79, 0x7A, 0x7C, 0x7D, 0x7F, 0x80, 0x82, 0x83, 0x85,
	0x86, 0x88, 0x8A, 0x8B, 0x8D, 0x8E, 0x90, 0x92, 0x93, 0x95, 0x97, 0x98, 0x9A, 0x9C, 0x9D, 0x9F,
	0xA1, 0xA3, 0xA4, 0xA6, 0xA8, 0xAA, 0xAB, 0xAD, 0xAF, 0xB1, 0xB3, 0xB5, 0xB7, 0xB8, 0xBA, 0xBC,
	0xBE, 0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE, 0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDC,
	0xDE, 0xE0, 0xE2, 0xE5, 0xE7, 0xE9, 0xEB, 0xED, 0xEF, 0xF2, 0xF4, 0xF6, 0xF8, 0xFA, 0xFD, 0xFF
};

alignas(64) const uint8 Color::srgbDelinearizationTable[256] =
{
	0x00, 0x0D, 0x16, 0x1C, 0x22, 0x26, 0x2A, 0x2E, 0x32, 0x35, 0x38, 0x3B, 0x3D, 0x40, 0x42, 0x45,
	0x47, 0x49, 0x4B, 0x4D, 0x4F, 0x51, 0x53, 0x55, 0x56, 0x58, 0x5A, 0x5C, 0x5D, 0x5F, 0x60, 0x62,
	0x63, 0x65, 0x66, 0x68, 0x69, 0x6A, 0x6C, 0x6D, 0x6E, 0x70, 0x71, 0x72, 0x73, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7A, 0x7C, 0x7D, 0x7E, 0x7F, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88,
	0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x94, 0x95, 0x96, 0x97,
	0x98, 0x99, 0x9A, 0x9B, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0x9F, 0xA0, 0xA1, 0xA2, 0xA3, 0xA3, 0xA4,
	0xA5, 0xA6, 0xA7, 0xA7, 0xA8, 0xA9, 0xAA, 0xAA, 0xAB, 0xAC, 0xAD, 0xAD, 0xAE, 0xAF, 0xAF, 0xB0,
	0xB1, 0xB2, 0xB2, 0xB3, 0xB4, 0xB4, 0xB5, 0xB6, 0xB6, 0xB7, 0xB8, 0xB9, 0xB9, 0xBA, 0xBB, 0xBB,
	0xBC, 0xBD, 0xBD, 0xBE, 0xBE, 0xBF, 0xC0, 0xC0, 0xC1, 0xC2, 0xC2, 0xC3, 0xC4, 0xC4, 0xC5, 0xC5,
	0xC6, 0xC7, 0xC7, 0xC8, 0xC8, 0xC9, 0xCA, 0xCA, 0xCB, 0xCB, 0xCC, 0xCD, 0xCD, 0xCE, 0xCE, 0xCF,
	0xD0, 0xD0, 0xD1, 0xD1, 0xD2, 0xD2, 0xD3, 0xD4, 0xD4, 0xD5, 0xD5, 0xD6, 0xD6, 0xD7, 0xD7, 0xD8,
	0xD8, 0xD9, 0xDA, 0xDA, 0xDB, 0xDB, 0xDC, 0xDC, 0xDD, 0xDD, 0xDE, 0xDE, 0xDF, 0xDF, 0xE0, 0xE0,
	0xE1, 0xE2, 0xE2, 0xE3, 0xE3, 0xE4, 0xE4, 0xE5, 0xE5, 0xE6, 0xE6, 0xE7, 0xE7, 0xE8, 0xE8, 0xE9,
	0xE9, 0xEA, 0xEA, 0xEB, 0xEB, 0xEC, 0xEC, 0xED, 0xED, 0xEE, 0xEE, 0xEE, 0xEF, 0xEF, 0xF0, 0xF0,
	0xF1, 0xF1, 0xF2, 0xF2, 0xF3, 0xF3, 0xF4, 0xF4, 0xF5, 0xF5, 0xF6, 0xF6, 0xF6, 0xF7, 0xF7, 0xF8,
	0xF8, 0xF9, 0xF9, 0xFA, 0xFA, 0xFB, 0xFB, 0xFB, 0xFC, 0xFC, 0xFD, 0xFD, 0xFE, 0xFE, 0xFF, 0xFF
};

alignas(64) const float Color::srgbFloatLinearizationTable[256] =
{
	0.0F, 0.0003F, 0.0006F, 0.00091F, 0.001214F, 0.0015175F, 0.0018211F, 0.0021246F, 0.0024281F, 0.0027316F, 0.0030351F, 0.0033464F, 0.0036764F, 0.0040246F, 0.0043914F, 0.0047768F,
	0.0051814F, 0.0056053F, 0.0060487F, 0.006512F, 0.0069953F, 0.0074989F, 0.0080231F, 0.008568F, 0.009134F, 0.0097211F, 0.0103297F, 0.01096F, 0.0116121F, 0.0122864F, 0.0129829F, 0.013702F,
	0.0144437F, 0.0152084F, 0.0159962F, 0.0168073F, 0.0176419F, 0.0185002F, 0.0193823F, 0.0202884F, 0.0212188F, 0.0221738F, 0.0231533F, 0.0241575F, 0.0251867F, 0.0262411F, 0.0273208F, 0.0284259F,
	0.0295567F, 0.0307134F, 0.0318959F, 0.0331046F, 0.0343397F, 0.0356012F, 0.0368894F, 0.0382043F, 0.0395461F, 0.0409151F, 0.0423113F, 0.0437349F, 0.0451861F, 0.046665F, 0.0481717F, 0.0497064F,
	0.0512694F, 0.0528606F, 0.0544801F, 0.0561283F, 0.0578054F, 0.0595111F, 0.061246F, 0.06301F, 0.0648032F, 0.0666259F, 0.0684781F, 0.07036F, 0.0722718F, 0.0742135F, 0.0761853F, 0.0781873F,
	0.0802197F, 0.0822826F, 0.084376F, 0.0865004F, 0.0886555F, 0.0908416F, 0.0930589F, 0.0953074F, 0.0975873F, 0.0998986F, 0.1022416F, 0.1046164F, 0.107023F, 0.1094616F, 0.1119323F, 0.1144353F,
	0.1169705F, 0.1195383F, 0.1221387F, 0.1247717F, 0.1274377F, 0.1301363F, 0.1328682F, 0.1356333F, 0.1384315F, 0.1412632F, 0.1441284F, 0.1470272F, 0.1499596F, 0.152926F, 0.1559264F, 0.1589608F,
	0.1620292F, 0.1651321F, 0.1682693F, 0.171441F, 0.1746473F, 0.1778882F, 0.1811641F, 0.1844749F, 0.1878206F, 0.1912016F, 0.1946177F, 0.1980692F, 0.2015562F, 0.2050786F, 0.2086367F, 0.2122306F,
	0.2158604F, 0.2195261F, 0.2232278F, 0.2269659F, 0.23074F, 0.2345505F, 0.2383975F, 0.2422811F, 0.2462013F, 0.2501583F, 0.2541521F, 0.2581828F, 0.2622506F, 0.2663556F, 0.2704977F, 0.2746772F,
	0.2788943F, 0.2831487F, 0.2874408F, 0.2917706F, 0.2961382F, 0.3005437F, 0.3049873F, 0.3094688F, 0.3139886F, 0.3185467F, 0.3231432F, 0.3277781F, 0.3324515F, 0.3371635F, 0.3419144F, 0.346704F,
	0.3515325F, 0.3564001F, 0.3613067F, 0.3662526F, 0.3712377F, 0.3762621F, 0.381326F, 0.3864294F, 0.3915725F, 0.3967552F, 0.4019777F, 0.4072402F, 0.4125427F, 0.4178851F, 0.4232677F, 0.4286905F,
	0.4341536F, 0.4396572F, 0.4452011F, 0.4507857F, 0.4564111F, 0.462077F, 0.4677838F, 0.4735316F, 0.4793202F, 0.48515F, 0.4910207F, 0.4969329F, 0.5028865F, 0.5088814F, 0.5149177F, 0.5209956F,
	0.5271152F, 0.5332765F, 0.5394796F, 0.5457245F, 0.5520114F, 0.5583404F, 0.5647115F, 0.5711249F, 0.5775804F, 0.5840784F, 0.5906188F, 0.5972018F, 0.6038274F, 0.6104956F, 0.6172066F, 0.6239603F,
	0.6307572F, 0.6375969F, 0.6444797F, 0.6514056F, 0.6583749F, 0.6653873F, 0.6724431F, 0.6795424F, 0.6866854F, 0.6938719F, 0.7011021F, 0.7083759F, 0.7156937F, 0.7230552F, 0.7304608F, 0.7379105F,
	0.7454043F, 0.7529422F, 0.7605246F, 0.7681512F, 0.7758221F, 0.7835378F, 0.791298F, 0.7991029F, 0.8069523F, 0.8148466F, 0.8227857F, 0.83077F, 0.8387989F, 0.8468731F, 0.8549928F, 0.8631572F,
	0.8713669F, 0.8796223F, 0.8879232F, 0.8962693F, 0.9046614F, 0.9130986F, 0.9215819F, 0.9301109F, 0.9386857F, 0.9473063F, 0.9559732F, 0.9646861F, 0.9734454F, 0.9822505F, 0.9911019F, 1.0F
};


void ColorRGB::GetHexString(char *string) const
{
	int32 r = int32(red * 255.0F);
	int32 g = int32(green * 255.0F);
	int32 b = int32(blue * 255.0F);

	string[0] = hexDigit[(r >> 4) & 15];
	string[1] = hexDigit[r & 15];
	string[2] = hexDigit[(g >> 4) & 15];
	string[3] = hexDigit[g & 15];
	string[4] = hexDigit[(b >> 4) & 15];
	string[5] = hexDigit[b & 15];
	string[6] = 0;
}

ColorRGB& ColorRGB::SetHexString(const char *string)
{
	int32 rh = 15;
	int32 rl = 15;
	int32 gh = 15;
	int32 gl = 15;
	int32 bh = 15;
	int32 bl = 15;

	int32 k = string[0];
	if (k != 0)
	{
		rh = k - '0';
		if (rh > 9)
		{
			rh -= 7;
		}

		k = string[1];
		if (k != 0)
		{
			rl = k - '0';
			if (rl > 9)
			{
				rl -= 7;
			}

			k = string[2];
			if (k != 0)
			{
				gh = k - '0';
				if (gh > 9)
				{
					gh -= 7;
				}

				k = string[3];
				if (k != 0)
				{
					gl = k - '0';
					if (gl > 9)
					{
						gl -= 7;
					}

					k = string[4];
					if (k != 0)
					{
						bh = k - '0';
						if (bh > 9)
						{
							bh -= 7;
						}

						k = string[5];
						if (k != 0)
						{
							bl = k - '0';
							if (bl > 9)
							{
								bl -= 7;
							}
						}
					}
				}
			}
		}
	}

	red = float(((rh << 4) | rl) * 0.00392156862745F);
	green = float(((gh << 4) | gl) * 0.00392156862745F);
	blue = float(((bh << 4) | bl) * 0.00392156862745F);

	return (*this);
}


void ColorRGBA::GetHexString(char *string) const
{
	int32 r = int32(red * 255.0F);
	int32 g = int32(green * 255.0F);
	int32 b = int32(blue * 255.0F);
	int32 a = int32(alpha * 255.0F);

	string[0] = hexDigit[(r >> 4) & 15];
	string[1] = hexDigit[r & 15];
	string[2] = hexDigit[(g >> 4) & 15];
	string[3] = hexDigit[g & 15];
	string[4] = hexDigit[(b >> 4) & 15];
	string[5] = hexDigit[b & 15];
	string[6] = hexDigit[(a >> 4) & 15];
	string[7] = hexDigit[a & 15];
	string[8] = 0;
}

ColorRGBA& ColorRGBA::SetHexString(const char *string)
{
	int32 rh = 15;
	int32 rl = 15;
	int32 gh = 15;
	int32 gl = 15;
	int32 bh = 15;
	int32 bl = 15;
	int32 ah = 15;
	int32 al = 15;

	int32 k = string[0];
	if (k != 0)
	{
		rh = k - '0';
		if (rh > 9)
		{
			rh -= 7;
		}

		k = string[1];
		if (k != 0)
		{
			rl = k - '0';
			if (rl > 9)
			{
				rl -= 7;
			}

			k = string[2];
			if (k != 0)
			{
				gh = k - '0';
				if (gh > 9)
				{
					gh -= 7;
				}

				k = string[3];
				if (k != 0)
				{
					gl = k - '0';
					if (gl > 9)
					{
						gl -= 7;
					}

					k = string[4];
					if (k != 0)
					{
						bh = k - '0';
						if (bh > 9)
						{
							bh -= 7;
						}

						k = string[5];
						if (k != 0)
						{
							bl = k - '0';
							if (bl > 9)
							{
								bl -= 7;
							}

							k = string[6];
							if (k != 0)
							{
								ah = k - '0';
								if (ah > 9)
								{
									ah -= 7;
								}

								k = string[7];
								if (k != 0)
								{
									al = k - '0';
									if (al > 9)
									{
										al -= 7;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	red = float(((rh << 4) | rl) * 0.00392156862745F);
	green = float(((gh << 4) | gl) * 0.00392156862745F);
	blue = float(((bh << 4) | bl) * 0.00392156862745F);
	alpha = float(((ah << 4) | al) * 0.00392156862745F);

	return (*this);
}


float Color::Linearize(float color)
{
	return ((color > 0.04045F) ? Pow((color + 0.055F) * (1.0F / 1.055F), 2.4F) : color * (1.0F / 12.92F));
}

float Color::Delinearize(float color)
{
	return ((color < 0.0031308F) ? color * 12.92F : Pow(color, 1.0F / 2.4F) * 1.055F - 0.055F);
}

ColorRGB Color::Linearize(const ColorRGB& color)
{
	float r = Linearize(color.red);
	float g = Linearize(color.green);
	float b = Linearize(color.blue);
	return (ColorRGB(r, g, b));
}

ColorRGB Color::Delinearize(const ColorRGB& color)
{
	float r = Delinearize(color.red);
	float g = Delinearize(color.green);
	float b = Delinearize(color.blue);
	return (ColorRGB(r, g, b));
}

ColorRGBA Color::Linearize(const ColorRGBA& color)
{
	float r = Linearize(color.red);
	float g = Linearize(color.green);
	float b = Linearize(color.blue);
	return (ColorRGBA(r, g, b, color.alpha));
}

ColorRGBA Color::Delinearize(const ColorRGBA& color)
{
	float r = Delinearize(color.red);
	float g = Delinearize(color.green);
	float b = Delinearize(color.blue);
	return (ColorRGBA(r, g, b, color.alpha));
}

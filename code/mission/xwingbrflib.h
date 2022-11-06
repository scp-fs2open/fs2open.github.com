#pragma once
#include <map>
#include <array>

std::map<int, std::vector<std::string> > create_animation_map();


#pragma pack(push, 1)

struct xwi_brf_header {
	short version;
	short icon_count;
	short coordinate_set_count;
};

#pragma pack(pop)


struct xwi_brf_ship {
	float coordinates_x, coordinates_y, coordinates_z;
	short icon_type;
	short iff;
	short wave_size;
	short num_waves;  // wave respawn count
	std::string designation;  //	self.ships[i]['designation'] = self._readFixedString(16)
	std::string cargo;  //	self.ships[i]['cargo'] = self._readFixedString(16)
	std::string alt_cargo;  // special ship's cargo
	short special_ship_in_wave;  // 0 ship one, 1 ship two, ...
	short rotation_x;  // Degrees around X-axis = value/256*360
	short rotation_y;  // Degrees around Y-axis = value/256*360
	short rotation_z;  // Degrees around Z-axis = value/256*360
};

class XWingBriefing
{
public:

	static bool load(XWingBriefing *b, const char *data);

	std::string message1;
	xwi_brf_header header;
	std::vector<xwi_brf_ship> ships;
};


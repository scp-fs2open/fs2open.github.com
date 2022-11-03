#pragma once
#include <map>
#include <array>
#include "globalincs/pstypes.h"



std::map<int, std::vector<std::string> > create_animation_map()
{
	std::map<int, std::vector<std::string> > m;
	m[1] = { "wait_for_click" };
	m[10] = { "clear_text" };
	m[11] = { "show_title", "i", "text_id" };
	m[12] = { "show_main", "i", "text_id" };
	m[15] = { "center_map", "f", "x", "f", "y" };
	m[16] = { "zoom_map", "f", "x", "f", "y" };
	m[21] = { "clear_boxes" };
	m[22] = { "box_1", "i", "ship_id" };
	m[23] = { "box_2", "i", "ship_id" };
	m[24] = { "box_3", "i", "ship_id" };
	m[25] = { "box_4", "i", "ship_id" };
	m[26] = { "clear_tags" };
	m[27] = { "tag_1", "i", "tag_id", "f", "x", "f", "y" };
	m[28] = { "tag_2", "i", "tag_id", "f", "x", "f", "y" };
	m[29] = { "tag_3", "i", "tag_id", "f", "x", "f", "y" };
	m[30] = { "tag_4", "i", "tag_id", "f", "x", "f", "y" };
	return m;
}
const std::map<int, std::vector<std::string> > ANIMATION_COMMANDS = create_animation_map();




#pragma pack(push, 1)

struct xwi_brf_header {
	short version;
	short icon_count;
	short coordinate_set_count;
};

#pragma pack(pop)


struct xwi_brf_ship {
	vec3d coordinates;
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

	XWingBriefing();
	~XWingBriefing();

	static 	XWingBriefing *load(const char *data);

	std::string message1;
	xwi_brf_header header;
	std::vector<xwi_brf_ship> ships;
};


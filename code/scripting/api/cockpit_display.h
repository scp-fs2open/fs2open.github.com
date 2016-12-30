#pragma once

#include "scripting/ade_api.h"
#include "ship/ship.h"


namespace scripting {
namespace api {


class cockpit_disp_info_h
{
 private:
	ship_info *m_sip;
	size_t m_display_num;

 public:
	cockpit_disp_info_h();
	cockpit_disp_info_h(ship_info *sip, size_t display_num);

	cockpit_display_info *Get();

	bool isValid();
};

DECLARE_ADE_OBJ(l_DisplayInfo, cockpit_disp_info_h);


class cockpit_display_h
{
 private:
	int obj_num;
	object *m_objp;
	size_t m_display_num;

 public:
	cockpit_display_h();
	cockpit_display_h(object *objp, size_t display_num);

	cockpit_display *Get();

	size_t GetId();

	bool isValid();
};

DECLARE_ADE_OBJ(l_CockpitDisplay, cockpit_display_h);


//**********HANDLE: CockpitDisplayArray
class cockpit_displays_info_h
{
 private:
	int m_ship_info_idx;
 public:
	cockpit_displays_info_h();
	explicit cockpit_displays_info_h(int ship_info_idx);

	ship_info *Get();

	bool isValid();
};
DECLARE_ADE_OBJ(l_CockpitDisplayInfos, cockpit_displays_info_h);

//**********HANDLE: CockpitDisplayArray
class cockpit_displays_h
{
 private:
	object *m_objp;
 public:
	cockpit_displays_h();
	explicit cockpit_displays_h(object *objp);

	bool isValid();
};
DECLARE_ADE_OBJ(l_CockpitDisplays, cockpit_displays_h);


}
}


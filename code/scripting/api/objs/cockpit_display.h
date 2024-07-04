#pragma once

#include "scripting/ade_api.h"
#include "ship/ship.h"


namespace scripting {
namespace api {


class cockpit_disp_info_h
{
 private:
	int m_ship_info_idx;
	size_t m_display_num;

 public:
	cockpit_disp_info_h();
	explicit cockpit_disp_info_h(int ship_info_idx, size_t display_num);

	cockpit_display_info *Get();

	bool isValid() const;
};

DECLARE_ADE_OBJ(l_DisplayInfo, cockpit_disp_info_h);


class cockpit_display_h
{
 private:
	int m_obj_num;
	size_t m_display_num;

 public:
	cockpit_display_h();
	explicit cockpit_display_h(int obj_num, size_t display_num);

	cockpit_display *Get();

	size_t GetId() const;

	bool isValid() const;
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

	const ship_info *GetShipInfoPtr() const;
	int GetShipInfoIndex() const;

	bool isValid() const;
};
DECLARE_ADE_OBJ(l_CockpitDisplayInfos, cockpit_displays_info_h);

//**********HANDLE: CockpitDisplayArray
class cockpit_displays_h
{
 private:
	int m_obj_num;
 public:
	cockpit_displays_h();
	explicit cockpit_displays_h(int obj_num);

	bool isValid() const;
};
DECLARE_ADE_OBJ(l_CockpitDisplays, cockpit_displays_h);


}
}


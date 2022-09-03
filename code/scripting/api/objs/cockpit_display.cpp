//
//

#include "cockpit_display.h"
#include "texture.h"


namespace scripting {
namespace api {

cockpit_disp_info_h::cockpit_disp_info_h() : m_sip( NULL ), m_display_num( INVALID_ID ) {}
cockpit_disp_info_h::cockpit_disp_info_h(ship_info* sip, size_t display_num) {
	this->m_sip = sip;
	this->m_display_num = display_num;
}
cockpit_display_info* cockpit_disp_info_h::Get() {
	if (!this->isValid())
		return NULL;

	return &m_sip->displays[m_display_num];
}
bool cockpit_disp_info_h::isValid() {
	if (m_sip == NULL)
	{
		return false;
	}

	if (m_display_num == INVALID_ID)
	{
		return false;
	}

	if ( m_display_num >= m_sip->displays.size())
	{
		return false;
	}

	if (!m_sip->hud_enabled)
	{
		return false;
	}

	return true;
}

ADE_OBJ(l_DisplayInfo, cockpit_disp_info_h, "display_info", "Ship cockpit display information handle");

ADE_FUNC(getName, l_DisplayInfo, NULL, "Gets the name of this cockpit display as defined in ships.tbl", "string", "Name string or empty string on error")
{
	cockpit_disp_info_h *cdh = NULL;

	if (!ade_get_args(L, "o", l_DisplayInfo.GetPtr(&cdh)))
		return ade_set_error(L, "s", "");

	if (!cdh->isValid())
		return ade_set_error(L, "s", "");

	cockpit_display_info *cdi = cdh->Get();

	if (cdi == NULL)
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", cdi->name);
}

ADE_FUNC(getFileName, l_DisplayInfo, NULL, "Gets the file name of the target texture of this cockpit display", "string", "Texture name string or empty string on error")
{
	cockpit_disp_info_h *cdh = NULL;

	if (!ade_get_args(L, "o", l_DisplayInfo.GetPtr(&cdh)))
		return ade_set_error(L, "s", "");

	if (!cdh->isValid())
		return ade_set_error(L, "s", "");

	cockpit_display_info *cdi = cdh->Get();

	if (cdi == NULL)
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", cdi->filename);
}

ADE_FUNC(getForegroundFileName, l_DisplayInfo, NULL, "Gets the file name of the foreground texture of this cockpit display", "string", "Foreground texture name string or nil if texture is not set or on error")
{
	cockpit_disp_info_h *cdh = NULL;

	if (!ade_get_args(L, "o", l_DisplayInfo.GetPtr(&cdh)))
		return ADE_RETURN_NIL;

	if (!cdh->isValid())
		return ADE_RETURN_NIL;

	cockpit_display_info *cdi = cdh->Get();

	if (cdi == NULL)
		return ADE_RETURN_NIL;

	if (cdi->fg_filename[0] == 0)
		return ADE_RETURN_NIL;

	return ade_set_args(L, "s", cdi->fg_filename);
}

ADE_FUNC(getBackgroundFileName, l_DisplayInfo, NULL, "Gets the file name of the background texture of this cockpit display", "string", "Background texture name string or nil if texture is not set or on error")
{
	cockpit_disp_info_h *cdh = NULL;

	if (!ade_get_args(L, "o", l_DisplayInfo.GetPtr(&cdh)))
		return ADE_RETURN_NIL;

	if (!cdh->isValid())
		return ADE_RETURN_NIL;

	cockpit_display_info *cdi = cdh->Get();

	if (cdi == NULL)
		return ADE_RETURN_NIL;

	if (cdi->bg_filename[0] == 0)
		return ADE_RETURN_NIL;

	return ade_set_args(L, "s", cdi->bg_filename);
}

ADE_FUNC(getSize,
	l_DisplayInfo,
	nullptr,
	"Gets the size of this cockpit display",
	"number, number",
	"Width and height of the display or -1, -1 on error")
{
	cockpit_disp_info_h *cdh = NULL;

	if (!ade_get_args(L, "o", l_DisplayInfo.GetPtr(&cdh)))
		return ade_set_error(L, "ii", -1, -1);

	if (!cdh->isValid())
		return ade_set_error(L, "ii", -1, -1);

	cockpit_display_info *cdi = cdh->Get();

	if (cdi == NULL)
		return ade_set_error(L, "ii", -1, -1);

	return ade_set_args(L, "ii", cdi->size[0], cdi->size[1]);
}

ADE_FUNC(getOffset,
	l_DisplayInfo,
	nullptr,
	"Gets the offset of this cockpit display",
	"number, number",
	"x and y offset of the display or -1, -1 on error")
{
	cockpit_disp_info_h *cdh = NULL;

	if (!ade_get_args(L, "o", l_DisplayInfo.GetPtr(&cdh)))
		return ade_set_error(L, "ii", -1, -1);

	if (!cdh->isValid())
		return ade_set_error(L, "ii", -1, -1);

	cockpit_display_info *cdi = cdh->Get();

	if (cdi == NULL)
		return ade_set_error(L, "ii", -1, -1);

	return ade_set_args(L, "ii", cdi->offset[0], cdi->offset[1]);
}

ADE_FUNC(isValid, l_DisplayInfo, NULL, "Detects whether this handle is valid", "boolean", "true if valid false otherwise")
{
	cockpit_disp_info_h *cdh = NULL;

	if (!ade_get_args(L, "o", l_DisplayInfo.GetPtr(&cdh)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", cdh->isValid());
}


cockpit_display_h::cockpit_display_h() : obj_num( -1 ), m_objp( NULL ), m_display_num( INVALID_ID ) {}
cockpit_display_h::cockpit_display_h(object* objp, size_t display_num) {
	this->obj_num = OBJ_INDEX(objp);
	this->m_objp = objp;

	this->m_display_num = display_num;
}
cockpit_display* cockpit_display_h::Get() {
	if (!isValid())
	{
		return NULL;
	}

	return &Player_displays[m_display_num];
}
size_t cockpit_display_h::GetId() {
	if (!isValid())
	{
		return INVALID_ID;
	}

	return m_display_num;
}
bool cockpit_display_h::isValid() {
	if (obj_num < 0 || obj_num >= MAX_OBJECTS)
	{
		return false;
	}

	if (m_objp == NULL || OBJ_INDEX(m_objp) != obj_num)
	{
		return false;
	}

	// Only player has cockpit displays
	if (m_objp != Player_obj)
	{
		return false;
	}

	if (m_display_num == INVALID_ID)
	{
		return false;
	}

	if (m_display_num >= Player_displays.size())
	{
		return false;
	}

	return true;
}


//**********HANDLE: Cockpit Display
ADE_OBJ(l_CockpitDisplay, cockpit_display_h, "display", "Cockpit display handle");

ADE_FUNC(startRendering, l_CockpitDisplay, "[boolean setClip = true]", "Starts rendering to this cockpit display. That means if you get a valid texture handle from this function then the rendering system is ready to do a render to texture. If setClip is true then the clipping region will be set to the region of the cockpit display.<br><b>Important:</b> You have to call stopRendering after you're done or this render target will never be released!", "texture", "texture handle that is being drawn to or invalid handle on error")
{
	cockpit_display_h *cdh = NULL;
	bool setClip = true;

	if (!ade_get_args(L, "o|b", l_CockpitDisplay.GetPtr(&cdh), &setClip))
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	if (!cdh->isValid())
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	int bm_handle = ship_start_render_cockpit_display(cdh->GetId());

	if (bm_is_valid(bm_handle) && setClip)
	{
		cockpit_display *cd = cdh->Get();
		gr_set_clip(cd->offset[0], cd->offset[1], cd->size[0], cd->size[1], GR_RESIZE_NONE);
	}

	return ade_set_args(L, "o", l_Texture.Set(texture_h(bm_handle)));
}

ADE_FUNC(stopRendering, l_CockpitDisplay, NULL, "Stops rendering to this cockpit display", "boolean", "true if successfull, false otherwise")
{
	cockpit_display_h *cdh = NULL;

	if (!ade_get_args(L, "o", l_CockpitDisplay.GetPtr(&cdh)))
		return ADE_RETURN_FALSE;

	if (!cdh->isValid())
		return ADE_RETURN_FALSE;

	ship_end_render_cockpit_display(cdh->GetId());
	gr_reset_clip();

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getBackgroundTexture, l_CockpitDisplay, NULL, "Gets the background texture handle of this cockpit display", "texture", "texture handle or invalid handle if no background texture or an error happened")
{
	cockpit_display_h *cdh = NULL;

	if (!ade_get_args(L, "o", l_CockpitDisplay.GetPtr(&cdh)))
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	if (!cdh->isValid())
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	cockpit_display *cd = cdh->Get();

	if (cd == NULL)
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	return ade_set_args(L, "o", l_Texture.Set(texture_h(cd->background)));
}

ADE_FUNC(getForegroundTexture, l_CockpitDisplay, NULL, "Gets the foreground texture handle of this cockpit display<br>"
	"<b>Important:</b> If you want to do render to texture then you have to use startRendering/stopRendering", "texture", "texture handle or invalid handle if no foreground texture or an error happened")
{
	cockpit_display_h *cdh = NULL;

	if (!ade_get_args(L, "o", l_CockpitDisplay.GetPtr(&cdh)))
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	if (!cdh->isValid())
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	cockpit_display *cd = cdh->Get();

	if (cd == NULL)
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	return ade_set_args(L, "o", l_Texture.Set(texture_h(cd->foreground)));
}

ADE_FUNC(getSize,
	l_CockpitDisplay,
	nullptr,
	"Gets the size of this cockpit display",
	"number, number",
	"Width and height of the display or -1, -1 on error")
{
	cockpit_display_h *cdh = NULL;

	if (!ade_get_args(L, "o", l_CockpitDisplay.GetPtr(&cdh)))
		return ade_set_error(L, "ii", -1, -1);

	if (!cdh->isValid())
		return ade_set_error(L, "ii", -1, -1);

	cockpit_display *cd = cdh->Get();

	if (cd == NULL)
		return ade_set_error(L, "ii", -1, -1);

	return ade_set_args(L, "ii", cd->size[0], cd->size[1]);
}

ADE_FUNC(getOffset,
	l_CockpitDisplay,
	nullptr,
	"Gets the offset of this cockpit display",
	"number, number",
	"x and y offset of the display or -1, -1 on error")
{
	cockpit_display_h *cdh = NULL;

	if (!ade_get_args(L, "o", l_CockpitDisplay.GetPtr(&cdh)))
		return ade_set_error(L, "ii", -1, -1);

	if (!cdh->isValid())
		return ade_set_error(L, "ii", -1, -1);

	cockpit_display *cd = cdh->Get();

	if (cd == NULL)
		return ade_set_error(L, "ii", -1, -1);

	return ade_set_args(L, "ii", cd->offset[0], cd->offset[1]);
}

ADE_FUNC(isValid, l_CockpitDisplay, NULL, "Detects whether this handle is valid or not", "boolean", "true if valid, false otherwise")
{
	cockpit_display_h *cdh = NULL;

	if (!ade_get_args(L, "o", l_CockpitDisplay.GetPtr(&cdh)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", cdh->isValid());
}


cockpit_displays_info_h::cockpit_displays_info_h() : m_ship_info_idx( -1 ) {}
cockpit_displays_info_h::cockpit_displays_info_h(int ship_info_idx) {
	this->m_ship_info_idx = ship_info_idx;
}
ship_info* cockpit_displays_info_h::Get() {
	if (!isValid())
		return NULL;

	return &Ship_info[m_ship_info_idx];
}
bool cockpit_displays_info_h::isValid() {
	if (m_ship_info_idx < 0 || m_ship_info_idx >= ship_info_size())
	{
		return false;
	}

	if (!Ship_info[m_ship_info_idx].hud_enabled)
	{
		return false;
	}

	return true;
}


//**********HANDLE: CockpitDisplayArray
ADE_OBJ(l_CockpitDisplayInfos, cockpit_displays_info_h, "cockpitdisplays", "Array of cockpit display information");

ADE_FUNC(__len, l_CockpitDisplayInfos, NULL, "Number of cockpit displays for this ship class", "number", "number of cockpit displays or -1 on error")
{
	cockpit_displays_info_h *cdih = NULL;
	if (!ade_get_args(L, "o", l_CockpitDisplayInfos.GetPtr(&cdih)))
		return ade_set_error(L, "i", -1);

	if (!cdih->isValid())
		return ade_set_error(L, "i", -1);

	return ade_set_args(L, "i", (int) cdih->Get()->displays.size());
}

ADE_INDEXER(l_CockpitDisplayInfos, "number/string",
            "Returns the handle at the requested index or the handle with the specified name", "display_info",
            "display handle or invalid handle on error")
{
	if (lua_isnumber(L, 2))
	{
		cockpit_displays_info_h *cdih = NULL;
		int index = -1;

		if (!ade_get_args(L, "oi", l_CockpitDisplayInfos.GetPtr(&cdih), &index))
		{
			return ade_set_error(L, "o", l_DisplayInfo.Set(cockpit_disp_info_h()));
		}

		if (index < 0)
		{
			return ade_set_error(L, "o", l_DisplayInfo.Set(cockpit_disp_info_h()));
		}

		index--; // Lua -> C/C++

		return ade_set_args(L, "o", l_DisplayInfo.Set(cockpit_disp_info_h(cdih->Get(), index)));
	}
	else
	{
		cockpit_displays_info_h *cdih = NULL;
		const char* name              = nullptr;

		if (!ade_get_args(L, "os", l_CockpitDisplayInfos.GetPtr(&cdih), &name))
		{
			return ade_set_error(L, "o", l_DisplayInfo.Set(cockpit_disp_info_h()));
		}

		if (!cdih->isValid())
		{
			return ade_set_error(L, "o", l_DisplayInfo.Set(cockpit_disp_info_h()));
		}

		if (name == NULL)
		{
			return ade_set_error(L, "o", l_DisplayInfo.Set(cockpit_disp_info_h()));
		}

		ship_info *sip = cdih->Get();

		if (!sip->hud_enabled)
		{
			return ade_set_error(L, "o", l_DisplayInfo.Set(cockpit_disp_info_h()));
		}

		size_t index = 0;

		for (SCP_vector<cockpit_display_info>::iterator iter = sip->displays.begin(); iter != sip->displays.end(); ++iter)
		{
			if (!strcmp(name, iter->name))
			{
				break;
			}
			else
			{
				index++;
			}
		}

		if (index == sip->displays.size())
		{
			LuaError(L, "Couldn't find cockpit display info with name \"%s\"", name);
			return ade_set_error(L, "o", l_DisplayInfo.Set(cockpit_disp_info_h()));
		}

		return ade_set_args(L, "o", l_DisplayInfo.Set(cockpit_disp_info_h(cdih->Get(), index)));
	}
}

ADE_FUNC(isValid, l_CockpitDisplayInfos, NULL, "Detects whether this handle is valid", "boolean", "true if valid, false otehrwise")
{
	cockpit_displays_info_h *cdih = NULL;
	if (!ade_get_args(L, "o", l_CockpitDisplayInfos.GetPtr(&cdih)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", cdih->isValid());
}


cockpit_displays_h::cockpit_displays_h() : m_objp( NULL ) {}
cockpit_displays_h::cockpit_displays_h(object* objp) {
	this->m_objp = objp;
}
bool cockpit_displays_h::isValid() {
	if (m_objp == NULL)
	{
		return false;
	}

	if (m_objp != Player_obj)
	{
		return false;
	}

	if ( Ship_info[Player_ship->ship_info_index].cockpit_model_num < 0 ) {
		return false;
	}

	if ( Player_cockpit_textures == NULL ) {
		return false;
	}

	return true;
}


//**********HANDLE: CockpitDisplayArray
ADE_OBJ(l_CockpitDisplays, cockpit_displays_h, "displays", "Player cockpit displays array handle");

ADE_FUNC(__len, l_CockpitDisplays, NULL, "Gets the number of cockpit displays for the player ship", "number", "number of displays or -1 on error")
{
	cockpit_displays_h *cdh = NULL;
	if(!ade_get_args(L, "o", l_CockpitDisplays.GetPtr(&cdh)))
		return ade_set_error(L, "i", -1);

	if (!cdh->isValid())
		return ade_set_error(L, "i", -1);

	return ade_set_args(L, "i", (int) Player_displays.size());
}

ADE_INDEXER(l_CockpitDisplays, "number/string", "Gets a cockpit display from the present player displays by either the index or the name of the display", "display", "Display handle or invalid handle on error")
{
	if (lua_isnumber(L, 2))
	{
		cockpit_displays_h *cdh = NULL;
		int index = -1;

		if (!ade_get_args(L, "oi", l_CockpitDisplays.GetPtr(&cdh), &index))
		{
			return ade_set_error(L, "o", l_CockpitDisplay.Set(cockpit_display_h()));
		}

		if (index < 0)
		{
			return ade_set_error(L, "o", l_CockpitDisplay.Set(cockpit_display_h()));
		}

		index--; // Lua -> C/C++

		return ade_set_args(L, "o", l_CockpitDisplay.Set(cockpit_display_h(Player_obj, index)));
	}
	else
	{
		cockpit_displays_h *cdh = NULL;
		const char* name        = nullptr;

		if (!ade_get_args(L, "os", l_CockpitDisplays.GetPtr(&cdh), &name))
		{
			return ade_set_error(L, "o", l_CockpitDisplay.Set(cockpit_display_h()));
		}

		if (!cdh->isValid())
		{
			return ade_set_error(L, "o", l_CockpitDisplay.Set(cockpit_display_h()));
		}

		if (name == NULL)
		{
			return ade_set_error(L, "o", l_CockpitDisplay.Set(cockpit_display_h()));
		}

		size_t index = 0;

		for (SCP_vector<cockpit_display>::iterator iter = Player_displays.begin(); iter != Player_displays.end(); ++iter)
		{
			if (!strcmp(name, iter->name))
			{
				break;
			}
			else
			{
				index++;
			}
		}

		if (index == Player_displays.size())
		{
			LuaError(L, "Couldn't find cockpit display info with name \"%s\"", name);
			return ade_set_error(L, "o", l_CockpitDisplay.Set(cockpit_display_h()));
		}

		return ade_set_args(L, "o", l_CockpitDisplay.Set(cockpit_display_h(Player_obj, index)));
	}
}

ADE_FUNC(isValid, l_CockpitDisplays, NULL, "Detects whether this handle is valid or not", "boolean", "true if valid, false otherwise")
{
	cockpit_displays_h *cdh = NULL;
	if(!ade_get_args(L, "o", l_CockpitDisplays.GetPtr(&cdh)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", cdh->isValid());
}

}
}


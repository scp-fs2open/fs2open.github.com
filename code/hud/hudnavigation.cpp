// HUDNavigation.cpp
// Derek Meek
// 4-30-2004



#include "hud/hudnavigation.h"
#include "autopilot/autopilot.h"
#include "hud/hudtarget.h"
#include "ship/ship.h"
#include "object/object.h"
#include "render/3d.h"
#include "hud/hud.h"
#include "hud/hudbrackets.h"
#include "hud/hudtargetbox.h"


extern void hud_target_show_dist_on_bracket(int x, int y, float distance, int font_num);
extern void draw_brackets_square_quick(int x1, int y1, int x2, int y2, int thick);

/**
 * Draws the Navigation stuff on the HUD
 */
void hud_draw_navigation()
{
	if (CurrentNav != -1 && Navs[CurrentNav].flags & NP_VALIDTYPE && !(Navs[CurrentNav].flags & NP_NOSELECT))
	{
		int in_cockpit;
		if (!(Viewer_mode & (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY)) )
			in_cockpit = 1;
		else
			in_cockpit = 0;

		vertex target_point;	// temp vertex used to find screen position for 3-D object;
		vec3d *target_pos = Navs[CurrentNav].GetPosition();

		color NavColor;
        
        memset(&target_point, 0, sizeof(target_point));

		unsigned int alpha = HUD_COLOR_ALPHA_MAX * 16;

		if (Navs[CurrentNav].flags & NP_VISITED)
			gr_init_alphacolor( &NavColor, 0xFF, 0xFF, 0x00, alpha);
		else
			gr_init_alphacolor( &NavColor, 0x80, 0x80, 0xff, alpha);

		g3_rotate_vertex(&target_point, target_pos);
		g3_project_vertex(&target_point);

		if ( in_cockpit )
			hud_target_add_display_list(NULL, &target_point, target_pos, 0, &NavColor, Navs[CurrentNav].m_NavName, TARGET_DISPLAY_DIST);
	}
}

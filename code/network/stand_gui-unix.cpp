
#ifndef WIN32	// Goober5000

#include "globalincs/pstypes.h"
#include "network/stand_gui.h"

void std_add_ban(char *name)
{
	STUB_FUNCTION;
}

void std_add_chat_text(char *text, int player_index, int add_id)
{
	STUB_FUNCTION;
}

void std_add_player(net_player *p)
{
	STUB_FUNCTION;
}

int std_connect_set_connect_count()
{
	STUB_FUNCTION;
	
	return 0;
}

void std_connect_set_gamename(char *name)
{
	STUB_FUNCTION;
}

void std_connect_set_host_connect_status()
{
	STUB_FUNCTION;
}

void std_create_gen_dialog(char *title)
{
	STUB_FUNCTION;
}

void std_debug_set_standalone_state_string(char *str)
{
	STUB_FUNCTION;
}

void std_debug_multilog_add_line(const char *str)
{
	STUB_FUNCTION;
}

void std_destroy_gen_dialog()
{
	STUB_FUNCTION;
}

void std_do_gui_frame()
{
	STUB_FUNCTION;
}

void std_gen_set_text(char *str, int field_num)
{
	STUB_FUNCTION;
}

int std_gen_is_active()
{
	return 0;
}

void std_init_standalone()
{
	STUB_FUNCTION;
}

int std_is_host_passwd()
{
	return 0;
}

void std_multi_add_goals()
{
	STUB_FUNCTION;
}

void std_multi_set_standalone_mission_name(char *mission_name)
{
	STUB_FUNCTION;
}

void std_multi_set_standalone_missiontime(float mission_time)
{
	STUB_FUNCTION;
}

void std_multi_setup_goal_tree()
{
	STUB_FUNCTION;
}

void std_multi_update_goals()
{
	STUB_FUNCTION;
}

void std_multi_update_netgame_info_controls()
{
	STUB_FUNCTION;
}

int std_player_is_banned(char *name)
{
	return 0;
}

int std_remove_player(net_player *p)
{
	STUB_FUNCTION;
	
	return 0;
}

void std_reset_standalone_gui()
{
	STUB_FUNCTION;
}

void std_reset_timestamps()
{
	STUB_FUNCTION;
}

void std_set_standalone_fps(float fps)
{
	STUB_FUNCTION;
}

void std_update_player_ping(net_player *p)
{
	STUB_FUNCTION;
}


#endif		// Goober5000 - #ifndef WIN32

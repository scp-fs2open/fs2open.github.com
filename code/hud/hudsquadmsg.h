/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _HUD_SQUADMSG
#define _HUD_SQUADMSG

#include "network/multi.h"
#include "hud/hud.h"

#define SM_MODE_TYPE_SELECT			1		//am I going to message a ship or a wing
#define SM_MODE_SHIP_SELECT			2		//choosing actual ship
#define SM_MODE_WING_SELECT			3		//choosing actual wing
#define SM_MODE_SHIP_COMMAND			4		//which command to send to a ship
#define SM_MODE_WING_COMMAND			5		//which command to send to a wing
#define SM_MODE_REINFORCEMENTS		6		//call for reinforcements
#define SM_MODE_REPAIR_REARM			7		//repair/rearm player ship
#define SM_MODE_REPAIR_REARM_ABORT	8		//abort repair/rearm of player ship
#define SM_MODE_ALL_FIGHTERS			9		//message all fighters/bombers

// define for trapping messages send to "all fighters"
#define MESSAGE_ALL_FIGHTERS		-999

class object;

// defines for messages that can be sent from the player.  Defined at bitfields so that we can enable
// and disable messages on a message by message basis

#define NUM_COMM_ORDER_ITEMS		16

#define ATTACK_TARGET_ITEM			(1<<0)
#define DISABLE_TARGET_ITEM			(1<<1)
#define DISARM_TARGET_ITEM			(1<<2)
#define PROTECT_TARGET_ITEM			(1<<3)
#define IGNORE_TARGET_ITEM			(1<<4)
#define FORMATION_ITEM				(1<<5)
#define COVER_ME_ITEM				(1<<6)
#define ENGAGE_ENEMY_ITEM			(1<<7)
#define CAPTURE_TARGET_ITEM			(1<<8)

// the next are for the support ship only
#define REARM_REPAIR_ME_ITEM		(1<<9)
#define ABORT_REARM_REPAIR_ITEM		(1<<10)
#define STAY_NEAR_ME_ITEM			(1<<11)
#define STAY_NEAR_TARGET_ITEM		(1<<12)
#define KEEP_SAFE_DIST_ITEM			(1<<13)

// next item for all ships again -- to try to preserve relative order within the message menu
#define DEPART_ITEM					(1<<14)

// out of order, but it was this way in the original source
#define DISABLE_SUBSYSTEM_ITEM		(1<<15)

// used for Message box gauge
#define NUM_MBOX_FRAMES		3

// data structure to hold character string of commands for comm menu
typedef struct comm_order {
	char name[NAME_LENGTH];
	int item;
} comm_order;

typedef struct sexp_com_order{ 
	char *name; 
	int xstring; 
	int item; 
}sexp_com_order;

extern comm_order Comm_orders[];
extern sexp_com_order Sexp_comm_orders[];

// following defines are the set of possible commands that can be given to a ship.  A mission designer
// might not allow some messages

//WMC - Formerly FIGHTER_MESSAGES
#define DEFAULT_MESSAGES	(ATTACK_TARGET_ITEM | DISABLE_TARGET_ITEM | DISARM_TARGET_ITEM | PROTECT_TARGET_ITEM | IGNORE_TARGET_ITEM | FORMATION_ITEM | COVER_ME_ITEM | ENGAGE_ENEMY_ITEM | DEPART_ITEM | DISABLE_SUBSYSTEM_ITEM)
/*
#define BOMBER_MESSAGES		FIGHTER_MESSAGES			// bombers can do the same things as fighters

#define TRANSPORT_MESSAGES	(ATTACK_TARGET_ITEM | CAPTURE_TARGET_ITEM | DEPART_ITEM )
#define FREIGHTER_MESSAGES	TRANSPORT_MESSAGES		// freighters can do the same things as transports

#define CRUISER_MESSAGES	(ATTACK_TARGET_ITEM | DEPART_ITEM)

#define CAPITAL_MESSAGES	(DEPART_ITEM)				// can't order capitals to do much!!!!

#define SUPERCAP_MESSAGES	(0)							// supercaps ignore you :p

#define SUPPORT_MESSAGES	(REARM_REPAIR_ME_ITEM | ABORT_REARM_REPAIR_ITEM | STAY_NEAR_ME_ITEM | STAY_NEAR_TARGET_ITEM | KEEP_SAFE_DIST_ITEM | DEPART_ITEM )
*/
// these messages require an active target.  They are also the set of messages
// which cannot be given to a ship when the target is on the same team, or the target
// is not a ship.
#define ENEMY_TARGET_MESSAGES		(ATTACK_TARGET_ITEM | DISABLE_TARGET_ITEM | DISARM_TARGET_ITEM | IGNORE_TARGET_ITEM | STAY_NEAR_TARGET_ITEM | CAPTURE_TARGET_ITEM | DISABLE_SUBSYSTEM_ITEM )
#define FRIENDLY_TARGET_MESSAGES	(PROTECT_TARGET_ITEM)

#define TARGET_MESSAGES	(ENEMY_TARGET_MESSAGES | FRIENDLY_TARGET_MESSAGES)



typedef struct squadmsg_history {
	int order_to;			// ship/wing that received the order
	int order;				// order that the ship/wing received (see defines above)
	int target;				// target of the order
	int order_from;			// ship that sent the order
	int special_index;		// any extra data the order might need (subsystem names for instance)
	fix order_time;			// when this order was sent (or received by the server in multiplayer)
	squadmsg_history(): order_to(-1), order(-1), target(-1), order_from(-1), special_index(-1), order_time(0) {}
} squadmsg_history;

extern SCP_vector<squadmsg_history> Squadmsg_history; 

/*
#define SQUADMSG_HISTORY_MAX 160

typedef struct squadmsg_history {
	int ship;  // ship that received the order
	int order;  // order that the ship received (see defines above)
	int target;  // ship that is the target of the order

	squadmsg_history(): ship(-1), order(-1), target(-1) {};
} squadmsg_history;


extern int squadmsg_history_index;
extern squadmsg_history Squadmsg_history[SQUADMSG_HISTORY_MAX];
*/

extern int Multi_squad_msg_local;
extern int Multi_squad_msg_targ; 

extern void hud_init_squadmsg();
extern void hud_init_comm_orders();
extern void hud_squadmsg_toggle();						// toggles the state of messaging mode
extern void hud_squadmsg_shortcut( int command );	// use of a shortcut key
extern int hud_squadmsg_hotkey_select( int k );	// a hotkey was hit -- maybe send a message to those ship(s)
extern void hud_squadmsg_save_keys( int do_scroll = 0 );					// saves into local area keys which need to be saved/restored when in messaging mode
extern int hud_squadmsg_do_frame();
extern int hud_query_order_issued(char *to, char *order_name, char *target = NULL, int timestamp = 0, char *from = NULL, char *special_index = NULL);
extern int hud_squadmsg_read_key( int k );			// called from high level keyboard code

extern void hud_squadmsg_repair_rearm( int toggle_state, object *obj = NULL );
extern void hud_squadmsg_repair_rearm_abort( int toggle_state, object *obj = NULL );
extern void hud_squadmsg_rearm_shortcut();


#define SQUADMSG_HISTORY_NO_UPDATE		0
#define SQUADMSG_HISTORY_UPDATE			1
#define SQUADMSG_HISTORY_ADD_ENTRY		2

extern int hud_squadmsg_send_ship_command( int shipnum, int command, int send_message, int update_history = SQUADMSG_HISTORY_ADD_ENTRY, int player_num = -1 );
extern int hud_squadmsg_send_wing_command( int wingnum, int command, int send_message, int update_history = SQUADMSG_HISTORY_ADD_ENTRY, int player_num = -1 );
extern void hud_squadmsg_send_to_all_fighters( int command, int player_num = -1 );
extern void hud_squadmsg_call_reinforcement(int reinforcement_num, int player_num = -1);

extern int hud_squadmsg_reinforcements_available(int team);

void hud_enemymsg_toggle();						// debug function to allow messaging of enemies

// Added for voicer implementation
void hud_squadmsg_do_mode( int mode );

class HudGaugeSquadMessage: public HudGauge
{
protected:
	hud_frames Mbox_gauge[NUM_MBOX_FRAMES];

	int Header_offsets[2];
	int Item_start_offsets[2];
	int Middle_frame_start_offset_y;
	int bottom_bg_offset;
	int Item_h;
	int Item_offset_x;

	int Pgup_offsets[2];
	int Pgdn_offsets[2];

	int flash_timer[2];
	bool flash_flag;
public:
	HudGaugeSquadMessage();
	void initBitmaps(char *fname_top, char *fname_middle, char *fname_bottom);
	void initHeaderOffsets(int x, int y);
	void initItemStartOffsets(int x, int y);
	void initMiddleFrameStartOffsetY(int y);
	void initBottomBgOffset(int offset);
	void initItemHeight(int h);
	void initItemOffsetX(int x);
	void initPgUpOffsets(int x, int y);
	void initPgDnOffsets(int x, int y);

	void render(float frametime);
	bool canRender();
	void pageIn();
	void initialize();
	void startFlashPageScroll(int duration = 1400);
	bool maybeFlashPageScroll(bool flash_fast = false);
};

#endif

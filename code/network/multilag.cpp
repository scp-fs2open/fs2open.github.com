/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef SCP_UNIX

#include <winsock.h>
#include <wsipx.h>

#include "network/multilag.h"
#include "io/timer.h"
#include "globalincs/linklist.h"
#include "network/psnet2.h"



// ----------------------------------------------------------------------------------------------------
// LAGLOSS DEFINES/VARS
//

// default LAGLOSS values
#define MULTI_LAGLOSS_DEF_LAG				(-1)
#define MULTI_LAGLOSS_DEF_LAGMIN			(-1)
#define MULTI_LAGLOSS_DEF_LAGMAX			(-1)
#define MULTI_LAGLOSS_DEF_LOSS			(-1.0f)
#define MULTI_LAGLOSS_DEF_LOSSMIN		(-1.0f)
#define MULTI_LAGLOSS_DEF_LOSSMAX		(-1.0f)
#define MULTI_LAGLOSS_DEF_STREAK			(2500)

// if we're running
int Multi_lag_inited = 0;

// lag values (base - max and min)
int Multi_lag_base = -1;
int Multi_lag_min = -1;
int Multi_lag_max = -1;

// packet loss values (base - max and min)
float Multi_loss_base = -1.0f;
float Multi_loss_min = -1.0f;
float Multi_loss_max = -1.0f;

// streaks for lagging
int Multi_streak_stamp = -1;				// timestamp telling when the streak of a certain lag is done
int Multi_streak_time = 0;					// how long each streak will last
int Multi_current_streak = -1;			// what lag the current streak has

// struct for buffering stuff on receives
typedef struct lag_buf {
	ubyte data[700];							// the data from the packet
	int data_len;								// length of the data
	uint socket;								// this can be either a PSNET_SOCKET or a PSNET_SOCKET_RELIABLE
	int stamp;									// when this expires, make this packet available	
	SOCKADDR_IN ip_addr;						// ip address when in TCP
	SOCKADDR_IPX ipx_addr;					// ipx address when in IPX mode

	struct	lag_buf * prev;				// prev in the list
	struct	lag_buf * next;				// next in the list
} lag_buf;

// lag buffers - malloced
#ifdef NDEBUG
	#define MAX_LAG_BUFFERS			1		// only 1 buffer in non-debug builds
#else
	#define MAX_LAG_BUFFERS			1000
#endif
lag_buf *Lag_buffers[MAX_LAG_BUFFERS];
int Lag_buf_count = 0;						// how many lag_buf's are currently in use

lag_buf Lag_free_list;
lag_buf Lag_used_list;


// ----------------------------------------------------------------------------------------------------
// LAGLOSS FORWARD DECLARATIONS
//

// get a value to lag a packet with (in ms)
int multi_lag_get_random_lag();

// boolean yes or no - should this packet be lost?
int multi_lag_should_be_lost();		    

// get a free packet buffer, return NULL on fail
lag_buf *multi_lag_get_free();

// put a lag buffer back
void multi_lag_put_free(lag_buf *buf);

// ----------------------------------------------------------------------------------------------------
// LAGLOSS FUNCTIONS
//

void multi_lag_init()
{	
	// never do lag in a non-debug build
#if defined(NDEBUG) || !defined(MULTI_USE_LAG)
	Multi_lag_inited = 0;
#else
	int idx;

	// if we're already inited, don't do anything
	if(Multi_lag_inited){
		return;
	}

	// try and allocate lag bufs
	for(idx=0; idx<MAX_LAG_BUFFERS; idx++){
		Lag_buffers[idx] = (lag_buf*)vm_malloc(sizeof(lag_buf));
		if(Lag_buffers[idx] == NULL){
			return;
		}
	}

	// initialize lag buffer lists
	list_init( &Lag_free_list );
	list_init( &Lag_used_list );

	// Link all object slots into the free list
	for (idx=0; idx<MAX_LAG_BUFFERS; idx++)	{
		list_append(&Lag_free_list, Lag_buffers[idx]);
	}
	
	// set the default lag values
	Multi_lag_base = MULTI_LAGLOSS_DEF_LAG;
	Multi_lag_min = MULTI_LAGLOSS_DEF_LAGMIN;
	Multi_lag_max = MULTI_LAGLOSS_DEF_LAGMAX;

	// set the default loss values
	Multi_loss_base = MULTI_LAGLOSS_DEF_LOSS;
	Multi_loss_min	= MULTI_LAGLOSS_DEF_LOSSMIN;
	Multi_loss_max = MULTI_LAGLOSS_DEF_LOSSMAX;

	// set the default lag streak time	
	Multi_streak_time = MULTI_LAGLOSS_DEF_STREAK;
	
	Multi_lag_inited = 1;
#endif
}

void multi_lag_close()
{	
	int idx;

	// if we're not inited already, don't do anything
	if(!Multi_lag_inited){
		return;
	}	

	// free up lag buffers
	for(idx=0; idx<MAX_LAG_BUFFERS; idx++){
		if(Lag_buffers[idx] != NULL){
			vm_free(Lag_buffers[idx]);
			Lag_buffers[idx] = NULL;
		}
	}

	Multi_lag_inited = 0;
}

// select for multi_lag
int multi_lag_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *except_fds, const timeval *timeout)
{		
	char t_buf[1024];
	int t_from_len;
	SOCKADDR_IN ip_addr;
	SOCKADDR_IPX ipx_addr;
	int ret_val;
	lag_buf *moveup, *item;

	Assert(readfds != NULL);
	Assert(writefds == NULL);
	Assert(except_fds == NULL);

	// clear out addresses
	memset(&ip_addr, 0, sizeof(SOCKADDR_IN));
	memset(&ipx_addr, 0, sizeof(SOCKADDR_IPX));

	// if there's data on the socket, read it
	if(select(nfds, readfds, writefds, except_fds, timeout)){		
		// read the data and stuff it
		if(Tcp_active){						
			t_from_len = sizeof(SOCKADDR_IN);
			ret_val = recvfrom(readfds->fd_array[0], t_buf, 1024, 0, (SOCKADDR*)&ip_addr, &t_from_len);
		} else {
			t_from_len = sizeof(SOCKADDR_IPX);
			ret_val = recvfrom(readfds->fd_array[0], t_buf, 1024, 0, (SOCKADDR*)&ipx_addr, &t_from_len);
		}
			
		// wacky socket error
		if(ret_val == SOCKET_ERROR){
			return SOCKET_ERROR;
		}

		// if we should be dropping this packet
		if(!multi_lag_should_be_lost()){
			// get a free packet buf and stuff the data
			item = multi_lag_get_free();
			if(item){
				Assert(ret_val < 700);
				memcpy(item->data, t_buf, ret_val);			
				item->data_len = ret_val;
				item->ip_addr = ip_addr;
				item->ipx_addr = ipx_addr;
				item->socket = readfds->fd_array[0];
				item->stamp = timestamp(multi_lag_get_random_lag());
			}		
		}
	}

	// always unset the readfds
	readfds->fd_count = 0;

	// now determine if we have any pending packets - find the first one
	// NOTE : this _could_ be the packet we just read. In fact, with a 0 lag, this will always be the case
	moveup=GET_FIRST(&Lag_used_list);
	while ( moveup!=END_OF_LIST(&Lag_used_list) )	{		
		// if the timestamp has elapsed and we have a matching socket
		if((readfds->fd_array[0] == (SOCKET)moveup->socket) && ((moveup->stamp <= 0) || timestamp_elapsed(moveup->stamp))){
			// set this so we think select returned yes
			readfds->fd_count = 1;
			return 1;
		}

		moveup = GET_NEXT(moveup);
	}

	// no data
	return 0;
}

// recvfrom for multilag
int multi_lag_recvfrom(uint s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen)
{
	lag_buf *moveup = NULL;
	lag_buf *item = NULL;

	// now determine if we have any pending packets - find the first one
	moveup=GET_FIRST(&Lag_used_list);
	while ( moveup!=END_OF_LIST(&Lag_used_list) )	{		
		// if the timestamp has elapsed
		if((s == (SOCKET)moveup->socket) && ((moveup->stamp <= 0) || timestamp_elapsed(moveup->stamp))){
			item = moveup;
			break;
		}

		moveup = GET_NEXT(moveup);
	}

	// if this happens, it means that the multi_lag_select() returned an improper value
	Assert(item);
	// stuff the data
	Assert(item->data_len <= len);
	memcpy(buf, item->data, item->data_len);
	if(Tcp_active){
		memcpy(from, &item->ip_addr, sizeof(SOCKADDR_IN));
	} else {
		memcpy(from, &item->ipx_addr, sizeof(SOCKADDR_IPX)); //-V512
	}

	// stick the item back on the free list
	multi_lag_put_free(item);

	// return the size in bytes
	return item->data_len;
}

// ----------------------------------------------------------------------------------------------------
// LAGLOSS FORWARD DEFINITIONS
//

int multi_lag_get_random_lag()
{
	// first determine the percentage we'll be checking against
	int ret;
	int mod;	

	// if the lag system isn't inited, don't do anything (no lag)
	if(!Multi_lag_inited){
		return 0;
	}
		
	// pick a value
	// see if we should be going up or down (loss max/loss min)
	mod = 0;
	if((float)rand()/(float)RAND_MAX < 0.5){
		// down
		if(Multi_lag_min >= 0){
			mod = - (int)((float)(Multi_lag_base - Multi_lag_min) * ((float)rand()/(float)RAND_MAX));
		}
	} else {
		// up
		if(Multi_lag_max >= 0){
			mod = (int)((float)(Multi_lag_max - Multi_lag_base) * ((float)rand()/(float)RAND_MAX));
		}
	}
	
	// if the current streak has elapsed, calculate a new one
	if((Multi_streak_stamp == -1) || (timestamp_elapsed(Multi_streak_stamp))){
		// timestamp the new streak
		Multi_streak_stamp = timestamp(Multi_streak_time);

		// set the return value
		ret = Multi_lag_base + mod;
		
		// set the lag value of this current streak
		Multi_current_streak = ret;
	} 
	// otherwise use the lag for the current streak
	else {
		ret = Multi_current_streak;
	}
			
	return ret;	
}

// this _may_ be a bit heavyweight, but it _is_ debug code
int multi_lag_should_be_lost()
{	
	// first determine the percentage we'll be checking against
	float mod;	

	// if the lag system isn't inited, don't do anything
	if(!Multi_lag_inited){
		return 0;
	}
		
	// see if we should be going up or down (loss max/loss min)
	mod = 0.0f;
	if((float)rand()/(float)RAND_MAX < 0.5){
		// down
		if(Multi_loss_min >= 0.0f){
			mod = - ((Multi_loss_base - Multi_loss_min) * ((float)rand()/(float)RAND_MAX));
		}
	} else {
		// up
		if(Multi_loss_max >= 0.0f){
			mod = ((Multi_loss_max - Multi_loss_base) * ((float)rand()/(float)RAND_MAX));
		}
	}	
	
	if((float)rand()/(float)RAND_MAX <= Multi_loss_base + mod){
		return 1;
	}	

	return 0;
}

// get a free packet buffer, return NULL on fail
lag_buf *multi_lag_get_free()
{
	lag_buf *lagp;

	// if we're out of buffers
	if(Lag_buf_count >= MAX_LAG_BUFFERS){
		nprintf(("Network", "Out of lag buffers!\n"));
		return NULL;
	}

	// get a free item
	lagp = GET_FIRST(&Lag_free_list);
	Assert( lagp != &Lag_free_list );		// shouldn't have the dummy element

	// remove trailp from the free list
	list_remove( &Lag_free_list, lagp );
	
	// insert trailp onto the end of used list
	list_append( &Lag_used_list, lagp );

	// increase the count
	Lag_buf_count++;
	return lagp;
}

// put a lag buffer back
void multi_lag_put_free(lag_buf *buf)
{
	// remove objp from the used list
	list_remove( &Lag_used_list, buf);

	// add objp to the end of the free
	list_append( &Lag_free_list, buf );

	// decrement counter
	Lag_buf_count--;
}

void multi_lagloss_dcf()
{
	// if the lag system isn't inited, don't do anything
	if(!Multi_lag_inited){
		dc_printf("Lag System Not Initialized!\n");
		return;
	}

	// display all available commands
	dc_printf("Usage :\nlag <ms>  (-1 to disable)\nlag_min <ms>\nlag_max <ms>\nloss <0-100>  (-1 to disable)\nloss_min <0-100>\nloss_max <0-100>\nlag_streak <ms>\nlagloss\n");

	// display lag settings
	dc_printf("Lag : ");		
	dc_printf("\n   Base %d\n   Min %d\n   Max %d\n   Streak %d\n", Multi_lag_base, Multi_lag_min, Multi_lag_max, Multi_streak_time);	

	// display loss settings
	dc_printf("Loss : ");		
	dc_printf("\n   Base %f\n   Min %f\n   Max %f\n", Multi_loss_base, Multi_loss_min, Multi_loss_max);	
}

DCF(lag, "")
{
	// if the lag system isn't inited, don't do anything
	if(!Multi_lag_inited){
		dc_printf("Lag System Not Initialized!\n");
		return;
	}

	dc_get_arg(ARG_INT);		
	// parse the argument and change things around accordingly
	if(Dc_arg_type & ARG_INT){			
		if(Dc_arg_int < 0){
			// switch the lag sim off
			Multi_lag_base = -1;
			Multi_lag_min = -1;
			Multi_lag_max = -1;
			dc_printf("Turning simulated lag off\n");
			multi_lagloss_dcf();
		} else if((Multi_lag_max >= 0) && (Dc_arg_int > Multi_lag_max)){
			dc_printf("Base value greater than max value, ignoring...");
		} else if((Multi_lag_min >= 0) && (Dc_arg_int < Multi_lag_min)){
			dc_printf("Base value smaller than min value, ignoring...");
		} else {
			Multi_lag_base = Dc_arg_int;
			multi_lagloss_dcf();
		}
	}	
}

DCF(lag_min, "")
{
	// if the lag system isn't inited, don't do anything
	if(!Multi_lag_inited){
		dc_printf("Lag System Not Initialized!\n");
		return;
	}

	dc_get_arg(ARG_INT);		
	// parse the argument and change things around accordingly
	if(Dc_arg_type & ARG_INT){			
		if(Dc_arg_int > Multi_lag_base){
			dc_printf("Min value greater than base value, ignoring...");
		} else {
			if(Dc_arg_int < 0){
				Multi_lag_min = -1;
			} else {
				Multi_lag_min = Dc_arg_int;
			}
			multi_lagloss_dcf();
		}
	}			
}

DCF(lag_max, "")
{
	// if the lag system isn't inited, don't do anything
	if(!Multi_lag_inited){
		dc_printf("Lag System Not Initialized!\n");
		return;
	}

	// parse the argument and change things around accordingly
	dc_get_arg(ARG_INT);
	if(Dc_arg_type & ARG_INT){			
		if((Dc_arg >=0) && (Dc_arg_int < Multi_lag_base)){
			dc_printf("Max value smaller than base value, ignoring...");
		} else {
			if(Dc_arg_int < 0){
				Multi_lag_max = -1;
			} else {
				Multi_lag_max = Dc_arg_int;
			}
			multi_lagloss_dcf();
		}
	}		
}

DCF(loss, "")
{
	// if the lag system isn't inited, don't do anything
	if(!Multi_lag_inited){
		dc_printf("Lag System Not Initialized!\n");
		return;
	}

	// parse the argument and change things around accordingly
	dc_get_arg(ARG_INT);
	if(Dc_arg_type & ARG_INT){
		float val = (float)Dc_arg_int / 100.0f;
			
		if(Dc_arg_int > 100){
			dc_printf("Illegal loss base value, ignoring...");
		} else if(Dc_arg_int < 0){
			// switch the loss sim off
			dc_printf("Turning simulated loss off\n");
			Multi_loss_base = -1.0f;
			Multi_loss_min = -1.0f;
			Multi_loss_max = -1.0f;
			multi_lagloss_dcf();
		} else if((Multi_loss_max >= 0.0f) && (val > Multi_loss_max)){
			dc_printf("Base value greater than max value, ignoring...");
		} else if((Multi_loss_min >= 0.0f) && (val < Multi_loss_min)){
			dc_printf("Base value smaller than min value, ignoring...");
		} else {
			Multi_loss_base = val;
			multi_lagloss_dcf();
		}
	}			
}

DCF(loss_min, "")
{
	// if the lag system isn't inited, don't do anything
	if(!Multi_lag_inited){
		dc_printf("Lag System Not Initialized!\n");
		return;
	}

	// parse the argument and change things around accordingly
	dc_get_arg(ARG_INT);
	if(Dc_arg_type & ARG_INT){			
      float val = (float)Dc_arg_int / 100.0f;

		if(val > Multi_loss_base){
			dc_printf("Min value greater than base value, ignoring...");
		} else {
			// otherwise set the value
			if(Dc_arg_int < 0){
				Multi_loss_min = -1.0f;
			} else {
				Multi_loss_min = val;
			}
			multi_lagloss_dcf();
		}
	}
}

DCF(loss_max, "")
{	
	// if the lag system isn't inited, don't do anything
	if(!Multi_lag_inited){
		dc_printf("Lag System Not Initialized!\n");
		return;
	}

	// parse the argument and change things around accordingly
	dc_get_arg(ARG_INT);
	if(Dc_arg_type & ARG_INT){			
      float val = (float)Dc_arg_int / 100.0f;

		if(val < Multi_loss_base){
			dc_printf("Max value smaller than base value, ignoring...");
		} else {
			// otherwise set the value
			if(Dc_arg_int < 0){
				Multi_loss_max = -1.0f;
			} else {
				Multi_loss_min = val;
			}
			multi_lagloss_dcf();
		}
	}			
}

DCF(lagloss, "")
{
	// if the lag system isn't inited, don't do anything
	if(!Multi_lag_inited){
		dc_printf("Lag System Not Initialized!\n");
		return;
	}

	multi_lagloss_dcf();
}

DCF(lag_streak, "")
{
	// if the lag system isn't inited, don't do anything
	if(!Multi_lag_inited){
		dc_printf("Lag System Not Initialized!\n");
		return;
	}

	dc_get_arg(ARG_INT);
	if(Dc_arg_type & ARG_INT){			      		
		if(Dc_arg_int >= 0){
			Multi_streak_time = Dc_arg_int;
		} 
	}
}

DCF(lag_bad, "")
{
	// if the lag system isn't inited, don't do anything
	if(!Multi_lag_inited){
		dc_printf("Lag System Not Initialized!\n");
		return;
	}

	dc_printf("Setting bad lag/loss parameters\n");

	// set good lagloss parameters
	Multi_lag_base = 500;
	Multi_lag_min = 400;
	Multi_lag_max = 600;
	
	Multi_loss_base = 0.2f;
	Multi_loss_min = 0.15f;
	Multi_loss_max = 0.23f;

	Multi_streak_time = 800;
	Multi_streak_stamp = -1;
	Multi_current_streak = -1;
}

DCF(lag_avg, "")
{
	// if the lag system isn't inited, don't do anything
	if(!Multi_lag_inited){
		dc_printf("Lag System Not Initialized!\n");
		return;
	}

	dc_printf("Setting avg lag/loss parameters\n");

	// set good lagloss parameters
	Multi_lag_base = 275;
	Multi_lag_min = 200;
	Multi_lag_max = 400;
	
	Multi_loss_base = 0.15f;
	Multi_loss_min = 0.1f;
	Multi_loss_max = 0.20f;

	Multi_streak_time = 900;
	Multi_streak_stamp = -1;
	Multi_current_streak = -1;
}

DCF(lag_good, "")
{
	// if the lag system isn't inited, don't do anything
	if(!Multi_lag_inited){
		dc_printf("Lag System Not Initialized!\n");
		return;
	}

	dc_printf("Setting good lag/loss parameters\n");

	// set good lagloss parameters
	Multi_lag_base = 100;
	Multi_lag_min = 35;
	Multi_lag_max = 200;
	
	Multi_loss_base = 0.08f;
	Multi_loss_min = 0.0f;
	Multi_loss_max = 0.1f;

	Multi_streak_time = 1000;
	Multi_streak_stamp = -1;
	Multi_current_streak = -1;
}

#endif // !SCP_UNIX

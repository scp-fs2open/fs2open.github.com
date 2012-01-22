/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "globalincs/pstypes.h"
#include "ship/ship.h"

extern int Multi_sexp_bytes_left; 

void initalise_sexp_packet();
void multi_start_callback();
void multi_end_callback();
void multi_do_callback(); // starts and ends a callback, used when there is no data to be written
void multi_sexp_flush_packet();

// server side packet fillers
void multi_send_int(int value);
void multi_send_ship(ship *shipp);
void multi_send_ship(int shipnum);
void multi_send_object(object *objp);
void multi_send_parse_object(p_object *pobjp);
void multi_send_string(char *string);
void multi_send_bool(bool value);
void multi_send_float(float value);
void multi_send_short(short value);
void multi_send_ushort(ushort value);

void sexp_packet_received(ubyte *received_packet, int num_ubytes);
int multi_sexp_get_next_operator(); 
int multi_sexp_get_operator();
bool multi_sexp_discard_operator();

void multi_discard_remaining_callback_data();

// client side packet emptiers
bool multi_get_int(int &value);
bool multi_get_ship(int &value);
bool multi_get_ship(ship*& shipp);
bool multi_get_object(object*& value);
bool multi_get_parse_object(p_object*& pobjp);
bool multi_get_string(char *buffer);
bool multi_get_bool(bool &value);
bool multi_get_float(float &value);
bool multi_get_short(short &value);
bool multi_get_ushort(ushort &value);
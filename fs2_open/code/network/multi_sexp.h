/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "globalincs/pstypes.h"
#include "ship/ship.h"

extern int Multi_sexp_bytes_left; 

void initalise_sexp_packet();
void multi_start_packet();
void multi_end_packet();
void multi_sexp_flush_packet();

// data fillers
void multi_send_int(int value);
void multi_send_ship(ship *shipp);
void multi_send_ship(int shipnum);
void multi_send_parse_object(p_object *pobjp);
void multi_send_string(char *string);
void multi_send_bool(bool value);
void multi_send_float(float value);

void sexp_packet_received(ubyte *received_packet, int num_ubytes);
int multi_sexp_get_next_operator(); 
int multi_sexp_get_operator();
bool multi_sexp_discard_operator();

bool multi_get_int(int &value);
bool multi_get_ship(int &value);
bool multi_get_ship(ship*& shipp);
bool multi_get_parse_object(p_object*& pobjp);
bool multi_get_string(char *buffer);
bool multi_get_bool(bool &value);
bool multi_get_float(float &value);

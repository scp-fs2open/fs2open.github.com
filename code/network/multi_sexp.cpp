/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "network/multi_sexp.h"
#include "network/psnet2.h"
#include "network/multimsgs.h"
#include "parse/sexp.h"
#include "network/multi.h"
#include "network/multiutil.h"

#define CALLBACK_TERMINATOR	255
int TEMP_DATA_SIZE = -1;

#define TYPE_NOT_DATA			-1
#define TYPE_SEXP_OPERATOR		0
#define TYPE_ARGUMENT_COUNT		1
#define TYPE_DATA_TERMINATES	2
#define TYPE_INT				3
#define TYPE_SHIP				4
#define TYPE_STRING				5
#define TYPE_PARSE_OBJECT		6
#define TYPE_BOOLEAN			7
#define TYPE_FLOAT				8
#define TYPE_SHORT				9
#define TYPE_USHORT				10
#define TYPE_OBJECT				11


// the type array holds information on the type of date held at the same index of the data array
// types are not sent to the client and the entire array could be replaced with a couple of variables indexing the end of 
// the previous SEXP. However it is much more helpful when debugging to have the array
int type[MAX_PACKET_SIZE];
int argument_count_index = -1;			// index in the type and data arrays for the argument count
int current_argument_count = 0;			// number of bytes the data for this SEXP currently takes up

// these 3 variable names must remain the same as those used in multimsgs.h in order for the macros to work
ubyte data[MAX_PACKET_SIZE]; 
int packet_size = 0;
int offset = 0; 

bool callback_started = false;

int Multi_sexp_bytes_left = 0;			// number of bytes in incoming packet that still require processing

int op_num = -1;
bool packet_flagged_invalid = false;

//forward declarations
void multi_sexp_ensure_space_remains(int data_size); 


/**************************
 HOST SIDE PACKET FUNCTIONS
 *************************/

void initalise_sexp_packet() 
{
	memset(data, 0, MAX_PACKET_SIZE); 
	memset(type, -1, MAX_PACKET_SIZE); 
	
	packet_size = 0;
	argument_count_index = -1;
	current_argument_count = 0;
}

void multi_start_callback() 
{
	if (!MULTIPLAYER_MASTER) {
		return;
	}

	callback_started = true;

	//Write OP into the Type buffer.
	type[packet_size] = TYPE_SEXP_OPERATOR; 
	//Write the SEXP_Operator number into the data buffer.
	Assert (!Current_sexp_operator.empty()); 
	ADD_INT(Current_sexp_operator.back()); 

	//Store the next data index as we'll need it later to write the COUNT.
	argument_count_index = packet_size; 
	// store an invalid count, we'll come back and store the correct value once we know what it is.	
	type[packet_size] = TYPE_ARGUMENT_COUNT; 
	ADD_INT(TEMP_DATA_SIZE); 
}

void multi_end_callback() 
{
	if (!MULTIPLAYER_MASTER) {
		return;
	}	

	callback_started = false;

	// something is wrong with the packet, blast it clean and start again
	if (packet_flagged_invalid) {
		initalise_sexp_packet();
		packet_flagged_invalid = false;
		return;
	}

	//write TERMINATOR into the Type and data buffers
	type[packet_size] = TYPE_DATA_TERMINATES; 
	ubyte b = CALLBACK_TERMINATOR; 
	ADD_DATA(b); 

	//Write the COUNT into the data buffer at the index we saved earlier.
	int temp_packet_size = packet_size; 
	packet_size = argument_count_index; 
	ADD_INT(current_argument_count);
	packet_size = temp_packet_size; 

	current_argument_count = 0; 
}

// convenience function that simply calls the two functions above
void multi_do_callback()
{
	multi_start_callback();
	multi_end_callback();
}

void multi_sexp_ensure_space_remains(int data_size) 
{
	if (!MULTIPLAYER_MASTER) {
		return;
	}

	int packet_end = 0;
	int sub_packet_size = 0;
	int i, j; 

	//If the index of the data buffer isn't high enough yet, bail
	if (packet_size + data_size + HEADER_LENGTH < MAX_PACKET_SIZE) {
		return;
	}

	//iterate back through the types array until we find a TERMINATOR and store the corresponding data index 
	for (i = packet_size-1; i > 0; i--) {
		if ( type[i] == TYPE_DATA_TERMINATES) {
			packet_end = i; 
			break; 
		}
	}

	// we want the number of bytes not the index of the last one
	sub_packet_size = packet_end + 1; 

	// At very least must include OP, COUNT, TERMINATOR 
	if (packet_end < 9 && !packet_flagged_invalid) {
		Warning(LOCATION, "Sexp %s has attempted to write too much data to a single packet. It is advised that you split this SEXP up into smaller ones",  Operators[Current_sexp_operator.back()].text ); 
		packet_flagged_invalid = true;
		return;
	}

	send_sexp_packet(data, sub_packet_size); 

	j = 0; 
	//Slide down any entries after the stored index to the start of the array.
	for (i = sub_packet_size; i < packet_size; i++) {
		data[j] = data[i]; 
		type[j] = type[i];
		j++;
	}

	packet_size = j; 

	// flush the remaining type buffer
	for (i = j ; i < MAX_PACKET_SIZE ; i++) {
		type[i] = -1;
	}

	// if we have an existing argument count we need to update where to put it too
	if (current_argument_count) {
		argument_count_index = argument_count_index - sub_packet_size; 
	}

	Assert(argument_count_index >=0);


}

// flushes out the packet and sends any data still in there
void multi_sexp_flush_packet() 
{
	if (!MULTIPLAYER_MASTER) {
		return;
	}

	// possible to get here when there is nothing in the packet to send
	if (packet_size == 0){
		return;
	}
	Assert (type[packet_size -1] == TYPE_DATA_TERMINATES); 
	Assert (!packet_flagged_invalid);

	send_sexp_packet(data, packet_size);

	initalise_sexp_packet(); 
}

bool cannot_send_data()
{
	if (!MULTIPLAYER_MASTER || packet_flagged_invalid ) {
		return true;
	}

	if (!callback_started) {
		Warning (LOCATION, "Attempt to send data in multi_sexp.cpp without first starting a callback");
		return true;
	}

	return false;
}

/********************************
 HOST SIDE DATA WRAPPER FUNCTIONS
 *******************************/

void multi_send_int(int value) 
{
	if (cannot_send_data()) {
		return;
	}

	multi_sexp_ensure_space_remains(sizeof(value)); 

	//Write INT into the Type buffer.
	type[packet_size] = TYPE_INT; 
	//Write the int into the data buffer
	ADD_INT(value); 
	//Increment the COUNT by 4 (i.e the size of an int).
	current_argument_count += sizeof(int); 
}

void multi_send_ship(int shipnum) 
{
	if (cannot_send_data()) {
		return;
	}

	multi_sexp_ensure_space_remains(sizeof(shipnum)); 

	multi_send_ship(&Ships[shipnum]);
}

void multi_send_ship(ship *shipp) 
{
	if (cannot_send_data()) {
		return;
	}

	multi_sexp_ensure_space_remains(sizeof(ushort)); 

	//write into the Type buffer.
	type[packet_size] = TYPE_SHIP; 
	//write the into the data buffer
	ADD_USHORT(Objects[shipp->objnum].net_signature); 
	current_argument_count += sizeof(ushort); 
}

void multi_send_object(object *objp) 
{
	if (cannot_send_data()) {
		return;
	}

	multi_sexp_ensure_space_remains(sizeof(ushort)); 

	//write into the Type buffer.
	type[packet_size] = TYPE_OBJECT; 
	//write the into the data buffer
	ADD_USHORT(objp->net_signature); 
	current_argument_count += sizeof(ushort); 
}

void multi_send_parse_object(p_object *pobjp) 
{
	if (cannot_send_data()) {
		return;
	}

	multi_sexp_ensure_space_remains(sizeof(ushort)); 

	//write into the Type buffer.
	type[packet_size] = TYPE_PARSE_OBJECT; 
	//write the into the data buffer
	ADD_USHORT(pobjp->net_signature); 
	current_argument_count += sizeof(ushort); 
}

void multi_send_string(char *string) 
{
	if (cannot_send_data()) {
		return;
	}

	multi_sexp_ensure_space_remains(strlen(string)+4); 

	int start_size = packet_size; 
	//write into the Type buffer.
	type[packet_size] = TYPE_STRING; 
	//write the into the data buffer
	ADD_STRING(string); 
	current_argument_count += packet_size - start_size; 
}

void multi_send_bool(bool value) 
{
	if (cannot_send_data()) {
		return;
	}

	multi_sexp_ensure_space_remains(sizeof(value)); 

	//write into the Type buffer.
	type[packet_size] = TYPE_BOOLEAN; 
	//Write the value into the data buffer
	ADD_DATA(value); 
	//Increment the COUNT 
	current_argument_count += sizeof(value); 
}

void multi_send_float(float value) 
{
	if (cannot_send_data()) {
		return;
	}

	multi_sexp_ensure_space_remains(sizeof(value)); 

	//write into the Type buffer.
	type[packet_size] = TYPE_FLOAT; 
	//Write the value into the data buffer
	ADD_FLOAT(value); 
	//Increment the COUNT 
	current_argument_count += sizeof(float); 
}

void multi_send_short(short value) 
{
	if (cannot_send_data()) {
		return;
	}

	multi_sexp_ensure_space_remains(sizeof(value)); 

	//Write the type into the Type buffer.
	type[packet_size] = TYPE_SHORT; 
	//Write the value into the data buffer
	ADD_SHORT(value); 
	//Increment the COUNT 
	current_argument_count += sizeof(short); 
}

void multi_send_ushort(ushort value) 
{
	if (cannot_send_data()) {
		return;
	}

	multi_sexp_ensure_space_remains(sizeof(value)); 

	//Write the type into the Type buffer.
	type[packet_size] = TYPE_USHORT; 
	//Write the value into the data buffer
	ADD_USHORT(value); 
	//Increment the COUNT 
	current_argument_count += sizeof(ushort); 
}


/****************************
 CLIENT SIDE PACKET FUNCTIONS
 ***************************/

void sexp_packet_received(ubyte *received_packet, int num_ubytes)
{	
	int i; 

	offset = 0;
	op_num = -1;

	for (i=0; i < MAX_PACKET_SIZE; i++) {
		data[i] = received_packet[i];
	}

	Multi_sexp_bytes_left = num_ubytes; 
	// start working through the packet
	multi_sexp_eval();
}

int multi_sexp_get_next_operator()
{
	if (current_argument_count != 0) {
		// we have a problem here, either the argument count is wrong or the last SEXP didn't remove all its data from the packet		
		ubyte possible_terminator;
		bool terminator_found = false;
		for (int i=0; i < current_argument_count ; i++) {			
			GET_DATA(possible_terminator); 
			Multi_sexp_bytes_left--; 

			if (possible_terminator == CALLBACK_TERMINATOR) {
				Warning(LOCATION, "%s has returned to multi_sexp_eval() claiming %d arguments left. %d actually found. Trace out and fix this!"), Operators[op_num].text, current_argument_count, i; 
				terminator_found = true;
				break;
			}
		}

		// if we still haven't found the terminator it probably means the last SEXP didn't remove all its data from the packet
		if (!terminator_found) {
			GET_DATA(possible_terminator); 
			Multi_sexp_bytes_left--;

			if (possible_terminator != CALLBACK_TERMINATOR) {
				// discard remainder of packet if we still haven't found the terminator as it is hopelessly corrupt
				Warning(LOCATION, "%s has returned to multi_sexp_eval() without finding the terminator. Discarding packet! Trace out and fix this!", Operators[op_num].text);
				Multi_sexp_bytes_left = 0; 
				return -1;
			}
			else {
				Warning(LOCATION, "%s has returned to multi_sexp_eval() without removing all the data the server wrote during its callback. Trace out and fix this!", Operators[op_num].text);
				op_num = -1;
			}
		}
	}

	GET_INT(op_num);
	Multi_sexp_bytes_left -= sizeof(int); 
	GET_INT(current_argument_count);
	Multi_sexp_bytes_left -= sizeof(int); 

	Assert (Multi_sexp_bytes_left); 
	return op_num; 
}

int multi_sexp_get_operator()
{
	return op_num; 
}

void multi_reduce_counts(int amount)
{
	ubyte terminator; 

	Multi_sexp_bytes_left -= amount; 
	current_argument_count -= amount; 

	if (Multi_sexp_bytes_left < 0 || current_argument_count < 0) {
		Warning(LOCATION, "multi_get_x function call has read an invalid amount of data. Trace out and fix this!"); 
	}

	if (current_argument_count == 0) {
		// read in the terminator
		GET_DATA(terminator); 
		if (terminator != CALLBACK_TERMINATOR) {
			Warning(LOCATION, "multi_get_x function call has been called on an improperly terminated callback. Trace out and fix this!"); 
			// discard remainder of packet
			Multi_sexp_bytes_left = 0; 
			return;
		}
		Multi_sexp_bytes_left--;
		op_num = -1;
	}
}

bool multi_sexp_discard_operator()
{
	int i; 
	ubyte dummy;
	ubyte terminator; 

	// read in a number of bytes equal to the count
	for (i=0; i<current_argument_count; i++) {
		GET_DATA(dummy);
		Multi_sexp_bytes_left--; 
	}

	GET_DATA(terminator); 
	Multi_sexp_bytes_left--; 
	op_num = -1;

	// the operation terminated correctly, probably a new SEXP that this version doesn't support. 
	if (terminator == CALLBACK_TERMINATOR) 
		return true; 

	// packet is probably corrupt
	else
		return false;
	
}

/**********************************
 CLIENT SIDE DATA WRAPPER FUNCTIONS
 *********************************/

bool multi_get_int(int &value)
{
	if (!Multi_sexp_bytes_left || !current_argument_count) {
		return false; 
	}

	GET_INT(value);
	multi_reduce_counts(sizeof(int)); 

	return true; 
}

bool multi_get_ship(int &value)
{
	ushort netsig; 
	object *objp;

	if (!Multi_sexp_bytes_left || !current_argument_count) {
		return false; 
	}

	// get the net signature of the ship
	GET_USHORT(netsig);
	multi_reduce_counts(sizeof(ushort)); 

	// lookup the object
	objp = multi_get_network_object(netsig);
	if((objp != NULL) && (objp->type == OBJ_SHIP) && (objp->instance >=0)){
		value = objp->instance;
		return true;
	}

	Warning(LOCATION, "multi_get_ship called for object %d even though it is not a ship", objp->instance); 
	return false; 
}

bool multi_get_ship(ship* &shipp)
{
	int shipnum;

	if (multi_get_ship(shipnum)) {
		shipp = &Ships[shipnum]; 
		return true;
	}

	return false; 
}

bool multi_get_object(object*& objp)
{
	ushort netsig; 

	if (!Multi_sexp_bytes_left || !current_argument_count) {
		return false; 
	}

	// get the net signature of the ship
	GET_USHORT(netsig);
	multi_reduce_counts(sizeof(ushort)); 

	// lookup the object
	objp = multi_get_network_object(netsig);
	if((objp != NULL) && (objp->instance >=0)){
		return true;
	}

	Warning(LOCATION, "multi_get_object called for non-existent object"); 
	return false; 
}


bool multi_get_parse_object(p_object*& pobjp)
{
	ushort netsig; 

	if (!Multi_sexp_bytes_left || !current_argument_count) {
		return false; 
	}

	// get the net signature of the ship
	GET_USHORT(netsig);
	multi_reduce_counts(sizeof(ushort)); 

	// lookup the object
	pobjp = mission_parse_get_arrival_ship(netsig);
	if(pobjp != NULL){
		return true;
	}

	return false; 
}

bool multi_get_string(char *buffer)
{
	int starting_offset = offset; 

	if (!Multi_sexp_bytes_left || !current_argument_count) {
		return false; 
	}

	GET_STRING(buffer);
	multi_reduce_counts(offset - starting_offset); 

	return true; 
}

bool multi_get_bool(bool &value)
{
	if (!Multi_sexp_bytes_left || !current_argument_count) {
		return false; 
	}

	GET_DATA(value);
	multi_reduce_counts(sizeof(value)); 

	return true; 
}

bool multi_get_float(float &value)
{
	if (!Multi_sexp_bytes_left || !current_argument_count) {
		return false; 
	}

	GET_FLOAT(value);
	multi_reduce_counts(sizeof(float)); 

	return true; 
}

bool multi_get_short(short &value)
{
	if (!Multi_sexp_bytes_left || !current_argument_count) {
		return false; 
	}

	GET_SHORT(value);
	multi_reduce_counts(sizeof(short)); 

	return true; 
}

bool multi_get_ushort(ushort &value)
{
	if (!Multi_sexp_bytes_left || !current_argument_count) {
		return false; 
	}

	GET_USHORT(value);
	multi_reduce_counts(sizeof(ushort)); 

	return true; 
}

void multi_discard_remaining_callback_data()
{
	if (!multi_sexp_discard_operator()) {
		Warning(LOCATION, "Attempt to discard remaining data failed! Callback for operator %d lacks proper termination. Entire packet may be corrupt. Discarding remaining packet"); 
	}
}
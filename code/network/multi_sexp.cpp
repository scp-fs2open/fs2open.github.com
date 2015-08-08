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

#define TYPE_NOT_DATA			255
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
ubyte type[MAX_PACKET_SIZE];
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

/**
* Set up the SEXP packet every frame when the game starts processing SEXPs.
*/
void initalise_sexp_packet() 
{
	if (!MULTIPLAYER_MASTER) {
		return;
	}

	memset(data, 0, MAX_PACKET_SIZE); 
	memset(type, -1, MAX_PACKET_SIZE); 
	
	packet_size = 0;
	argument_count_index = -1;
	current_argument_count = 0;
}

/**
* Called when a server is currently processing a SEXP that needs to send an update to the clients.
*/
void multi_start_callback() 
{
	if (!MULTIPLAYER_MASTER) {
		return;
	}
	
	// Ensure that there is enough space in the packet. At present the function writes the Operator number and the argument count into the packet. Both are ints 
	// so we must ensure we have space left to write two ints. If this function is changed to write other data, this line MUST be updated too!
	multi_sexp_ensure_space_remains(sizeof(int)*2);

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

/**
* Called when a server has written all the data it needs to write for this SEXP.
*/
void multi_end_callback() 
{
	if (!MULTIPLAYER_MASTER) {
		return;
	}	

	// ensure that there is enough space in the packet. If this function is ever updated to write anything other than the terminator, this MUST be taken into account here!
	multi_sexp_ensure_space_remains(sizeof(ubyte));

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
/**
* Convenience function that simply calls the two functions above. Used when the server merely needs to signal the clients that a 
* SEXP has been processed but no additional data needs to be sent.
*/
void multi_do_callback()
{
	multi_start_callback();
	multi_end_callback();
}

/**
* Checks if there is enough space in the packet currently being stuffed for the data that is about to be written into it
* 
* If there is not enough space, it will send everything in the packet apart from any data from the SEXP currently being processed 
* and then create a new packet containing the data for this SEXP only.
*/
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
		type[i] = (ubyte) -1;
	}

	// if we have an existing argument count we need to update where to put it too
	if (current_argument_count) {
		argument_count_index = argument_count_index - sub_packet_size; 
	}

	Assert(argument_count_index >=0);
}

/**
* Flushes out the SEXP packet and sends any data still in there. Called when the game finishes processing SEXPs.
*/
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

/**
* Checks if the SEXP packet is ready to recieve data. 
*/
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

/**
* Add an int to the SEXP packet.
*/
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

/**
* Adds a ship's net sig to the SEXP packet.
*/
void multi_send_ship(int shipnum) 
{
	if (cannot_send_data()) {
		return;
	}

	multi_sexp_ensure_space_remains(sizeof(shipnum)); 

	multi_send_ship(&Ships[shipnum]);
}

/**
* Adds a ship's net sig to the SEXP packet.
*/
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

/**
* Add the net sig of an object to the SEXP packet.
*/
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

/**
* Add the net sig of a parse object to the SEXP packet.
*/
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

/**
* Add a string to the SEXP packet. Should only be used for strings TOKEN_LENGTH in size or smaller. 
*/
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

void multi_send_string(const SCP_string &string) 
{
	if (cannot_send_data()) {
		return;
	}

	multi_sexp_ensure_space_remains(string.length()+4); 

	int start_size = packet_size; 
	//write into the Type buffer.
	type[packet_size] = TYPE_STRING; 
	//write the into the data buffer
	ADD_STRING(string.c_str()); 
	current_argument_count += packet_size - start_size; 
}

/**
* Add a boolean to the SEXP packet.
*/
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

/**
* Add a float to the SEXP packet.
*/
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

/**
* Add a short to the SEXP packet.
*/
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

/**
* Add an unsigned short to the SEXP packet.
*/
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

/**
* Called when the client recieves a SEXP packet from the server. Removes the data from the packet that underlying code uses and puts
* into a new array that will work with the rest of the client-side code.
*/
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

/**
* Checks that the previous SEXP in the packet has correctly removed all its data from the packet. Attempts to fix it if it hasn't.
*/
bool argument_count_is_valid()
{
	if (current_argument_count != 0) {
		// we have a problem here, either the argument count is wrong or the last SEXP didn't remove all its data from the packet		
		ubyte possible_terminator;
		bool terminator_found = false;
		for (int i=0; i < current_argument_count ; i++) {			
			GET_DATA(possible_terminator); 
			Multi_sexp_bytes_left--; 

			if (possible_terminator == CALLBACK_TERMINATOR) {
				Warning(LOCATION, "%s has returned to multi_sexp_eval() claiming %d arguments left. %d actually found. Trace out and fix this!", Operators[op_num].text, current_argument_count, i); 
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
				return false;
			}
			else {
				// the previous SEXP hasn't removed all it's data from the packet correctly but it appears we've managed to fix it
				Warning(LOCATION, "%s has returned to multi_sexp_eval() without removing all the data the server wrote during its callback. Trace out and fix this!", Operators[op_num].text);
				op_num = -1;
			}
		}
	}

	return true;
}

/**
* Gets the next operator from the SEXP packet. Returns the number of the operator or -1 if there are no more operators because the 
* packet is corrupt.
*/
int multi_sexp_get_next_operator()
{
	if (!argument_count_is_valid()) {
		return -1;
	}

	GET_INT(op_num);
	Multi_sexp_bytes_left -= sizeof(int); 
	GET_INT(current_argument_count);
	Multi_sexp_bytes_left -= sizeof(int); 

	Assert (Multi_sexp_bytes_left); 
	return op_num; 
}

/**
* Returns the current operator number but does not touch the SEXP packet.
*/
int multi_sexp_get_operator()
{
	return op_num; 
}

/**
* Ensures that the variables tracking how much data is left in the packet are updated correctly when data is removed.
*/
void multi_reduce_counts(int amount)
{
	Multi_sexp_bytes_left -= amount; 
	current_argument_count -= amount; 

	if (Multi_sexp_bytes_left < 0 || current_argument_count < 0) {
		Warning(LOCATION, "multi_get_x function call has read an invalid amount of data. Trace out and fix this!"); 
	}
}

/**
* Called when the SEXP code has finished processing the current SEXP.
*/ 
void multi_finished_callback()
{
	ubyte terminator; 

	Assert(current_argument_count == 0);

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

/**
* Used to discard the rest of a callback on the client machine. 
*/ 
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

/**
* Attempts to remove an int from the SEXP packet and assign it to the value parameter. Returns false if it is unable to do so. 
*/
bool multi_get_int(int &value)
{
	if (!Multi_sexp_bytes_left || !current_argument_count) {
		return false; 
	}

	GET_INT(value);
	multi_reduce_counts(sizeof(int)); 

	return true; 
}

/**
* Attempts to get an index for the Ships array based on the net sig it removes from the SEXP packet. Returns it as the value 
* parameter. Returns false if unable to do so.  
*/
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

/**
* Attempts to get a ship pointer based on the net sig it removes from the SEXP packet. Returns it as the value parameter. 
* Returns false if unable to do so.  
*/
bool multi_get_ship(ship* &shipp)
{
	int shipnum;

	if (multi_get_ship(shipnum)) {
		shipp = &Ships[shipnum]; 
		return true;
	}

	return false; 
}

/**
* Attempts to get an object pointer based on the net sig it removes from the SEXP packet. Returns it as the value parameter. 
* Returns false if unable to do so.  
*/
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

/**
* Attempts to get a parse objects pointer based on the net sig it removes from the SEXP packet. Returns it as the value parameter. 
* Returns false if unable to do so.  
*/
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

/**
* Attempts to remove a string from the SEXP packet and assign it to the value parameter. Returns false if it is unable to do so. 
*/
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

bool multi_get_string(SCP_string &buffer)
{
	char tempstring[TOKEN_LENGTH];
	int starting_offset = offset; 

	if (!Multi_sexp_bytes_left || !current_argument_count) {
		return false; 
	}

	GET_STRING(tempstring);
	buffer = tempstring;
	multi_reduce_counts(offset - starting_offset); 

	return true; 
}

/**
* Attempts to remove a boolean from the SEXP packet and assign it to the value parameter. Returns false if it is unable to do so. 
*/
bool multi_get_bool(bool &value)
{
	if (!Multi_sexp_bytes_left || !current_argument_count) {
		return false; 
	}

	GET_DATA(value);
	multi_reduce_counts(sizeof(value)); 

	return true; 
}

/**
* Attempts to remove a float from the SEXP packet and assign it to the value parameter. Returns false if it is unable to do so. 
*/
bool multi_get_float(float &value)
{
	if (!Multi_sexp_bytes_left || !current_argument_count) {
		return false; 
	}

	GET_FLOAT(value);
	multi_reduce_counts(sizeof(float)); 

	return true; 
}

/**
* Attempts to remove a short from the SEXP packet and assign it to the value parameter. Returns false if it is unable to do so. 
*/
bool multi_get_short(short &value)
{
	if (!Multi_sexp_bytes_left || !current_argument_count) {
		return false; 
	}

	GET_SHORT(value);
	multi_reduce_counts(sizeof(short)); 

	return true; 
}

/**
* Attempts to remove an unsigned short from the SEXP packet and assign it to the value parameter. Returns false if it is unable to do so. 
*/
bool multi_get_ushort(ushort &value)
{
	if (!Multi_sexp_bytes_left || !current_argument_count) {
		return false; 
	}

	GET_USHORT(value);
	multi_reduce_counts(sizeof(ushort)); 

	return true; 
}

/**
* attempts to remove all remaining data for the current operator.
*/
void multi_discard_remaining_callback_data()
{
	if (!multi_sexp_discard_operator()) {
		Warning(LOCATION, "Attempt to discard remaining data failed! Callback lacks proper termination. Entire packet may be corrupt. Discarding remaining packet");
	}
}

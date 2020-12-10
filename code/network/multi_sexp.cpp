/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "network/multi_sexp.h"
#include "parse/sexp.h"
#include "network/multi.h"
#include "network/multiutil.h"

static const std::uint8_t CALLBACK_TERMINATOR = 255;
static const std::int16_t TEMP_DATA_SIZE = -1;

sexp_network_packet Current_sexp_network_packet;

/****************************
 CLIENT SIDE PACKET FUNCTIONS
 ***************************/

/**
* Called when the client recieves a SEXP packet from the server. Removes the data from the packet that underlying code uses and puts
* into a new array that will work with the rest of the client-side code.
*/
void sexp_packet_received(ubyte *received_packet, int num_ubytes)
{	
    Current_sexp_network_packet.set_data(received_packet, num_ubytes);

	// start working through the packet
	multi_sexp_eval();
}

void sexp_network_packet::ensure_space_remains(size_t data_size)
{
    if (!MULTIPLAYER_MASTER) {
        return;
    }

    int packet_end = 0;
    int sub_packet_size = 0;
    int i, j;

	// If the index of the data buffer isn't high enough yet, bail
	if (packet_size + static_cast<int>(data_size) < SEXP_MAX_PACKET_SIZE) {
		return;
	}

    //iterate back through the types array until we find a TERMINATOR and store the corresponding data index 
    for (i = packet_size - 1; i > 0; i--) {
        if (type[i] == packet_data_type::DATA_TERMINATES) {
            packet_end = i;
            break;
        }
    }

    // we want the number of bytes not the index of the last one
    sub_packet_size = packet_end + 1;

    // At very least must include OP, COUNT, TERMINATOR 
	if (packet_end < MIN_SEXP_PACKET_SIZE && !packet_flagged_invalid) {
        Warning(LOCATION, "Sexp %s has attempted to write too much data to a single packet. It is advised that you split this SEXP up into smaller ones", Operators[Current_sexp_operator.back()].text.c_str());
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
	for (i = j; i < SEXP_MAX_PACKET_SIZE; i++) {
        type[i] = packet_data_type::NOT_DATA;
    }

    // if we have an existing argument count we need to update where to put it too
    if (current_argument_count) {
        argument_count_index = argument_count_index - sub_packet_size;
    }

    Assert(argument_count_index >= 0);
}

void sexp_network_packet::reduce_counts(int amount)
{
    sexp_bytes_left -= amount;
    current_argument_count -= amount;

    if (sexp_bytes_left < 0 || current_argument_count < 0) {
        Warning(LOCATION, "multi_get_x function call has read an invalid amount of data. Trace out and fix this!");
    }
}

bool sexp_network_packet::argument_count_is_valid()
{
    if (current_argument_count != 0) {
        // we have a problem here, either the argument count is wrong or the last SEXP didn't remove all its data from the packet		
        ubyte possible_terminator;
        bool terminator_found = false;
        for (int i = 0; i < current_argument_count; i++) {
            GET_DATA(possible_terminator);
            sexp_bytes_left--;

            if (possible_terminator == CALLBACK_TERMINATOR) {
                Warning(LOCATION, "%s has returned to multi_sexp_eval() claiming %d arguments left. %d actually found. Trace out and fix this!", Operators[op_num].text.c_str(), current_argument_count, i);
                terminator_found = true;
                break;
            }
        }

        // if we still haven't found the terminator it probably means the last SEXP didn't remove all its data from the packet
        if (!terminator_found) {
            GET_DATA(possible_terminator);
            sexp_bytes_left--;

            if (possible_terminator != CALLBACK_TERMINATOR) {
                // discard remainder of packet if we still haven't found the terminator as it is hopelessly corrupt
                Warning(LOCATION, "%s has returned to multi_sexp_eval() without finding the terminator. Discarding packet! Trace out and fix this!", Operators[op_num].text.c_str());
                sexp_bytes_left = 0;
                return false;
            }
            else {
                // the previous SEXP hasn't removed all it's data from the packet correctly but it appears we've managed to fix it
                Warning(LOCATION, "%s has returned to multi_sexp_eval() without removing all the data the server wrote during its callback. Trace out and fix this!", Operators[op_num].text.c_str());
                op_num = -1;
            }
        }
    }

    return true;
}

void sexp_network_packet::set_data(ubyte * received_packet, int num_ubytes)
{
    offset = 0;
    op_num = -1;

	const auto r_data_size = std::min(SEXP_MAX_PACKET_SIZE, num_ubytes);
	memcpy(data, received_packet, static_cast<size_t>(r_data_size));

	sexp_bytes_left = r_data_size;
}

void sexp_network_packet::initialize()
{
    if (!MULTIPLAYER_MASTER) {
        return;
    }

	for (int i = 0; i < SEXP_MAX_PACKET_SIZE; ++i) {
        data[i] = 0;
        type[i] = packet_data_type::NOT_DATA;
    }

    packet_size = 0;
    argument_count_index = -1;
    current_argument_count = 0;
}

void sexp_network_packet::start_callback()
{
    if (!MULTIPLAYER_MASTER) {
        return;
    }

    // Ensure that there is enough space in the packet. At present the function writes the Operator number and the argument count into the packet. Both are ints 
    // so we must ensure we have space left to write two ints. If this function is changed to write other data, this line MUST be updated too!
    ensure_space_remains(sizeof(int) * 2);

    callback_started = true;

    //Write OP into the Type buffer.
    type[packet_size] = packet_data_type::SEXP_OPERATOR;
    //Write the SEXP_Operator number into the data buffer.
    Assert(!Current_sexp_operator.empty());
    ADD_INT(Current_sexp_operator.back());

    //Store the next data index as we'll need it later to write the COUNT.
    argument_count_index = packet_size;
    // store an invalid count, we'll come back and store the correct value once we know what it is.	
    type[packet_size] = packet_data_type::ARGUMENT_COUNT;
	ADD_SHORT(TEMP_DATA_SIZE);
}

void sexp_network_packet::end_callback()
{
    if (!MULTIPLAYER_MASTER) {
        return;
    }

    // ensure that there is enough space in the packet. If this function is ever updated to write anything other than the terminator, this MUST be taken into account here!
    ensure_space_remains(sizeof(ubyte));

    callback_started = false;

    // something is wrong with the packet, blast it clean and start again
    if (packet_flagged_invalid) {
        initialize();
        packet_flagged_invalid = false;
        return;
    }

    //write TERMINATOR into the Type and data buffers
    type[packet_size] = packet_data_type::DATA_TERMINATES;
	ADD_DATA(CALLBACK_TERMINATOR);

    //Write the COUNT into the data buffer at the index we saved earlier.
    int temp_packet_size = packet_size;
    packet_size = argument_count_index;
	ADD_SHORT(static_cast<short>(current_argument_count));
    packet_size = temp_packet_size;

    current_argument_count = 0;
}

void sexp_network_packet::do_callback()
{
    start_callback();
    end_callback();
}

void sexp_network_packet::sexp_flush_packet()
{
    if (!MULTIPLAYER_MASTER) {
        return;
    }

    // possible to get here when there is nothing in the packet to send
    if (packet_size == 0) {
        return;
    }
    Assert(type[packet_size - 1] == packet_data_type::DATA_TERMINATES);
    Assert(!packet_flagged_invalid);

    send_sexp_packet(data, packet_size);

    initialize();
}

bool sexp_network_packet::cannot_send_data()
{
    if (!MULTIPLAYER_MASTER || packet_flagged_invalid) {
        return true;
    }

    if (!callback_started) {
        Warning(LOCATION, "Attempt to send data in multi_sexp.cpp without first starting a callback");
        return true;
    }

    return false;
}

void sexp_network_packet::send_int(int value)
{
    if (cannot_send_data()) {
        return;
    }

    ensure_space_remains(sizeof(value));

    //Write INT into the Type buffer.
    type[packet_size] = packet_data_type::INT;
    //Write the int into the data buffer
    ADD_INT(value);
    //Increment the COUNT by 4 (i.e the size of an int).
    current_argument_count += sizeof(int);
}

void sexp_network_packet::send_wing(wing * wingp)
{
	if (cannot_send_data()) {
		return;
	}

	ensure_space_remains(sizeof(ushort));

	//write into the Type buffer.
	type[packet_size] = packet_data_type::WING;
	//write the into the data buffer
	ADD_USHORT(wingp->net_signature);
	current_argument_count += sizeof(ushort);
}

void sexp_network_packet::send_ship(ship * shipp)
{
    if (cannot_send_data()) {
        return;
    }

    ensure_space_remains(sizeof(ushort));

    //write into the Type buffer.
    type[packet_size] = packet_data_type::SHIP;
    //write the into the data buffer
    ADD_USHORT(Objects[shipp->objnum].net_signature);
    current_argument_count += sizeof(ushort);
}

void sexp_network_packet::send_object(object * objp)
{
    if (cannot_send_data()) {
        return;
    }

    ensure_space_remains(sizeof(ushort));

    //write into the Type buffer.
    type[packet_size] = packet_data_type::OBJECT;
    //write the into the data buffer
    ADD_USHORT(objp->net_signature);
    current_argument_count += sizeof(ushort);
}

void sexp_network_packet::send_parse_object(p_object * pobjp)
{
    if (cannot_send_data()) {
        return;
    }

    ensure_space_remains(sizeof(ushort));

    //write into the Type buffer.
    type[packet_size] = packet_data_type::PARSE_OBJECT;
    //write the into the data buffer
    ADD_USHORT(pobjp->net_signature);
    current_argument_count += sizeof(ushort);
}

void sexp_network_packet::send_string(char * string)
{
    if (cannot_send_data()) {
        return;
    }

	ensure_space_remains(strlen(string) + sizeof(uint16_t));

    int start_size = packet_size;
    //write into the Type buffer.
    type[packet_size] = packet_data_type::STRING;
    //write the into the data buffer
	ADD_STRING_16(string);
    current_argument_count += packet_size - start_size;
}

void sexp_network_packet::send_string(const SCP_string & string)
{
    if (cannot_send_data()) {
        return;
    }

	ensure_space_remains(string.length() + sizeof(uint16_t));

    int start_size = packet_size;
    //write into the Type buffer.
    type[packet_size] = packet_data_type::STRING;
    //write the into the data buffer
	ADD_STRING_16(string.c_str());
    current_argument_count += packet_size - start_size;
}

void sexp_network_packet::send_bool(bool value)
{
    if (cannot_send_data()) {
        return;
    }

    ensure_space_remains(sizeof(value));

    //write into the Type buffer.
    type[packet_size] = packet_data_type::BOOLEAN;
    //Write the value into the data buffer
    ADD_DATA(value);
    //Increment the COUNT 
    current_argument_count += sizeof(value);
}

void sexp_network_packet::send_float(float value)
{
    if (cannot_send_data()) {
        return;
    }

    ensure_space_remains(sizeof(value));

    //write into the Type buffer.
    type[packet_size] = packet_data_type::FLOAT;
    //Write the value into the data buffer
    ADD_FLOAT(value);
    //Increment the COUNT 
    current_argument_count += sizeof(float);
}

void sexp_network_packet::send_vec3d(vec3d *value)
{
	for (int i = 0; i < 3; ++i)     // NOLINT
		send_float(value->a1d[i]);
}

void sexp_network_packet::send_matrix(matrix *value)
{
	for (int i = 0; i < 9; ++i)     // NOLINT
		send_float(value->a1d[i]);
}

void sexp_network_packet::send_short(short value)
{
    if (cannot_send_data()) {
        return;
    }

    ensure_space_remains(sizeof(value));

    //Write the type into the Type buffer.
    type[packet_size] = packet_data_type::SHORT;
    //Write the value into the data buffer
    ADD_SHORT(value);
    //Increment the COUNT 
    current_argument_count += sizeof(short);
}

void sexp_network_packet::send_ushort(ushort value)
{
    if (cannot_send_data()) {
        return;
    }

    ensure_space_remains(sizeof(value));

    //Write the type into the Type buffer.
    type[packet_size] = packet_data_type::USHORT;
    //Write the value into the data buffer
    ADD_USHORT(value);
    //Increment the COUNT 
    current_argument_count += sizeof(ushort);
}

bool sexp_network_packet::get_int(int & value)
{
    if (!sexp_bytes_left || !current_argument_count) {
        return false;
    }

    GET_INT(value);
    reduce_counts(sizeof(int));

    return true;
}

bool sexp_network_packet::get_ship(int & value)
{
    ushort netsig;
    object *objp;

    if (!sexp_bytes_left || !current_argument_count) {
        return false;
    }

    // get the net signature of the ship
    GET_USHORT(netsig);
    reduce_counts(sizeof(ushort));

    // lookup the object
    objp = multi_get_network_object(netsig);
    if ((objp != NULL) && (objp->type == OBJ_SHIP) && (objp->instance >= 0)) {
        value = objp->instance;
        return true;
    }

    Warning(LOCATION, "Current_sexp_network_packet.get_ship called for net signature %d even though it is not a ship", netsig);
    return false;
}

bool sexp_network_packet::get_ship(ship *& shipp)
{
    int shipnum;

    if (get_ship(shipnum)) {
        shipp = &Ships[shipnum];
        return true;
    }

    return false;
}

bool sexp_network_packet::get_wing(wing *& wingp)
{
	int i;
    ushort netsig;

    if (!sexp_bytes_left || !current_argument_count) {
        return false;
    }

    // get the net signature of the wing
    GET_USHORT(netsig);
    reduce_counts(sizeof(ushort));

    // lookup the wing
	for (i = 0; i < Num_wings; i++) {
		if (Wings[i].net_signature == netsig) {
			wingp = &Wings[i];
			return true;
		}
	}

    Warning(LOCATION, "Current_sexp_network_packet.get_wing called for net signature %d even though it is not a ship", netsig);
    return false;
}

bool sexp_network_packet::get_object(object *& value)
{
    ushort netsig;

    if (!sexp_bytes_left || !current_argument_count) {
        return false;
    }

    // get the net signature of the ship
    GET_USHORT(netsig);
    reduce_counts(sizeof(ushort));

    // lookup the object
    value = multi_get_network_object(netsig);
    if ((value != NULL) && (value->instance >= 0)) {
        return true;
    }

    Warning(LOCATION, "multi_get_object called for non-existent object");
    return false;
}

bool sexp_network_packet::get_parse_object(p_object *& pobjp)
{
    ushort netsig;

    if (!sexp_bytes_left || !current_argument_count) {
        return false;
    }

    // get the net signature of the ship
    GET_USHORT(netsig);
    reduce_counts(sizeof(ushort));

    // lookup the object
    pobjp = mission_parse_get_arrival_ship(netsig);
    if (pobjp != NULL) {
        return true;
    }

    return false;
}

bool sexp_network_packet::get_string(char * buffer, const size_t buf_len)
{
	char tempstring[SEXP_MAX_PACKET_SIZE];
    int starting_offset = offset;

    if (!sexp_bytes_left || !current_argument_count) {
        return false;
    }

	GET_STRING_16(tempstring);
	strcpy_s(buffer, buf_len, tempstring);
    reduce_counts(offset - starting_offset);

    return true;
}

bool sexp_network_packet::get_string(SCP_string & buffer)
{
	char tempstring[SEXP_MAX_PACKET_SIZE];
    int starting_offset = offset;

    if (!sexp_bytes_left || !current_argument_count) {
        return false;
    }

	GET_STRING_16(tempstring);
    buffer = tempstring;
    reduce_counts(offset - starting_offset);

    return true;
}

bool sexp_network_packet::get_bool(bool & value)
{
    if (!sexp_bytes_left || !current_argument_count) {
        return false;
    }

    GET_DATA(value);
    reduce_counts(sizeof(value));

    return true;
}

bool sexp_network_packet::get_float(float & value)
{
    if (!sexp_bytes_left || !current_argument_count) {
        return false;
    }

    GET_FLOAT(value);
    reduce_counts(sizeof(float));

    return true;
}

bool sexp_network_packet::get_vec3d(vec3d *value)
{
	for (int i = 0; i < 3; ++i)     // NOLINT
	{
		if (!get_float(value->a1d[i]))
			return false;
	}
	return true;
}

bool sexp_network_packet::get_matrix(matrix *value)
{
	for (int i = 0; i < 9; ++i)     // NOLINT
	{
		if (!get_float(value->a1d[i]))
			return false;
	}
	return true;
}

bool sexp_network_packet::get_short(short & value)
{
    if (!sexp_bytes_left || !current_argument_count) {
        return false;
    }

    GET_SHORT(value);
    reduce_counts(sizeof(short));

    return true;
}

bool sexp_network_packet::get_ushort(ushort & value)
{
    if (!sexp_bytes_left || !current_argument_count) {
        return false;
    }

    GET_USHORT(value);
    reduce_counts(sizeof(ushort));

    return true;
}

int sexp_network_packet::get_next_operator()
{
    if (!argument_count_is_valid()) {
        return -1;
    }

	short count;

    GET_INT(op_num);
    sexp_bytes_left -= sizeof(int);
	GET_SHORT(count);
	current_argument_count = count;
	sexp_bytes_left -= sizeof(short);

    Assert(sexp_bytes_left);
    return op_num;

}

int sexp_network_packet::get_operator()
{
    return op_num;
}

void sexp_network_packet::finished_callback()
{
    ubyte terminator;

    Assert(current_argument_count == 0);

    // read in the terminator
    GET_DATA(terminator);
    if (terminator != CALLBACK_TERMINATOR) {
        Warning(LOCATION, "multi_get_x function call has been called on an improperly terminated callback. Trace out and fix this!");
        // discard remainder of packet
        sexp_bytes_left = 0;
        return;
    }
    sexp_bytes_left--;
    op_num = -1;
}

bool sexp_network_packet::sexp_discard_operator()
{
    int i;
    ubyte dummy;
    ubyte terminator;

    // read in a number of bytes equal to the count
    for (i = 0; i<current_argument_count; i++) {
        GET_DATA(dummy);
        sexp_bytes_left--;
    }

    GET_DATA(terminator);
    sexp_bytes_left--;
    op_num = -1;

    // the operation terminated correctly, probably a new SEXP that this version doesn't support. 
    if (terminator == CALLBACK_TERMINATOR)
        return true;

    // packet is probably corrupt
    else
        return false;
}

void sexp_network_packet::discard_remaining_callback_data()
{
    if (!sexp_discard_operator()) {
        Warning(LOCATION, "Attempt to discard remaining data failed! Callback lacks proper termination. Entire packet may be corrupt. Discarding remaining packet");
    }
}

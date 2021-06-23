/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "globalincs/pstypes.h"
#include "network/psnet2.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "ship/ship.h"


void sexp_packet_received(ubyte *received_packet, int num_ubytes);

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
// This suppresses a GCC bug where it thinks that some of the enum fields below shadow global declarations even though
// the enum class names are not visible in the global namespace
#pragma GCC diagnostic ignored "-Wshadow"
#endif

enum class packet_data_type {
    NOT_DATA			= 255,
    SEXP_OPERATOR		= 0,
    ARGUMENT_COUNT		= 1,
    DATA_TERMINATES	    = 2,
    INT				    = 3,
    SHIP				= 4,
    STRING				= 5,
    PARSE_OBJECT		= 6,
    BOOLEAN			    = 7,
    FLOAT				= 8,
    SHORT				= 9,
    USHORT				= 10,
    OBJECT				= 11,
	WING				= 12,
};

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

// Minimum size that a valid sexp packet can be
//  4 bytes - OP
//  2 bytes - COUNT
//  1 byte  - TERMINATOR
#define MIN_SEXP_PACKET_SIZE	7

class sexp_network_packet {
private:
	#define SEXP_MAX_PACKET_SIZE	(MAX_PACKET_SIZE - HEADER_LENGTH - static_cast<int>(sizeof(ushort)))

	ubyte data[SEXP_MAX_PACKET_SIZE];
    int packet_size = 0;
    int offset = 0;

    // the type array holds information on the type of date held at the same index of the data array
    // types are not sent to the client and the entire array could be replaced with a couple of variables indexing the end of 
    // the previous SEXP. However it is much more helpful when debugging to have the array
	packet_data_type type[SEXP_MAX_PACKET_SIZE];
    int argument_count_index = -1;			// index in the type and data arrays for the argument count
	int current_argument_count = 0;			// number of bytes the data for this SEXP currently takes up
    bool packet_flagged_invalid = false;
    bool callback_started = false;
    int op_num = 0;

    /**
    * Checks if there is enough space in the packet currently being stuffed for the data that is about to be written into it
    *
    * If there is not enough space, it will send everything in the packet apart from any data from the SEXP currently being processed
    * and then create a new packet containing the data for this SEXP only.
    */
    void ensure_space_remains(size_t data_size);

    /**
    * Ensures that the variables tracking how much data is left in the packet are updated correctly when data is removed.
    */
    void reduce_counts(int amount);

    /**
    * Checks that the previous SEXP in the packet has correctly removed all its data from the packet. Attempts to fix it if it hasn't.
    */
    bool argument_count_is_valid();
public:
    int sexp_bytes_left = 0;

    void set_data(ubyte* data, int num_ubytes);

    /**
    * Set up the SEXP packet every frame when the game starts processing SEXPs.
    */
    void initialize();

    /**
    * Called when a server is currently processing a SEXP that needs to send an update to the clients.
    */
    void start_callback();

    /**
    * Called when a server has written all the data it needs to write for this SEXP.
    */
    void end_callback();

    /**
    * Convenience function that simply calls the two functions above. Used when the server merely needs to signal the clients that a
    * SEXP has been processed but no additional data needs to be sent.
    */
    void do_callback(); 

    /**
    * Flushes out the SEXP packet and sends any data still in there. Called when the game finishes processing SEXPs.
    */
    void sexp_flush_packet();

    /**
    * Checks if the SEXP packet is ready to recieve data.
    */
    bool cannot_send_data();

    /********************************
    HOST SIDE DATA WRAPPER FUNCTIONS
    *******************************/

    /**
    * Add an int to the SEXP packet.
    */
    void send_int(int value);
    
    /**
    * Adds a ship's net sig to the SEXP packet.
    */
    void send_ship(ship *shipp);
    
	/**
	* Adds a wing's net sig to the SEXP packet.
	*/
	void send_wing(wing *wingp);

	/**
    * Add the net sig of an object to the SEXP packet.
    */
    void send_object(object *objp);

    /**
    * Add the net sig of a parse object to the SEXP packet.
    */
    void send_parse_object(p_object *pobjp);

    /**
    * Add a string to the SEXP packet. Should only be used for strings TOKEN_LENGTH in size or smaller.
    */
    void send_string(char *string);
    void send_string(const SCP_string &string);

    /**
    * Add a boolean to the SEXP packet.
    */
    void send_bool(bool value);

    /**
    * Add a float to the SEXP packet.
    */
    void send_float(float value);

	/**
	 * Add three floats in a row.
	 */
	void send_vec3d(vec3d *value);

	/**
	 * Add nine floats in a row.
	 */
	void send_matrix(matrix *value);

	/**
    * Add a short to the SEXP packet.
    */
    void send_short(short value);

    /**
    * Add an unsigned short to the SEXP packet.
    */
    void send_ushort(ushort value);
    
    /**
    * Add flag value for use with flagsets to the SEXP packet.
    */
    template<typename T>
    void send_flag(T value)
    {
        if (cannot_send_data()) {
            return;
        }

        ensure_space_remains(sizeof(int));

        //Write INT into the Type buffer.
        type[packet_size] = packet_data_type::INT;
        //Write the int into the data buffer
        ADD_INT(static_cast<int>(value));
        //Increment the COUNT by 4 (i.e the size of an int).
        current_argument_count += sizeof(int);
    }

    /**********************************
    CLIENT SIDE DATA WRAPPER FUNCTIONS
    *********************************/

    /**
    * Attempts to remove an int from the SEXP packet and assign it to the value parameter. Returns false if it is unable to do so.
    */   
    bool get_int(int &value);

	/**
	* Attempts to get an index for the Ships array based on the net sig it removes from the SEXP packet. Returns it as the value
	* parameter. Returns false if unable to do so.
	*/
	bool get_ship(int &value);

	/**
    * Attempts to get a ship pointer based on the net sig it removes from the SEXP packet. Returns it as the value parameter.
    * Returns false if unable to do so.
    */
    bool get_ship(ship*& shipp);
    
	/**
	* Attempts to get a wing pointer based on the net sig it removes from the SEXP packet. Returns it as the value parameter.
	* Returns false if unable to do so.
	*/
	bool get_wing(wing*& wingp);

	/**
    * Attempts to get an object pointer based on the net sig it removes from the SEXP packet. Returns it as the value parameter.
    * Returns false if unable to do so.
    */
    bool get_object(object*& value);

    /**
    * Attempts to get a parse objects pointer based on the net sig it removes from the SEXP packet. Returns it as the value parameter.
    * Returns false if unable to do so.
    */
    bool get_parse_object(p_object*& pobjp);

    /**
    * Attempts to remove a string from the SEXP packet and assign it to the value parameter. Returns false if it is unable to do so.
    */
	bool get_string(char *buffer, const size_t buf_len);
    bool get_string(SCP_string &buffer);
	template<size_t size>
	inline bool get_string(char (&buffer)[size]) {
		return get_string(buffer, size);
	}

    /**
    * Attempts to remove a boolean from the SEXP packet and assign it to the value parameter. Returns false if it is unable to do so.
    */
    bool get_bool(bool &value);

    /**
    * Attempts to remove a float from the SEXP packet and assign it to the value parameter. Returns false if it is unable to do so.
    */
    bool get_float(float &value);

	/**
	* Attempts to remove a vec3d from the SEXP packet and assign it to the value parameter. Returns false if it is unable to do so.
	*/
	bool get_vec3d(vec3d *value);

	/**
	* Attempts to remove a matrix from the SEXP packet and assign it to the value parameter. Returns false if it is unable to do so.
	*/
	bool get_matrix(matrix *value);

	/**
    * Attempts to remove a short from the SEXP packet and assign it to the value parameter. Returns false if it is unable to do so.
    */
    bool get_short(short &value);

    /**
    * Attempts to remove an unsigned short from the SEXP packet and assign it to the value parameter. Returns false if it is unable to do so.
    */
    bool get_ushort(ushort &value);

    /**
    * Attempts to remove a flagset value from the SEXP packet and assign it to the value parameter. Returns false if it is unable to do so.
    */
    template <typename T>
    bool get_flag(T &value)
    {
        if (!sexp_bytes_left || !current_argument_count) {
            return false;
        }
        int tmp = 0;
        GET_INT(tmp);

        value = static_cast<T>(tmp);

        reduce_counts(sizeof(int));

        return true;
    }

    /**
    * Gets the next operator from the SEXP packet. Returns the number of the operator or -1 if there are no more operators because the
    * packet is corrupt.
    */
    int get_next_operator();

    /**
    * Returns the current operator number but does not touch the SEXP packet.
    */
    int get_operator();

    /**
    * Called when the SEXP code has finished processing the current SEXP.
    */
    void finished_callback();

    /**
    * Used to discard the rest of a callback on the client machine.
    */
    bool sexp_discard_operator();

    /**
    * attempts to remove all remaining data for the current operator.
    */
    void discard_remaining_callback_data();
};

extern sexp_network_packet Current_sexp_network_packet;

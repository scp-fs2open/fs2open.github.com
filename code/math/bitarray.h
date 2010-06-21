/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 



#ifndef _BIT_ARRAY_H
#define _BIT_ARRAY_H

#include "globalincs/pstypes.h"


// the following four functions are adapted from http://www.codeproject.com/cpp/BitArray.asp; their explanation
// is as follows:
/*
 * The Bit Array structure provides a compacted arrays of Booleans, with one bit for each Boolean value.
 * A 0 (1) bit corresponds to the Boolean value false (true), respectively. We can look at a stream of bytes
 * as a stream of bits; each byte contains 8 bits, so any n bytes hold n*8 bits. And the operation to
 * manipulate this stream or bit array is so easy, just read or change the bit's state or make any Boolean
 * operation on the whole bits array, like ‘AND’, ‘OR’, or ‘XOR’.
 *
 * As each byte contains 8 bits, we need to divide the bit number by 8 to reach the byte that holds the bit.
 * Then, we can seek to the right bit in the reached byte by the remainder of dividing the bit number by 8.
 * So to read or change the bit state, the operations will be like that.
 *
 * Note that to divide by 8, we need only to shift right by 3 (>>3), and to get the remainder of dividing
 * by 8, we need only to AND with 7 (&7).
 */

// returns bit state (0 or 1)
#define get_bit(array, bitnum)			((((ubyte *) array)[(bitnum) >> 3] >> ((bitnum) & 7)) & 1)

// sets bit to 1
#define set_bit(array, bitnum)			(((ubyte *) array)[(bitnum) >> 3] |= (1 << ((bitnum) & 7)))

// clears bit to 0
#define clear_bit(array, bitnum)		(((ubyte *) array)[(bitnum) >> 3] &= ~(1 << ((bitnum) & 7)))

// toggles bit (xor)
#define toggle_bit(array, bitnum)		(((ubyte *) array)[(bitnum) >> 3] ^= (1 << ((bitnum) & 7)))


// calculate number of bytes from number of bits
#define calculate_num_bytes(num_bits)	((num_bits >> 3) + 1)


#endif	// _BIT_ARRAY_H

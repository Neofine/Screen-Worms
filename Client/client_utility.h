#ifndef SK2_CLIENT_UTILITY_H
#define SK2_CLIENT_UTILITY_H

#include "client_structures.h"

// Prints out on error output given arguments and exits program
// returning EXIT_FAILURE.
void syserr(const string& out);

// Changes current players worm direction depending on information got from GUI.
void change_direction(game_info *game, const string& command);

// Given array of bytes and its length returns its CRC32 encryption.
uint32_t calc_crc(const uint8_t* data, size_t bytes_amount);

// Returns current time in useconds.
suseconds_t get_time_now();

// Changing 4 consecutive bytes wrote in big endian to uint32_t.
uint32_t get32(uint8_t *mess);

#endif //SK2_CLIENT_UTILITY_H

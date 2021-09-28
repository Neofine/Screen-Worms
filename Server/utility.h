#ifndef SK2_UTILITY_H
#define SK2_UTILITY_H

#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <cmath>

#include "utility.h"
#include "structures.h"

// Prints out given string and exits program with EXIT_FAILURE code.
void syserr(const string& out);

// Given a number and a string representing this number
// if those two don't match exit is called.
void validate_length(uint64_t number, const string& str_number);

// Given filled structure constants, exits if any of them
// have invalid value.
void validate_values(constants *prog);

// Given argc and argv arguments if those contain correct
// values then fills structure constants up, if not then
// exits programme with certain error message.
void validate_flags(int argc, char *argv[], constants *prog);

// Checks if player of given name already exists.
bool exists_player_with_name(memory *mem, const string &player_name);

// Writes into on_what string "client_address:client_port"
// where client_address and client_port are variables.
void sockaddr_to_string(string &on_what, sockaddr_in6 *client_info);

// Given vector and certain number with amount of bytes it adds to vector
// this number byte by byte in big endian order.
void append_vector(vector<uint8_t> *left_side, uint32_t by_what, int bytes_num);

// Comparator to sort worms lexicographically by their names.
bool cmp_lex(worm *a, worm *b);

// Given double returns its floor cast on integer.
int double_to_int(double number);

// Moves worm by one in direction it is facing
// returns true if integer part of coordinates changed during this move.
bool move_by_one(worm *to_move);

// Deterministic number generator based on once given seed.
uint32_t det_rand(uint32_t seed = 0);

// Given vector of bytes calculates and returns its CRC32.
uint32_t calc_crc(vector<uint8_t> *data);

// Changes message values to big endian and validates it.
bool is_message_valid(player_message *mess, ssize_t mess_len);

#endif //SK2_UTILITY_H

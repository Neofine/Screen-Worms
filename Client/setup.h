#ifndef SK2_SETUP_H
#define SK2_SETUP_H

#include "client_utility.h"

// Given argc and argv arguments if those contain correct
// values then fills structures communication and current_state up, if not then
// exits programme with certain error message.
void validate_flags(int argc, char *argv[], communication *prog, game_info *game);

// Depending on to_gui flag connects either to GUI via TCP or to server via UDP
void set_connection(communication *comm, bool to_gui = false);

#endif //SK2_SETUP_H

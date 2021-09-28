#ifndef SK2_EVENTS_H
#define SK2_EVENTS_H

#include "utility.h"
#include "structures.h"
#include "communicator.h"

// Given event data and its type, sends it to all connected
// clients and archives it for further use for updating clients.
void generate_and_send_event(communication *comm, memory *mem,
                             const vector<uint8_t>& event_data, event_types what);

// Given number of eliminated player makes event data with them and calls
// generate_and_send_event to send this event to every conencted client.
void player_eliminated_event(communication *comm, memory *mem, int p_number);

// Given players worm makes event data with them and calls
// generate_and_send_event to send this event to every conencted client
void pixel_eaten_event(communication *comm, memory *mem, worm *pawn);

// Sends game beginning event, function which is called when every player
// sent at least one packet with turn direction other than forward.
void new_game_event(communication *comm, constants *prog, memory *mem);

#endif //SK2_EVENTS_H

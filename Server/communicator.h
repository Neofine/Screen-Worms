#ifndef SK2_TIMERS_H
#define SK2_TIMERS_H

// Creates timer at specified index, if tick_time is not zero then
// creates server timer with time between ticks equal to tick_time
// nanoseconds, else it sets timeout value (CLIENT_TIMEOUT) for a client.
void create_timer(communication *comm, int id, long tick_time);

// If we read message from client then we need to reset its
// timer counting down to disconnection.
void renew_timer(communication *comm, int id);

// Creates IPv6 socket accepting both IPv4 and IPv6,
// sets timers array to default unused values.
void set_connection(communication *comm, constants *prog);

// Connects client to the game if it is possible, sets their timer,
// makes new worm if client isn't spectator and updates structure memory.
void connect_client(communication *comm, memory *mem, player_message *mess,
                    const string &socket, sockaddr_in6 *client_info);

// Sends events from from_when variable up to current event to the client.
void send_latest_events(communication *comm, memory *mem,int from_when,
                        sockaddr_in6 *to_whom);

// Checks if client should be disconnected because of lack of communication
// from their side, if no information has been read in the span
// of last CLIENT_TIMEOUT seconds then disconnects him,
// returns true it does, false in opposite case.
bool client_disconnect_check(communication *comm, memory *mem, int timer_id);

// Checks timers of all clients for a disconnection,
// if client hasn't communicated in recent time then disconnect them.
void check_for_disconnect(communication *comm, memory *mem);

#endif //SK2_TIMERS_H

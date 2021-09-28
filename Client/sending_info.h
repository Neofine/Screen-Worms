#ifndef SK2_SENDING_INFO_H
#define SK2_SENDING_INFO_H

#include "client_utility.h"

// Sends message struct to server.
void send_to_server(communication *comm, game_info *game);

// Given board dimensions and vector of player names sends
// NEW_GAME event to GUI.
void send_new_game(communication *comm, uint32_t maxx, uint32_t maxy,
                   vector<string> &names);

// Given player name and its coordinates sends PIXEL event to GUI.
void send_eaten_pixel(communication *comm, const string& name,
                      uint32_t x, uint32_t y);

// Given player name sends PLAYER_ELIMINATED event to GUI.
void send_player_eliminated(communication *comm, const string& name);

#endif //SK2_SENDING_INFO_H

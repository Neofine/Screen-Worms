#include "sending_info.h"
#include <unistd.h>

void send_to_server(communication *comm, game_info *game) {

    // Changes from endian of current PC to big endian in order to
    // correctly send this struct.
    game->server_info.session_id = htobe64(game->server_info.session_id);
    game->server_info.next_event_no = htobe32(game->server_info.next_event_no);

    // Sending message of size CLIENT_MESS_NO_NAME + length of player name.
    ssize_t snd_len = write(comm->game_socket, &game->server_info,
                            CLIENT_MESS_NO_NAME + game->player_name_length);

    if (snd_len == 0) {
        syserr("Game server socket closed!\n");
    }
    else if (snd_len < 0 || snd_len != 13 + game->player_name_length) {
        printf("Error in sending message to game server.\n");
    }

    // Reversing endian conversion to correctly operate on this information
    game->server_info.session_id = be64toh(game->server_info.session_id);
    game->server_info.next_event_no = be32toh(game->server_info.next_event_no);
}

// Sends char array to GUI.
void send_to_gui(communication *comm, const string &to_send) {
    char buffer[to_send.size() + 1];
    // Converting string into char array in order to correcly send it.
    strcpy(buffer, to_send.c_str());

    if (write(comm->gui_socket, buffer, to_send.size()) !=
        (long int) to_send.size()) {
        syserr("Error in sending information to GUI");
    }
}

void send_new_game(communication *comm, uint32_t maxx, uint32_t maxy,
                   vector<string> &names) {
    string to_send = "NEW_GAME ";
    to_send += std::to_string(maxx) + " " + std::to_string(maxy);

    for (auto &name: names) {
        to_send += " " + name;
    }

    to_send += "\n";

    send_to_gui(comm, to_send);
}

void send_eaten_pixel(communication *comm, const string& name,
                      uint32_t x, uint32_t y) {
    string to_send = "PIXEL ";
    to_send += std::to_string(x) + " " + std::to_string(y);
    to_send += " " + name + "\n";

    send_to_gui(comm, to_send);
}

void send_player_eliminated(communication *comm, const string& name) {
    string to_send = "PLAYER_ELIMINATED ";
    to_send += name + "\n";

    send_to_gui(comm, to_send);
}
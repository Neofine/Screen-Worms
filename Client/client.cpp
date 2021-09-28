#include "client_structures.h"
#include "client_utility.h"
#include "../setup.h"
#include "client_events.h"
#include "sending_info.h"

// Reads information from GUI and responds to it if it is a full command.
void read_from_gui(communication *comm, game_info *game) {
    ssize_t rcv_len;
    char single_char;

    // Reads character by character.
    while ((rcv_len = read(comm->gui_socket, &single_char, 1)) > 0) {
        game->gui_message += single_char;
        if (single_char == '\n')
            break;
    }

    if (rcv_len == 0)
        syserr("Gui socket was closed");

    // If full command was read then process it.
    if (single_char == '\n') {
        change_direction(game, game->gui_message);
        game->gui_message.clear();
    }
}

// Given message and its length this function parses it and processes
// single commands, if there is something wrong with this datagram
// then it either ignores it or exits program.
void process_message(communication *comm, game_info *game,
                     uint8_t *mess, size_t mess_len) {
    uint32_t game_id = get32(mess);

    // Tells how much to add to mess for datagram to have new command to parse.
    uint16_t to_add = UINT32_BYTE_SIZE;

    while(to_add < mess_len) {
        uint16_t event_size = process_event(comm, game, game_id,
                                            mess + to_add, mess_len - to_add);
        if (event_size == IGNORE_REST_DGRAM)
            return;
        to_add += event_size;
    }
}

// Tries reading message from server if it succeeds then processes and
// responds to it in appropriate way.
void read_from_server(communication *comm, game_info *game) {
    uint8_t buffer[SERVER_MSG_MAX + 1];

    ssize_t rcv_len = read(comm->game_socket, buffer, SERVER_MSG_MAX + 1);

    if (rcv_len <= 0 || rcv_len == SERVER_MSG_MAX + 1 ||
        rcv_len < MIN_SERV_MESS_SIZE) {
        if (rcv_len == 0)
            syserr("connection with server has ended.");
        return;
    }

    process_message(comm, game, buffer, rcv_len);
}

int main(int argc, char *argv[]) {
    communication comm{};
    game_info game{};

    validate_flags(argc, argv, &comm, &game);

    // Set current session id.
    game.server_info.session_id = get_time_now();

    // Set connection to GUI and game server.
    set_connection(&comm, true);
    set_connection(&comm);

    // Variable telling when to send next message to the server.
    suseconds_t when_to_send = 0;

    // Playing games endlessly.
    while(true) {
        if (when_to_send <= get_time_now() || when_to_send == 0) {
            read_from_gui(&comm, &game);
            send_to_server(&comm, &game);
            if (when_to_send == 0)
                when_to_send = get_time_now();
            when_to_send += 30'000;
        }
        else {
            read_from_server(&comm, &game);
        }
    }
}
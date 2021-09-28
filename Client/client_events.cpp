#include "client_events.h"

// Responds to event 0, which is new game event, if it has invalid values then
// exits with syserr.
void respond_to_event_0(communication *comm, game_info *game, uint8_t *mess,
                        uint16_t event_data_len) {

    game->board_width = get32(mess + NEW_GAME_WIDTH_IDX);
    game->board_height = get32(mess + NEW_GAME_HEIGHT_IDX);

    // Check if board is in acceptable sizes.
    if (game->board_width > MAX_BOARD_WIDTH ||
        game->board_height > MAX_BOARD_HEIGHT) {
        syserr("Got event with board of enormous sizes.");
    }

    game->players.clear();
    string single_nick_name;

    // Iterate over nick names of all players.
    for (int i = NEW_GAME_NAMES_BEGIN;
         i < NEW_GAME_NAMES_BEGIN + event_data_len - NEW_GAME_PRE_NAMES_SIZE; i++) {
        if (mess[i] != 0) {
            if (mess[i] < MIN_ACCEPT_CHAR || mess[i] > MAX_ACCEPT_CHAR) {
                syserr("Got event with invalid player name.");
            }
            single_nick_name += mess[i];
        }
        else {
            if (single_nick_name.empty()) {
                syserr("Player name of size 0, blunder!");
            }

            if (!game->players.empty() &&
                game->players.back() >= single_nick_name) {

                syserr("Not alphabetically ordered players!");
            }


            game->players.push_back(single_nick_name);
            single_nick_name.clear();
        }
    }

    if (game->players.size() > MAX_PLAYERS || game->players.size() < MIN_PLAYERS) {
        syserr("Got new game event with too many players.");
    }

    send_new_game(comm, game->board_width, game->board_height, game->players);
}

// Responds to event 0, which is pixel eaten event, if it has invalid values then
// exits with syserr.
void respond_to_event1(communication *comm, game_info *game, uint8_t *mess,
                       uint16_t event_data_len) {

    if (event_data_len != PIXEL_EV_SIZE)
        syserr("Invalid values in datagram");

    uint8_t player_number = mess[PIXEL_PL_NUM_IDX];

    if (player_number > game->players.size() - 1) {
        syserr("Got event with too big player number");
    }

    uint32_t x = get32(mess + PIXEL_X_IDX);
    uint32_t y = get32(mess + PIXEL_Y_IDX);

    if (x >= game->board_width || y >= game->board_height) {
        syserr("Got event happening out of board borders.");
    }

    send_eaten_pixel(comm, game->players[player_number], x, y);
}


uint16_t process_event(communication *comm, game_info *game, uint32_t game_id,
                       uint8_t *mess, size_t bytes_left) {
    if (bytes_left < 13)
        syserr("Datagram with invalid values was read.");

    uint32_t event_len = get32(mess);
    uint32_t event_no = get32(mess + MSG_EVNO_IDX);
    uint8_t event_type = mess[MSG_EVTP_IDX];

    if (game_id != game->game_id) {
        if (event_type == 0 && event_no == 0) {
            game->game_id = game_id,
            game->server_info.next_event_no = 0;
        }
        else if (event_type == 0)
            syserr("New game not beginning with event_no = 0.");
        else return IGNORE_REST_DGRAM;
    }

    if (bytes_left < LEN_AND_CRC_SIZE + event_len) {
        syserr("Datagram with invalid values was read.");
    }

    uint32_t crc32_calculated = calc_crc(mess, event_len + LEN_FIELD_SIZE);
    uint32_t crc32_got = get32(mess + event_len + LEN_FIELD_SIZE);

    if (crc32_got != crc32_calculated)
        return IGNORE_REST_DGRAM;

    if (event_no < game->server_info.next_event_no)
        return event_len + LEN_AND_CRC_SIZE;
    else if (event_no > game->server_info.next_event_no)
        return IGNORE_REST_DGRAM;

    uint16_t event_data_len = event_len - 5;

    if ((event_no != 0 && event_type == 0) || (event_no == 0 && event_type != 0))
        syserr("New game event can only occur at the very first event of the game.");

    if (event_type == 0) {
        if (game->server_info.next_event_no == 0 &&
            event_data_len >= NEW_GAME_EVENT_MIN) {
            respond_to_event_0(comm, game, mess, event_data_len);
            game->game_id = game_id;
        }
        else {
            syserr("Datagram with invalid values was read.");
        }
    }
    else if (event_type == 1) {
        respond_to_event1(comm, game, mess, event_data_len);
    }
    else if (event_type == 2) {
        uint8_t player_number = mess[9];

        if (player_number > game->players.size() - 1) {
            syserr("Got event with too big player number");
        }

        send_player_eliminated(comm, game->players[player_number]);
    }
    // Event GAME OVER is ignored.

    game->server_info.next_event_no++;
    return event_len + LEN_AND_CRC_SIZE;
}

#ifndef SK2_CLIENT_STRUCTURES_H
#define SK2_CLIENT_STRUCTURES_H

#include <unistd.h>
#include <regex>
#include <iostream>

#define SERVER_MSG_MAX          550
#define CRC_STARTING_NUM        4'294'967'295
#define LEN_AND_CRC_SIZE        8
#define LEN_FIELD_SIZE          4
#define NEW_GAME_EVENT_MIN      8
#define MAX_BOARD_HEIGHT        3'000
#define MAX_BOARD_WIDTH         3'000
#define IGNORE_REST_DGRAM       5'7575

#define MIN_ACCEPT_CHAR         33
#define MAX_ACCEPT_CHAR         126
#define MAX_PLAYERS             25

#define NEW_GAME_NAMES_BEGIN    17
#define NEW_GAME_PRE_NAMES_SIZE 8
#define NEW_GAME_WIDTH_IDX      9
#define NEW_GAME_HEIGHT_IDX     13

#define PIXEL_EV_SIZE           9
#define PIXEL_PL_NUM_IDX        9
#define PIXEL_X_IDX             10
#define PIXEL_Y_IDX             14

#define UINT32_BYTE_SIZE        4

#define MSG_EVNO_IDX            4
#define MSG_EVTP_IDX            8

#define CLIENT_MESS_NO_NAME     13
#define MIN_SERV_MESS_SIZE      4
#define MIN_PLAYERS             2

#define BYTE_4_OF_32            16777216
#define BYTE_3_OF_32            65536
#define BYTE_2_OF_32            256

#define USECS_IN_SEC            1'000'000


using std::string;
using std::vector;

// Enumerator for current direction of players worm.
enum direction {
    FORWARD,
    RIGHT,
    LEFT,
};

// Struct which holds necessary information to perform exchange
// of information with server and GUI.
struct communication {
    int gui_socket;
    int game_socket;

    string game_server_address;
    uint16_t game_server_port = 2021;

    string gui_server_address = "localhost";
    uint16_t gui_server_port = 20210;
};

// Message holding current state of the client, used to send
// information to the server.
struct current_state {
    uint64_t session_id = 0;
    uint8_t turn_direction = 0;
    uint32_t next_event_no = 0;
    uint8_t player_name[20]{};
} __attribute__((packed)); // Without attribute packed there is
                           // extra padding on turn direction.

// Holds all of the necessary information about currently played game.
struct game_info {
    uint32_t board_width = 0;
    uint32_t board_height = 0;
    uint32_t game_id = 0;
    uint8_t player_name_length = 0;

    // Used to hold current message from GUI.
    string gui_message;
    // Names of all the players in current game.
    vector<string> players;
    current_state server_info;
};

#endif //SK2_CLIENT_STRUCTURES_H

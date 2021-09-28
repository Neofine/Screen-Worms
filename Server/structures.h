#ifndef SK2_STRUCTURES_H
#define SK2_STRUCTURES_H

#include <netinet/in.h>
#include <cstring>
#include <regex>
#include <poll.h>
#include "../crc_table.h"

#define CLIENTS_MAX         25
#define CLIENT_MESS_MAX     33
#define CLIENT_MESS_MIN     13
#define CLIENT_TIMEOUT      2

#define RAND_MULTI          279'410'273
#define RAND_MODULO         4'294'967'291

#define CRC_STARTING_NUM    4'294'967'295

#define MESS_NAME_BEGIN     13
#define MESS_NAME_END       34

#define MASK_8BIT           255

#define NSECS_IN_SECOND     1'000'000'000

#define MAX_BOARD_HEIGHT    2'000
#define MAX_BOARD_WIDTH     2'000

#define MIN_BOARD_HEIGHT    16
#define MIN_BOARD_WIDTH     16

using std::string;
using std::vector;
using std::map;

// All types of events server is supporting.
enum event_types {
    new_game = 0,
    pixel,
    player_eliminated,
    game_over
};

// Constants defined in programme flags.
struct constants {
    /* -p flag, port number */
    uint16_t port_nr = 2021;
    /* -s flag, generator seed */
    uint32_t seed = time(nullptr);
    /* -t flag */
    uint8_t turning_speed = 6;
    /* -v flag */
    uint8_t rounds_per_sec = 50;
    /* -w flag */
    uint16_t width = 640;
    /* -h flag */
    uint16_t height = 480;
};

// Information about single worm in a game.
struct worm {
    // Coordinates of a worm.
    double x = 0.0, y = 0.0;
    // Angle in which worm is moving.
    uint16_t turning_angle = 0;
    // Last registered player direction.
    uint8_t turn_direction = 0;
    // Players (worms) name.
    string name;
    // ID of a worm, given by a server.
    int number = -1;
};

// Information about a player.
struct player {
    // Players socket.
    sockaddr_in6 client{};
    // Unique player ID.
    int upid = -1;
    // Players session id.
    uint64_t session_id = 0;
    // Number of timer in descriptors array which
    // is calculating time to players disconnection.
    int timer_id = -1;
    // If at the start of the game player entered non-forward direction.
    bool ready = false;
};

// Information about current game and all of its players.
struct memory {
    // Flag if it is first game of the server.
    bool first_server_game = true;
    // Flag if server is gathering players, not playing game.
    bool gathering_phase = true;
    // Game ID, given via det_rand function.
    uint32_t game_id = 0;
    // Largest not given unique player ID.
    int not_taken_upid = 0;

    // Eaten pixels by worms on map, if pixel is eaten
    // then eaten_pix of its coordinates is true.
    bool eaten_pix[MAX_BOARD_HEIGHT][MAX_BOARD_WIDTH]{};
    // Events history.
    vector<vector<uint8_t>> game_course;
    // Currently connected players, key of this
    // map is string equal to "client_address:client_port".
    map<string, player> connected_clients;
    // Worms that players have, key of this
    // map is unique player ID (one player - one worm).
    map<int, struct worm> players_worms;
    // Vector of pointers to currently alive worms, not empty during games.
    vector<struct worm*> moving_worms;

    // Overall amount of connected players.
    size_t connected_amount = 0;
    // Amount of players eager to play.
    size_t eager_amount = 0;
    // Amount of players eager to play who sent non-forward direction message.
    size_t ready_amount = 0;
};

// Network information about server.
struct communication {
    // Socket.
    int sock = 0;
    // Socket address structure.
    struct sockaddr_in6 server_address{};
    // Timers which measure the last time when player
    // communicated with the server.
    pollfd timers[CLIENTS_MAX + 1]{};
};

// Structure made to easily receive information from player on UDP.
struct player_message {
    // Session identificator.
    uint64_t session_id = 0;
    // Turn direction, valid is in range [0, 2].
    uint8_t turn_direction = 0;
    // Which event number player expects to receive next.
    uint32_t next_event_no = 0;
    // Name.
    char player_name[21]{};
} __attribute__((packed));


#endif //SK2_STRUCTURES_H

#include "client_utility.h"
#include "../crc_table.h"
#include <sys/time.h>

void syserr(const string& out) {
    std::cerr << out << std::endl;
    exit(EXIT_FAILURE);
}

void change_direction(game_info *game, const string& command) {
    if (command == "LEFT_KEY_DOWN\n") {
        game->server_info.turn_direction = LEFT;
    }
    else if (command == "LEFT_KEY_UP\n") {
        if (game->server_info.turn_direction == LEFT)
            game->server_info.turn_direction = FORWARD;
    }
    else if (command == "RIGHT_KEY_DOWN\n") {
        game->server_info.turn_direction = RIGHT;
    }
    else if (command == "RIGHT_KEY_UP\n") {
        if (game->server_info.turn_direction == RIGHT)
            game->server_info.turn_direction = FORWARD;
    }
}

uint32_t calc_crc(const uint8_t* data, size_t bytes_amount) {
    uint32_t crc = CRC_STARTING_NUM;

    uint32_t lookup_idx;
    for (size_t i = 0; i < bytes_amount; i++) {
        uint8_t u = data[i];
        lookup_idx = (crc ^ u) & 255;
        crc = (crc >> 8) ^ crc32_tab[lookup_idx];
    }

    crc ^= CRC_STARTING_NUM;

    return crc;
}

suseconds_t get_time_now() {
    timeval timeval{};
    gettimeofday(&timeval, nullptr);
    return timeval.tv_sec * USECS_IN_SEC + timeval.tv_usec;
}

uint32_t get32(uint8_t *mess) {
    return mess[0] * BYTE_4_OF_32 + mess[1] * BYTE_3_OF_32 +
           mess[2] * BYTE_2_OF_32 + mess[3];
}
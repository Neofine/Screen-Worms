#include "utility.h"

void syserr(const string& out) {
    std::cerr << out << std::endl;
    exit(EXIT_FAILURE);
}

void validate_length(uint64_t number, const string& str_number) {
    if (std::to_string(number) != str_number || str_number[0] == '-') {
        syserr("One of arguments is too big or containing forbidden characters");
    }
}

void validate_values(constants *prog) {
    if (prog->port_nr == 0) {
        syserr("Port number 0 is for system, sorry.");
    }
    if (prog->turning_speed == 0 || prog->turning_speed > 90) {
        syserr("Ridiculously value of turning speed.");
    }
    if (prog->rounds_per_sec == 0 || prog->rounds_per_sec > 250) {
        syserr("Ridiculously value of server tick rate.");
    }
    if (prog->width < MIN_BOARD_WIDTH || prog->width > MAX_BOARD_WIDTH ||
        prog->height < MIN_BOARD_HEIGHT || prog->height > MAX_BOARD_HEIGHT) {
        syserr("Invalid board size.");
    }
}

void validate_flags(int argc, char *argv[], constants *prog) {
    char *stop;
    int opt;

    while ((opt = getopt(argc, argv, "p:s:t:v:w:h:")) != -1) {
        switch (opt) {
            case 'p':
                prog->port_nr = strtol(optarg, &stop, 10);
                validate_length(prog->port_nr, optarg);
                break;
            case 's':
                prog->seed = strtol(optarg, &stop, 10);
                validate_length(prog->seed, optarg);
                break;
            case 't':
                prog->turning_speed = strtol(optarg, &stop, 10);
                validate_length(prog->turning_speed, optarg);
                break;
            case 'v':
                prog->rounds_per_sec = strtol(optarg, &stop, 10);
                validate_length(prog->rounds_per_sec, optarg);
                break;
            case 'w':
                prog->width = strtol(optarg, &stop, 10);
                validate_length(prog->width, optarg);
                break;
            case 'h':
                prog->height = strtol(optarg, &stop, 10);
                validate_length(prog->height, optarg);
                break;
            default:
                syserr("Unknown flag!");
        }
    }

    validate_values(prog);

    // If there are arguments other than processed.
    if (optind < argc) {
        syserr("Programme isn't expecting any argument except options\n");
    }
}

bool exists_player_with_name(memory *mem, const string &player_name) {
    return (std::any_of(mem->players_worms.begin(), mem->players_worms.end(),
                        [player_name](std::pair<const int, worm>& pawn) {
                            return pawn.second.name == player_name;
                        }));
}

void sockaddr_to_string(string &on_what, sockaddr_in6 *client_info) {
    auto *client_address6 = reinterpret_cast<sockaddr_in6*>(client_info);

    char str[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, client_address6->sin6_addr.s6_addr,
              str, INET6_ADDRSTRLEN);

    on_what = str;
    on_what += ':' + std::to_string(client_address6->sin6_port);
}

void append_vector(vector<uint8_t> *left_side, uint32_t by_what, int bytes_num) {
    for (int i = bytes_num - 1; i >= 0; i--) {
        left_side->push_back((by_what >> i * 8) & MASK_8BIT);
    }
}

bool cmp_lex(worm *a, worm *b) {
    return a->name < b->name;
}

int double_to_int(double number) {
    return static_cast<int>(floor(number));
}

bool move_by_one(worm *to_move) {
    double angle = to_move->turning_angle;
    double angle_in_degrees = angle * M_PI / 180.0;
    int prev_x = double_to_int(to_move->x),
        prev_y = double_to_int(to_move->y);

    to_move->x += cos(angle_in_degrees);
    to_move->y += sin(angle_in_degrees);

    return prev_x != double_to_int(to_move->x) ||
           prev_y != double_to_int(to_move->y);
}

uint32_t det_rand(uint32_t seed) {
    static uint32_t rand_now;

    if (seed != 0) {
        rand_now = seed;
    }
    else {
        rand_now = ((uint64_t)rand_now * RAND_MULTI) % RAND_MODULO;
    }

    return rand_now;
}

uint32_t calc_crc(vector<uint8_t> *data) {
    uint32_t crc = CRC_STARTING_NUM;
    uint32_t lookup_idx;

    for (const auto u: *data) {
        lookup_idx = (crc ^ (uint8_t)u) & 255;
        crc = (crc >> 8) ^ crc32_tab[lookup_idx];
    }

    crc ^= CRC_STARTING_NUM;

    return crc;
}

bool is_message_valid(player_message *mess, ssize_t mess_len) {
    mess->session_id = be64toh(mess->session_id);
    mess->next_event_no = be32toh(mess->next_event_no);

    if (mess->turn_direction > 2)
        return false;

    for (int i = MESS_NAME_BEGIN; i < MESS_NAME_END; i++) {
        if (i < mess_len) {
            unsigned char c = mess->player_name[i - 13];
            if (c < 33 || c > 126)
                return false;
        }
        else {
            mess->player_name[i - 13] = 0;
        }
    }

    return true;
}
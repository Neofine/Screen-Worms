#include "setup.h"
#include <netdb.h>
#include <netinet/tcp.h>
#include <unistd.h>

// If given argument isn't a valid player name then exits with
// certain text on stderr.
void validate_name(const string &name) {
    if (name.size() > 20) {
        syserr("Player name is too long");
    }
    for (auto c: name) {
        if (c < MIN_ACCEPT_CHAR || c > MAX_ACCEPT_CHAR) {
            syserr("Player name doesn't meet server specifications");
        }
    }
}

// Given a number and a string representing this number
// if those two don't match exit is called.
void validate_length(uint64_t number, const string& str_number) {
    if (std::to_string(number) != str_number || str_number[0] == '-') {
        syserr("One of arguments is too big or containing forbidden characters");
    }
}

void validate_flags(int argc, char *argv[], communication *prog, game_info *game) {
    string tmp;
    char *stop;
    int opt;

    while ((opt = getopt(argc, argv, "n:p:i:r:")) != -1) {
        switch (opt) {
            case 'n':
                tmp = optarg;
                for (size_t i = 0; i < tmp.size(); i++) {
                    game->server_info.player_name[i] = tmp[i];
                }

                game->player_name_length = tmp.size();
                validate_name(tmp);
                break;
            case 'p':
                prog->game_server_port = strtol(optarg, &stop, 10);
                validate_length(prog->game_server_port, optarg);
                break;
            case 'i':
                prog->gui_server_address = optarg;
                break;
            case 'r':
                prog->gui_server_port = strtol(optarg, &stop, 10);
                validate_length(prog->gui_server_port, optarg);
                break;
            default:
                syserr("Invalid flags were given.");
        }
    }

    int amount = 0;
    for (int index = optind; index < argc; index++)
        prog->game_server_address = argv[index], amount++;

    if (amount > 1 || amount == 0)
        syserr("Either game server address wasn't given or it was too many times");
}

void set_connection(communication *comm, bool to_gui) {
    addrinfo addr_hints{};
    addrinfo *addr_result;

    int sock;
    int err;

    memset(&addr_hints, 0, sizeof(struct addrinfo));
    addr_hints.ai_family = AF_UNSPEC; // Accepting IPv4 and IPv6.

    // If to_gui flag is true then communication with GUI is via TCP.
    if (to_gui) {
        addr_hints.ai_socktype = SOCK_STREAM;
        addr_hints.ai_protocol = IPPROTO_TCP;
    }
    // If to_gui flag is false then communication with server is via UDP.
    else {
        addr_hints.ai_socktype = SOCK_DGRAM;
        addr_hints.ai_protocol = IPPROTO_UDP;
    }

    // If to_gui flag is true then getaddrinfo function
    // needs to have gui address and port.
    if (to_gui) {
        err = getaddrinfo(comm->gui_server_address.c_str(),
                          std::to_string(comm->gui_server_port).c_str(),
                          &addr_hints, &addr_result);
    }
    // If to_gui flag is false then getaddrinfo function
    // needs to have server address and port
    else {
        err = getaddrinfo(comm->game_server_address.c_str(),
                          std::to_string(comm->game_server_port).c_str(),
                          &addr_hints, &addr_result);
    }

    if (err != 0)
        syserr("Error in getting server addr info");

    sock = socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol);
    if (sock < 0)
        syserr("socket");


    // Setting timeout not to block on receive.
    timeval read_timeout{};
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 100;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout) < 0)
        syserr("setsockopt rcvtimeo");

    if (to_gui) {
        // 1 to turn off nagle algorithm, 0 to turn it on.
        int turn_off = 1;
        if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &turn_off, sizeof(int)) < 0)
            syserr("setsockopt turning off nagle algorithm failed");
    }

    if (connect(sock, addr_result->ai_addr, addr_result->ai_addrlen) < 0)
        syserr("connect");

    if (to_gui) {
        comm->gui_socket = sock;
    }
    else {
        comm->game_socket = sock;
    }
}
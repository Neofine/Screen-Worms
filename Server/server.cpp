#include "structures.h"
#include "communicator.h"
#include "utility.h"
#include "events.h"

// Given player message and its socket address processes the message
// and responds to it.
void process_player_message(communication *comm, memory *mem,
                            sockaddr_in6 *client_info, player_message *mess) {
    string client_socket;
    sockaddr_to_string(client_socket, client_info);

    auto client = mem->connected_clients.find(client_socket);

    // If client isn't newly connected.
    if (client != mem->connected_clients.end()) {
        player *current_player = &client->second;
        auto players_worm = mem->players_worms.find(current_player->upid);
        bool has_worm = players_worm != mem->players_worms.end();

        // If session id value is less than actual or
        // player name changes between messages then ignores it.
        if (current_player->session_id < mess->session_id ||
           (has_worm && players_worm->second.name != mess->player_name)) {
            return;
        }
        else if (current_player->session_id == mess->session_id) {
            if (client_disconnect_check(comm, mem, current_player->timer_id)) {
                return;
            }

            current_player->client = *client_info;
            renew_timer(comm, current_player->timer_id);

            if (has_worm)
                mem->players_worms[current_player->upid].turn_direction =
                        mess->turn_direction;

            if (mem->moving_worms.empty() && !current_player->ready &&
                mess->turn_direction != 0 && has_worm)
                current_player->ready = true, mem->ready_amount++;
        }
        // If session id is greater than current then assuming that new player
        // has connected, disconnecting current client and connecting new one.
        else {
            mem->ready_amount -= current_player->ready;
            if (has_worm) {
                mem->eager_amount--;
                if (mem->gathering_phase)
                    mem->players_worms.erase(players_worm);
                else
                    players_worm->second.name = "";
            }
            mem->connected_amount--;

            close(comm->timers[current_player->timer_id].fd);
            comm->timers[current_player->timer_id].fd = -1;
            comm->timers[current_player->timer_id].events = POLLIN;
            comm->timers[current_player->timer_id].revents = 0;

            if (exists_player_with_name(mem, mess->player_name))
                return;

            connect_client(comm, mem, mess, client_socket, client_info);
        }
    }
    else {
        if (exists_player_with_name(mem, mess->player_name))
            return;

        connect_client(comm, mem, mess, client_socket, client_info);
    }

    // If client message is processed successfully then updating
    // them about events they haven't gotten yet.
    if (!mem->game_course.empty())
        send_latest_events(comm, mem, mess->next_event_no, client_info);
}

// Gathers players to play game, if everyone is ready
// then sends NEW_GAME event to everyone connected.
void gather_players(communication *comm, constants *prog, memory *mem) {
    mem->gathering_phase = true;

    sockaddr_in6 client_info{};
    socklen_t rcva_len;
    ssize_t len;
    player_message current_message;

    while(true) {
        rcva_len = (socklen_t) sizeof(client_info);

        // Reading on purpose message of length CLIENT_MESS_MAX + 1, because
        // of need to ignore messages that are exceeding length than permitted one.
        len = recvfrom(comm->sock, &current_message, CLIENT_MESS_MAX + 1, 0,
                       (struct sockaddr *) &client_info, &rcva_len);

        if (len >= CLIENT_MESS_MIN && len <= CLIENT_MESS_MAX &&
            is_message_valid(&current_message, len)) {
            process_player_message(comm, mem, &client_info, &current_message);
        }

        check_for_disconnect(comm, mem);
        if (mem->eager_amount == mem->ready_amount && mem->eager_amount >= 2) {
            new_game_event(comm, prog, mem);
            return;
        }
    }
}

// Simulates single turn of worms movement,
// returns true if there is only one worm left on the map.
bool play_turn(communication *comm, constants *prog, memory *mem) {
    for (size_t i = 0; i < mem->moving_worms.size(); i++) {
        worm *current_worm = mem->moving_worms[i];

        if (current_worm->turn_direction == 1)
            current_worm->turning_angle =
                    (current_worm->turning_angle + prog->turning_speed) % 360;
        else if (current_worm->turn_direction == 2)
            current_worm->turning_angle =
                    (current_worm->turning_angle + 360 - prog->turning_speed) % 360;

        if (!move_by_one(current_worm))
            continue;

        int x_pos = double_to_int(current_worm->x);
        int y_pos = double_to_int(current_worm->y);

        if (x_pos < 0 || y_pos < 0 || y_pos >= prog->height ||
            x_pos >= prog->width || mem->eaten_pix[y_pos][x_pos]) {

            player_eliminated_event(comm, mem, current_worm->number);
            mem->moving_worms.erase(mem->moving_worms.begin() + i);
            i--;
        }
        else {
            pixel_eaten_event(comm, mem, current_worm);
            mem->eaten_pix[y_pos][x_pos] = true;
        }

        if (mem->moving_worms.size() == 1) {
            return true;
        }
    }

    return false;
}

// Times server ticks,
// returns true if after playing turns there is one worm left.
bool server_tick(communication *comm, constants *prog, memory *mem) {
    poll(comm->timers, 1, 0);
    if (!(comm->timers[0].revents & POLLIN))
        return false;

    uint64_t turn_count;
    if (read(comm->timers[0].fd, &turn_count, 8) < 0)
        return false;
    comm->timers[0].revents = 0;

    while(turn_count--) {
        if (play_turn(comm, prog, mem))
            return true;
    }

    check_for_disconnect(comm, mem);
    return false;
}

// Plays single game, reads players messages and calls server_tick to try
// to play turn, ends when there is only one worm left - the game is over.
void play_game(communication *comm, constants *prog, memory *mem) {
    sockaddr_in6 client_info{};
    socklen_t rcva_len;
    ssize_t len;
    player_message current_message;

    while(true) {

        // Checks if it is time to play a turn and if it is, playing it.
        if (server_tick(comm, prog, mem))
            return;

        rcva_len = (socklen_t) sizeof(client_info);
        len = recvfrom(comm->sock, &current_message, CLIENT_MESS_MAX + 1, 0,
                       (struct sockaddr*) &client_info, &rcva_len);

        if (len >= CLIENT_MESS_MIN && len <= CLIENT_MESS_MAX &&
            is_message_valid(&current_message, len)) {
            process_player_message(comm, mem, &client_info, &current_message);
        }
    }
}

// Ends game, sends game_over event to every connected client
// and clears fields to play next game without any data collisions.
void end_game(communication *comm, constants *prog, memory *mem) {
    generate_and_send_event(comm, mem, vector<uint8_t>(0), game_over);

    close(comm->timers[0].fd);
    comm->timers[0].fd = -1;
    comm->timers[0].events = POLLIN;
    comm->timers[0].revents = 0;

    mem->moving_worms.clear();
    mem->eager_amount = 0;
    mem->ready_amount = 0;

    for (int y = 0; y < prog->height; y++) {
        for (int x = 0; x < prog->width; x++) {
            mem->eaten_pix[y][x] = false;
        }
    }

    map<int, struct worm> to_add;
    for (auto &client: mem->connected_clients) {
        if (mem->players_worms.find(client.second.upid) != mem->players_worms.end())
            to_add[client.second.upid].name = mem->players_worms[client.second.upid].name,
            mem->eager_amount++;

        client.second.ready = false;
    }
    mem->players_worms = to_add;
}

int main(int argc, char *argv[]) {
    static constants prog;
    static memory mem;
    static communication comm;

    validate_flags(argc, argv, &prog);

    set_connection(&comm, &prog);

    // Plays game endlessly.
    while(true) {
        gather_players(&comm, &prog, &mem);
        play_game(&comm, &prog, &mem);
        end_game(&comm, &prog, &mem);
    }
}
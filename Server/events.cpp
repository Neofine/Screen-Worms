#include "events.h"

void generate_and_send_event(communication *comm, memory *mem,
                             const vector<uint8_t>& event_data, event_types what) {
    uint32_t game_id = mem->game_id;
    uint32_t len = event_data.size() + 5;
    uint32_t event_no = mem->game_course.size();
    uint8_t event_type = what;

    // Data in bytes to send to clients
    vector<uint8_t> new_event;

    // Appending vector number by number with necessary event information.
    append_vector(&new_event, len, 4);
    append_vector(&new_event, event_no, 4);
    append_vector(&new_event, event_type, 1);
    new_event.insert(new_event.end(), event_data.begin(), event_data.end());
    append_vector(&new_event, calc_crc(&new_event), 4);

    // Saving in memory single event without game_id
    mem->game_course.push_back(new_event);

    // I can add now game_id at the front
    vector<uint8_t> game_id_vec;
    append_vector(&game_id_vec, game_id, 4);
    new_event.insert(new_event.begin(), game_id_vec.begin(), game_id_vec.end());

    // Length of sent packet.
    ssize_t snd_len;
    // Size of sockaddr_in6 of a single connected client.
    socklen_t rcva_len;

    // Changing vector to array in order to properly send the message.
    uint8_t start_game_message[new_event.size()];
    std::copy(new_event.begin(), new_event.end(), start_game_message);

    // Iterating over connected clients.
    for (auto client: mem->connected_clients) {
        rcva_len = (socklen_t) sizeof(client.second.client);
        snd_len = sendto(comm->sock, start_game_message, new_event.size(), 0,
                         (struct sockaddr *) &client.second.client, rcva_len);

        if (snd_len != (long int)new_event.size()) {
            printf("Error in communicating with %s\n", client.first.c_str());
        }
    }
}

void player_eliminated_event(communication *comm, memory *mem, int p_number) {
    vector<uint8_t> player_number;
    player_number.push_back(p_number);

    generate_and_send_event(comm, mem, player_number, player_eliminated);
}

void pixel_eaten_event(communication *comm, memory *mem, worm *pawn) {
    vector<uint8_t> pixel_event;
    pixel_event.push_back(pawn->number);
    append_vector(&pixel_event, double_to_int(pawn->x), 4);
    append_vector(&pixel_event, double_to_int(pawn->y), 4);

    generate_and_send_event(comm, mem, pixel_event, pixel);
}

void new_game_event(communication *comm, constants *prog, memory *mem) {
    mem->game_course.clear();
    mem->gathering_phase = false;

    if (mem->first_server_game)
        mem->game_id = det_rand(prog->seed),
        mem->first_server_game = false;
    else
        mem->game_id = det_rand();

    // Only adding worms of connected clients.
    for (auto& u : mem->connected_clients) {
        if (mem->players_worms.find(u.second.upid) == mem->players_worms.end())
            continue;

        mem->moving_worms.push_back(&mem->players_worms[u.second.upid]);
    }

    // Sorting worms lexicographically.
    sort(mem->moving_worms.begin(), mem->moving_worms.end(), cmp_lex);

    // Construct event data.
    vector<uint8_t> event_data;
    append_vector(&event_data, prog->width, 4);
    append_vector(&event_data, prog->height, 4);

    int worm_numeration = 0;
    for (auto pawn: mem->moving_worms) {
        pawn->number = worm_numeration++;
        for (auto character: pawn->name) {
            event_data.push_back(character);
        }
        event_data.push_back(0);
    }

    // Send event to every player.
    generate_and_send_event(comm, mem, event_data, new_game);

    create_timer(comm, 0, NSECS_IN_SECOND / prog->rounds_per_sec);

    for (size_t i = 0; i < mem->moving_worms.size(); i++) {
        worm *pawn = mem->moving_worms[i];
        pawn->x = (det_rand() % prog->width) + 0.5;
        pawn->y = (det_rand() % prog->height) + 0.5;
        pawn->turning_angle = (uint16_t)(det_rand() % 360);

        int curr_x = double_to_int(pawn->x);
        int curr_y = double_to_int(pawn->y);

        if (mem->eaten_pix[curr_y][curr_x]) {
            player_eliminated_event(comm, mem, pawn->number);
            mem->moving_worms.erase(mem->moving_worms.begin() + i);
            i--;
        }
        else {
            pixel_eaten_event(comm, mem, pawn);
            mem->eaten_pix[curr_y][curr_x] = true;
        }
    }
}
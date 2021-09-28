#include "structures.h"
#include "communicator.h"
#include "utility.h"
#include <sys/timerfd.h>

void create_timer(communication *comm, int id, long tick_time) {
    itimerspec new_timer{};

    if (tick_time != 0) {
        new_timer.it_value.tv_sec = 0;
        new_timer.it_value.tv_nsec = tick_time;
        new_timer.it_interval.tv_sec = 0;
        new_timer.it_interval.tv_nsec = tick_time;
    }
    else {
        new_timer.it_value.tv_sec = CLIENT_TIMEOUT;
        new_timer.it_value.tv_nsec = 0;
        new_timer.it_interval.tv_sec = 0;
        new_timer.it_interval.tv_nsec = 0;
    }

    // Creating timer descriptor.
    if ((comm->timers[id].fd = timerfd_create(CLOCK_REALTIME, 0)) == -1)
        syserr("Error while binding timerfd_create to a descriptor");

    // Binding timer to a descriptor.
    if (timerfd_settime(comm->timers[id].fd, 0, &new_timer, nullptr) == -1)
        syserr("Error while making timerfd_settime on a descriptor");
}

void renew_timer(communication *comm, int id) {
    itimerspec server_timer{};

    server_timer.it_value.tv_sec = CLIENT_TIMEOUT;
    server_timer.it_value.tv_nsec = 0;
    server_timer.it_interval.tv_sec = 0;
    server_timer.it_interval.tv_nsec = 0;

    if (timerfd_settime(comm->timers[id].fd, 0, &server_timer, nullptr) == -1)
        syserr("Error during setting timer.");
}

void set_connection(communication *comm, constants *prog) {
    comm->sock = socket(AF_INET6, SOCK_DGRAM, 0);

    if (comm->sock < 0) {
        syserr("Couldn't create the IPv6 sock.");
    }

    // Setting recvfrom not to block while waiting for a message
    // instead it waits maximum of read_timeout seconds for message.
    timeval read_timeout{};
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 500;
    if (setsockopt(comm->sock, SOL_SOCKET, SO_RCVTIMEO,
                   &read_timeout, sizeof read_timeout) != 0)
        syserr("setsockopt rcvtimeo");

    // Setting up sock listening on both IPv4 and IPv6.
    int ipv6_only_mode = 0;
    if (setsockopt(comm->sock, IPPROTO_IPV6, IPV6_V6ONLY,
                   &ipv6_only_mode, sizeof(ipv6_only_mode)) != 0)
        syserr("Setting listening on both IPv4 and IPv6 failed.");

    // Setting sockaddr_in6 values to correctly bind the port.
    comm->server_address.sin6_family = AF_INET6;
    comm->server_address.sin6_flowinfo = 0;
    comm->server_address.sin6_addr = in6addr_any;
    comm->server_address.sin6_port = htons(prog->port_nr);
    if (bind(comm->sock, (struct sockaddr *)&comm->server_address,
             sizeof(comm->server_address)) < 0)
        syserr("Couldn't bind the IPv6 port.");

    // Setting timers array to default values.
    for (int i = 0; i <= CLIENTS_MAX; i++) {
        comm->timers[i].fd = -1;
        comm->timers[i].events = POLLIN;
        comm->timers[i].revents = 0;
    }
}

void connect_client(communication *comm, memory *mem, player_message *mess,
                    const string &socket, sockaddr_in6 *client_info) {

    // If maximum amount of clients is already connected then
    // reject this player.
    if (mem->connected_amount >= CLIENTS_MAX)
        return;

    player *creating = &mem->connected_clients[socket];

    // Searching for not occupied descriptor, there is always at least one.
    for (int i = 1; i <= CLIENTS_MAX; i++) {
        if (comm->timers[i].fd == -1) {
            creating->timer_id = i;
            create_timer(comm, i, 0);
            break;
        }
    }

    string worm_name = mess->player_name;

    creating->upid = mem->not_taken_upid++;
    creating->client = *client_info;
    creating->session_id = mess->session_id;
    mem->connected_amount++;

    // If client isn't a spectator.
    if (!worm_name.empty()) {
        if (mess->turn_direction == 1 || mess->turn_direction == 2)
            creating->ready = true, mem->ready_amount++;

        mem->eager_amount++;
        mem->players_worms[creating->upid].name = worm_name;
        mem->players_worms[creating->upid].turn_direction = mess->turn_direction;
    }
}

void send_latest_events(communication *comm, memory *mem,int from_when,
                        sockaddr_in6 *to_whom) {

    // If client wants information from the future.
    if (from_when >= (int) mem->game_course.size() - 1)
        return;

    socklen_t rcva_len = sizeof(*to_whom);

    // Trying to gather as much events as possible into a single datagram.
    vector<uint8_t> accumulated_events;
    append_vector(&accumulated_events, mem->game_id, 4);

    for (size_t i = from_when; i < mem->game_course.size(); i++) {

        // Sending datagram if next event doesn't fit.
        if (accumulated_events.size() + mem->game_course[i].size() > 550) {
            uint8_t current_event[accumulated_events.size()];
            std::copy(accumulated_events.begin(), accumulated_events.end(),
                      current_event);

            sendto(comm->sock, current_event, accumulated_events.size(), 0,
                   (struct sockaddr *)to_whom, rcva_len);

            accumulated_events.clear();
            append_vector(&accumulated_events, mem->game_id, 4);
        }

        for (auto byte: mem->game_course[i])
            accumulated_events.push_back(byte);
    }

    // If last datagram hasn't been fully filled but isn't empty.
    if (!accumulated_events.empty() && accumulated_events.size() != 1) {
        uint8_t current_event[accumulated_events.size()];
        std::copy(accumulated_events.begin(), accumulated_events.end(),
                  current_event);

        sendto(comm->sock, current_event, accumulated_events.size(), 0,
               (struct sockaddr *) to_whom, rcva_len);
    }
}

bool client_disconnect_check(communication *comm, memory *mem, int timer_id) {

    // If at current array index exists timer descriptor and
    // at least one event happened there: timeout or error.
    if (comm->timers[timer_id].fd != -1 &&
        (comm->timers[timer_id].revents & (POLLIN | POLLERR))) {

        map<string, player>::iterator it;
        for (it = mem->connected_clients.begin();
             it != mem->connected_clients.end(); it++) {
            if (it->second.timer_id == timer_id) {
                break;
            }
        }

        mem->ready_amount -= it->second.ready;
        mem->connected_amount--;

        auto players_worm = mem->players_worms.find(it->second.upid);

        // If a player wasn't spectator - had a worm.
        if (players_worm != mem->players_worms.end()) {
            mem->eager_amount --;

            if (mem->gathering_phase)
                mem->players_worms.erase(players_worm);
            else
                players_worm->second.name = "";
        }

        mem->connected_clients.erase(it);

        // Reseting descriptior.
        close(comm->timers[timer_id].fd);
        comm->timers[timer_id].fd = -1;
        comm->timers[timer_id].events = POLLIN;
        comm->timers[timer_id].revents = 0;

        return true;
    }

    return false;
}

void check_for_disconnect(communication *comm, memory *mem) {
    if (poll(comm->timers, CLIENTS_MAX + 1, 0)) {
        for (int i = 1; i <= CLIENTS_MAX; i++) {
            client_disconnect_check(comm, mem, i);
        }
    }
}
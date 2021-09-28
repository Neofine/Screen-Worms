#ifndef SK2_CLIENT_EVENTS_H
#define SK2_CLIENT_EVENTS_H

#include "client_utility.h"
#include "sending_info.h"

// Given message its length and number processes current command and
// responds to it adequately, returns length of this command.
uint16_t process_event(communication *comm, game_info *game, uint32_t game_id,
                       uint8_t *mess, size_t bytes_left);

#endif //SK2_CLIENT_EVENTS_H

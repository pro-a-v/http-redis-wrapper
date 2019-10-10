#ifndef REDIS_CONNECTION_STATUS_H
#define REDIS_CONNECTION_STATUS_H
#include <stdint.h>
#include <string>

enum _state { connected, disconnected };
enum _type {master, slave};

struct _config {
    uint16_t slot_from;
    uint16_t slot_to;
    _state state;
    _type type;
    std::string id;
    std::string master_id;
    std::string ip;
    uint16_t port;
    uint16_t uid;

};


#endif // REDIS_CONNECTION_STATUS_H

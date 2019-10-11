#ifndef HIREDIS_MANAGER_H
#define HIREDIS_MANAGER_H

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <cstdint>
#include <algorithm>

#include "hiredis_client.hpp"
#include "request.hpp"
#include "crc16.hpp"

class hiredis_manager
{
public:
    hiredis_manager();
    ~hiredis_manager();

    std::string get(request& req);
    std::string set(request &req);
    void del(request &req);
    bool stoped = false;

private:
    void worker_updater();
    void updater();
    uint16_t HASH_SLOT(const char* key, int keylen);

    rapidjson::Document d;
    std::vector<hiredis_client *> clients;
    std::mutex _mutex;
    boost::thread *WorkerUpdaterThread = nullptr;

    struct range { uint16_t slot_from; uint16_t slot_to; uintptr_t ptr;};
    range data;
    std::vector<range> slots_array;

};

#endif // HIREDIS_MANAGER_H

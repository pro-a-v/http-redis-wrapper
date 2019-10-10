#ifndef HIREDIS_MANAGER_H
#define HIREDIS_MANAGER_H

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <iostream>
#include "hiredis_client.hpp"
#include "request.hpp"
#include <mutex>
#include <cstdint>

class hiredis_manager
{
public:
    hiredis_manager();
    std::string get(request& req);
    std::string set(request &req);
private:
    rapidjson::Document d;
    std::vector<hiredis_client *> clients;

    void worker_updater();
    void fill_slots();
    std::mutex _mutex;
    boost::thread *WorkerUpdaterThread = nullptr;
    std::array<std::atomic<uintptr_t>, 16384> slots_array={};

};

#endif // HIREDIS_MANAGER_H

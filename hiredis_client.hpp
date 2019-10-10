#ifndef HIREDIS_CLIENT_HPP
#define HIREDIS_CLIENT_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdexcept>

#include "request.hpp"
#include <hiredis/hiredis.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp> // split
#include <boost/lexical_cast.hpp>   // string to int
#include <mutex>
#include <thread>

#include "redis_connection_status.h"


#include "rapidjson/schema.h"
#include "rapidjson/writer.h"

class hiredis_client
{
public:
    hiredis_client(std::string _hostname, uint16_t _port, uint16_t uid);
    ~hiredis_client();
    void del(request& req);
    std::string get(request& req);
    std::string set(request& req);
    _config get_config();

    bool stoped = false;



private:
    void connect();
    bool is_alive();
    void parse_cluster_nodes_string();
    void config_updater();

    unsigned int j;
    bool get_succsess = false;
    boost::posix_time::time_duration diff;

    redisContext *c = NULL;
    redisReply *reply;

    _config con_config;
    std::mutex con_config_mutex;

    boost::thread *ConfigUpdaterThread = nullptr;
    std::vector <std::string> raw_string_parts;
    std::string myself;

};



#endif // HIREDIS_CLIENT_HPP

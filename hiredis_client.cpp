#include "hiredis_client.hpp"

hiredis_client::hiredis_client(std::string _hostname, uint16_t _port, uint16_t uid)
{
    con_config.ip = _hostname;
    con_config.port = _port;
    con_config.uid = uid;

    if (c == nullptr)
        connect();

    // Создадим поток для валидатора соединения обработчика
    ConfigUpdaterThread    = new boost::thread(   boost::bind(&hiredis_client::config_updater, this) , nullptr );

}

hiredis_client::~hiredis_client()
{
    /* Disconnects and frees the context */
    redisFree(c);
}

_config hiredis_client::get_config()
{
    std::lock_guard<std::mutex> guard(con_config_mutex);
    return con_config;
}

std::string hiredis_client::get(request& req)
{
    std::string ret;
    req.key = req.request_path.substr(1,req.request_path.length()-1);
    get_succsess = false;
    while(!get_succsess)
    {

        try {
            reply = static_cast<redisReply*>(redisCommand(c,"GET %s", req.key.c_str()) );
            if (reply == nullptr)
            {
                std::string err_text = std::string("No answer on get ") + req.key;
                throw std::runtime_error( err_text.c_str() );
            }
            else{
                ret.append(reply->str, reply->len);
                freeReplyObject(reply);
                get_succsess = true;
            }
        } catch (...) {
            redisFree(c);
            connect();

        }
        diff = boost::posix_time::second_clock::local_time() - req.start;
        if ( diff.total_seconds() > 60 )
        {
            throw std::runtime_error( "No data for " + req.key + " more than 1 min  " );
        }
    }




    return ret;

}

bool hiredis_client::is_alive()
{
    /* PING server */
    reply = static_cast<redisReply*>(redisCommand(c, "PING"));
    if (reply == nullptr)
        return false;
    else
        return true;
}

void hiredis_client::parse_cluster_nodes_string()
{
    split(raw_string_parts, myself , boost::is_any_of(":@, -"));
    if (raw_string_parts[5] == std::string("master"))
    {
        std::lock_guard<std::mutex> guard(con_config_mutex);
        con_config.type = _type::master;
        con_config.id = raw_string_parts[0];
        con_config.slot_from = boost::lexical_cast<uint16_t>( raw_string_parts[12] );
        con_config.slot_to = boost::lexical_cast<uint16_t>( raw_string_parts[13] );
        if (raw_string_parts[11] == std::string("connected"))
            con_config.state = _state::connected;
        else
            con_config.state = _state::disconnected;
    }
    else  // slave
    {
        std::lock_guard<std::mutex> guard(con_config_mutex);
        con_config.type = _type::slave;
        con_config.id = raw_string_parts[0];
        con_config.master_id = raw_string_parts[6];
        if (raw_string_parts[10] == std::string("connected"))
            con_config.state = _state::connected;
        else
            con_config.state = _state::disconnected;



    }
}

std::string hiredis_client::set(request& req)
{
    rapidjson::Document d;

    if (d.Parse(req.body.c_str()).HasParseError()) {
        throw std::runtime_error( "Bad Json" );
    }

    if (!d.HasMember("key") or !d.HasMember("value") )
    {
        throw std::runtime_error( "Bad Json" );
    }


    std::string result;
    get_succsess = false;
    while(!get_succsess)
    {
        try {
            reply = static_cast<redisReply*>(redisCommand(c, "SET %b %b", d["key"].GetString(), d["value"].GetString() ));

            if (reply == nullptr)
            {
                std::string err_text = std::string("No answer on set ") + req.key;
                throw std::runtime_error( err_text.c_str() );
            }
            else{
                result = reply->str;
                freeReplyObject(reply);
                get_succsess = true;
            }
        } catch (...) {
            redisFree(c);
            connect();

        }
        diff = boost::posix_time::second_clock::local_time() - req.start;
        if ( diff.total_seconds() > 60 )
        {
            throw std::runtime_error( "Can't save data for " + req.key + " more than 1 min  " );
        }
    }

    return result;
}

void hiredis_client::del(request& req)
{
    std::string ret;
    req.key = req.request_path.substr(1,req.request_path.length()-1);
    get_succsess = false;
    while(!get_succsess)
    {

        try {
            reply = static_cast<redisReply*>(redisCommand(c,"DEL %s", req.key.c_str()) );
            if (reply == nullptr)
            {
                std::string err_text = std::string("No answer on del ") + req.key;
                throw std::runtime_error( err_text.c_str() );
            }
            else{
                ret.append(reply->str, reply->len);
                freeReplyObject(reply);
                get_succsess = true;
            }
        } catch (...) {
            redisFree(c);
            connect();

        }
        diff = boost::posix_time::second_clock::local_time() - req.start;
        if ( diff.total_seconds() > 60 )
        {
            throw std::runtime_error( "No data for del " + req.key + " more than 1 min  " );
        }
    }
}

void hiredis_client::config_updater()
{
    while(!stoped)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 0.5 sec

        try {
            reply = static_cast<redisReply*>(redisCommand(c,"CLUSTER NODES" ));
            if (reply == nullptr)
            {
                std::cout << "No answer on CLUSTER NODES " << std::endl;
                std::lock_guard<std::mutex> guard(con_config_mutex);
                con_config.state = _state::disconnected;
            }
            else{

                split(raw_string_parts, reply->str , boost::is_any_of("\n"));
                // ===================================================
                for (const auto &part : raw_string_parts)
                {
                    std::size_t found = part.find("myself");
                    if (found!=std::string::npos)
                        myself = part;
                        break;

                }
                raw_string_parts.clear();
                parse_cluster_nodes_string();
                freeReplyObject(reply);

            }
        } catch (...) {
            std::lock_guard<std::mutex> guard(con_config_mutex);
            con_config.state = _state::disconnected;

            redisFree(c);
            connect();

        }
    }

}

void hiredis_client::connect()
{
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
     c = redisConnectWithTimeout(con_config.ip.c_str(), con_config.port, timeout);

    if (c == nullptr || c->err)
    {
        if (c)
        {
            printf("Connection error: %s %i %s\n", con_config.ip.c_str(), con_config.port , c->errstr);
            redisFree(c);
            throw std::runtime_error( c->errstr );
        }
        else
        {
            throw std::runtime_error( "Connection error: can't allocate redis context\n" );
        }

    }



}

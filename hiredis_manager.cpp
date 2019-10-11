#include "hiredis_manager.hpp"

hiredis_manager::hiredis_manager()
{
    std::ifstream ifs("hiredis.conf");
    rapidjson::IStreamWrapper isw(ifs);

    d.ParseStream(isw);
    ifs.close();

    if (d.HasParseError())
        throw std::runtime_error( "hiredis.conf not valid JSON" );


    if (d.HasMember("servers_list"))
    {
        const rapidjson::Value& servers_list  = d["servers_list"];

        std::string ip = "";
        uint16_t port = 0;
        uint16_t uid = 0;

        assert(servers_list.IsArray()); // servers_list is an array
        for (rapidjson::Value::ConstValueIterator itr = servers_list.Begin(); itr != servers_list.End(); ++itr) {
            const rapidjson::Value& attribute = *itr;
            assert(attribute.IsObject()); // each attribute is an object

            for (rapidjson::Value::ConstMemberIterator itr2 = attribute.MemberBegin(); itr2 != attribute.MemberEnd(); ++itr2) {
                if (std::string(itr2->name.GetString()) == std::string("ip") )  ip = itr2->value.GetString();
                if (std::string(itr2->name.GetString()) == std::string("port") ) port = itr2->value.GetInt();
            }


            clients.push_back(new hiredis_client(ip, port, ++uid));

        }
    }
    updater();

    // Создадим поток
    WorkerUpdaterThread    = new boost::thread(   boost::bind(&hiredis_manager::worker_updater, this) , nullptr );


}

hiredis_manager::~hiredis_manager()
{
    stoped = true;
    WorkerUpdaterThread->join();
}

std::string hiredis_manager::get(request &req)
{
    uint16_t key_slot;
    try {
        req.key = req.request_path.substr(1,req.request_path.length()-1);
        key_slot = HASH_SLOT(req.key.c_str(), req.key.size());
        for (const auto &slot: slots_array)
        {
            if  (key_slot >= slot.slot_from )
            if  (key_slot <= slot.slot_to)
            {
                return reinterpret_cast<hiredis_client*>(slot.ptr)->get(req);
            }
        }
    } catch (std::exception &except)
    {
        throw std::runtime_error( except.what() );
    }

    throw std::runtime_error( "No servers for" + std::to_string(key_slot)+ " slot found" );
}

std::string hiredis_manager::set(request &req)
{
    uint16_t key_slot;
    try {
        req.key = req.request_path.substr(1,req.request_path.length()-1);
        key_slot = HASH_SLOT(req.key.c_str(), req.key.size());
        for (const auto &slot: slots_array)
        {
            if  (key_slot >= slot.slot_from )
            if  (key_slot <= slot.slot_to)
            {
                return reinterpret_cast<hiredis_client*>(slot.ptr)->set(req);
            }
        }
    } catch (std::exception &except)
    {
        throw std::runtime_error( except.what() );
    }

    throw std::runtime_error( "No servers for" + std::to_string(key_slot)+ " slot found" );
}

void hiredis_manager::del(request &req)
{
    uint16_t key_slot;
    try {
        req.key = req.request_path.substr(1,req.request_path.length()-1);
        key_slot = HASH_SLOT(req.key.c_str(), req.key.size());
        for (const auto &slot: slots_array)
        {
            if  (key_slot >= slot.slot_from )
            if  (key_slot <= slot.slot_to)
            {
                reinterpret_cast<hiredis_client*>(slot.ptr)->del(req);
            }
        }
    } catch (std::exception &except)
    {
        throw std::runtime_error( except.what() );
    }

    throw std::runtime_error( "No servers for" + std::to_string(key_slot)+ " slot found" );
}

void hiredis_manager::worker_updater()
{
    while(!stoped)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1 sec
        updater();
    }

}

void hiredis_manager::updater()
{
    std::vector<_config> infos;

    // Check all conections
    for (const auto &client : clients)
        infos.push_back(client->get_config());

    // Lock for updates
    std::lock_guard<std::mutex> guard(_mutex);

    // Clean up slots
    slots_array.clear();

    // For each connection info fill slots
    for (const auto &info : infos)
    {
        if (info.type == _type::master)
        {
            data.slot_from = info.slot_from;
            data.slot_to = info.slot_to;
            data.ptr = info.ref;
            slots_array.push_back( data );
        }
    }



}

uint16_t hiredis_manager::HASH_SLOT(const char *key, int keylen)
{
    int s, e; /* start-end indexes of { and } */

    /* Search the first occurrence of '{'. */
    for (s = 0; s < keylen; s++)
        if (key[s] == '{') break;

    /* No '{' ? Hash the whole key. This is the base case. */
    if (s == keylen) return crc16(key,keylen) & 16383;

    /* '{' found? Check if we have the corresponding '}'. */
    for (e = s+1; e < keylen; e++)
        if (key[e] == '}') break;

    /* No '}' or nothing between {} ? Hash the whole key. */
    if (e == keylen || e == s+1) return crc16(key,keylen) & 16383;

    /* If we are here there is both a { and a } on its right. Hash
     * what is in the middle between { and }. */
    return crc16(key+s+1,e-s-1) & 16383;

}



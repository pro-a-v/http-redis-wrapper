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
    worker_updater();

    // Создадим поток
    WorkerUpdaterThread    = new boost::thread(   boost::bind(&hiredis_manager::worker_updater, this) , nullptr );


}

std::string hiredis_manager::get(request &req)
{
    return std::string("");
}

std::string hiredis_manager::set(request &req)
{
    return std::string("");
}

void hiredis_manager::worker_updater()
{
    while(!stoped)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1 sec
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
}

void hiredis_manager::fill_slots()
{

}

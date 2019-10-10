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

        assert(servers_list.IsArray()); // servers_list is an array
        for (rapidjson::Value::ConstValueIterator itr = servers_list.Begin(); itr != servers_list.End(); ++itr) {
            const rapidjson::Value& attribute = *itr;
            assert(attribute.IsObject()); // each attribute is an object
            std::string ip = "";
            uint16_t port = 0;
            uint16_t uid =0;

            for (rapidjson::Value::ConstMemberIterator itr2 = attribute.MemberBegin(); itr2 != attribute.MemberEnd(); ++itr2) {
                if (std::string(itr2->name.GetString()) == std::string("ip") )  ip = itr2->value.GetString();
                if (std::string(itr2->name.GetString()) == std::string("port") ) port = itr2->value.GetInt();
            }


            clients.push_back(new hiredis_client(ip, port, ++uid));

        }
    }

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
    //
}

void hiredis_manager::fill_slots()
{
    for (const auto &client : clients)
    {
         std::lock_guard<std::mutex> guard(_mutex);
    }
}

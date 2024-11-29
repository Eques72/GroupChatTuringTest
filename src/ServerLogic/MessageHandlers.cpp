#include "ServerLogic.hpp"

using json = nlohmann::json;

auto get_default_error_data() -> json
{
    json resp;

    resp["msgType"] = -1;
    resp["errorCode"] = 0;
    resp["note"] = "Generic error message";

    return resp;
}

auto ServerLogic::client_registration_req_handler(nlohmann::json const & data) -> json
{
    json resp;
    
    if (data.contains("username") == false)
    {
        // Return an error message?
        return get_default_error_data();
    }
    resp["username"] = data["username"];

    if (data.contains("note"))
    {
        // Someting something note
        // Maybe log the note
    }

    resp["clientId"] = UniqueIdGenerator::generate_random_unique_id();

    resp["msgType"] = 2;

    return resp;
}

#include "ServerLogic.hpp"

using json = nlohmann::json;

auto ServerLogic::client_registration_req_handler(nlohmann::json const & data) -> json
{
    json resp;
    
    if (data.contains("username") == false)
    {
        // Return an error message?
        return json{};
    }
    resp["username"] = data["username"];

    if (data.contains("note"))
    {
        // Someting something note
        // Maybe log the note
    }

    resp["id"] = UniqueIdGenerator::generate_random_unique_id();

    return resp;
}

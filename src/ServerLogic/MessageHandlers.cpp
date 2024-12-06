#include "ServerLogic.hpp"

using json = nlohmann::json;

auto ServerLogic::get_default_error_data() -> json
{
    json resp;

    resp["msgType"] = static_cast<int32_t>(MsgType::ERROR);
    resp["errorCode"] = 0;
    resp["note"] = "Generic error message";

    return resp;
}

auto ServerLogic::client_registration_req_handler(nlohmann::json const & data, uWS::WebSocket<false, true, PerSocketData> * & ws) -> json
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

    resp["msgType"] = static_cast<int32_t>(MsgType::CLIENT_REGISTRATION_RESP);

    ws->getUserData()->id = resp.value<int32_t>("clientId", -1);
    assert(ws->getUserData()->id != -1);

    ws->getUserData()->username = resp.value<std::string>("username", "");
    assert(ws->getUserData()->username.length() != 0);

    ws->subscribe(std::to_string(ws->getUserData()->id));

    m_usernames.insert({
        ws->getUserData()->id,
        ws->getUserData()->username
    });

    return resp;
}

auto ServerLogic::create_lobby_req_handler(nlohmann::json && data, uWS::WebSocket<false, true, PerSocketData> * & ws) -> json
{
    if (data.contains("clientId") == false
        || data.contains("username") == false
        || data.contains("maxUsers") == false
        || data.contains("roundsNumber") == false
    )
    {
        return get_default_error_data();
    }

    if (ws->getUserData()->id != data.value<int32_t>("clientId", -1))
    {
        return get_default_error_data();
    }

    int32_t newLobbyId = UniqueIdGenerator::generate_random_unique_id();

    m_lobbies.insert({
        newLobbyId,
        std::make_unique<Lobby>(
            m_server,
            mp_loop,
            newLobbyId,
            data.value<std::string>("username", ""),
            data.value<int32_t>("maxUsers", 5),
            data.value<int32_t>("roundsNumber", 5)
        )
    });
    m_lobbies.at(newLobbyId)->pass_msg(std::move(data));
    m_lobbies.at(newLobbyId)->startLobbyThread();

    ws->getUserData()->lobbyId = newLobbyId;

    return json{};
}

auto ServerLogic::pass_msg_to_lobby_handler(nlohmann::json && data, uWS::WebSocket<false, true, PerSocketData> * & ws) -> nlohmann::json
{
    if (data.value<int32_t>("clientId", -1) != ws->getUserData()->id)
    {
        json resp;
        resp["msgType"] = static_cast<int32_t>(MsgType::ERROR);
        resp["errorCode"] = 0;
        resp["note"] = "Bad client id!";
        return resp;
    }
    
    int32_t lobbyId = data.value<int32_t>("lobbyId", -1);
    if (m_lobbies.contains(lobbyId) == false)
    {
        json resp;
        resp["msgType"] = static_cast<int32_t>(MsgType::ERROR);
        resp["errorCode"] = 0;
        resp["note"] = "Bad lobby id!";
        return resp;
    }

    if (m_lobbies.at(lobbyId)->isLobbyRunning() == false)
    {
        json resp;
        resp["msgType"] = static_cast<int32_t>(MsgType::ERROR);
        resp["errorCode"] = 0;
        resp["note"] = "Lobby is no longer running!";

        m_lobbies.erase(lobbyId);

        return resp;
    }

    m_lobbies.at(lobbyId)->pass_msg(std::move(data));

    if (ws->getUserData()->lobbyId == -1)
    {
        ws->getUserData()->lobbyId = lobbyId;
    }

    return json{};
}

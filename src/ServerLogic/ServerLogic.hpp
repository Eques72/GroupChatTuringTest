#pragma once

#include <iostream>
#include <unordered_map>
#include <memory>
#include <cassert>

#include "App.h"
#include "PerSocketData.hpp"
#include "UniqueIdGenerator.hpp"
#include "json.hpp"
#include "Lobby.hpp"

#define SSL false

class ServerLogic
{
public:

    explicit ServerLogic(uWS::App & server, uWS::Loop * const p_loop);

    ServerLogic() = delete;
    ServerLogic(ServerLogic const &) = delete;
    ServerLogic(ServerLogic &&) = delete;
    ServerLogic& operator=(ServerLogic const &) = delete;
    ServerLogic& operator=(ServerLogic &&) = delete;

    ~ServerLogic() = default;

    void connection_established_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws);
    void message_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws, std::string_view msg, uWS::OpCode opCode);
    void connection_closed_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws, int code, std::string_view msg);

private:

    uWS::App & m_server;
    uWS::Loop * mp_loop;
    std::unordered_map<int32_t, std::unique_ptr<Lobby>> m_lobbies;

    auto get_default_error_data() -> nlohmann::json;

    auto client_registration_req_handler(nlohmann::json const & data, uWS::WebSocket<false, true, PerSocketData> * & ws) -> nlohmann::json;
    auto create_lobby_req_handler(nlohmann::json && data, uWS::WebSocket<false, true, PerSocketData> * & ws) -> nlohmann::json;

};



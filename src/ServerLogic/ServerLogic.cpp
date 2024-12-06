#include "ServerLogic.hpp"


ServerLogic::ServerLogic(uWS::App & server, uWS::Loop * const p_loop)
: m_server{server}
{
    assert(p_loop != nullptr);

    mp_loop = p_loop;
}

void ServerLogic::connection_established_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws)
{
    ws->getUserData()->id = -1;
    ws->getUserData()->lobbyId = -1;
    ws->getUserData()->username = "";
}

void ServerLogic::message_handler(uWS::WebSocket<false, true, PerSocketData> * ws, std::string_view msg, uWS::OpCode opCode)
{
    uWS::App * l_server = &m_server;

    using json = nlohmann::json;

    json data = json::parse(msg, nullptr, false);
    if (data.is_discarded())
    {
        json respData = get_default_error_data();
        mp_loop->defer(
            [ws, respData]()
            {
                ws->send(
                    respData.dump(),
                    uWS::OpCode::TEXT
                );
            }
        );

        return;
    }

    if (data.contains("msgType") == false)
    {
        json respData = get_default_error_data();
        mp_loop->defer(
            [ws, respData]()
            {
                ws->send(
                    respData.dump(),
                    uWS::OpCode::TEXT
                );
            }
        );

        return;
    }

    json respData;

    switch (data.value<int32_t>("msgType", 0))
    {
        case MsgType::ERROR:
        {
            // Got an error message from client?
        } break;

        case MsgType::UNDEFINED: 
        // case MsgType::CLIENT_REGISTRATION_RESP:
        // case MsgType::USER_JOINED:
        default:
        { // Server should NOT receive messages with those values
            respData["msgType"] = static_cast<int32_t>(MsgType::ERROR);
            respData["errorCode"] = 0;
            respData["note"] = "The server should not receive a message with that msgType id!";
        } break;

        case MsgType::CLIENT_REGISTRATION_REQ:
        {
            respData = client_registration_req_handler(data, ws);
        } break;

        case MsgType::CREATE_LOBBY_REQ:
        {
            respData = create_lobby_req_handler(std::move(data), ws);
        } break;

        case MsgType::JOIN_LOBBY_REQ:
        case MsgType::POST_NEW_CHAT:
        case MsgType::START_GAME:
        case MsgType::GUESS_BOT_RESP:
        {
            respData = pass_msg_to_lobby_handler(std::move(data), ws);
        } break;
    }

    if (respData.empty() == false)
    {
        std::string clientId = std::to_string(ws->getUserData()->id);
        mp_loop->defer(
            [ws, respData]()
            {
                ws->send(
                    respData.dump(),
                    uWS::OpCode::TEXT
                );
            }
        );
    }
}


void ServerLogic::connection_closed_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws, int code, std::string_view msg)
{
    int32_t lobbyId = ws->getUserData()->lobbyId;
    if (m_lobbies.contains(lobbyId) && m_lobbies.at(lobbyId)->isLobbyRunning())
    {
        m_lobbies.at(lobbyId)->client_disconnected(ws->getUserData()->id);
    }
}

auto ServerLogic::get_username_by_client_id(int32_t clientId) -> std::string
{
    if (m_usernames.contains(clientId))
    {
        return m_usernames.at(clientId);
    }
    else
    {
        return "";
    }
}

std::unordered_map<int32_t, std::string> ServerLogic::m_usernames;

#include "ServerLogic.hpp"


ServerLogic::ServerLogic(uWS::App & server, uWS::Loop * const p_loop)
: m_server{server}
{
    assert(p_loop != nullptr);

    mp_loop = p_loop;
}


void ServerLogic::connection_established_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws)
{
    //
}

/*
    {
        // Every thread creates its own Loop
        // We can pass the pointer to said Loop to different threads?
        // Each thread can then call Loop::defer() (which is the only thread safe function in the library as far as I'm aware)
        //      to execute something in a safe manner
        // Here is an example of sending some data
        
        // From other threads you can call
        p_loop->defer([&ws, &msg]() {
            // Change them to smart pointers!!!
            uWS::WebSocket<false, true, PerSocketData> * p_ws = &ws; // or just "ws" depending on the types
            std::string * p_msg = msg;
            
            // IDK If creating the additional pointers is needed because IDK how memory / objects / data works with lambas when passing lambdas to be executed on different thread

            p_ws->send(*p_msg, uWS::OpCode::TEXT);
        });

        // The lambda should execute and send the data in a thread safe manner
    }
*/

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

    switch (data.value<int32_t>("msgType", 0))
    {
        case -1: { // error
            // Got an error message from client?
        } break;

        case 0: 
        case 2: { // Server should NOT receive messages with those values
            json respData;
            respData["msgType"] = -1;
            respData["errorCode"] = 0;
            respData["note"] = "The server should not receive a message with that msgType id!";

            mp_loop->defer(
                [ws, respData]()
                {
                    ws->send(
                        respData.dump(),
                        uWS::OpCode::TEXT
                    );
                }
            );
        } break;

        case 1: { // client-registration-req
            json respData = client_registration_req_handler(data, ws);

            std::string clientId = std::to_string(ws->getUserData()->id);
            mp_loop->defer(
                [l_server, clientId, respData]()
                {
                    l_server->publish(
                        clientId,
                        respData.dump(),
                        uWS::OpCode::TEXT
                    );
                }
            );
        } break;

        case 3: { // create-lobby-req
            json respData = create_lobby_req_handler(std::move(data), ws);

            // Send back an error msg only. If the lobby thread gets created: it will send back create-lobby-resp message
            if (respData.empty() == false)
            {
                std::string clientId = std::to_string(ws->getUserData()->id);
                mp_loop->defer(
                    [l_server, clientId, respData]()
                    {
                        l_server->publish(
                            clientId,
                            respData.dump(),
                            uWS::OpCode::TEXT
                        );
                    }
                );
            }
        } break;

        default: {
            // Send back an error message (no such msgType was defined in the Communication Protocol spec)
        } break;
    }
}


void ServerLogic::connection_closed_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws, int code, std::string_view msg)
{
    /* You may access ws->getUserData() here */

    std::cout << "Connection closed with " << ws->getRemoteAddressAsText() << std::endl;
}

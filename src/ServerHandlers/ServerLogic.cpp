#include "ServerLogic.hpp"


ServerLogic::ServerLogic(uWS::Loop * const p_loop)
{
    assert(p_loop != nullptr);

    mp_loop = p_loop;
}


void ServerLogic::connection_established_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws)
{
    // TODO Move this to register user handler
    // ws->getUserData()->id = UniqueIdGenerator::generate_random_unique_id();

    std::cout << "Connection established with " << ws->getRemoteAddressAsText() << std::endl;
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
    using json = nlohmann::json;

    json data = json::parse(msg, nullptr, false);
    if (data.is_discarded())
    {
        // Send back an error message here?
        return;
    }

    if (data.contains("msgType") == false)
    {
        // Send back an error message here?
        return;
    }

    switch (data.value<int32_t>("msgType", 0))
    {
        case -1: {
            // Got an error message
        } break;

        case 0: {
            // This should never happen
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

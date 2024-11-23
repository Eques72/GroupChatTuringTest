#include <iostream>
#include <cstdint>

#include "App.h"
#include "PerSocketData.hpp"
#include "ServerHandlers.hpp"


auto main(int32_t argc, char * argv[]) -> int32_t
{
    /*
        IMPORTANT INFO FOR FUTURE - DO NOT DELETE
        If changing from non-SSL to SSL encrypted communication you have to change all
        uWS::WebSocket<false, true, PerSocketData> *
        types / arguments to 
        uWS::WebSocket<true, true, PerSocketData> *
    */

    // The port number is hardcoded since it is also hardcoded into Dockerfile and docker-compose
    // IDK If there is a way to parametrize it in all 3 places at once from one config
    int constexpr PORT = 12345;

    uWS::App server = uWS::App()
    .ws<PerSocketData>(
        /* url path */
        "/*",
        /* Settings */
        {
            .compression = uWS::DISABLED,                       // Don't need compression, also I feel it can cause problems
            .maxPayloadLength         = 1 * 1024 * 1024,        // 1 MB
            .idleTimeout              = 60,                     // In seconds
            .maxBackpressure          = 100 * 1024 * 1024,      // 100 MB
            .closeOnBackpressureLimit = false,
            .resetIdleTimeoutOnSend   = true,
            .sendPingsAutomatically   = true,
            
            /* Handlers */
            .upgrade = nullptr, // Some random handlers for functionalities we don't need
            
            .open = [](auto * ws) {
                connection_established_handler(ws);
            },
            
            .message = [](auto * ws, std::string_view msg, uWS::OpCode opCode) 
            {
                message_handler(ws, msg, opCode);
            },
            
            .drain = nullptr,   //
            .ping  = nullptr,   // Some random handlers for functionalities we don't need
            .pong  = nullptr,   //
            
            .close = [](auto * ws, int code, std::string_view msg) {
                connection_closed_handler(ws, code, msg);
            }
        }
    )
    .listen(
        PORT,
        [](auto * listenSocket) { }
    )
    .run();

    /*
        Use map<int, ws *> for storing websockeets for each game
        OR 
        use websocket pub / sub topics for messaging people automatically?
    */

    /*
    {
        // Every thread creates its own Loop
        // We can pass the pointer to said Loop to different threads?
        // Each thread can then call Loop::defer() (which is the only thread safe function in the library as far as I'm aware)
        //      to execute something in a safe manner
        // Here is an example of sending some data
        
        // Pass this to other threads that use this server thread
        auto * p_loop = uWS::Loop::get();

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

    return EXIT_SUCCESS;
}

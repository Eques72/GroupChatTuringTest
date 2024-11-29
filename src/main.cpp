#include <iostream>
#include <cstdint>

#include "App.h"
#include "PerSocketData.hpp"
#include "ServerLogic.hpp"


auto main(int32_t argc, char * argv[]) -> int32_t
{
    /*
        IMPORTANT INFO FOR FUTURE - DO NOT DELETE
        If changing from non-SSL to SSL encrypted communication you have to change all
        uWS::WebSocket<false, true, PerSocketData> *
        types / arguments to 
        uWS::WebSocket<true, true, PerSocketData> *
    */

   /*
        IMPORTANT INFO FOR FUTURE - DO NOT DELETE
        There is no guarantee information anywhere that the address of each websocket pointer will remain the same 
        throughout the entire lifetime of the websocket, so instead of using the websocket and ids directly
        we will subscribe each websocket to the topic equal to the id that that websocket / client received
        at registration and use per topic message publishing mechanism for sending the messages to that client
    */

   /*
        IMPORTANT INFO FOR FUTURE - DO NOT DELETE
        There should be some kind of verification mechanism that the client does not change their id
        but no such mechanism exists now and frankly IDK how would I even implement it
    */

   // TODO Add logging

    int constexpr PORT = 12345;

    uWS::App server = uWS::App();

    ServerLogic logic(server, uWS::Loop::get());

    server.ws<PerSocketData>(
        /* url path - not used in this project so we have a wildcard */
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
            
            .open = [&logic](auto * ws) {
                logic.connection_established_handler(ws);
            },
            
            .message = [&logic](auto * ws, std::string_view msg, uWS::OpCode opCode) 
            {
                logic.message_handler(ws, msg, opCode);
            },
            
            .drain = nullptr,   //
            .ping  = nullptr,   // Some random handlers for functionalities we don't need
            .pong  = nullptr,   //
            
            .close = [&logic](auto * ws, int code, std::string_view msg) {
                logic.connection_closed_handler(ws, code, msg);
            }
        }
    )
    .listen(
        PORT,
        [](auto * listenSocket) { }
    );
    
    server.run();

    return EXIT_SUCCESS;
}

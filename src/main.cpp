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

   // TODO Add logging

    int constexpr PORT = 12345;

    assign_loop_ptr(uWS::Loop::get());

    uWS::App server = uWS::App()
    .ws<PerSocketData>(
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

    return EXIT_SUCCESS;
}

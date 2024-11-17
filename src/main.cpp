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
            // TODO What is compression here?
            // .compression = uWS::CompressOptions(uWS::DEDICATED_COMPRESSOR_4KB | uWS::DEDICATED_DECOMPRESSOR),
            .compression = uWS::DISABLED,
            // TODO 100 MB max payload is way too much -> figure out new one
            .maxPayloadLength = 100 * 1024 * 1024,
            .idleTimeout = 60, // In seconds
            .maxBackpressure = 100 * 1024 * 1024,
            .closeOnBackpressureLimit = false,
            .resetIdleTimeoutOnSend = true,
            .sendPingsAutomatically = true,
            
            /* Handlers */
            .upgrade = nullptr,
            
            .open = [](auto * ws) {
                connection_established_handler(ws);
            },
            
            .message = [](auto * ws, std::string_view msg, uWS::OpCode opCode) 
            {
                message_handler(ws, msg, opCode);
            },
            
            .drain = nullptr,
            .ping = nullptr,
            .pong = nullptr,
            
            .close = [](auto * ws, int code, std::string_view msg) {
                connection_closed_handler(ws, code, msg);
            }
        }
    )
    .listen(
        PORT,
        [](auto * listenSocket)
        {
            if (listenSocket)
            {
                std::cout << "Listening on port " << PORT << std::endl;
            }
        }
    )
    .run();

    return EXIT_SUCCESS;
}

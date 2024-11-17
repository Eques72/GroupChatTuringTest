#include <iostream>
#include <cstdint>

#include "App.h"
#include "PerSocketData.hpp"


auto main(int32_t argc, char * argv[]) -> int32_t
{
    int constexpr PORT = 12345;

    uWS::App server = uWS::App()
    .ws<PerSocketData>(
        "/*",
        {
            /* Settings */
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
                /* Open event here, you may access ws->getUserData() which points to a PerSocketData struct */
                std::cout << "Connection opened!" << std::endl;
            },
            
            .message = [](auto * ws, std::string_view msg, uWS::OpCode opCode) {
                std::cout << "Got message: " << msg << std::endl;
                ws->send(msg, opCode);
            },
            
            .drain = nullptr, .ping = nullptr, .pong = nullptr,
            
            .close = [](auto * ws, int code, std::string_view msg) {
                /* You may access ws->getUserData() here */
                std::cout << "Connection closed" << std::endl;
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

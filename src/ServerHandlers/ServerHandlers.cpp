#include "ServerHandlers.hpp"


void connection_established_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws)
{
    /* You may access ws->getUserData() here */

    std::cout << "Connection established with " << ws->getRemoteAddressAsText() << std::endl;
}


void message_handler(uWS::WebSocket<false, true, PerSocketData> * ws, std::string_view msg, uWS::OpCode opCode)
{
    /* You may access ws->getUserData() here */

    std::cout << "Got message: " << msg << std::endl;

    // ws->send(msg, opCode);
    ws->send("Does this work?", uWS::OpCode::TEXT);
}


void connection_closed_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws, int code, std::string_view msg)
{
    /* You may access ws->getUserData() here */

    std::cout << "Connection closed with " << ws->getRemoteAddressAsText() << std::endl;
}

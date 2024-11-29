#pragma once

#include <iostream>
#include <cassert>

#include "App.h"
#include "PerSocketData.hpp"
#include "UniqueIdGenerator.hpp"
#include "json.hpp"

#define SSL false

class ServerLogic
{
public:

    ServerLogic(uWS::Loop * const p_loop);

    void connection_established_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws);
    void message_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws, std::string_view msg, uWS::OpCode opCode);
    void connection_closed_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws, int code, std::string_view msg);

private:

    uWS::Loop * mp_loop;

};



#pragma once

#include <iostream>

#include "App.h"
#include "PerSocketData.hpp"

#define SSL false

void connection_established_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws);

void message_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws, std::string_view msg, uWS::OpCode opCode);

void connection_closed_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws, int code, std::string_view msg);

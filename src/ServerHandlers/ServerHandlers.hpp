#pragma once

#include <iostream>
#include <cassert>

#include "App.h"
#include "PerSocketData.hpp"
#include "UniqueIdGenerator.hpp"

#define SSL false

void assign_loop_ptr(uWS::Loop * const p_loop);

auto get_loop() -> uWS::Loop * const;

void connection_established_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws);

void message_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws, std::string_view msg, uWS::OpCode opCode);

void connection_closed_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws, int code, std::string_view msg);

#pragma once

/* ws->getUserData() returns one of these */
struct PerSocketData {
    int32_t id;
    std::string username;
};

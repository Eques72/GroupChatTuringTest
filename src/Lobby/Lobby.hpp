#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <mutex>
#include <thread>
#include <queue>
#include <semaphore>
#include "App.h"
#include "json.hpp"

// TODO Rewrite the class.
// Each Lobby (or rename LobbyThread) should be able to be run using a runThreaded() member function
// And there should be a functionalities so that the main thread can pass on the message / json data
//      to the LobbyThread and the thread handles the message

class Lobby
{
public:

    explicit Lobby(uWS::App & server, uWS::Loop * p_loop, std::string const & creatorUsername, int32_t maxUsers, int32_t roundsNumber);

    Lobby() = delete;
    Lobby(Lobby const &) = delete;
    Lobby(Lobby &&) = delete;
    Lobby& operator=(Lobby const &) = delete;
    Lobby& operator=(Lobby &&) = delete;

    ~Lobby() = default;

    void pass_msg(nlohmann::json && data);

    void startLobbyThread();

private:

    uWS::App & m_server;
    uWS::Loop * const mp_loop;
    std::string const m_creatorUsername;
    int32_t const m_maxUsers;
    int32_t const m_roundsNumber;

    std::mutex m_mutex;
    std::vector<int32_t> m_clientsIds;
    std::queue<nlohmann::json> m_msgs;
    std::counting_semaphore<100> m_msgsSmph;

    void add_client_to_lobby(int32_t clientId);

    // TODO Add message handlers for each message that can be passed to the lobby (create-lobby-req included)
};

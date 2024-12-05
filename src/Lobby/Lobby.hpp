#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <mutex>
#include <thread>
#include <queue>
#include <semaphore>
#include <format>
#include <chrono>
#include "App.h"
#include "json.hpp"
#include "MsgTypeEnum.hpp"
#include "ServerLogic.hpp"

// TODO Rewrite the class.
// Each Lobby (or rename LobbyThread) should be able to be run using a runThreaded() member function
// And there should be a functionalities so that the main thread can pass on the message / json data
//      to the LobbyThread and the thread handles the message

int32_t constexpr MAX_MSGS_QUEUE_LEN = 100;
int64_t constexpr READY_STATE_CLIENT_WAIT_SECODS = 10;
int64_t constexpr ROUND_LENGHT_SECONDS = 10;

class Lobby
{
public:

    explicit Lobby(uWS::App & server, uWS::Loop * p_loop, int32_t lobbyId, std::string const & creatorUsername, int32_t maxUsers, int32_t roundsNumber);

    Lobby() = delete;
    Lobby(Lobby const &) = delete;
    Lobby(Lobby &&) = delete;
    Lobby& operator=(Lobby const &) = delete;
    Lobby& operator=(Lobby &&) = delete;

    ~Lobby();

    void pass_msg(nlohmann::json && data);

    void startLobbyThread();
    auto requestThreadStop() -> bool;

private:

    int32_t     const m_id;
    uWS::App &        m_server;
    uWS::Loop * const mp_serverLoop;
    std::string const m_creatorUsername;
    int32_t     const m_maxUsers;
    int32_t     const m_roundsNumber;
    bool              m_startGame;
    bool              m_startNewRound;
    bool              m_acceptsNewUsers;
    std::chrono::time_point<std::chrono::system_clock> m_newRoundStartTimepoint;
    std::chrono::time_point<std::chrono::system_clock> m_msgWaitTimeout;

    std::mutex                   m_mutex;
    std::vector<int32_t>         m_clientsIds;
    std::queue<nlohmann::json>   m_msgs;
    std::counting_semaphore<MAX_MSGS_QUEUE_LEN> m_msgsSmph;

    std::jthread m_thread;

    void add_client_to_lobby(int32_t clientId);
    auto check_if_valid_client(int32_t clientId) -> bool;
    void send_to_all_clients(nlohmann::json const & msg, std::vector<int32_t> excludedIds = {});

    auto create_lobby_req_handler(Lobby * self, nlohmann::json const & data) -> nlohmann::json;
    auto join_lobby_req_handler(Lobby * self, nlohmann::json const & data) -> nlohmann::json;
    void user_joined_notify(Lobby * self, int32_t newUserId);
    auto post_new_chat_handler(Lobby * self, nlohmann::json const & data) -> nlohmann::json;
    auto start_game_handler(Lobby * self, nlohmann::json const & data) -> nlohmann::json;
    // TODO Add message handlers for each message that can be passed to the lobby
};

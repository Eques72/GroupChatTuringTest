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
#include <filesystem>
#include <fstream>
#include <atomic>
#include "App.h"
#include "json.hpp"
#include "MsgTypeEnum.hpp"
#include "ServerLogic.hpp"


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
    void client_disconnected(int32_t clientId);

    void startLobbyThread();
    auto isLobbyRunning() -> bool;
    auto requestThreadStop() -> bool;

private:

    enum class LobbyState
    {
        IN_LOBBY,
        GAME_STARTING,
        NEW_ROUND,
        PLAYING,
        VOTING,
        ROUND_ENDED,
    };

    bool              m_isRunning;
    int32_t     const m_id;
    uWS::App &        m_server;
    uWS::Loop * const mp_serverLoop;
    std::string const m_creatorUsername;
    int32_t     const m_maxUsers;
    int32_t     const m_roundsNumber;
    int32_t           m_currentRound;
    std::atomic<LobbyState> m_state;
    std::chrono::time_point<std::chrono::system_clock> m_lastStateTimepoint;
    std::chrono::time_point<std::chrono::system_clock> m_msgWaitTimeout;
    // TODO Delete when not needed
    // bool              m_inLobby;
    // bool              m_startGame;
    // bool              m_startNewRound;
    // bool              m_startVoting;
    // std::chrono::time_point<std::chrono::system_clock> m_newRoundStartTimepoint;

    std::string                  m_currentBotNickname;
    std::mutex                   m_mutex;
    std::vector<int32_t>         m_clientsIds;
    std::unordered_map<int32_t, int32_t> m_clientScores;
    std::queue<nlohmann::json>   m_msgs;
    std::counting_semaphore<100/*Max len?*/> m_msgsSmph;

    std::jthread m_thread;

    void add_client_to_lobby(int32_t clientId);
    auto check_if_valid_client(int32_t clientId) -> bool;
    void send_to_all_clients(nlohmann::json const & msg, std::vector<int32_t> excludedIds = {});

    auto create_lobby_req_handler(Lobby * self, nlohmann::json const & data) -> nlohmann::json;
    auto join_lobby_req_handler(Lobby * self, nlohmann::json const & data) -> nlohmann::json;
    void user_joined_notify(Lobby * self, int32_t newUserId);
    auto post_new_chat_handler(Lobby * self, nlohmann::json const & data) -> nlohmann::json;
    auto start_game_handler(Lobby * self, nlohmann::json const & data) -> nlohmann::json;
    auto guess_bot_handler(Lobby * self, nlohmann::json const & data) -> nlohmann::json;
    // TODO Add message handlers for each message that can be passed to the lobby

    static auto read_topic_from_file(std::filesystem::path const & path) -> std::string;

    void close_lobby();
};

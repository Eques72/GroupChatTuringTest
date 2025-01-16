#include "Lobby.hpp"

using json         = nlohmann::json;
using seconds      = std::chrono::seconds;
using time_point   = std::chrono::time_point<std::chrono::system_clock>;
using system_clock = std::chrono::system_clock;


int64_t constexpr TIME_BUFFER = 3;
int64_t constexpr READY_STATE_CLIENT_WAIT_SECONDS = 2;
int64_t constexpr ROUND_LENGTH_SECONDS            = 60;
int64_t constexpr VOTING_LENGTH_SECONDS           = 15;
int64_t constexpr ROUND_ENDED_LENGTH_SECONDS      = 10;
int64_t constexpr CHATBOT_MESSAGE_PERIOD_SECONDS  = 10;
std::filesystem::path const DEFAULT_TOPIC_FILE = "data/topics.txt";
std::filesystem::path const DEFAULT_NICKNAME_FILE = "data/nickname_color.csv";


Lobby::Lobby(uWS::App & server, uWS::Loop * p_loop, int32_t lobbyId, std::string const & creatorUsername, int32_t maxUsers, int32_t roundsNumber)
    :m_isRunning{false},
     m_server{server},
     mp_serverLoop{p_loop},
     m_id{lobbyId},
     m_creatorUsername{creatorUsername}, 
     m_maxUsers{maxUsers}, 
     m_roundsNumber{roundsNumber},
     m_currentRound{0},
     m_state{LobbyState::IN_LOBBY},
     m_zmqSocketType{zmqpp::socket_type::req},
     m_zmqSocket{zmqpp::socket(m_zmqContext, m_zmqSocketType)},
     m_isSocketConnected{false},
     m_msgsSmph{0}
{
    m_clientsIds.reserve(m_maxUsers);

    // Placeholder / inits
    m_msgWaitTimeout = std::chrono::system_clock::now() + std::chrono::hours(1); 
    m_currentBotNickname = "chatbot";

    // Generate the port number
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int32_t> dis(49153, 65535);

    std::string port = std::to_string(dis(gen));
    // TODO Is there a way to check if any given port is free?
    
    m_zmqEndpoint = "tcp://localhost:" + port;

    // Start the chatbot helper script
    m_chatbotThread = std::jthread([](std::string port)
    {
        std::string command{"python3 ./bin/chatbot.py " + port};

        system(command.c_str());
    }, port);
    m_chatbotThread.detach();
    m_lastChatbotMessage = system_clock::now();
}

Lobby::~Lobby()
{
    close_lobby();
    std::cout << "~Lobby (" << m_id << ") Is being destructed" << std::endl;
}

void Lobby::pass_msg(json && data)
{
    std::lock_guard lg{m_mutex};

    if (m_msgs.size() >= static_cast<std::size_t>(m_msgsSmph.max()))
    {
        // Send back an error?
    }

    m_msgs.push(std::move(data));
    m_msgsSmph.release();
}

void Lobby::client_disconnected(int32_t clientId)
{
    std::unique_lock lock{m_mutex, std::defer_lock};

    lock.lock();
    auto pos = std::find(m_clientsIds.begin(), m_clientsIds.end(), clientId);
    
    bool isClientFromThisLobby = pos != m_clientsIds.end();
    if (isClientFromThisLobby == false)
    {
        return;
    }

    m_clientsIds.erase(pos);
    m_clientScores.erase(clientId);

    if (m_clientsIds.size() == 0)
    {
        requestThreadStop();

        return;
    }

    lock.unlock();

    json msg;
    msg["msgType"] = static_cast<int32_t>(MsgType::USER_LEFT);
    msg["lobbyId"] = m_id;
    msg["username"] = ServerLogic::get_username_by_client_id(clientId);
    send_to_all_clients(msg);
}

void Lobby::startLobbyThread()
{
    Lobby * self{this};

    m_thread = std::jthread(
        [self](std::stop_token l_stopToken)
        {
            uWS::App * l_server = &(self->m_server);

            std::unique_lock l_lock{self->m_mutex, std::defer_lock};

            int32_t msgType{0};
            json l_msg;
            json l_resp;
            std::string l_clientId;

            while (l_stopToken.stop_requested() == false)
            {
                if (self->m_msgsSmph.try_acquire_until(self->m_msgWaitTimeout) == false)
                {
                    // Timedout trying to wait for new messages
                    goto timedout;
                }

                l_lock.lock();
                l_msg = self->m_msgs.front();
                self->m_msgs.pop();
                l_lock.unlock();

                l_clientId = std::to_string(l_msg.value<int32_t>("clientId", -1));
                if (l_clientId.length() == 0 || l_msg.empty())
                {
                    continue;
                }

                msgType = l_msg.value<int32_t>("msgType", 0);

                if (msgType != MsgType::CREATE_LOBBY_REQ && msgType != MsgType::JOIN_LOBBY_REQ)
                {
                    if (self->check_if_valid_client(l_msg.value<int32_t>("clientId", -1)) == false)
                    {
                        continue;
                    }
                }

                switch (msgType)
                {
                    case MsgType::ERROR:
                    {
                        // Later or ignore
                    } break;

                    case MsgType::CREATE_LOBBY_REQ:
                    {
                        l_resp = self->create_lobby_req_handler(self, l_msg);
                    } break;

                    case MsgType::JOIN_LOBBY_REQ:
                    {
                        l_resp = self->join_lobby_req_handler(self, l_msg);

                        if (l_resp.value<int32_t>("msgType", 0) != MsgType::ERROR)
                        {
                            json data;
                            data["msgType"] = static_cast<int32_t>(MsgType::USER_JOINED);
                            data["lobbyId"] = self->m_id;
                            data["newUser"] = ServerLogic::get_username_by_client_id(l_msg.value<int32_t>("clientId", -1));
                            self->send_to_all_clients(data, {l_msg.value<int32_t>("clientId", -1)});
                        }

                    } break;

                    case MsgType::POST_NEW_CHAT:
                    {
                        l_resp = self->post_new_chat_handler(self, l_msg);
                    } break;

                    case MsgType::START_GAME:
                    {
                        l_resp = self->start_game_handler(self, l_msg);
                    } break;

                    case MsgType::GUESS_BOT_RESP:
                    {
                        l_resp = self->guess_bot_handler(self, l_msg);
                    } break;

                    case MsgType::UNDEFINED:
                    case MsgType::CLIENT_REGISTRATION_REQ:
                    case MsgType::CLIENT_REGISTRATION_RESP:
                    case MsgType::USER_JOINED:
                    case MsgType::NEW_CHAT:
                    case MsgType::NEW_ROUND:
                    default:
                    {
                        l_resp["msgType"] = static_cast<int32_t>(MsgType::ERROR);
                        l_resp["errorCode"] = 0;
                        l_resp["note"] = "bad msgType";
                    } break;
                }

                if (l_resp.empty() == false)
                {
                    self->mp_serverLoop->defer(
                        [l_server, l_clientId, l_resp]()
                        {
                            l_server->publish(
                                l_clientId,
                                l_resp.dump(),
                                uWS::OpCode::TEXT
                            );
                        }
                    );    
                }

                timedout:
                if (std::chrono::duration_cast<seconds>(system_clock::now() - self->m_lastChatbotMessage).count() > CHATBOT_MESSAGE_PERIOD_SECONDS
                    && self->m_isSocketConnected
                    && self->m_state == LobbyState::PLAYING)
                {
                    zmqpp::message message;
                    if (self->m_chatLogs.empty())
                    {
                        message << "...";
                    }
                    else
                    {
                        message << self->m_chatLogs;
                    }
                    self->m_zmqSocket.send(message);
                    self->m_chatLogs.clear();
                    std::string buffer;
                    self->m_zmqSocket.receive(buffer);

                    if (auto pos = std::find(buffer.begin(), buffer.end(), ':'); pos != buffer.end())
                    {
                        // If the AI returns the message in a <user>: <message> format, try to cut it
                        buffer.erase(buffer.begin(), pos);
                    }

                    if (buffer[buffer.length() - 1] == '\n')
                    {
                        buffer.pop_back();
                    }

                    json msgData;
                    msgData["chatMsg"] = buffer;

                    (void) self->post_new_chat_handler(self, msgData, true);

                    self->m_lastChatbotMessage = system_clock::now();
                    if (self->m_msgWaitTimeout > (system_clock::now() + seconds(CHATBOT_MESSAGE_PERIOD_SECONDS)))
                    {
                        self->m_msgWaitTimeout = system_clock::now() + seconds(CHATBOT_MESSAGE_PERIOD_SECONDS);
                    }
                }

                switch (self->m_state)
                {
                    case LobbyState::IN_LOBBY:
                    {
                        // Do nothing
                    } break;

                    case LobbyState::GAME_STARTING:
                    {
                        // Check if the timeout (waiting for READY_STATE_CLIENT_WAIT_SECONDS) happened
                        bool clientReadyStateTimeout = std::chrono::duration_cast<seconds>(system_clock::now() - self->m_lastStateTimepoint).count() > READY_STATE_CLIENT_WAIT_SECONDS;
                        
                        if (clientReadyStateTimeout)
                        {
                            self->m_state = LobbyState::NEW_ROUND;
                            [[fallthrough]];
                        }
                        else
                        {
                            break;
                        }
                    } // No breaking! Intentional fallthrough (sometimes)

                    case LobbyState::NEW_ROUND:
                    {
                        self->m_chatLogs.clear();
                        
                        self->m_currentRound++;

                        std::unique_lock lock{self->m_mutex};
                        std::vector<std::pair<std::string, std::string>> nicknameColors = Lobby::get_random_nicknames(self->m_clientsIds.size() + 1, DEFAULT_NICKNAME_FILE);
                        
                        assert(nicknameColors.size() == self->m_clientsIds.size() + 1);

                        int32_t i{0};
                        for (auto & client : self->m_clientNicknames)
                        {
                            client.second = nicknameColors[i].first;
                            i++;
                        }
                        lock.unlock();
                        self->m_currentBotNickname = nicknameColors[i].first;

                        self->m_currentTopic = Lobby::read_topic_from_file(DEFAULT_TOPIC_FILE);

                        {
                            std::string nicknamesAuxStr;
                            for (auto const & p : nicknameColors)
                            {
                                nicknamesAuxStr += p.first + ", ";
                            }
                            nicknamesAuxStr.pop_back(); // Remove ' '
                            nicknamesAuxStr.pop_back(); // Remove ','
                            nicknamesAuxStr += ",NEW";  // Identifier
                            zmqpp::message message;
                            message << nicknamesAuxStr;
                            self->m_zmqSocket.send(message);
                            std::string doNotDelete;
                            self->m_zmqSocket.receive(doNotDelete);

                            zmqpp::message asd;
                            asd << self->m_currentTopic;
                            self->m_zmqSocket.send(asd);
                            self->m_zmqSocket.receive(doNotDelete);
                        }

                        json msg;
                        msg["msgType"] = static_cast<int32_t>(MsgType::NEW_ROUND);
                        msg["lobbyId"] = self->m_id;
                        msg["topic"] = Lobby::read_topic_from_file(DEFAULT_TOPIC_FILE);
                        msg["roundDurationSec"] = static_cast<int32_t>(ROUND_LENGTH_SECONDS);
                        msg["roundNum"] = self->m_currentRound;

                        msg["nicknameColors"] = json::object();
                        for (std::pair<std::string, std::string> const & nc : nicknameColors)
                        {
                            msg["nicknameColors"][nc.first] = nc.second;
                        }

                        self->send_to_all_clients(msg);

                        self->m_state = LobbyState::PLAYING;
                        self->m_lastStateTimepoint = system_clock::now();
                        self->m_msgWaitTimeout = self->m_lastStateTimepoint + seconds(ROUND_LENGTH_SECONDS);
                        if (self->m_msgWaitTimeout > (system_clock::now() + seconds(CHATBOT_MESSAGE_PERIOD_SECONDS)))
                        {
                            self->m_msgWaitTimeout = system_clock::now() + seconds(CHATBOT_MESSAGE_PERIOD_SECONDS);
                        }
                    } break;

                    case LobbyState::PLAYING:
                    {
                        bool startVote = std::chrono::duration_cast<seconds>(system_clock::now() - self->m_lastStateTimepoint).count() > ROUND_LENGTH_SECONDS;

                        if (startVote == false)
                        {
                            int64_t secondsUntillNextState      = ROUND_ENDED_LENGTH_SECONDS - std::chrono::duration_cast<seconds>(system_clock::now() - self->m_lastStateTimepoint).count();
                            int64_t secondsUntillNextChatbotMsg = CHATBOT_MESSAGE_PERIOD_SECONDS - std::chrono::duration_cast<seconds>(system_clock::now() - self->m_lastChatbotMessage).count();

                            if (secondsUntillNextState > secondsUntillNextChatbotMsg)
                            {
                                self->m_msgWaitTimeout = system_clock::now() + seconds(secondsUntillNextChatbotMsg);
                            }
                            else
                            {
                                self->m_msgWaitTimeout = system_clock::now() + seconds(secondsUntillNextState);
                            }

                            break;
                        }

                        json msg;
                        msg["msgType"] = static_cast<int32_t>(MsgType::GUESS_BOT_REQ);
                        msg["lobbyId"] = self->m_id;
                        msg["votingTimeSec"] = static_cast<int32_t>(VOTING_LENGTH_SECONDS - TIME_BUFFER);
                        msg["usersNicknames"] = json::object();
                        std::unique_lock lock{self->m_mutex};
                        for (std::pair<int32_t, std::string> const & clientNickname : self->m_clientNicknames)
                        {
                            msg["usersNicknames"][std::to_string(clientNickname.first)/*clientId*/] = clientNickname.second/*nickname*/;
                        }
                        msg["usersNicknames"]["-1"] = self->m_currentBotNickname;
                        lock.unlock();
                        self->send_to_all_clients(msg);

                        self->m_state = LobbyState::VOTING;
                        self->m_lastStateTimepoint = system_clock::now();
                        self->m_msgWaitTimeout = self->m_lastStateTimepoint + seconds(VOTING_LENGTH_SECONDS);
                    } break;

                    case LobbyState::VOTING:
                    {
                        bool voteTimeout = std::chrono::duration_cast<seconds>(system_clock::now() - self->m_lastStateTimepoint).count() > VOTING_LENGTH_SECONDS;

                        if (voteTimeout == false)
                        {
                            break;
                        }

                        json msg;
                        msg["msgType"] = static_cast<int32_t>(MsgType::ROUND_ENDED);
                        msg["lobbyId"] = self->m_id;
                        msg["scoreboard"] = json::object();
                        std::unique_lock lock{self->m_mutex};
                        for (std::pair<int32_t, int32_t> const & score : self->m_clientScores)
                        {
                            std::string username = ServerLogic::get_username_by_client_id(score.first);
                            msg["scoreboard"][username] = score.second;
                        }
                        lock.unlock();
                        msg["chatbotNickname"] = self->m_currentBotNickname;
                        self->send_to_all_clients(msg);

                        self->m_state = LobbyState::ROUND_ENDED;
                        self->m_lastStateTimepoint = system_clock::now();
                        self->m_msgWaitTimeout = self->m_lastStateTimepoint + seconds(ROUND_ENDED_LENGTH_SECONDS);
                    } break;

                    case LobbyState::ROUND_ENDED:
                    {
                        bool timeForNewRound = std::chrono::duration_cast<seconds>(system_clock::now() - self->m_lastStateTimepoint).count() > ROUND_ENDED_LENGTH_SECONDS;

                        if (timeForNewRound == false)
                        {
                            break;
                        }

                        if (self->m_currentRound >= self->m_roundsNumber)
                        {
                            json msg;
                            msg["msgType"] = static_cast<int32_t>(MsgType::GAME_OVER);
                            msg["lobbyId"] = self->m_id;
                            self->send_to_all_clients(msg);
                            self->requestThreadStop();
                        }

                        self->m_state = LobbyState::NEW_ROUND;
                        self->m_lastStateTimepoint = system_clock::now();
                        self->m_msgWaitTimeout = self->m_lastStateTimepoint + seconds(1); // This is here only because otherwise the try_acquire breaks
                    } break;

                    default:
                    {
                        // Should never happen
                    } break;
                }
            }

            std::cout << "Lobby (" << self->m_id << ") Getting out of the logic loop" << std::endl;

            zmqpp::message message;
            message << ";;;;;;;;";
            self->m_zmqSocket.send(message);

            self->close_lobby();

            self->m_isRunning = false;
        }
    );

    self->m_isRunning = true;
}

auto Lobby::isLobbyRunning() -> bool
{
    return m_isRunning;
}

auto Lobby::requestThreadStop() -> bool
{
    return m_thread.request_stop();
}

void Lobby::add_client_to_lobby(int32_t clientId)
{
    std::lock_guard lg{m_mutex};
    
    m_clientsIds.push_back(clientId);
    m_clientScores.insert({clientId, 0});
    m_clientNicknames.insert({clientId, ""});
}

auto Lobby::check_if_valid_client(int32_t clientId) -> bool
{
    return std::find(m_clientsIds.begin(), m_clientsIds.end(), clientId) != m_clientsIds.end();
}

void Lobby::send_to_all_clients(nlohmann::json const & msg, std::vector<int32_t> excludedIds)
{
    std::vector<int32_t> l_ids;

    std::unique_lock lock{m_mutex};
    for (int32_t id : m_clientsIds)
    {
        l_ids.push_back(id);
    }
    lock.unlock();

    uWS::App * l_server = &(m_server);

    for (int32_t id : l_ids)
    {
        if (std::find(excludedIds.begin(), excludedIds.end(), id) != excludedIds.end())
        {
            // If the id is in excludedIds -> do not send
            continue;
        }

        mp_serverLoop->defer(
            [l_server, id, msg]()
            {
                l_server->publish(
                    std::to_string(id),
                    msg.dump(),
                    uWS::OpCode::TEXT
                );
            }
        );
    }
}

auto Lobby::create_lobby_req_handler(Lobby * self, json const & data) -> json
{
    if (self->m_state != LobbyState::IN_LOBBY)
    {
        json msg;
        msg["msgType"] = static_cast<int32_t>(MsgType::ERROR);
        msg["errorCode"] = 0;
        msg["note"] = "create-loby-req when not in lobby!";
        return msg;
    }

    json resp;
    resp["msgType"] = static_cast<int32_t>(MsgType::CREATE_LOBBY_RESP);
    resp["lobbyId"] = self->m_id;

    self->add_client_to_lobby(data.value<int32_t>("clientId", -1));

    self->m_state = LobbyState::IN_LOBBY;

    return resp;
}

auto Lobby::join_lobby_req_handler(Lobby * self, nlohmann::json const & data) -> nlohmann::json
{
    json resp;

    if (data.value<int32_t>("lobbyId", -1) != self->m_id)
    {
        resp["msgType"] = static_cast<int32_t>(MsgType::ERROR);
        resp["errorCode"] = 0;
        resp["note"] = "bad lobby id";
        return resp;
    }

    if (self->m_clientsIds.size() >= self->m_maxUsers)
    {
        resp["msgType"] = static_cast<int32_t>(MsgType::ERROR);
        resp["errorCode"] = 0;
        resp["note"] = "too many users already in the lobby";
        return resp;
    }

    if (self->m_state != LobbyState::IN_LOBBY)
    {
        resp["msgType"] = static_cast<int32_t>(MsgType::ERROR);
        resp["errorCode"] = 0;
        resp["note"] = "The lobby started playing! And therefore does not accept new users";
        return resp;
    }

    self->add_client_to_lobby(data.value<int32_t>("clientId", -1));

    resp["msgType"] = static_cast<int32_t>(MsgType::JOIN_LOBBY_RESP);
    resp["lobbyId"] = self->m_id;
    
    resp["userList"] = json::array();
    std::unique_lock lock{self->m_mutex};
    for (int32_t id : self->m_clientsIds)
    {
        std::string username = ServerLogic::get_username_by_client_id(id);
        resp["userList"].push_back(username);
    }
    lock.unlock();

    resp["lobbyCreator"] = self->m_creatorUsername;
    resp["maxUsers"] = self->m_maxUsers;
    resp["roundsNumber"] = self->m_roundsNumber;

    return resp;
}

auto Lobby::post_new_chat_handler(Lobby * self, nlohmann::json const & data, bool chatbot) -> nlohmann::json
{
    bool isMsgInvalid = data.value<std::string>("chatMsg", "").length() == 0;
    if (isMsgInvalid)
    {
        json resp;
        resp["msgType"] = static_cast<int32_t>(MsgType::ERROR);
        resp["errorCode"] = 0;
        resp["note"] = "No chatMsg field in the json!";
        return resp;
    }

    json chatMsg;
    chatMsg["msgType"] = static_cast<int32_t>(MsgType::NEW_CHAT);
    chatMsg["lobbyId"] = self->m_id;
    chatMsg["chatMsg"] = data.value<std::string>("chatMsg", "");
    chatMsg["senderId"] = data.value<int32_t>("clientId", -1);
    if (chatbot)
    {
        chatMsg["senderUsername"] = "chatbot";
        chatMsg["senderNickname"] = self->m_currentBotNickname;
    }
    else
    {
        chatMsg["senderUsername"] = ServerLogic::get_username_by_client_id(data.value<int32_t>("clientId", -1));
        chatMsg["senderNickname"] = self->m_clientNicknames.at(data.value<int32_t>("clientId", -1));

        self->m_chatLogs += self->m_clientNicknames.at(data.value<int32_t>("clientId", -1)) 
                            + ": " 
                            + data.value<std::string>("chatMsg", "") 
                            + '\n';
    }

    self->send_to_all_clients(chatMsg);

    return json{};
}

auto Lobby::start_game_handler(Lobby * self, nlohmann::json const & data) -> nlohmann::json
{
    if (self->m_state != LobbyState::IN_LOBBY)
    {
        json msg;
        msg["msgType"] = static_cast<int32_t>(MsgType::ERROR);
        msg["errorCode"] = 0;
        msg["note"] = "start-game when not in lobby!";
        return msg;
    }

    json msg;
    msg["msgType"] = static_cast<int32_t>(MsgType::GAME_STARTED);
    msg["lobbyId"] = self->m_id;

    self->send_to_all_clients(msg);

    self->m_state = LobbyState::GAME_STARTING;
    self->m_lastStateTimepoint = std::chrono::system_clock::now();
    
    self->m_msgWaitTimeout = std::chrono::system_clock::now() + seconds(READY_STATE_CLIENT_WAIT_SECONDS);

    self->m_zmqSocket.connect(self->m_zmqEndpoint);
    m_isSocketConnected = true;
    self->m_lastChatbotMessage = std::chrono::system_clock::now();

    return json{};
}

auto Lobby::guess_bot_handler(Lobby * self, nlohmann::json const & data) -> nlohmann::json
{
    if (self->m_state != LobbyState::VOTING)
    {
        json msg;
        msg["msgType"] = static_cast<int32_t>(MsgType::ERROR);
        msg["errorCode"] = 0;
        msg["note"] = "Sent guess-bot-resp when the voting is not open!";
        return msg;
    }

    std::string nicknameGuess = data.value<std::string>("chatbotNickname", "");
    if (nicknameGuess.length() == 0)
    {
        json msg;
        msg["msgType"] = static_cast<int32_t>(MsgType::ERROR);
        msg["errorCode"] = 0;
        msg["note"] = "Sent guess-bot-resp with not chatbotNickname field!";
        return msg;
    }

    if (nicknameGuess == self->m_currentBotNickname)
    {
        int32_t clientId = data.value<int32_t>("clientId", -1);

        std::lock_guard lg{self->m_mutex};
        self->m_clientScores[clientId]++;
    }

    return json{};
}

auto Lobby::read_topic_from_file(std::filesystem::path const & path) -> std::string
{
    bool fileExists = std::filesystem::is_regular_file(path);

    if (fileExists == false)
    {
        return "Did not find file with topics!";
    }

    std::ifstream file(path);
    if (file.is_open() == false) {
        return "Failed to open the file";
    }

    size_t lineCount{0};
    std::string line;
    while (std::getline(file, line)) {
        lineCount++;
    }

    if (lineCount == 0) 
    {
        return "File has 0 lines!";
    }

    file.clear();
    file.seekg(0);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    size_t randomLineNumber = std::rand() % lineCount;

    size_t currentLine = 0;
    while (currentLine < randomLineNumber && std::getline(file, line)) {
        currentLine++;
    }

    return line;
}

auto Lobby::get_random_nicknames(int32_t count, std::filesystem::path const & path) -> std::vector<std::pair<std::string, std::string>>
{
    std::vector<std::string> nicknames;
    std::vector<std::pair<std::string, std::string>> nicknameColor;

    bool fileExists = std::filesystem::is_regular_file(path);

    if (fileExists == false)
    {
        for (int32_t i{0}; i < count; i++)
        {
            nicknameColor.emplace_back(std::pair<std::string, std::string>{"NicknamesNotFound", "#000000"});
        }
        return nicknameColor;
    }

    std::ifstream file(path);
    if (file.is_open() == false) {
        for (int32_t i{0}; i < count; i++)
        {
            nicknameColor.emplace_back(std::pair<std::string, std::string>{"FailedToOpenFile", "#000000"});
        }
        return nicknameColor;
    }

    size_t lineCount{0};
    std::string line;
    while (std::getline(file, line)) {
        lineCount++;
    }

    if (lineCount == 0) 
    {
        for (int32_t i{0}; i < count; i++)
        {
            nicknameColor.emplace_back(std::pair<std::string, std::string>{"NoNicknamesInFile", "#000000"});
        }
        return nicknameColor;
    }

    if (lineCount < count)
    {
        for (int32_t i{0}; i < count; i++)
        {
            nicknameColor.emplace_back(std::pair<std::string, std::string>{"TooFewNicknamesInFile", "#000000"});
        }
        return nicknameColor;
    }

    file.clear();
    file.seekg(0);

    int32_t randomLineNumber{0};
    std::vector<int32_t> uniqueLineNumbers;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    for (int32_t i{0}; i < count; i++)
    {
        do
        {
            randomLineNumber = std::rand() % lineCount;
            if (randomLineNumber == 0) continue;
        } while (std::find(uniqueLineNumbers.begin(), uniqueLineNumbers.end(), randomLineNumber) != uniqueLineNumbers.end());

        uniqueLineNumbers.push_back(randomLineNumber);        

        size_t currentLine = 0;
        while (std::getline(file, line)) {
            currentLine++;
            if (currentLine == randomLineNumber)
            {
                break;
            }
        }
        
        nicknames.push_back(line);

        file.clear();
        file.seekg(0);
    }

    for (std::string & nicknameLine : nicknames)
    {
        std::size_t commaPos = nicknameLine.find(',');

        nicknameColor.emplace_back(std::pair<std::string, std::string>{nicknameLine.substr(0, commaPos), nicknameLine.substr(commaPos + 1)});
    }

    return nicknameColor;
}

void Lobby::close_lobby()
{
    json msg;
    msg["msgType"] = static_cast<int32_t>(MsgType::LOBBY_SHUTDOWN);
    msg["lobbyId"] = m_id;

    send_to_all_clients(msg);
}

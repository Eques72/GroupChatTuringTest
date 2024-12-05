#include "Lobby.hpp"

using json = nlohmann::json;

Lobby::Lobby(uWS::App & server, uWS::Loop * p_loop, int32_t lobbyId, std::string const & creatorUsername, int32_t maxUsers, int32_t roundsNumber)
    :m_server{server},
     mp_serverLoop{p_loop},
     m_id{lobbyId},
     m_creatorUsername{creatorUsername}, 
     m_maxUsers{maxUsers}, 
     m_roundsNumber{roundsNumber},
     m_msgsSmph{0}
{
    m_clientsIds.reserve(m_maxUsers);
}

Lobby::~Lobby()
{
    // TODO This! Make sure to send some game-ended or something similar to the websockets?
    // I can't close them here, but wtf am I supposed to do?
}

void Lobby::pass_msg(json && data)
{
    std::lock_guard lg{m_mutex};

    if (m_msgs.size() >= MAX_MSGS_QUEUE_LEN)
    {
        // TODO Send back an error?
    }

    m_msgs.push(std::move(data));
    m_msgsSmph.release();
}

void Lobby::startLobbyThread()
{
    Lobby * self{this};

    m_thread = std::jthread(
        [self](std::stop_token l_stopToken)
        {
            uWS::App * l_server = &(self->m_server);

            std::unique_lock l_lock{self->m_mutex, std::defer_lock};

            json l_msg;
            json l_resp;
            std::string l_clientId;

            while (l_stopToken.stop_requested() == false)
            {
                self->m_msgsSmph.acquire();
                l_lock.lock();
                l_msg = self->m_msgs.front();
                self->m_msgs.pop();
                l_lock.unlock();

                l_clientId = std::to_string(l_msg.value<int32_t>("clientId", -1));
                if (l_clientId.length() == 0 || l_msg.empty())
                {
                    continue;
                }

                switch (l_msg.value<int32_t>("msgType", 0))
                {
                    case -1: // error
                    {
                        // TODO
                    } break;

                    case 3: // create-lobby-req
                    {
                        l_resp = self->create_lobby_req_handler(self, l_msg);
                    } break;

                    case 0: // bad msgType
                    case 1:
                    case 2:
                    default:
                    {
                        l_resp["msgType"] = -1;
                        l_resp["errorCode"] = 0;
                        l_resp["note"] = "bad msgType";
                    } break;
                }

                if (l_resp.empty())
                {
                    continue;
                }

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

                // TODO Logic 
            }
        }
    );
}

auto Lobby::requestThreadStop() -> bool
{
    return m_thread.request_stop();
}

void Lobby::add_client_to_lobby(int32_t clientId)
{
    std::lock_guard lg{m_mutex};
    
    m_clientsIds.push_back(clientId);
}

auto Lobby::create_lobby_req_handler(Lobby * self, json const & data) -> json
{
    json resp;
    resp["msgType"] = 4;
    resp["lobbyId"] = self->m_id;

    return resp;
}

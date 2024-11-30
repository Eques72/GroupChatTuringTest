#include "Lobby.hpp"

using json = nlohmann::json;

Lobby::Lobby(uWS::App & server, uWS::Loop * p_loop, std::string const & creatorUsername, int32_t maxUsers, int32_t roundsNumber)
    :m_server{server},
     mp_loop{p_loop},
     m_creatorUsername{creatorUsername}, 
     m_maxUsers{maxUsers}, 
     m_roundsNumber{roundsNumber},
     m_msgsSmph{0}
{
    m_clientsIds.reserve(m_maxUsers);
}


void Lobby::add_client_to_lobby(int32_t clientId)
{
    std::lock_guard lg{m_mutex};
    
    m_clientsIds.push_back(clientId);
}

void Lobby::pass_msg(json && data)
{
    std::lock_guard lg{m_mutex};

    m_msgs.push(std::move(data));
    m_msgsSmph.release();
}

void Lobby::startLobbyThread()
{
    // TODO
}

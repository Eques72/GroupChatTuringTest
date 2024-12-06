#pragma once

enum MsgType
{
    ERROR                    = -1,
    UNDEFINED                = 0,
    CLIENT_REGISTRATION_REQ  = 1,
    CLIENT_REGISTRATION_RESP = 2,
    CREATE_LOBBY_REQ         = 3,
    CREATE_LOBBY_RESP        = 4,
    JOIN_LOBBY_REQ           = 5,
    JOIN_LOBBY_RESP          = 6,
    USER_JOINED              = 7,
    GAME_STARTED             = 8,
    NEW_ROUND                = 9,
    POST_NEW_CHAT            = 10,
    NEW_CHAT                 = 11,
    GUESS_BOT_REQ            = 12,
    GUESS_BOT_RESP           = 13,
    ROUND_ENDED              = 14,
    GAME_OVER                = 15,
    START_GAME               = 16,
    USER_LEFT                = 17,
    LOBBY_SHUTDOWN           = 18,
};

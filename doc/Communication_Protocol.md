Glossary:
- "server" - the server software / backend logic
- "client" - the Android app

# Server - launch
Nothing special, inits all resources and waits for connections, lobbies, etc.

# Client - launch
1. At launch the client does not immediately connect to the server (but may PING it to make sure that is is reachable?)
2. The proper connection to the server is tried to be established when the user either: tries to create a new lobby OR tries to connect to an existing lobby

# Communication Protocol - overview
The proposed-by-me communication protocol between the clients and the server is designed to be easy to understand and simple to implement (and debug). That's why I propose the following:
- The whole of messaging / data exchange be implemented using websockets since it's a simple event-driven architecture, so no need for 3rd party frameworks or creating a whole raw TCP-based server logic from scratch.
- Each message / request would be in the JSON text format, becuase it would be easy to implement and debug (since it's text based) and to parse the message into program data (since there's no shortage of JSON libraries for each language).
- Each message would specify what kind of message it is, it would have some kind of payload depending on the message type, and the program (both the server's and the client's) would act accordingly depending on the received message.<br>

### Examples
- "error" messages would be for indicating that the last message / request failed
- Messages identified by the type "create-lobby-req" would indicate that the client wants to create new game lobby
- When the client receives a message identified by the type "chat-message" it would add the payload of the message to the UI (chat box)
- and so on...

# Communication Protocol - message types
The common (and absolutely required) field for each message regardless of the message type would be the "msgType" field, that would indicate what sort of message it is (needed for approperiate parsing and processing of the message).
| Field name | Type |
| ---------- | ---- | 
| msgType | int32 |

### Message types table
| Message type | id |
| ------------ | -- | 
| error | -1 |
| client-registration | 1 |
| client-registration | 2 |
| create-lobby-req | 3 |
| create-lobby-resp | 4 |
| join-lobby-req | 5 |
| join-lobby-resp | 6 |
| user-joined | 7 |
| game-started | 8 |
| new-round | 9 |
| post-new-chat | 10 |
| new-chat | 11 |
| round-ended | 12 |
| impostor-vote | 13 |
| user-voted-out | 14 |
| confidence-poll-req | 15 |
| confidence-poll-resp | 16 |
| game-over | 17 |

# Communication Protocol - reporting an error
Whenever the server or the client try to act according to the received message / request type but something goes wrong (like the received data contained bad data, or insufficent data, or something similar) then they should sent the "error" message, that should implement the following schema:

### Communication Protocol - "error" message schema
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| errorCode | int32 | Yes |
| note | string | No |

```
// Example error message JSON
{
    "msgType": "-1",
    "errorCode": "1",
    "note": "This is the optional note regarding the error message. Can be used for additional info when debugging or something"
}
```
### List of error codes

> Na razie "sunny day scenario", pozniej moze uzupelnic liste bledow jak bedziemy to implementowac

| error code | Description |
| ---------- | ----------- |
| 0 | RESERVED (Generic error when no other error code fits) |
| 1 | The client tried to request something without registering themselves first |
| 2 | The created lobby would have too little players (maxUsers) |
| TODO | TODO |

# To be determined
We can either use numeric values to represent message type and other things OR string values?<br>
I don't really care which it is<br>

```
// Example with numeric values
{
    "msgType": "-1",
    "errorCode": "1",
    "note": "This is the optional note regarding the error message. Can be used for additional info when debugging or something"
}
```
```
// Example with string values
{
    "msgType": "error",
    "errorCode": "user-not-registered", // or something similar
    "note": "This is the optional note regarding the error message. Can be used for additional info when debugging or something"
}
```

# Communication Protocol - establishing the conncetion
The connection to the server should be established like a regular Websocket connection. IP address obviously depends on the enviroment in which the server is set up.<br>
Addressing info:
| 1 | Port | 12345 |
| - | --- | --- |
| 2 | IP addr | Depends on the enviroment, since we are not renting out a public server |

Upon establishing a connection to the server the first message the client should send is the "client-registration" message (message code 1).
> If the client makes a request / sends a message without registering themselves first: the server responds with "error" message (error code 1)

The "client-registration" message should implement the following schema:
### Communication Protocol - "client-registration" message schema
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| username | string | Yes |
| note | string | No |

```
// Example client-registration message JSON
{
    "msgType": 1,
    "username": "Jane Doe",
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```

## "registration-successful" response
After a successful registration the server will respond with "registration-successful" message, in which the server will include all the identifying data of the user, and will add the client's new generated id that the client needs to use when making subsequent calls / messages.
<br>
<br>
The client-registration should not have any specific reasons to return an error.

### Communication Protocol - "registration-successful" response schema
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| clientId | int32 | Yes |
| username | string | Yes |
| note | string | No |

```
// Example registration-successful message JSON
{
    "msgType": "2",
    "clientId": "13",
    "username": "Jane Doe",
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```

# Communication Protocol - creating / joining a game lobby
In similar fashion the client will send a "create-lobby-req" or "join-lobby-req" messsage for creating or joining a lobby (game).

## Creating a lobby - "create-lobby-req" message
To create a new lobby (game) the client should send a message that implements the following schema:
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| clientId | int32 | Yes |
| username | string | Yes |
| maxUsers | int32 | Yes |
| note | string | No |

```
// Example create-lobby-req message JSON
{
    "msgType": 3,
    "clientId": 13,
    "username": "Jane Doe",
    "maxUsers": 5,
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```
> For now we can just hard-code all of the settings (like time per round or something)

Possbile reasons for failing to create a lobby
| error code | Description |
| ---------- | ----------- |
| 2 | The created lobby would have too little players (maxUsers) |

## "create-lobby-resp" response message
If the lobby is successfully created the server will respond with a message that implements the following schema:
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| lobbyId | int32 | Yes |
| note | string | No |

```
// Example create-lobby-resp message JSON
{
    "msgType": 4,
    "lobbyId": 183,
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```

## Joining an exisiting lobby - "join-lobby-req" message
> ASSUMPTION: To make our lives easier let's assume that joining a lobby is only available when the game has not started yet

In order for the user to join an existing lobby the client needs to send a message that implements the following schema:
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| clientId | int32 | Yes |
| lobbyId | int32 | Yes |
| note | string | No |

```
// Example join-lobby-req message JSON
{
    "msgType": 5,
    "clientId": 55,
    "lobbyId": 183,
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```

## "join-lobby-resp" response
If the user was added to the requested lobby, the server will respond with a message that implements the following schema:
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| lobbyId | int32 | Yes |
| userList | array (string) | Yes |
| lobbyCreator | string | Yes |
| note | string | No |

```
// Example join-lobby-resp message JSON
{
    "msgType": 6,
    "lobbyId": 183,
    "userList": [
        "Adrian",
        "Jane Doe",
        "User1",
        "Żaba",
    ],
    "lobby-creator": "Jane Doe",
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```
> The userList array field will include the username of the client / user that made the request (sent a join-lobby-req message)

# Communication protocol - mid-game messages

## "user-joined" message
Callback message that will be sent out to all clients associated with a specific lobby when a new user joins the game (needed for example for updating the "waiting lobby UI")

### "user-joined" schema
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| lobbyId | int32 | Yes |
| newUser | string | Yes |
| note | string | No |

```
// Example user-joined message JSON
{
    "msgType": 7,
    "lobbyId": 183,
    "newUser": "John Doe",
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```

> TODO Maybe also add "user-left" messages?

## "game-started" message
Message sent out to all clients associated with a specific lobby when the lobby will start the game.

### "game-started" schema
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| lobbyId | int32 | Yes |
| note | string | No |

```
// Example game-started message JSON
{
    "msgType": 8,
    "lobbyId": 183,
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```

## "new-round" message
Message sent out to all clients associated with a specific lobby when a new round starts

### "new-round" schema
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| lobbyId | int32 | Yes |
| topic | string | Yes |
| roundDurationSec | int32 | Yes |
| note | string | No |

```
// Example game-started message JSON
{
    "msgType": 9,
    "lobbyId": 183,
    "topic": "What do you like about being alive and definetly not about being a chatbot",
    "roundDurationSec": 180,
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```

## "post-new-chat" message
Message sent out by a client to the server in order to "post a new chat message". If the message is accepted, the server will send out a "new-chat" message to all of the lobby's clients (the sender included)

### "post-new-chat" schema
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| clientId | int32 | Yes |
| lobbyId | int32 | Yes |
| chatMsg | string | Yes |
| note | string | No |

```
// Example post-new-chat message JSON
{
    "msgType": 10,
    "clientId": 13,
    "lobbyId": 183,
    "chatMsg": "Hi guys! This is my first ever game and my fisrt ever chat message!",
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```

## "new-chat" message
Callback message sent out by the server to all of the lobby's clients (original message sender included) that informs a new chat message has been sent out.

### "new-chat" schema
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| lobbyId | int32 | Yes |
| chatMsg | string | Yes |
| note | string | No |

```
// Example new-chat message JSON
{
    "msgType": 11,
    "lobbyId": 183,
    "chatMsg": "Hi guys! This is my first ever game and my fisrt ever chat message!",
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```

## "round-ended" message
Message sent out to all clients associated with a specific lobby when a round ends

### "round-ended" schema
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| lobbyId | int32 | Yes |
| note | string | No |

```
// Example round-ended message JSON
{
    "msgType": 12,
    "lobbyId": 183,
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```

## "impostor-vote" message
Message sent by the client upon receiving the "round-ended" message AND after the user casts their vote about who the chatbot is

### "impostor-vote" schema
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| lobbyId | int32 | Yes |
| impostor | string | Yes |
| note | string | No |

```
// Example impostor-vote message JSON
{
    "msgType": 13,
    "lobbyId": 183,
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```

## "user-voted-out" message
Message sent out to all clients associated with a specific lobby when a user has been voted out

### "user-voted-out" schema
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| lobbyId | int32 | Yes |
| userVotedOut | string | Yes |
| note | string | No |

```
// Example user-voted-out message JSON
{
    "msgType": 14,
    "lobbyId": 183,
    "userVotedOut": "User1",
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```

## "confidence-poll-req" message
After a user is voted out by the game participants, the server then will send out this message (confidence-poll-req) to request the user (player) to make a choice if the game should continue (the player is confident that the chatbot has been kicked out already) or whether the game should continue (the player thinks  that the chatbot may still be in the game).
> Upon receiving this message the clients should respond with "confidence-poll-resp" message

### "confidence-poll-req" schema
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| lobbyId | int32 | Yes |
| note | string | No |

```
// Example confidence-poll-req message JSON
{
    "msgType": 15,
    "lobbyId": 183,
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```

## "confidence-poll-resp" message
Response message to the "confidence-poll-req" message. The client should respond with info whether the user (player) is confident that the chatbot has been kicked out already

### "confidence-poll-resp" schema
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| lobbyId | int32 | Yes |
| continueTheGame | bool | Yes |
| note | string | No |

```
// Example confidence-poll-resp message JSON
{
    "msgType": 16,
    "lobbyId": 183,
    "continueTheGame": false,
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```

## "game-over" message
Game over message. Message sent out to all clients associated with a specific lobby when that lobby's game has ended.

### "game-over" schema
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| lobbyId | int32 | Yes |
| wasChatbotVotedOut | bool | Yes |
| chatbotUsername | string | Yes |
| note | string | No |

```
// Example game-over message JSON
{
    "msgType": 17,
    "lobbyId": 183,
    "wasChatbotVotedOut": true,
    "chatbotUsername": "user1",
    "note": "This is the optional note, not needed for the communication protocol. Can be used for additional info when debugging or something"
}
```

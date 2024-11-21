Glossary:
- "server" - the server software / backend logic
- "client" - the Android app

# Server - launch
Nothing special, init's all resources and waits for connections, lobbies, etc.

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
- Messages identified by the type "create-lobby" would indicate that the client wants to create new game lobby
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

# Communication Protocol - reporting an error
Whenever the server or the client try to act according to the received message / request type but something goes wrong (like the received data contained bad data, or insufficent data, or something similar) then they should sent the "error" message, that follows the following schema:

### Communication Protocol - "error" schema
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
| error code | Description |
| ---------- | ----------- |
| 0 | RESERVED |
| 1 | The client tried to request something without registering themselves first |
| TODO | TODO |

# Communication Protocol - establishing the conncetion
The connection to the server should be established like a regular Websocket connection. IP address obviously depends on the enviroment in which the server is set up.<br>
Addressing info:
| Port | 12345 |
| --- | --- |
| IP addr | Depends on the enviroment, since we are not renting out a public server |

Upon establishing a connection to the server the first message the client should send is the "client-registration" message (message code 1).
> If the client makes a request / sends a message without registering themselves first: the server responds with "error" message (error code 1)

### Communication Protocol - "client-registration" schema
| Field name | Type | Required |
| ---------- | ---- | -------- |
| msgType | int32 | Yes |
| TODO | TODO | TODO |
| note | string | No |

```
// Example client-registration message JSON
{
    "msgType": "1",
    // TODO More
    "note": "This is the optional note. Can be used for additional info when debugging or something"
}
```

# Communication Protocol - creating a lobby
TODO

# Communication protocol - joining a lobby
TODO 

# Communication protocol - in-game communications
TODO

# IDK if we need anything more?

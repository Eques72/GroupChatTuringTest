Glossary:
- "server" - the server software / backend logic
- "client" - the Android app

# Server - launch
Nothing special, init's all resources and waits for connections, lobbies, etc.

# Client - launch
1. At launch the client does not immediately connect to the server (but may PING it to make sure that is is reachable?)
2. The proper connection to the server is tried to be established when the user either: tries to create a new lobby OR tries to connect to an existing lobby

# Communication Protocol - establishing the conncetion
TODO Something like: first message is always "hi, I'm so and so and this are my details"
TODO If the client makes a request / sends a message without registering themselves -> server responds with "register yourself first!"
TODO Add the json schema here

# Communication Protocol - creating a lobby
TODO

# Communication protocol - joining a lobby
TODO 

# Communication protocol - in-game communications
TODO

# IDK if we need anything more?

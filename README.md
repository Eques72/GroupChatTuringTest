# The server is set to run on port: 12345 (it is hardcoded for now)

# Game simulation
> You can find the javascript scripts in the "test" directory

1. fire up the server
2. launch the lobby_creator.js script in a console on the same pc as the server
<br>2.1 Copy the lobbyId from the message received from the server
3. launch the lobby_user.js script in a *different* console on the same pc as the server
<br>3.1 paste the lobbyId code when asked by the console and hit Enter
4. Repeat the step 3. for however many users you want to simulate
5. Watch the game go on

# Build and run with docker
docker compose up --build

# Run local build
bash buildDebug.sh

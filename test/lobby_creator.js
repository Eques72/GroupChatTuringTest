function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

async function main() {
    let lastRcvdMsg = {};
    let anotherUserJoined = false;
    let logSentMsgs = true;

    const socket = new WebSocket("ws://localhost:12345");
    socket.addEventListener('open', () => {
        console.log('WebSocket connection established.');
    });
    socket.addEventListener('message', (event) => {
        lastRcvdMsg = JSON.parse(event.data);
        console.log("Recevied:\n", lastRcvdMsg);

        if (lastRcvdMsg['msgType'] == 7) {
            anotherUserJoined = true;
        }
    });
    socket.addEventListener('close', () => {
        console.log('WebSocket connection closed.');
    });
    socket.addEventListener('error', (error) => {
        console.error('WebSocket error:', error);
    });

    process.on('SIGINT', () => {
        console.log("\nSIGINT received. Closing the connection");
        socket.close();
        process.exit(0);
    });

    await sleep(500);

    // Register
    let msg = {
        msgType: 1,
        username: "User - Creator"
    };
    socket.send(JSON.stringify(msg));
    if (logSentMsgs) {
        console.log('Sent:\n', JSON.stringify(msg));
    }
    await sleep(500);
    const myClientId = lastRcvdMsg['clientId'];

    // Create a lobby
    msg = {
        msgType: 3,
        clientId: myClientId,
        username: lastRcvdMsg['username'],
        maxUsers: 5,
        roundsNumber: 5,
    };
    socket.send(JSON.stringify(msg));
    if (logSentMsgs) {
        console.log('Sent:\n', JSON.stringify(msg));
    }
    await sleep(500);
    const myLobbyId = lastRcvdMsg['lobbyId'];

    // Wait untill another user joins the lobby
    while (anotherUserJoined == false) {
        await sleep(100);
    }

    // Send a message
    msg = {
        msgType: 10,
        clientId: myClientId,
        lobbyId: myLobbyId,
        chatMsg: 'Hi new user! I\'ll start the game in few seconds!'
    };
    socket.send(JSON.stringify(msg));
    if (logSentMsgs) {
        console.log('Sent:\n', JSON.stringify(msg));
    }
    await sleep(2000);

    // Start the game
    msg = {
        msgType: 16,
        clientId: myClientId,
        lobbyId: myLobbyId,
    };
    socket.send(JSON.stringify(msg));
    if (logSentMsgs) {
        console.log('Sent:\n', JSON.stringify(msg));
    }
    await sleep(500);
}

main();

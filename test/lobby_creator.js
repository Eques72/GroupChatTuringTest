function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

async function main() {
    let lastRcvdMsg = {};
    let anotherUserJoined = false;
    let logSentMsgs = true;
    let myClientId = 0;
    let myLobbyId = 0;

    const socket = new WebSocket("ws://localhost:12345");
    socket.addEventListener('open', () => {
        console.log('WebSocket connection established.');
    });
    socket.addEventListener('message', (event) => {
        lastRcvdMsg = JSON.parse(event.data);
        console.log("\nRecevied:\n", lastRcvdMsg, '\n');

        if (lastRcvdMsg['msgType'] == 7) {
            anotherUserJoined = true;
        }
        
        switch (lastRcvdMsg['msgType']) {
            case 12: {
                msg = {
                    msgType: 13,
                    clientId: myClientId,
                    lobbyId: myLobbyId,
                    chatbotNickname: 'chatbot'
                };
                socket.send(JSON.stringify(msg));
            } break;
        
            case 15:
            case 18: {
                console.log("Game ended");
                throw "Game ended";
            } break;
        }
    });
    socket.addEventListener('close', () => {
        console.log('WebSocket connection closed.');
        throw "Websocket connection closed!";
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
        username: "Creator"
    };
    socket.send(JSON.stringify(msg));
    if (logSentMsgs) {
        console.log('Sent:\n', JSON.stringify(msg));
    }
    await sleep(500);
    myClientId = lastRcvdMsg['clientId'];

    // Create a lobby
    msg = {
        msgType: 3,
        clientId: myClientId,
        username: lastRcvdMsg['username'],
        maxUsers: 5,
        roundsNumber: 2,
    };
    socket.send(JSON.stringify(msg));
    if (logSentMsgs) {
        console.log('Sent:\n', JSON.stringify(msg));
    }
    await sleep(500);
    myLobbyId = lastRcvdMsg['lobbyId'];

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

    const intervalId = setInterval(() => {
        msg = {
            msgType: 10,
            clientId: myClientId,
            lobbyId: myLobbyId,
            chatMsg: 'I don\'t know what\'s going!'
        };
        socket.send(JSON.stringify(msg));
        if (logSentMsgs) {
            console.log('Sent:\n', JSON.stringify(msg));
        }    
    }, 4000);
}

main();

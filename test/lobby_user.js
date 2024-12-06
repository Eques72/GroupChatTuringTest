const { read } = require('fs');

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

function read_lobby_id() {
    const readline = require('readline');

    return new Promise((resolve, reject) => {
        const rl = readline.createInterface({
            input: process.stdin,
            output: process.stdout
        });

        rl.question('Enter the lobby id: ', (input) => {
            const number = parseInt(input, 10); // Parse input as an integer
            if (!isNaN(number)) {
                console.log("You entered:", number);
                resolve(number); // Resolve the promise with the number
            } else {
                console.log("That's not a valid number.");
                reject(new Error('Failed to parse ' + input)); // Reject the promise with an error
            }
            rl.close();
        });
    });
}

async function main() {
    let lastRcvdMsg = {};
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
        username: "User - User"
    };
    socket.send(JSON.stringify(msg));
    if (logSentMsgs) {
        console.log('Sent:\n', JSON.stringify(msg));
    }
    await sleep(500);
    const myClientId = lastRcvdMsg['clientId'];

    let myLobbyId = await read_lobby_id();

    // Join a lobby
    msg = {
        msgType: 5,
        clientId: myClientId,
        lobbyId: myLobbyId
    };
    socket.send(JSON.stringify(msg));
    if (logSentMsgs) {
        console.log('Sent:\n', JSON.stringify(msg));
    }
    await sleep(500);

    // Send a message
    msg = {
        msgType: 10,
        clientId: myClientId,
        lobbyId: myLobbyId,
        chatMsg: 'LOBBY USER sends greetings!'
    };
    socket.send(JSON.stringify(msg));
    if (logSentMsgs) {
        console.log('Sent:\n', JSON.stringify(msg));
    }
    // await sleep(500);

    // 
}

main();

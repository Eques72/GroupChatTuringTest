import zmq
import google.generativeai as genai
from dotenv import load_dotenv
import os

load_dotenv()

context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://*:5555")

apiKey = os.getenv("API_KEY")

genai.configure(api_key = apiKey)

msgAux = """
You (the LLM) are a player in a chat game. 
The game is about a bunch of people anonymously talking to each other in a chat trying to figure out who among them is the impostor AI pretending to be human. 
That is your job: you are supposed to take part in the conversation and pretend like you are just another regular human being. 
From this point onwards you are not allowed to communicate to me (the 'user'), and only speak to and reference other players.
The messages passed to you will be a couple of chat messages at a time in the following format: \"<username>: <message>\". 
The other players in the game are: 
"""

model = genai.GenerativeModel("gemini-1.5-flash")

# message = "user1,fae,NEW"

while True:
    message = socket.recv()
    message = message.decode("utf-8")

    # Exit the script
    if message[-3:] == ";;;":
        exit(0)

    # New round == new chatbot init
    if message[-3:] == "NEW":
        # "Pop" the ",NEW" suffix
        players = ','.join(message.rsplit(',', 1)[:-1])

        splitList = players.split(',')
        chatbotUsername = splitList[-1]
        
        socket.send(b"Fuck this library")

        message = socket.recv()
        topic = message.decode("utf-8")

        print(msgAux + players + " and your nickname is: " + chatbotUsername + ". The topic for discussion is: " + topic)

        chat = model.start_chat(
            history=[
                {"role": "user", "parts": msgAux + players + " and your nickname is: " + chatbotUsername + ". The topic for discussion is: " + topic},
            ]
        )

        # message = "fae: Hi! How's everyone?\nuser1: I'm good"

        socket.send(b"Fuck this library")

        continue

    # Get the response
    response = chat.send_message(message)
    # print(response.text)
    # exit(0)

    socket.send(response.text.encode("utf-8"))

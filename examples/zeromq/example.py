import zmq

context = zmq.Context()
from pathlib import Path

file_path = Path(__file__).with_name('adaydreamjourney.txt')
file_contents = file_path.read_text(encoding='utf-8')

#  Socket to talk to server
try: 
    print("Connecting to hello world server…")
    socket = context.socket(zmq.REQ)
    socket.connect("tcp://127.0.0.1:57411")

    s = {
        "action": "ADD_OBJECTS",
        "objects": file_contents
    }

    socket.send_json(s)
    print("listening")
    s = socket.recv()
    print(s)

except Exception as e:
    print("something wrong")
    print(e)
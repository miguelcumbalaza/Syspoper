import socket
import threading

def send( command ):
    print(command)
    command = list(map(str, command.split()))
    if(command[0] == "create"):
        sock = socket.create_connection(('localhost', 15600))
        print("Socked opened")
        try:
            petition = ("create " + command[1] + " " + command[2]).encode()
            sock.sendall(petition)
        finally:
            print('Done!')
            sock.close()
    elif(command[0] == "stop"):
        sock = socket.create_connection(('localhost', 15600))
        print("Socked opened")
        try:
            petition = ("stop " + command[1]).encode()
            sock.sendall(petition)
        finally:
            print('Done!')
            sock.close()
    elif(command[0] == "delete"):
        sock = socket.create_connection(('localhost', 15600))
        print("Socked opened")
        try:
            petition = ("delete " + command[1]).encode()
            sock.sendall(petition)
        finally:
            print('Done!')
            sock.close()
    elif(command[0] == "list"):
        sock = socket.create_connection(('localhost', 15600))
        print("Socked opened")
        try:
            petition = "list".encode()
            sock.sendall(petition)
        finally:
            print('Done!')
            sock.close()
    else:
        print("Comando invalido.")

threads = []
for i in range(10):
    t = threading.Thread(target=send, args=("delete cont" + str(i + 1),))
    threads.append(t)
    t.start()


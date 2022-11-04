import socket
import sys

def main():
    done = False
    while(not done):
        command = list(map(str, sys.stdin.readline().split()))
        if(command[0] == "create"):
            sock = socket.create_connection(('localhost', 15600))
            print("Socked opened")
            try:
                petition = ("create " + command[1] + " " + command[2]).encode()
                sock.sendall(petition)
            finally:
                print('Done!')
                sock.close()
            done = True
        elif(command[0] == "stop"):
            sock = socket.create_connection(('localhost', 15600))
            print("Socked opened")
            try:
                petition = ("stop " + command[1]).encode()
                sock.sendall(petition)
            finally:
                print('Done!')
                sock.close()
            done = True
        elif(command[0] == "delete"):
            sock = socket.create_connection(('localhost', 15600))
            print("Socked opened")
            try:
                petition = ("delete " + command[1]).encode()
                sock.sendall(petition)
            finally:
                print('Done!')
                sock.close()
            done = True
        elif(command[0] == "list"):
            sock = socket.create_connection(('localhost', 15600))
            print("Socked opened")
            try:
                petition = "list".encode()
                sock.sendall(petition)
            finally:
                print('Done!')
                sock.close()
            done = True
        else:
            print("Comando invalido.")

main()


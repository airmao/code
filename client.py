#!/usr/bin/python
import socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_name = 'localhost'
port_number = 30113
client_socket.connect((server_name,port_number))
data = client_socket.recv(1024)
print data

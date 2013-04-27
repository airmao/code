#!/usr/bin/python
import socket
my_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
hostname = 'localhost'
port_number = 30113
my_socket.bind((hostname,port_number))
my_socket.listen(5)
while True:
    connection, address = my_socket.accept()
    connection.send("you connnet")

from socklib import *
server = server_socket(AF_INET,SOCK_STREAM,false)
server.Start(backlog=5,port=1337)
client = server.Accept()
client.Sendall('whats up')
client.Close()
server.Close()


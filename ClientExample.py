from socklib import *
client = client_socket(AF_INET,SOCK_STREAM,false)
host = gethostbyname('localhost')
client.Connect(host,1337)
reply = client.Receive(100)
print(reply.value)
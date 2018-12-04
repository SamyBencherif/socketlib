from socklib import *
host = gethostbyname("andysserver.xyz")
fd = client_socket(AF_INET,SOCK_STREAM)
client_connect(fd,AF_INET,host,1337)
print receive(fd,1024)
print send_buff(fd,"00000000003hey",len("00000000003hey"))
stuff = raw_input(">:")
print receive(fd,1024)
close_socket(fd);


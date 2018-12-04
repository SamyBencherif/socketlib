from ctypes import *
cdll.LoadLibrary('./libsock.so')
libc = CDLL('./libsock.so')
class client_sock(Structure):
	_fields_ = [("fd",c_int),("ssl",c_int)
	,("receive",CFUNCTYPE(c_ssize_t,c_int,c_char_p,c_size_t)),
	("sendall",CFUNCTYPE(c_ssize_t,c_char_p,c_size_t)),
	("send_msg",CFUNCTYPE(c_ssize_t,c_int,c_char_p,c_size_t)),
	("recv_msg",CFUNCTYPE(c_ssize_t,c_int,c_char_p,c_size_t)),
	("close",CFUNCTYPE(c_int,c_int))]
	def close_sock(self):
		return True if self.close(self.fd) == 0 else False
	def isSSL(self):
		return True if self.ssl == 1 else False
class server_sock(Structure):
	_fields_ = [("fd",c_int),("ssl",c_int)
	,("receive",CFUNCTYPE(c_ssize_t,c_int,c_char_p,c_size_t)),
	("sendall",CFUNCTYPE(c_ssize_t,c_char_p,c_size_t)),
	("send_msg",CFUNCTYPE(c_ssize_t,c_int,c_char_p,c_size_t)),
	("recv_msg",CFUNCTYPE(c_ssize_t,c_int,c_char_p,c_size_t)),
	("accept",CFUNCTYPE(c_int))]
	def close_sock(self):
		return True if self.close(self.fd) == 0 else False
	def isSSL(self):
		return True if self.ssl == 1 else False
libc.client_socket.restype = client_sock
libc.server_socket.restype = server_sock
	# Setupt library functions
def client_socket(prot,typ):
    return libc.client_socket(c_int(prot),c_int(typ),c_int(0))
def client_connect(fd,prot,host,port):
    return libc.client_connect(c_int(fd),c_int(prot),c_char_p(host),c_int(port))
def receive(fd,size):
    buff = create_string_buffer(size)
    libc.receive(c_int(fd),buff,c_size_t(size))
    return buff.value
def close_socket(fd):
    return libc.close_socket(c_int(fd))
def send_buff(fd,data,length):
    return libc.send_buff(c_int(fd),c_char_p(data),c_size_t(length))
AF_INET = 2
SOCK_STREAM = 1
from socket import gethostbyname

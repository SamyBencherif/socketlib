"""
BSD 3-Clause License

Copyright (c) 2018, Andrew Strickland
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
Contributors: Andrew Strickland
"""
from ctypes import *
from platform import system as osname
systype = osname()
libc = None
if systype == 'Darwin':
	cdll.LoadLibrary('./libsock.dylib')
	libc = CDLL('./libsock.dylib')
elif systype == 'Linux':
	cdll.LoadLibrary('./libsock.so')
	libc = CDLL('./libsock.so')
true = 1
false = 0
class ssl_st(Structure):
	pass
class commlayer(Structure):
	pass
commlayer._fields_ = [("fd",c_int),("ssl",POINTER(ssl_st))]
class client_sock(Structure):
	def Close(self):
		return self.close(self.handle) == 0
	def isSSL(self):
		return self.ssl == true
	def Receive(self,size):
		buff = create_string_buffer(size)
		self.receive(POINTER(self.handle),buff,c_size_t(size))
		return buff
	def Connect(self,host,port):
		return self.connect(POINTER(self),c_char_p(host),c_int(port)) == true
	def Sendall(self,data):
		length = c_size_t(len(data))
		return self.sendall(POINTER(self.handle),c_char_p(data),length)
	def Recv_Msg(self):
		return self.recv_msg(POINTER(self.handle))
	def Send_Msg(self,data):
		length = c_size_t(len(data))
		self.send_msg(POINTER(self.handle),c_char_p(data),length)
	
client_sock._fields_ = [("ssl",c_bool),("prot",c_int),("type",c_int),
	("handle",POINTER(commlayer)),
	("err",c_char_p),
	("receive",CFUNCTYPE(c_ssize_t,POINTER(commlayer),c_char_p,c_size_t)),
	("sendall",CFUNCTYPE(c_ssize_t,POINTER(commlayer),c_char_p,c_size_t)),
	("send_msg",CFUNCTYPE(c_ssize_t,POINTER(commlayer),c_char_p,c_size_t)),
	("recv_msg",CFUNCTYPE(c_char_p,POINTER(commlayer))),
	("connect",CFUNCTYPE(c_int,client_sock,c_char_p,c_int)),
	("settimeout",CFUNCTYPE(None,c_int,c_int)),
	("close",CFUNCTYPE(c_int,POINTER(commlayer)))]
	
class server_sock(Structure):
	def Close(self):
		return self.close(self.handle) == 0
	def isSSL(self):
		return self.ssl == true
	def Receive(self,size):
		buff = create_string_buffer(size)
		self.receive(POINTER(self.handle),buff,c_size_t(size))
		return buff
	def Sendall(self,data):
		length = c_size_t(len(data))
		return self.sendall(POINTER(self.handle),c_char_p(data),length)
	def Start(self,backlog,port):
		return self.start(POINTER(self),c_int(backlog),c_int(port)) == true
	def Accept(self):
		return self.accept(self)
	def Recv_Msg(self):
		return self.recv_msg(POINTER(self.handle))
	def Send_Msg(self,data):
		length = c_size_t(len(data))
		self.send_msg(POINTER(self.handle),c_char_p(data),length)
server_sock._fields_ = [("ssl",c_bool),("prot",c_int),("type",c_int),
	("handle",commlayer),
	("err",c_char_p),
	("receive",CFUNCTYPE(c_ssize_t,POINTER(commlayer),c_char_p,c_size_t)),
	("sendall",CFUNCTYPE(c_ssize_t,POINTER(commlayer),c_char_p,c_size_t)),
	("send_msg",CFUNCTYPE(c_ssize_t,POINTER(commlayer),c_char_p,c_size_t)),
	("recv_msg",CFUNCTYPE(c_char_p,POINTER(commlayer))),
	("accept",CFUNCTYPE(client_sock,server_sock)),
	("settimeout",CFUNCTYPE(None,c_int,c_int)),
	("start",CFUNCTYPE(c_int,server_sock,c_int,c_int)),
	("close",CFUNCTYPE(c_int,POINTER(commlayer)))]
	
libc.client_socket.restype = client_sock
libc.server_socket.restype = server_sock
	# Setupt library functions
def client_socket(prot,typ,ssl):
    return libc.client_socket(c_int(prot),c_int(typ),c_bool(ssl))
def server_socket(prot,type,ssl):
	return libc.server_socket(c_int(prot),c_int(type),c_bool(ssl))
AF_INET = 2
SOCK_STREAM = 1
from socket import gethostbyname

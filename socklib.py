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
"""
from ctypes import *
cdll.LoadLibrary('./libsock.so')
libc = CDLL('./libsock.so')
class client_sock(Structure):
	_fields_ = [("fd",c_int),("ssl",c_int)
	,("receive",CFUNCTYPE(c_ssize_t,c_int,c_char_p,c_size_t)),
	("sendall",CFUNCTYPE(c_ssize_t,c_char_p,c_size_t)),
	("send_msg",CFUNCTYPE(c_ssize_t,c_int,c_char_p,c_size_t)),
	("recv_msg",CFUNCTYPE(c_ssize_t,c_int,c_char_p,c_size_t)),
	("settimeout",CFUNCTYPE(None,c_int,c_int)),
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

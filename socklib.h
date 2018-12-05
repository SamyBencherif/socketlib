/*
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
*/
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define ssl_on 1
#ifdef ssl_on
//ssl includs
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#define true 1
#define false 0
#define TIMEOUT 2
#define REUSE 1
// Abstract handle for a client socket. This is returned by client_socket where all the appropriate function
// pointers are set. If the socket uses ssl, the io functions are set to the appropriate ssl functions.
struct client_sock{
	int fd,ssl;
	ssize_t (*receive)(int sock_fd, char* buff, size_t count);
	ssize_t (*sendall)(int sock_fd, const char* data, size_t length);
	ssize_t (*send_msg)(int sock_fd,const char* data,size_t length);
	ssize_t (*recv_msg)(int sock_fd, char* buff, size_t count);
	void (*settimeout)(int sock_fd,int seconds);
	int (*close)(int fd);
};
// Abstract handle for a client socket. This is returned by server_socket where all the appropriate function
// pointers are set. If the socket uses ssl, the io functions are set to the appropriate ssl functions.
struct server_sock{
	int fd,ssl;
	ssize_t (*receive)(int sock_fd, char* buff, size_t count);
	ssize_t (*sendall)(int sock_fd, const char* data, size_t length);
	ssize_t (*send_msg)(int sock_fd,const char* data,size_t length);
	ssize_t (*recv_msg)(int sock_fd, char* buff, size_t count);
	void (*accept)(struct server_sock server);
	void (*settimeout)(int sock_fd,int seconds);
	int (*close)(int fd);
};

#ifdef ssl_on
void init_ssl(int sock_fd,const char* path_to_chain, const char* path_to_key,int client);
#endif

struct server_sock server_socket(int prot,int type,int ssl);

struct client_sock client_socket(int prot,int type, int ssl);
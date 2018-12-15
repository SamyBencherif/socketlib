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
 Contributors: Andrew Strickland, JK2Designs
 */
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include "NetworkingErrors.h"
#define true 1
#define false 0
#define TIMEOUT 2
#define REUSE 1

//Forward declarations of SSL and the generic socket type.

typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;
// An abstract socket that contains either a client_sock* or a server_sock*
struct commlayer{
    int fd;
    SSL* ssl;
};
typedef struct commlayer COMM;
// Abstract handle for a client socket. This is returned by client_socket where all the appropriate function
// pointers are set. If the socket uses ssl, the io functions are set to the appropriate ssl functions.
struct client_sock{
    bool ssl;
    int prot,type;
    COMM handle;
    const char* err;
    ssize_t (*receive)(COMM* handle, char* buff, size_t count);
    ssize_t (*sendall)(COMM* handle, const char* data, size_t length);
    ssize_t (*send_msg)(COMM* handle,const char* data,size_t length);
    const char* (*recv_msg)(COMM* handle);
    int (*connect)(struct client_sock* cli, const char* hostname, int port);
    void (*settimeout)(int sock_fd,int seconds);
    int (*close)(COMM* handle);
};
// Abstract handle for a client socket. This is returned by server_socket where all the appropriate function
// pointers are set. If the socket uses ssl, the io functions are set to the appropriate ssl functions.
struct server_sock{
    bool ssl;
    int prot,type;
    COMM handle;
    ssize_t (*receive)(COMM* handle, char* buff, size_t count);
    ssize_t (*sendall)(COMM* handle, const char* data, size_t length);
    ssize_t (*send_msg)(COMM* handle,const char* data,size_t length);
    const char* (*recv_msg)(COMM* handle);
    struct client_sock (*accept)(struct server_sock server);
    void (*settimeout)(int sock_fd,int seconds);
    int (*start)(struct server_sock* server, int backlog, int port);
    int (*close)(COMM* handle);
};
extern void testSSL(COMM* handler);
int* read_select(int*sock_fds, unsigned long length, int timeout);

struct server_sock server_socket(int prot,int type,bool ssl);

struct client_sock client_socket(int prot,int type, bool ssl);

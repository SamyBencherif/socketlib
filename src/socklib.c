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



#include "socklib.h"
#include "sslfunctions.h"
#include "io.h"
#include "sockfunctions.h"
#include "clientfunctions.h"
#include "serverfunctions.h"
#include <stdarg.h>
#include <signal.h>
// Blocking function. If timeout is -1, this function will block indefinitely.
int* read_select(int*sock_fds, unsigned long length, int timeout){
    fd_set rfds;
    //select and for each socket to be read, run the callback on it
    FD_ZERO(&rfds);
    int max_fd_local = 0;
    for(int i = 0; i < length; i++){
        if (sock_fds[i] == -1){
            continue;
        }
        max_fd_local = (sock_fds[i] > max_fd_local) ? sock_fds[i] : max_fd_local;
        FD_SET(sock_fds[i],&rfds);
    }
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    int ready = select(max_fd_local+1,&rfds,NULL,NULL,(timeout >=0) ? &tv : NULL);
    int* readyToRead = NULL;
    if(ready > 0){
        readyToRead = malloc(sizeof(int)*ready);
        int rread_index = 0;
        for(int i = 0; i < length; i++){
            if(FD_ISSET(sock_fds[i],&rfds)){
                readyToRead[rread_index++] = sock_fds[i];
            }
        }
    }
    return readyToRead;
}

// Initializes the library and open ssl if the library was compiled with ssl
void socklib_init()
{
    output.fd = socket(AF_UNIX,SOCK_STREAM,0);
    signal(SIGPIPE, SIG_IGN);
    #ifdef ssl_on
    initOpenSSL();
    #endif
}
// Closes the log output socket stream and shutsdown any oppened ssl configurations.
void socklib_deinit()
{
    close_socket(&output);
    shutdownSSL();
}

struct client_sock client_socket(int prot,int type, bool ssl)
{
    struct client_sock client;
    memset(&client, '0', sizeof(client));
    COMM handle = {0,0};
    client.handle = handle;
    // set close and timeout functions
    client.close = close_socket;
    client.settimeout = settimeout;
    client.err = NULL;
    // set ssl bool
    client.ssl = ssl;
    int fd = create_socket(prot,type);
    if(fd > 0){
        //printf("Received file descriptor %d from sys.\n",fd);
        //fputs("Received file descriptor "+itoa(fd)+" from sys.\n", output);
        output_write("Received file descriptor %d from sys.\n",fd);
        client.handle.fd = fd;
        client.handle.ssl = NULL;
        client.prot = prot;
        client.type = type;
        client.connect = client_connect;
        client.receive = receive;
        client.sendall = send_buff;
        client.recv_msg = recv_msg;
        client.send_msg = send_msg;
    }
    return client;
}

struct server_sock server_socket(int prot,int type,bool ssl)
{
    struct server_sock server;
    memset(&server,'0' , sizeof(server));
    COMM handle;
    server.handle = handle;
    int fd = create_socket(prot,type);
    server.close = close_socket;
    server.settimeout = settimeout;
    server.prot = prot;
    server.type = type;
    server.ssl = ssl;
    if(fd > 0){
        void* val = malloc(sizeof(int));
        *((int*)val) = 1;
        setopt(fd,1,val);
        server.handle.fd = fd;
        server.handle.ssl = NULL;
        server.receive = receive;
        server.sendall = send_buff;
        server.send_msg = send_msg;
        server.recv_msg = recv_msg;
        server.start = start;
        server.accept = sock_acc;
    }else{
        server.handle.fd = -1;
    }
    return server;
}

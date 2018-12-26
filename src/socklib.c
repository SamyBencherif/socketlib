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
#include <stdarg.h>
// Specify minframe, usuaally sie of an integer since SSL sends int sizes of files rather than long sizes
ssize_t MIN_FRAME = 4;
// Read descriptor vector

FILE* output;
void socklib_init()
{
    output = tmpfile();

}
FILE* libOutput()
{
    return output;
}
void output_write(const char* str,...)
{
    va_list args;
    va_start(args,str);
    // add 50 extra bytes to allow space for numbers or other format values
    char* buffer = (char*)malloc(strlen(str)+50);
    if(sprintf(buffer,str,args) > 0){
        fputs(buffer,output);
    }
    free(buffer);
}



// Creates a general purpose socket, this method is not disclosed to the header file.
// Users should call the appropriate methods generating either a server_sock or client_sock struct
int create_socket(int prot,int type)
{
    int protocol = (prot == AF_INET) ? AF_INET : AF_INET6;
    int typespec = (type == SOCK_STREAM) ? SOCK_STREAM : SOCK_DGRAM;
    return socket(protocol,typespec,0);
}

/* Sets options for a file descriptor. Flag indicates the type and option for arg:
 flag = 1 : arg is an integer, set the reuse address and reuse port option;
 flag = 2 : arg is a struct timeval, set the timeout for the socket
 */
void setopt(int sock_fd, int flag, void* arg)
{
    if(flag == REUSE){
        int opt = *((int*)arg);
        setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    }else if(flag == TIMEOUT){
        struct timeval tv = *((struct timeval*)arg);
        setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    }
}
// sets the timeout for the specified file desc
void settimeout(int sock_fd,int seconds){
    struct timeval* tv = (struct timeval*)malloc(sizeof(struct timeval));
    tv->tv_sec = seconds;
    tv->tv_usec = 0;
    setopt(sock_fd,2,(void*)tv);
}
// SSL Currently not supported


// server functions
int start(struct server_sock* server, int backlog, int port){
    struct sockaddr_in serverAddr;
    memset(&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family = server->prot;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);
    int one = 1;
    setsockopt(server->handle.fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if(bind(server->handle.fd,(struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0){
        return false;
    }
    if(listen(server->handle.fd,backlog) < 0){
        return false;
    }
    return true;
}
bool sock_state_valid(COMM* handle){
    errno = 0;
    if(handle->fd == -1){
        return false;
    }
    return true;
}
//end server functions
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


// Connects the client to a specified host and port. The host must be a propper ip address. (Hostnames must
// be resolved before passing it to this function)
int client_connect(struct client_sock* client, const char* host, int port){
    if(!sock_state_valid(&client->handle)){
        return INVALIDSTATE;
    }
    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(client->prot, host, &serv_addr.sin_addr)<=0)
    {
        //printf("\nInvalid address/ Address not supported \n");
        output_write("Invalid address/ Address not supported \n");
        return -1;
    }
    bool connected = connect(client->handle.fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0;
    if(connected){
        if(client->ssl){
            initOpenSSL();
            wrapClient(&client->handle);
        }
    }else{
        //printf("Failed to connect.\n");
        output_write("Failed to connect.\n");
        return -1;
    }
    return 1;
}

int close_socket(COMM* handle){
    if(handle->fd == -1){
        return -1;
    }
    // set default return in case ssl is disabled
    int shut = 0x1;
    if(handle->ssl){
        shut = 0;// reset the default value in case ssl is enabled.
        shut |= SSL_shutdown(handle->ssl);
        SSL_free(handle->ssl);
        handle->ssl = NULL;
    }
    errno = 0;
    //printf("Attempting to shutdown %d\n",handle->fd);
    output_write("Attempting to shutdown %d\n",handle->fd);
    if(shutdown(handle->fd,SHUT_RDWR) == 0){
        if(errno == 0){
            //printf("Shutdown successful! Attempting to close!\n");
            output_write("Shutdown successful! Attempting to close!\n");
            shut = shut<<1;
            shut |= close(handle->fd);
            if(shut == 2){
                output_write("Closed!\n");
            }
        }
    }else{
        output_write("Error %d\n",errno);
    }
    return (shut == 2) ? 0 : -1;
}
// Start io functions


// Simple wrapper to send buffer
ssize_t send_buff(COMM* handle, const char* data, size_t length)
{
    if(!sock_state_valid(handle)){
        return -1;
    }
    int n = 1;
    #ifdef __linux
    #else
    setsockopt(handle->fd, SOL_SOCKET, SO_NOSIGPIPE, &n, sizeof(n));
    #endif
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(handle->fd,&wfds);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    errno = 0;
    ssize_t sent = 0;
    int ready = select(handle->fd +1,NULL,&wfds,NULL,&tv);
    if(ready < 1 || !FD_ISSET(handle->fd,&wfds)){
        //printf("Fd not able to be written to!\n");
        output_write("Fd not able to be written to!\n");
        return -1;
    }
    if(errno == 0){
        if(handle->ssl){
            sent = SSL_write(handle->ssl, data, (int)length);
        }else{
            sent = send(handle->fd,data,length,0);
        }
    }
    else{
        //printf("Error in send_buff %d\n",errno);
        //fputs("Error in send_buff"+itoa(errno)+"\n",output);
        output_write("Error in send_buff %d\n",errno);
    }
    return sent;
}
// Simple recv function receives data with size x: 0<= x <= count
ssize_t receive(COMM* handle, char* buff, size_t count)
{
    ssize_t reecved = 0;
    if(!sock_state_valid(handle)){
        return -1;
    }
    if(handle->ssl){
        reecved = SSL_read(handle->ssl, buff, (int)count);
    }else{
       reecved = recv(handle->fd,buff,count,0);
    }
    if(errno != 0){
        //printf("Error in recv %d\n",errno);
        //fputs("Error in recv"+itoa(errno)+"\n",output);
        output_write("Error in recv %d\n",errno);
    }
    return reecved;
}
// Encode the size of the message to a char* that can be sent to the remote connection
char* encodeInteger(size_t length){
    char* bytes = (char*)malloc(sizeof(char)*MIN_FRAME);
    bytes[0] = (length >> 24) & 0xFF;
    bytes[1] = (length >> 16) & 0xFF;
    bytes[2] = (length >> 8) & 0xFF;
    bytes[3] = length & 0xFF;
    return bytes;
}
// Decodes the frame received into an integer size
unsigned int decodeInteger(char* frame){
    unsigned int size = (frame[0] << 24) | (frame[1] << 16) | (frame[2] << 8) | frame[3];
    return size;
}
// Formats the buffer sent to be. First sends the size encoded as 4 bytes which
// the server identifies as the MIN_FRAME. Upon success it sends the message. This
// ensures every byte of the message is received.
ssize_t send_msg(COMM* handle,const char* data,size_t length){
    if(!sock_state_valid(handle)){
        return -1;
    }
    char* bytes = encodeInteger(length);
    if(bytes){
        if( send_buff(handle, bytes, 4) > 0){
            free(bytes);
            if(data){
                return send_buff(handle, data, strlen(data));
            }else{
                return 4;
            }
        }else{
            free(bytes);
            return 0;
        }
    }
    return 0;
}
// Safe read function. Note this must be used on the server side for it to work porpperly.
// It formats the msg by sending the size of the message before the message itself.
// THE RECEIVER IS RESPONSIBLE FOR FREEING THE RETURN VALUE
const char* recv_msg(COMM* handle){
    if(!sock_state_valid(handle)){
        return NULL;
    }
    char frame[MIN_FRAME];
    memset(frame,'0',MIN_FRAME);
    char* msg = NULL;
    //Receive the minimum frame size (4 bytes for 32 bit, 64 hasnt been implemented yet)
    ssize_t read = receive(handle, frame,MIN_FRAME);
    if( read == MIN_FRAME){
        unsigned int size = decodeInteger(frame);
        // alocate space for the incoming message
        msg = (char*)calloc(size, sizeof(char));
        if(msg){
            char buffer[1024];
            while(strlen(msg) < size){
                //clear the buffer before the next read
                int BUFF_MAX = 1024;
                memset(buffer, 0, BUFF_MAX);
                read = receive(handle, buffer, BUFF_MAX-1);
                // nothing read or an error occurred no need to get stuck in a loop
                if(read == 0 || read == -1){
                    break;
                }
                // append the conents to our msg
                buffer[read] = '\0';
                msg = strcat(msg, buffer);
            }
        }
    }
    return msg;
}

// end io functions

struct client_sock sock_acc(struct server_sock server)
{
    struct client_sock client;
    memset(&client,'0',sizeof(client));
    struct sockaddr clientaddr;
    memset(&clientaddr,'0',sizeof(clientaddr));
    COMM handle;
    client.handle = handle;
    memset(&client.handle, '0', sizeof(client.handle));
    
    socklen_t addr_len = sizeof(clientaddr);
    int fd = accept(server.handle.fd,&clientaddr,&addr_len);
    if(fd >= 0){
        client.handle.fd = fd;
        client.handle.ssl = NULL;
        // set prototype and socket type
        client.prot = server.prot;
        client.type = server.prot;
        // Set io functions
        client.receive = receive;
        client.sendall = send_buff;
        client.send_msg = send_msg;
        client.recv_msg = recv_msg;
        // disable connecting
        client.connect = NULL;
        client.close = close_socket;
        client.settimeout = settimeout;
        client.err = NULL;
        if(server.ssl == true){
            // set communication handle
            #ifdef ssl_on
            wrapServer(&client.handle);
            client.ssl = true;
            #endif   
        }
    }else{
        client.err = "Could not obtain file descriptor from the system.";
    }
    return client;
    
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
void release_client(struct client_sock client){
    
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

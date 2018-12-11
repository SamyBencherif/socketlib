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
#ifdef ssl_on
SSL_CTX *sslctx;
SSL *cSSL;
int sslsetupt = false;
#endif


// Specify minframe
ssize_t MIN_FRAME = 4;
// Read descriptor vector
fd_set rfds;
// Number of connections
int connections = 0;

int max_fd;

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
#ifdef ssl_on
// SSL start
void InitializeSSL()
{
	if(sslsetupt == false){
		SSL_load_error_strings();
		SSL_library_init();
		OpenSSL_add_all_algorithms();
		sslsetupt = true;
	}
}

void DestroySSL()
{
    ERR_free_strings();
    EVP_cleanup();
}

void ShutdownSSL()
{
	if(cSSL){
		SSL_shutdown(cSSL);
		SSL_free(cSSL);
	}
}

void init_ssl(int sock_fd,const char* path_to_chain, const char* path_to_key, int client){
	InitializeSSL();
	if(client == true){
		sslctx = SSL_CTX_new(SSLv23_client_method());
	}else{
		sslctx = SSL_CTX_new( SSLv23_server_method());
	}
	SSL_CTX_set_options(sslctx, SSL_OP_SINGLE_DH_USE);
	int use_cert = SSL_CTX_use_certificate_file(sslctx, path_to_chain , SSL_FILETYPE_PEM);

	int use_prv = SSL_CTX_use_PrivateKey_file(sslctx, path_to_key, SSL_FILETYPE_PEM);

	cSSL = SSL_new(sslctx);
	SSL_set_fd(cSSL, sock_fd );
	//Here is the SSL Accept portion.  Now all reads and writes must use SSL
	int ssl_err = SSL_accept(cSSL);
	if(ssl_err <= 0)
	{
		//Error occurred, log and close down ssl
		ShutdownSSL();
	}
}
// End ssl
#endif

// server functions

int start(struct server_sock server, int backlog, int port){
	struct sockaddr_in serverAddr;
	memset(&serverAddr,0,sizeof(serverAddr));
	serverAddr.sin_family = server.prot;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(port);
	if(bind(server.fd,(struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0){
		return false;
	}
	if(listen(server.fd,backlog) < 0){
		return false;
	}
	return true;
}
//end server functions
// Blocking function. If timeout is -1, this function will block indefinitely.
int* read_select(int*sock_fds, unsigned long length, int timeout){
	//select and for each socket to be read, run the callback on it
    FD_ZERO(&rfds);
    int max_fd_local = 0;
    for(int i = 0; i < length; i++){
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
int client_connect(struct client_sock client, const char* host, int port){
	struct sockaddr_in serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port);  
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(client.prot, host, &serv_addr.sin_addr)<=0)
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(client.fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    }
	return 1;
}

int close_socket(int sock_fd){
    FD_CLR(sock_fd,&rfds);
    connections--;
	return close(sock_fd);
}
// Start io functions

// Simple wrapper to send buffer
ssize_t send_buff(int sock_fd, const char* data, size_t length)
{
	return send(sock_fd,data,length,0);
}
// Simple recv function receives data with size x: 0<= x <= count
ssize_t receive(int sock_fd, char* buff, size_t count)
{
	return recv(sock_fd,buff,count,0);
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
ssize_t send_msg(int sock_fd,const char* data,size_t length){
	char* bytes = encodeInteger(length);
    if( send_buff(sock_fd, bytes, 4) > 0){
		free(bytes);
        return send_buff(sock_fd, data, strlen(data));
    }else{
        return 0;
    }
	
}
// Safe read function. Note this must be used on the server side for it to work porpperly.
// It formats the msg by sending the size of the message before the message itself.
// THE RECEIVER IS RESPONSIBLE FOR FREEING THE RETURN VALUE
const char* recv_msg(int sock_fd){
    char frame[MIN_FRAME];
    memset(frame,'0',MIN_FRAME);
    char* msg = NULL;
    //Receive the minimum frame size (4 bytes for 32 bit, 64 hasnt been implemented yet)
    ssize_t read = receive(sock_fd, frame,MIN_FRAME);
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
                read = receive(sock_fd, buffer, BUFF_MAX-1);
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

struct client_sock sock_acc(struct server_sock server){
	struct client_sock client;
	struct sockaddr clientaddr;
	int addr_len = sizeof(clientaddr);
	int fd = accept(server.fd,&clientaddr,&addr_len);
	if(fd >= 0){
		if(server.ssl == false){
			client.fd = fd;
			client.prot = server.prot;
			client.type = server.prot;
			client.receive = receive;
			client.sendall = send_buff;
			client.send_msg = send_msg;
			client.recv_msg = recv_msg;
			client.connect = NULL;
			client.close = close_socket;
			client.settimeout = settimeout;
			client.err = NULL;
		}else{
			#ifdef ssl_on
			//add ssl code
			#endif
		}
	}else{
		client.fd = -1;
		client.err = "Could not obtain file descriptor from the system.";
	}
	return client;
	
}
struct client_sock client_socket(int prot,int type, int ssl){
	struct client_sock client;
    memset(&client, '0', sizeof(client));
	client.close = close_socket;
	client.settimeout = settimeout;
    client.err = NULL;
	client.ssl = ssl;
	int fd = create_socket(prot,type);
	if(fd > 0){
		printf("Received file descriptor %d from sys\n",fd);
        connections++;
        max_fd = (fd > max_fd) ? fd : max_fd;
		client.fd = fd;
        client.prot = prot;
        client.type = type;
        client.connect = client_connect;
		if(ssl == true){
			#ifdef ssl_on
			// add ssl code to set function read and write pointers
			
			#endif
		}else{
			client.ssl = false;
			client.receive = receive;
			client.sendall = send_buff;
			client.send_msg = send_msg;
			client.recv_msg = recv_msg;
		}
    }else{
        client.fd = -1;
        client.err = "Failed to receive a file descriptor for the client.";
    }
	return client;
}

struct server_sock server_socket(int prot,int type,int ssl){
	struct server_sock server;
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
		server.fd = fd;
		if(ssl == true){

		}else{
			server.receive = receive;
			server.sendall = send_buff;
			server.send_msg = send_msg;
			server.recv_msg = recv_msg;
			server.start = start;
			server.accept = sock_acc;
		}
	}else{
		server.fd = -1;
	}
	return server;
}

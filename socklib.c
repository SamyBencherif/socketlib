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

#include "socklib.h"
SSL_CTX *sslctx;
SSL *cSSL;
int sslsetupt = false;
fd_set rfds;
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
// 
void settimeout(int sock_fd,int seconds){
	struct timeval* tv = (struct timeval*)malloc(sizeof(struct timeval));
	tv->tv_sec = seconds;
	tv->tv_usec = 0;
	setopt(sock_fd,2,(void*)tv);
}

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


int sock_acc(int sock_fd){
	struct sockaddr clientaddr;
	int addr_len = sizeof(clientaddr);
	int client = accept(sock_fd,&clientaddr,&addr_len);
	return client;
}

void add_rfds(int sock_fd){
	
}

void select_thread(int*sock_fds, void (*callback)(int)){
	//select and for each socket to be read, run the callback on it
}



int client_connect(struct client_sock* client,int prot, const char* host, int port){
	struct sockaddr_in serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port);  
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(prot, host, &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(client->fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    }
	return 1;
}

int close_socket(int sock_fd){
	return close(sock_fd);
}
// Start io functions

ssize_t send_buff(int sock_fd, const char* data, size_t length)
{
	return send(sock_fd,data,strlen(data),0);
}

ssize_t receive(int sock_fd, char* buff, size_t count)
{
	return read(sock_fd,buff,count);
}
// Formats the buffer sent to be 
ssize_t send_msg(int sock_fd,const char* data,size_t length){
	return 0;
}

ssize_t recv_msg(int sock_fd, char* buff, size_t count){
	return 0;
}

// end io functions
struct client_sock client_socket(int prot,int type, int ssl){
	struct client_sock client;
	client.close = close_socket;
	client.settimeout = settimeout;
	int fd = create_socket(prot,type);
	if(fd > 0){
		printf("Received file descriptor %d from sys\n",fd);
		client.fd = fd;
		if(ssl == true){
			#ifdef ssl_on
			// add ssl code to set function read and write pointers
			
			#endif
		}else{
			client.receive = receive;
			client.sendall = send_buff;
			client.send_msg = send_msg;
			client.recv_msg = recv_msg;
		}
	}
	return client;
}

struct server_sock server_socket(int prot,int type,int ssl){
	struct server_sock server;
	int fd = create_socket(prot,type);
	server.close = close_socket;
	server.settimeout = settimeout;
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
		}
	}else{
		server.fd = -1;
	}
	return server;
}
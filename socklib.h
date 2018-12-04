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
struct client_sock{
	int fd,ssl;
	ssize_t (*receive)(int sock_fd, char* buff, size_t count);
	ssize_t (*sendall)(int sock_fd, const char* data, size_t length);
	ssize_t (*send_msg)(int sock_fd,const char* data,size_t length);
	ssize_t (*recv_msg)(int sock_fd, char* buff, size_t count);
	int (*close)(int fd);
};
struct server_sock{
	int fd,ssl;
	ssize_t (*receive)(int sock_fd, char* buff, size_t count);
	ssize_t (*sendall)(int sock_fd, const char* data, size_t length);
	ssize_t (*send_msg)(int sock_fd,const char* data,size_t length);
	ssize_t (*recv_msg)(int sock_fd, char* buff, size_t count);
	void (*accept)();
};
int create_socket(int prot,int type);

void setopt(int sock_fd, int reuse);

void settimeout(int sock_fd,int seconds);
#ifdef ssl_on

void InitializeSSL();

void DestroySSL();

void ShutdownSSL();

void init_ssl(int sock_fd,const char* path_to_chain, const char* path_to_key,int client);
#endif

struct server_sock* server_socket(int prot,int type);

struct client_sock client_socket(int prot,int type, int ssl);

int client_connect(struct client_sock* ,int prot, const char* host, int port);

int close_socket(int sock_fd);

ssize_t send_buff(int sock_fd, const char* data, size_t length);

// This implements a safe-send safe-read protocol where the size of the content is sent bore the content to ensure
// all data is received. Note the receiving end must implment this same protocol or use the recv_msg function
// in this library.
ssize_t send_msg(int sock_fd,const char* data,size_t length);

ssize_t recv_msg(int sock_fd, char* buff, size_t count);

ssize_t receive(int sock_fd, char* buff, size_t count);
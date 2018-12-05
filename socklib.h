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
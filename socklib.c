#include "socklib.h"
SSL_CTX *sslctx;
SSL *cSSL;
int sslsetupt = false;
fd_set rfds;

int create_socket(int prot,int type)
{
	int protocol = (prot == AF_INET) ? AF_INET : AF_INET6;
	int typespec = (type == SOCK_STREAM) ? SOCK_STREAM : SOCK_DGRAM;
	return socket(protocol,typespec,0);
}


void setopt(int sock_fd, int reuse)
{
	int opt = 1;
	if(reuse == true){
		setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
	}
}

void settimeout(int sock_fd,int seconds){
	struct timeval tv;
	tv.tv_sec = seconds;
	tv.tv_usec = 0;
	setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
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


struct server_sock* server_socket(int prot,int type){
	struct server_sock *serverptr,server;
	serverptr = &server;
	int fd = create_socket(prot,type);
	if(fd > 0){
		setopt(fd,1);
		server.fd = fd;
	}else{
		return NULL;
	}
	return serverptr;
}
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


ssize_t send_buff(int sock_fd, const char* data, size_t length)
{
	return send(sock_fd,data,strlen(data),0);
}

ssize_t receive(int sock_fd, char* buff, size_t count)
{
	return read(sock_fd,buff,count);
}
ssize_t send_msg(int sock_fd,const char* data,size_t length){
	return 0;
}

ssize_t recv_msg(int sock_fd, char* buff, size_t count){
	return 0;
}
struct client_sock client_socket(int prot,int type, int ssl){
	struct client_sock client;
	client.close = close_socket;
	int fd = create_socket(prot,type);
	if(fd > 0){
		printf("Received file descriptor %d from sys\n",fd);
		settimeout(fd,3);
		client.fd = fd;
		if(ssl > 0){
			#ifdef ssl_on
			// add ssl code to set function read and write pointers
			#endif
		}
	}
	return client;
}
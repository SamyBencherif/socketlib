// Creates a general purpose socket, this method is not disclosed to the header file.
// Users should call the appropriate methods generating either a server_sock or client_sock struct
int create_socket(int prot,int type)
{
    return socket(prot,type,0);
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


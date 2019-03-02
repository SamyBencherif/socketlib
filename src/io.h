bool sock_state_valid(COMM* handle){
    errno = 0;
    if(handle->fd == -1){
        return false;
    }
    return true;
}
COMM output = {0,NULL};
// Returns output stream for the library.
int libOutput()
{
    return output.fd;
}
/*
TODO: Fix this so the library can print to something other than stdout or stderr
*/
void output_write(const char* str,...)
{
    // if(output.fd < 0){
    //     return;
    // }
    // va_list args;
    // va_start(args,str);
    // // add 50 extra bytes to allow space for numbers or other format values
    // char* buffer = (char*)malloc(strlen(str)+50);
    // if(sprintf(buffer,str,args) > 0){
    //     send(output.fd,buffer,strlen(buffer),0);
    // }
    // va_end(args);
    // free(buffer);
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
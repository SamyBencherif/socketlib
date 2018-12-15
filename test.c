#include "socklib.h"
#include <stdio.h>
int main(){
    signal(SIGPIPE, SIG_IGN);
    printf("Getting server socket\n");
    struct server_sock server = server_socket(2,1,false);
    printf("Starting server socket\n");
    const char* msg = NULL;
    if(server.start(&server,5,1337) == 1){
        printf("Started\n");
        struct client_sock client = server.accept(server);
        while(true){
            if(!msg){
                printf("Receiving!\n");
                msg = client.recv_msg(&client.handle);
            }
            printf("Writing!\n");
            if(msg){
                ssize_t sent = client.send_msg(&client.handle,msg,strlen(msg));
                if(sent <= 0){
                    break;
                }
            }
            usleep(1000000);
        }
    }else{
        printf("Failed to start");
    }
    return 0;
}

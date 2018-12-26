#include "socklib.h"
#include <signal.h>

#include <stdio.h>
int main(){
    signal(SIGPIPE, SIG_IGN);
    printf("Getting server socket\n");
    struct server_sock server = server_socket(2,1,true);
    initOpenSSL();
    bool loaded = LoadCertificates("certificate.pem", "key.pem");
    printf("Starting server socket\n");
    const char* msg = NULL;
    if(server.start(&server,5,1337) == 1){
        printf("Started\n");
        struct client_sock client = server.accept(server);
        client.settimeout(client.handle.fd,3);
        if(!msg){
            printf("Receiving!\n");
            msg = client.recv_msg(&client.handle);
            printf("%s\n",msg);
            free((void*)msg);
        }
        client.close(&client.handle);
        server.close(&server.handle);
    }else{
        printf("Failed to start\n");
    }
    printf("Closing!\n");
    
    return 0;
}

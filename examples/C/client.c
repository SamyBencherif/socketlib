#include "socklib.h"
#include <stdio.h>
typedef const char* CString;
CLIENT* create_client(int proto, int type, bool isSSL, CString addr, int port){
  CLIENT* client = malloc(sizeof(CLIENT));
  memset(&client,0,sizeof(CLIENT));
  *(client) = client_socket(proto,type,isSSL);
  if(client->connect(client,addr,port) != 1){
    printf("Could not connect!\n");
    return NULL;
  } 
  return client;
}
int main(){
  socklib_init();
   // In case the server disconnects we dont want to fault on a broken pipe.
   // Get a non-ssl client.
   CLIENT* client = create_client(AF_INET,SOCK_STREAM,false,"192.168.1.69",1337);
   if(!client){
     exit(EXIT_FAILURE);
   }
  char buff[128];
  buff[127] = '\0';
  printf("%s",buff);
  // Connect to a server on the given hostname at port 1337
  
  CString mystr = "Hey what is going on!";

  // Securely sends data to the server provided the server is using recv_msg.
  // in this case, the server will wait until all the data you sent is received before moving on.
  client->send_msg(&client->handle,mystr,strlen(mystr)); 

  // closes the connection to the server
  client->close(&client->handle);
  socklib_deinit();
  return 0;
}
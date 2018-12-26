#include "socklib.h"
#include <signal.h>
#include <stdio.h>
int main(){
  signal(SIGPIPE, SIG_IGN); // In case the server disconnects we dont want to fault on a broken pipe.
  struct client_sock client = client_socket(AF_INET,SOCK_STREAM,true); // Get a non-ssl client.

  // Connect to a server on the given hostname at port 1337
  if(client.connect(&client,"192.168.1.88",1337) != 1){
    printf("Could not connect!\n");
    return 0;
  } 
  const char* mystr = "Hey what is going on!";

  // Securely sends data to the server provided the server is using recv_msg.
  // in this case, the server will wait until all the data you sent is received before moving on.
  client.send_msg(&client.handle,mystr,strlen(mystr)); 

  // closes the connection to the server
  client.close(&client.handle);
  return 0;
}
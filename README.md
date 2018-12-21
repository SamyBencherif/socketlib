# SocketLib
SocketLib is a library written in C to perform socket networking operations with efficiency, speed, and reliability.
The versatility of C allows this library (so in unix) to be loaded by any language supporting library loading (C++, C, Python, Java, etc.) and wrapper functions can be writen for ease of use.
Note C and C++ will natively support this library by using the included header file along with the library itself. Other languages may benefit from wrapper functions and classes.
- Currently supports Linux and Mac OS
## When should I use a dynamic Library?
Dynamic libraries are compiled and linked programs that are loaded into memory (along with their symbols) when requested either via dlopen or similar library loading functions. Common uses of dynamic libraries are compatibility with non C-interoperable code, selecting which resources a program needs and keeping memory footprint low until resources in the library are needed, etc.
## When should I use the Static Library
Static libraries (often .a files) are compiled and linked code that contain the functions and resources required by your main program. Often times the header file for the library may need the "extern" tag to preface functions that are in the library to indicate these resources are not located in your main program. Static libraries are used in C-interoperable languages (C, C++, Objective-C, Swift,...) and are embeded into your main program at compile time (usually using -l<frameworkname> in gcc). Common uses are compiling these sources into the main program to reduce runtime complexity (dll loading isnt super fast sometimes) and minimizing code (no calls to dlopen or retrieving function pointers).

## Python wrapper example
Currently under construction. Some io functions are not completely usable. Users may want to write their own wrapper using the ctypes library in python.
```c
//Static library examples
// This is an example of a hello world client in C. A class wrapper can be written in C++ to avoid passing the struct and its
// communication handle for many functions.

#include "socklib.h"
#include <stdio.h>
int main(){
  signal(SIGPIPE, SIG_IGN); // In case the server disconnects we dont want to fault on a broken pipe.

  struct client_sock client = client_socket(AF_INET,SOCK_STREAM,false); // Get a non-ssl client.

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


/* -------------------------------------- Server ---------------------------------------------*/
//This is an example of a hello world server in C. A class wrapper can be written in C++ to avoid passing the struct and its
// communication handle for many functions.
#include "socklib.h"
#include <stdio.h>
int main(){
  signal(SIGPIPE, SIG_IGN); // In case the client disconnects we dont want to fault on a broken pipe.

  struct server_sock server = server_socket(2,1,false);// Get a non-ssl server.

// Start listening on port 1337 with a maximum backlog of 5 clients queued for connection
  if(server.start(&server,5,1337) != 1){
    printf("Could not start server!\n");
    return 0;
  }

  //Start accepting connections
  while (true){
    struct client_sock client = server.accept(server);
  
    // Receive a message from the connected client.
    const char* msg = client.recv_msg(&client.handle);
    printf("%s",msg);
  
    //FREE the message memory allocated!
    free(msg);
  
    //Close the client
    client.close(&client.handle);
  }
  server.close(&server.handle);
  return 0;
}
```

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
```python
from socklib import *
client = client_socket(AF_INET,SOCK_STREAM)
client.close_sock()
```

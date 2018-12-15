linux:
	@gcc -fPIC -c ./src/socklib.c -o ./objectfiles/socklib.o -lssl
	@gcc -shared -fPIC -Wl,-soname,libsock.so -o ./dynamiclibs/libsock.so ./objectfiles/socklib.o -lc -lssl
	@echo Made libsock.so
mac:
	@rm -rf *.dylib
	@gcc -std=c11 -fPIC -c ./src/socklib.c -o ./objectfiles/socklib.o
	@gcc -dynamiclib -fPIC -Wl -o ./dynamiclibs/socklib.dylib ./objectfiles/socklib.o
	@rm -rf *.o
	@echo Made socklib.dylib	
clean:
	@rm -rf dynamiclibs/
	@rm -rf objectfiles/
	@mkdir dynamiclibs/
	@mkdir objectfiles/

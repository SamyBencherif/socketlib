linux:
	@gcc -fPIC -c ./src/socklib.c -o ./objectfiles/socklib.o -lssl
	@gcc -shared -fPIC -Wl,-soname,libsock.so -o ./dynamiclibs/libsock.so ./objectfiles/socklib.o -lc -lssl
	@echo Made libsock.so
mac:
	@rm -rf *.dylib
	@gcc -std=c11 -fPIC -c ./src/socklib.c -o ./objectfiles/socklib.o  -I/usr/local/Cellar/openssl/1.0.2q/include 
	@gcc -dynamiclib -fPIC -Wl -o ./dynamiclibs/socklib.dylib ./objectfiles/socklib.o -I/usr/local/Cellar/openssl/1.0.2q/include -L/usr/local/Cellar/openssl/1.0.2q/lib -lssl -lcrypto
	@echo Made socklib.dylib
	@ar rcs ./staticlibs/libsocklib.a ./objectfiles/socklib.o
	@echo Made libsocklib.a Static library
clean:
	@rm -rf dynamiclibs/
	@rm -rf objectfiles/
	@rm -rf staticlibs/
	@mkdir dynamiclibs/
	@mkdir objectfiles/
	@mkdir staticlibs/

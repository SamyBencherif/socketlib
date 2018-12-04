build:
	@gcc -fPIC -c socklib.c -o socklib.o -lssl
	@gcc -shared -fPIC -Wl,-soname,libsock.so -o libsock.so socklib.o -lc -lssl
	@rm -rf *.o
	@echo Made libsock.so

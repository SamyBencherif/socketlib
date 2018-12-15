linux:
	@gcc -fPIC -c socklib.c -o socklib.o -lssl
	@gcc -shared -fPIC -Wl,-soname,libsock.so -o libsock.so socklib.o -lc -lssl
	@rm -rf *.o
	@echo Made libsock.so
mac:
	@rm -rf *.dylib
	@gcc -std=c11 -fPIC -c socklib.c -o socklib.o
	@gcc -dynamiclib -fPIC -Wl -o socklib.dylib socklib.o
	@rm -rf *.o
	@echo Made socklib.dylib	

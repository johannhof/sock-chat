CC = clang

server: server.o socket.o crypto.o ws.o -lcrypto

-lcrypto:

jsx:
	jsx -w -x jsx ./public/ ./public/

clean:
	rm *.o
	rm server

.PHONY: jsx, clean

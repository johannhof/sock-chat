CFLAGS = -std=c99

server: server.o socket.o crypto.o ws.o -lcrypto -lpthread

-lpthread:

-lcrypto:

jsx:
	jsx -w -x jsx ./public/ ./public/

clean:
	rm *.o
	rm server

.PHONY: jsx, clean

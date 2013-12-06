CC = clang

server: server.o socket.o crypto.o ws.o -lcrypto

-lcrypto:

clean:
	rm *.o
	rm server

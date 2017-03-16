
CC = gcc

all: server client

server: server
	$(CC) server.c -o server

client: client.c
	$(CC) client.c -o client
	
clean: 
	rm -f client server result.txt
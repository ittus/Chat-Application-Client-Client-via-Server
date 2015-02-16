all: client server
client: client.c
	gcc -Wall client.c -lpthread -o client -I.
server: server.c
	gcc -Wall server.c -lpthread -o server -I.
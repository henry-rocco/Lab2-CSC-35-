# Variáveis
CC = gcc
CFLAGS = -Wall -pthread -Iutils
OBJ_SERVER = server.o utils/functions.o
OBJ_CLIENT = client.o utils/functions.o

# Alvos phony
.PHONY: all clean run_server run_client

all: server client

server: $(OBJ_SERVER)
	$(CC) $(CFLAGS) -o server $(OBJ_SERVER)

client: $(OBJ_CLIENT)
	$(CC) $(CFLAGS) -o client $(OBJ_CLIENT)

server.o: server.cpp
	$(CC) $(CFLAGS) -c server.cpp -o server.o

client.o: client.cpp
	$(CC) $(CFLAGS) -c client.cpp -o client.o

utils/functions.o: utils/functions.cpp
	$(CC) $(CFLAGS) -c utils/functions.cpp -o utils/functions.o

run_server: server
	./server

run_client: client
	./client

clean:
	rm -f server client server.o client.o utils/functions.o
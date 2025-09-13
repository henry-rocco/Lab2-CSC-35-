#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void trim_newline(char* str);
ssize_t recv_line(int socket, char *buffer, size_t size);
void handle_received_bytes(int bytes_received, int socket);

#endif
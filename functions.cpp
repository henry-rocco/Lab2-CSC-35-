#include "functions.h"


void trim_newline(char* str){
    size_t len = strlen(str);
    while(len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r' || str[len - 1] == ' ')) {
        str[--len] = '\0';
    }
}



ssize_t recv_line(int socket, char *buffer, size_t size) {
    ssize_t total_received = 0;
    char c;

    while ((size_t)total_received < size - 1) {
        ssize_t bytes_received = recv(socket, &c, 1, 0);
        if (bytes_received <= 0) {
            return bytes_received;
        }
        buffer[total_received++] = c;
        if (c == '\n') {
            break;
        }
    }

    buffer[total_received] = '\0';
    return total_received;
}



void handle_received_bytes(int bytes_received, int socket) {
    if(bytes_received <= 0) {
        if(bytes_received == 0) {
            printf("Server closed the connection.\n");
        } else {
            perror("Error receiving data from server");
        }
        close(socket);
        exit(EXIT_FAILURE);
    }
}
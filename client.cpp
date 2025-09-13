#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "utils/functions.h"

#define PORT 8000
#define BUFFER_SIZE 4096

static void receive_file(int socket) {

    char buffer[BUFFER_SIZE];

    int bytes_received = recv_line(socket, buffer, BUFFER_SIZE);

    handle_received_bytes(bytes_received, socket);
    buffer[bytes_received] = '\0';
    printf("%s", buffer);

    if (strncmp(buffer, "Deu certo\n", 3) != 0) return;
    bytes_received = recv_line(socket, buffer, BUFFER_SIZE);
    handle_received_bytes(bytes_received, socket);

    buffer[bytes_received] = '\0';
    long filesize = atol(buffer);
    long total_received = 0;

    while (total_received < filesize) {
        bytes_received = recv(socket, buffer, BUFFER_SIZE, 0);
        handle_received_bytes(bytes_received, socket);

        fwrite(buffer, 1, bytes_received, stdout);

        total_received += bytes_received;
    }

    printf("\n");
}

static void receive_line(int socket) {

    char buffer[BUFFER_SIZE];

    int bytes_received = recv_line(socket, buffer, BUFFER_SIZE);

    handle_received_bytes(bytes_received, socket);

    buffer[bytes_received] = '\0';

    printf("%s", buffer);
}

void receive_response(int socket, const char* command) {
    if (strncmp(command, "MyGet ", 6) == 0) {
        receive_file(socket);
    } else {
        receive_line(socket);
    }
}

static int connect_to_server(const char* ip, uint16_t port) {

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1) {
        perror("Erro ao criar o socket.\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("Endereço inválido.\n");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao conectar ao servidor.\n");
        exit(EXIT_FAILURE);
    }
    return sock;
}

int main(int argc, char* argv[]) {
    signal(SIGPIPE, SIG_IGN);

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <ip_servidor> [porta]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* server_ip = argv[1];
    uint16_t port = (argc >= 3) ? atoi(argv[2]) : PORT;

    int sock = connect_to_server(server_ip, port);

    char command[BUFFER_SIZE];

    printf("Conectando ao servidor %s:%d\n", server_ip, port);
    printf("Use 'MyGet <file_path>' ou 'MyLastAccess'\n");

    while (1) {
        printf("> ");
        if (!fgets(command, BUFFER_SIZE, stdin)) break;
        trim_newline(command);
        strcat(command, "\n");
        if (strncmp(command, "exit", 4) == 0) break;
        if (send(sock, command, strlen(command), 0) < 0) {
            perror("Erro ao enviar comando.\n");
            break;
        }
        receive_response(sock, command);
    }

    close(sock);
    return 0;
}

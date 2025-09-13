#include <signal.h>
#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <time.h>
#include "utils/functions.h"


#define PORT 8000
#define BUFFER_SIZE 4096


typedef struct ClientInfo {
    char ip_address[INET_ADDRSTRLEN];
    unsigned short port;
    time_t last_access;
    struct ClientInfo *next;
} ClientInfo;


ClientInfo *client_list = NULL;
pthread_mutex_t client_list_mutex;


void update_client_access(const char *ip_address) {
    pthread_mutex_lock(&client_list_mutex);

    ClientInfo *current = client_list;
    time_t now = time(NULL);

    while (current != NULL) {
        if (strcmp(current->ip_address, ip_address) == 0) {
            current->last_access = now;
            pthread_mutex_unlock(&client_list_mutex);
            return;
        }
        current = current->next;
    }

    ClientInfo *new_client = (ClientInfo *)malloc(sizeof(ClientInfo));
    if (new_client != NULL) {
        strncpy(new_client->ip_address, ip_address, INET_ADDRSTRLEN-1);
        new_client->ip_address[INET_ADDRSTRLEN-1] = '\0';
        new_client->last_access = now;
        new_client->next = client_list;
        client_list = new_client;
    }

    pthread_mutex_unlock(&client_list_mutex);
}


time_t get_client_last_access(const char* ip_address) {
    pthread_mutex_lock(&client_list_mutex);

    ClientInfo* current = client_list;
    time_t last_access = 0;

    while (current != NULL) {
        if (strcmp(current->ip_address, ip_address) == 0) {
            last_access = current->last_access;
            break;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&client_list_mutex);
    return last_access;
}


static void send_msg(int sock, const char* msg) {
    send(sock, msg, strlen(msg), 0);
}


static void send_file(int sock, FILE* file, char* buffer) {
    size_t n;
    while ((n = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(sock, buffer, n, 0);
    }
}


void* handle_client(void* arg) {
    int client_socket = *(int*) arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);

    getpeername(client_socket, (struct sockaddr*)&addr, &addr_size);
    inet_ntop(AF_INET, &addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    unsigned short client_port = ntohs(addr.sin_port);


    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv_line(client_socket, buffer, BUFFER_SIZE);

        if (bytes_received <= 0) {
            if (bytes_received == 0)
                printf("Cliente desconectado: %s:%d\n", client_ip, client_port);
            else
                perror("Erro ao receber dados");
            close(client_socket);
            pthread_exit(NULL);
        }

        trim_newline(buffer);
        printf("Comando recebido de %s:%d '%s'\n", client_ip, client_port, buffer);

        if (strncmp(buffer, "MyGet ", 6) == 0) {
            const char* file_path = buffer + 6;
            FILE* file = fopen(file_path, "rb");
            if (!file) {
                send_msg(client_socket, "ERRO: Arquivo não encontrado.\n");
            } else {
                send_msg(client_socket, "OK\n");
                fseek(file, 0, SEEK_END);
                long filesize = ftell(file);
                fseek(file, 0, SEEK_SET);
                snprintf(buffer, BUFFER_SIZE, "%ld\n", filesize);
                send_msg(client_socket, buffer);
                send_file(client_socket, file, buffer);
                fclose(file);
                update_client_access(client_ip);
            }
        } else if (strncmp(buffer, "MyLastAccess", strlen("MyLastAccess")) == 0) {
            time_t last_access = get_client_last_access(client_ip);
            if (last_access == 0) {
                send_msg(client_socket, "Último acesso = Nulo\n");
            } else {
                char msg[BUFFER_SIZE];
                struct tm* tm_info = localtime(&last_access);
                strftime(msg, BUFFER_SIZE, "Último acesso=%Y-%m-%d %H:%M:%S\n", tm_info);
                send_msg(client_socket, msg);
            }
        } else {
            send_msg(client_socket, "ERRO: Comando inválido.\n");
        }
    }

    close(client_socket);
    pthread_exit(NULL);
}



static int create_server_socket() {

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Não foi possível criar o socket.\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);


    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Não foi possível fazer o bind\n");
        exit(EXIT_FAILURE);
    }

    listen(server_socket, 5);
    printf("Servidor ouvindo na porta %d\n", PORT);
    return server_socket;
}


static void accept_clients(int server_socket) {
    pthread_t thread_id;
    struct sockaddr_in client_addr;
    socklen_t client_size = sizeof(struct sockaddr_in);
    int* new_sock;

    while ((new_sock = (int*)malloc(sizeof(int)), *new_sock = accept(server_socket, (struct sockaddr*)&client_addr, &client_size)) >= 0) {
        if (pthread_create(&thread_id, NULL, handle_client, (void*)new_sock) != 0) {
            perror("Não foi possível criar uma nova thread.\n");
            free(new_sock);
        } else {
            pthread_detach(thread_id);
        }
    }

    if (*new_sock < 0) {
        perror("Erro ao aceitar conexão.\n");
        free(new_sock);
        exit(EXIT_FAILURE);
    }
}


int main() {
    signal(SIGPIPE, SIG_IGN);
    pthread_mutex_init(&client_list_mutex, NULL);

    int server_socket = create_server_socket();
    accept_clients(server_socket);

    close(server_socket);
    pthread_mutex_destroy(&client_list_mutex);
    return 0;
}
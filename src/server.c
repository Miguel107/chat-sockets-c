#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT        9000
#define MAX_CLIENTS 100
#define BUF_SIZE    1024

typedef struct {
    int  sockfd;
    char username[32];
} client_t;

static client_t *clients[MAX_CLIENTS];
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
static FILE *log_file;

// Loguea un evento con timestamp en logs/chat.log
void log_event(const char *format, ...) {
    time_t now = time(NULL);
    char tbuf[64];
    strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    pthread_mutex_lock(&clients_mutex);
    fprintf(log_file, "[%s] ", tbuf);
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    fflush(log_file);
    pthread_mutex_unlock(&clients_mutex);
}

void add_client(client_t *cl) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (!clients[i]) {
            clients[i] = cl;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(client_t *cl) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] == cl) {
            clients[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void broadcast_message(const char *message) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if (send(clients[i]->sockfd, message, strlen(message), 0) < 0) {
                perror("send");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    client_t *cli = (client_t *)arg;
    char buf[BUF_SIZE];
    int  nbytes;

    // 1) Pedir nombre de usuario
    send(cli->sockfd, "Usuario: ", 9, 0);
    if ((nbytes = recv(cli->sockfd, cli->username, sizeof(cli->username)-1, 0)) <= 0) {
        close(cli->sockfd);
        free(cli);
        pthread_detach(pthread_self());
        return NULL;
    }
    cli->username[nbytes-1] = '\0';  // elimina '\n'

    log_event("%s se ha conectado\n", cli->username);
    snprintf(buf, sizeof(buf), "%s se ha unido al chat\n", cli->username);
    broadcast_message(buf);

    // 2) Bucle de recepción y retransmisión
    while ((nbytes = recv(cli->sockfd, buf, sizeof(buf)-1, 0)) > 0) {
        buf[nbytes] = '\0';
        char message[BUF_SIZE + 64];
        snprintf(message, sizeof(message), "%s: %s", cli->username, buf);
        log_event("%s", message);
        broadcast_message(message);
    }

    // 3) Desconexión
    close(cli->sockfd);
    remove_client(cli);
    snprintf(buf, sizeof(buf), "%s ha salido del chat\n", cli->username);
    log_event("%s", buf);
    broadcast_message(buf);

    free(cli);
    pthread_detach(pthread_self());
    return NULL;
}

int main() {
    int listenfd, connfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    // Abrir archivo de log
    log_file = fopen("logs/chat.log", "a");
    if (!log_file) { perror("fopen"); exit(EXIT_FAILURE); }

    // Crear socket TCP
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket"); exit(EXIT_FAILURE);
    }

    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(PORT);

    // Asociar socket
    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones
    if (listen(listenfd, 10) < 0) {
        perror("listen");
        close(listenfd);
        exit(EXIT_FAILURE);
    }
    printf("Servidor escuchando en puerto %d\n", PORT);

    // Bucle principal: aceptar clientes
    while (1) {
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &cli_len);
        if (connfd < 0) {
            perror("accept");
            continue;
        }

        client_t *cli = malloc(sizeof(client_t));
        cli->sockfd = connfd;
        memset(cli->username, 0, sizeof(cli->username));

        add_client(cli);

        pthread_t tid;
        if (pthread_create(&tid, NULL, &handle_client, cli) != 0) {
            perror("pthread_create");
            free(cli);
        }
    }

    fclose(log_file);
    close(listenfd);
    return 0;
}

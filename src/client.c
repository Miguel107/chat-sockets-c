#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 1024

static int sockfd;

// Hilo de recepción de mensajes
void *recv_handler(void *arg) {
    char buf[BUF_SIZE];
    int  nbytes;
    while ((nbytes = recv(sockfd, buf, sizeof(buf)-1, 0)) > 0) {
        buf[nbytes] = '\0';
        printf("%s", buf);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in serv_addr;
    pthread_t recv_tid;

    if (argc != 3) {
        fprintf(stderr, "Uso: %s <IP_servidor> <PUERTO>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Crear y conectar socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket"); return EXIT_FAILURE;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        perror("inet_pton"); return EXIT_FAILURE;
    }
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect"); return EXIT_FAILURE;
    }

    // Enviar nombre de usuario
    char prompt[32];
    recv(sockfd, prompt, sizeof(prompt)-1, 0);
    printf("%s", prompt);
    char username[32];
    fgets(username, sizeof(username), stdin);
    send(sockfd, username, strlen(username), 0);

    // Iniciar hilo de recepción
    pthread_create(&recv_tid, NULL, recv_handler, NULL);

    // Bucle de envío de mensajes
    char buf[BUF_SIZE];
    while (fgets(buf, sizeof(buf), stdin)) {
        if (strncmp(buf, "exit", 4) == 0) break;
        send(sockfd, buf, strlen(buf), 0);
    }

    close(sockfd);
    pthread_cancel(recv_tid);
    return 0;
}

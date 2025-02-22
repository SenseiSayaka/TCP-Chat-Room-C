#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int sock = 0;

void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    int valread;
    while (1) {
        valread = read(sock, buffer, BUFFER_SIZE - 1);
        if (valread > 0) {
            buffer[valread] = '\0';
            printf("\n%s\n", buffer);
            
            printf("Введите сообщение или quit для выхода: ");
            fflush(stdout);
        }
    }
    return NULL;
}

int main() {
    struct sockaddr_in serv_addr;

    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Ошибка создания сокета");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Неверный адрес / адрес не поддерживается");
        return -1;
    }

    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Ошибка подключения");
        return -1;
    }

    printf("Подключено к серверу 127.0.0.1:%d\n", PORT);

    
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, receive_messages, NULL);

    char buffer[BUFFER_SIZE];
    while(1) {
        
        printf("Введите сообщение или quit для выхода: ");
        fflush(stdout);
        if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
            
            if (strcmp(buffer, "quit\n") == 0 || strcmp(buffer, "quit") == 0) {
                printf("Вы вышли из чата.\n");
                break;
            }
            send(sock, buffer, strlen(buffer), 0);
        }
    }

    close(sock);
    return 0;
}

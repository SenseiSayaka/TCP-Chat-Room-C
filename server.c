#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>

#define PORT 12345
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket, activity, i, valread, sd;
    int client_socket[MAX_CLIENTS];
    int client_id[MAX_CLIENTS];  // идентификаторы клиентов
    int next_id = 1;             // следующий идентификатор
    int max_sd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    fd_set readfds;

    // Инициализация массивов для клиентов
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_socket[i] = 0;
        client_id[i] = 0;
    }

    // Создание серверного сокета
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Опция переиспользования адреса
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // слушаем на всех интерфейсах
    address.sin_port = htons(PORT);

    // Привязка сокета
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Сервер запущен. Слушаем порт %d...\n", PORT);

    // Прослушивание входящих подключений
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(1) {
        // Очистка множества сокетов
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        // Добавляем все клиентские сокеты
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];
            if(sd > 0)
                FD_SET(sd, &readfds);
            if(sd > max_sd)
                max_sd = sd;
        }

        // Ожидание активности
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }

        // Новое подключение
        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // Поиск свободного места в массиве
            for (i = 0; i < MAX_CLIENTS; i++) {
                if(client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    client_id[i] = next_id++;
                    printf("Новый клиент подключился: Пользователь %d, socket fd = %d, IP = %s, PORT = %d\n",
                           client_id[i], new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    break;
                }
            }
        }

        // Проверка клиентских сокетов
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];
            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                if ((valread = read(sd, buffer, BUFFER_SIZE - 1)) <= 0) {
                    // Клиент отключился (или ошибка чтения)
                    getpeername(sd, (struct sockaddr*)&address, &addrlen);
                    printf("Пользователь %d вышел из чата\n", client_id[i]);
                    close(sd);
                    client_socket[i] = 0;
                    client_id[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    // Если клиент отправил команду выхода
                    if (strcmp(buffer, "quit\n") == 0 || strcmp(buffer, "quit") == 0) {
                        printf("Пользователь %d вышел из чата (quit)\n", client_id[i]);
                        close(sd);
                        client_socket[i] = 0;
                        client_id[i] = 0;
                        continue;
                    }
                    // Логирование сообщения
                    printf("Пользователь %d написал: %s", client_id[i], buffer);
                    
                    // Формируем сообщение с именем пользователя для рассылки
                    char out_buffer[BUFFER_SIZE + 50];
                    snprintf(out_buffer, sizeof(out_buffer), "Пользователь %d: %s", client_id[i], buffer);
                    
                    // Отправляем сообщение всем остальным клиентам
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (client_socket[j] != 0 && client_socket[j] != sd) {
                            send(client_socket[j], out_buffer, strlen(out_buffer), 0);
                        }
                    }
                }
            }
        }
    }
    return 0;
}

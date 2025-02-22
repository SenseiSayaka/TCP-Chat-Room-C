# Документация проекта TCP Chat Room на языке C

Данный документ описывает проект *TCP Chat Room*, состоящий из двух файлов: `client.c` и `server.c`. В документе приведён подробный разбор кода, а также процесс сборки, установки и запуска проекта.

---

## Содержание

- [Описание проекта](#описание-проекта)
- [Процесс сборки, установки и запуска](#процесс-сборки-установки-и-запуска)
- [Разбор кода](#разбор-кода)
  - [Файл: client.c](#файл-clientc)
  - [Файл: server.c](#файл-serverc)

---

## Описание проекта

Проект реализует простой TCP-чат на языке C. Он состоит из двух частей:
- **Клиентская часть (`client.c`)**: Клиент подключается к серверу, отправляет сообщения и в асинхронном режиме принимает входящие сообщения с помощью потоков.
- **Серверная часть (`server.c`)**: Сервер принимает подключения нескольких клиентов, принимает их сообщения и пересылает их другим подключенным пользователям.

---

## Процесс сборки, установки и запуска

### Требования

- Компилятор GCC (или другой компилятор для языка С).
- POSIX-система (Linux, macOS или другая Unix-подобная ОС).

### Сборка

1. Склонируйте репозиторий проекта:
   ```bash
   git clone https://github.com/SenseiSayaka/TCP-Chat-Room-C.git
   cd TCP-Chat-Room-C
   ```

2. Скомпилируйте сервер:
   ```bash
   gcc server.c -o server
   ```

3. Скомпилируйте клиента (обязательно подключите поддержку потоков):
   ```bash
   gcc client.c -o client -pthread
   ```

### Установка

Проект не требует дополнительной установки. Достаточно скомпилированных бинарных файлов (сервер и клиент) для запуска.

### Запуск

1. Запустите сервер:
   ```bash
   ./server
   ```
   Сервер начнёт прослушивание порта 12345, ожидая подключений.

2. В отдельном терминале запустите клиент:
   ```bash
   ./client
   ```
   Клиент подключится к серверу и вы сможете отправлять и получать сообщения.

3. Для выхода из чата введите команду:
   ```
   quit
   ```

---

## Разбор кода

Ниже приведён детальный построчный разбор кода для каждого файла.

### Файл: client.c

```c
#include <stdio.h>      // Подключает стандартную библиотеку ввода/вывода, предоставляющую функции printf, scanf и др.
#include <stdlib.h>     // Подключает стандартную библиотеку для управления памятью, работы с процессом (exit) и др.
#include <string.h>     // Подключает библиотеку для работы со строками: функции strlen, strcmp, strcpy и т.д.
#include <unistd.h>     // Подключает POSIX-функции, такие как read(), write(), close().
#include <arpa/inet.h>  // Подключает функции для преобразования IP-адресов (например, inet_pton) и работу с сетевыми структурами.
#include <sys/socket.h> // Подключает функции для работы с сокетами (socket(), connect(), send(), recv() и др.).
#include <pthread.h>    // Подключает библиотеку для работы с потоками (pthread_create, pthread_join и т.д.).

#define PORT 12345          // Определяет макрос PORT со значением 12345, используемое для подключения к серверу.
#define BUFFER_SIZE 1024    // Определяет макрос BUFFER_SIZE, задающий размер буфера для отправки и приема сообщений.

int sock = 0;  // Глобальная переменная для хранения файлового дескриптора сокета клиента.

void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];  // Буфер для приема сообщений размером BUFFER_SIZE.
    int valread;               // Переменная для хранения количества прочитанных байт.
    
    while (1) {  // Бесконечный цикл для постоянного чтения сообщений с сервера.
        valread = read(sock, buffer, BUFFER_SIZE - 1);  // Чтение данных из сокета, оставляя место для символа завершения строки.
        if (valread > 0) {  // Если данные успешно прочитаны.
            buffer[valread] = '\0';  // Завершение полученной строки нулевым символом.
            printf("\n%s\n", buffer);  // Вывод полученного сообщения на экран.
            
            printf("Введите сообщение или quit для выхода: ");  // Вывод приглашения для повторного ввода.
            fflush(stdout);  // Принудительный сброс буфера вывода.
        }
    }
    return NULL;  // Функция возвращает NULL. На практике цикл бесконечен.
}

int main() {
    struct sockaddr_in serv_addr;  // Структура для хранения адреса сервера.

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {  // Создание TCP сокета. При ошибке вывод сообщения и завершение.
        perror("Ошибка создания сокета");
        return -1;
    }

    serv_addr.sin_family = AF_INET;  // Указывает, что используется IPv4.
    serv_addr.sin_port = htons(PORT);  // Устанавливает порт сервера с преобразованием в сетевой порядок байтов.

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {  // Преобразование строкового IP в числовой формат.
        perror("Неверный адрес / адрес не поддерживается");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {  // Подключение к серверу.
        perror("Ошибка подключения");
        return -1;
    }

    printf("Подключено к серверу 127.0.0.1:%d\n", PORT);  // Сообщение об успешном подключении.

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, receive_messages, NULL);  // Создание потока для асинхронного приема сообщений.

    char buffer[BUFFER_SIZE];  // Локальный буфер для ввода сообщений.
    while(1) {  // Цикл для отправки сообщений.
        printf("Введите сообщение или quit для выхода: ");  // Приглашение ко вводу сообщения.
        fflush(stdout);
        if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {  // Считывание строки с ввода.
            if (strcmp(buffer, "quit\n") == 0 || strcmp(buffer, "quit") == 0) {  // Проверка команды выхода.
                printf("Вы вышли из чата.\n");
                break;
            }
            send(sock, buffer, strlen(buffer), 0);  // Отправка введенного сообщения на сервер.
        }
    }

    close(sock);  // Закрытие сокета после завершения работы.
    return 0;  // Завершение программы.
}
```

### Файл: server.c

```c
#include <stdio.h>       // Подключение стандартной библиотеки ввода/вывода.
#include <stdlib.h>      // Библиотека для работы с памятью и утилитами.
#include <string.h>      // Функции для работы со строками.
#include <unistd.h>      // Функции POSIX, такие как read(), write(), close().
#include <errno.h>       // Предоставляет переменную errno и определения для обработки ошибок.
#include <arpa/inet.h>   // Функции для работы с интернет-адресами.
#include <netinet/in.h>  // Структуры и константы, связанные с интернет-протоколами.
#include <sys/socket.h>  // Функции и структуры для работы с сокетами.
#include <sys/types.h>   // Системные типы данных, используемые в вызовах.
#include <sys/select.h>  // Функции и определения для работы с мультиплексированным вводом/выводом (select).

#define PORT 12345          // Макрос, определяющий порт, на котором работает сервер.
#define MAX_CLIENTS 10      // Максимальное количество одновременно подключенных клиентов.
#define BUFFER_SIZE 1024    // Размер буфера для чтения и записи сообщений.

int main() {
    int server_fd, new_socket, activity, i, valread, sd;
    int client_socket[MAX_CLIENTS];  // Массив для хранения файловых дескрипторов клиентских сокетов.
    int client_id[MAX_CLIENTS];      // Массив для хранения идентификаторов подключенных клиентов.
    int next_id = 1;                 // Счетчик для присвоения нового уникального идентификатора клиенту.
    int max_sd;                      // Переменная для хранения максимального дескриптора.
    struct sockaddr_in address;      // Структура для хранения адреса сервера и клиентов.
    socklen_t addrlen = sizeof(address);  // Размер структуры адреса.
    char buffer[BUFFER_SIZE];        // Буфер для получения сообщений.

    fd_set readfds;  // Множество файловых дескрипторов для функции select.

    // Инициализация массивов для клиентских сокетов и идентификаторов.
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_socket[i] = 0;
        client_id[i] = 0;
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {  // Создание серверного TCP сокета.
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {  // Включение опции переиспользования адреса.
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;             // Использование IPv4.
    address.sin_addr.s_addr = INADDR_ANY;       // Принимаем соединения на любом доступном IP адресе.
    address.sin_port = htons(PORT);             // Установка порта с преобразованием в сетевой байтовый порядок.

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {  // Привязка сокета к адресу.
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Сервер запущен. Слушаем порт %d...\n", PORT);  // Вывод сообщения о запуске сервера.

    if (listen(server_fd, 3) < 0) {  // Перевод сокета в режим прослушивания входящих подключений.
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(1) {  // Главный цикл для отслеживания активности.
        FD_ZERO(&readfds);  // Очистка множества дескрипторов.
        FD_SET(server_fd, &readfds);  // Добавление серверного сокета для приёма новых подключений.
        max_sd = server_fd;

        // Добавление активных клиентских сокетов в множество.
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];
            if(sd > 0)
                FD_SET(sd, &readfds);
            if(sd > max_sd)
                max_sd = sd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);  // Ожидание активности на одном из сокетов.
        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");  // Обработка ошибки select.
        }

        if (FD_ISSET(server_fd, &readfds)) {  // Проверка активности на серверном сокете.
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // Поиск свободного места для нового клиента.
            for (i = 0; i < MAX_CLIENTS; i++) {
                if(client_socket[i] == 0) {
                    client_socket[i] = new_socket;  // Сохранение нового дескриптора клиента.
                    client_id[i] = next_id++;         // Присвоение уникального идентификатора.
                    printf("Новый клиент подключился: Пользователь %d, socket fd = %d, IP = %s, PORT = %d\n",
                           client_id[i], new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    break;
                }
            }
        }

        // Обработка сообщений от подключенных клиентов.
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];
            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                if ((valread = read(sd, buffer, BUFFER_SIZE - 1)) <= 0) {  // Проверка, отключился ли клиент.
                    getpeername(sd, (struct sockaddr*)&address, &addrlen);  // Получение адреса клиента для логирования.
                    printf("Пользователь %d вышел из чата\n", client_id[i]);
                    close(sd);              // Закрытие сокета клиента.
                    client_socket[i] = 0;
                    client_id[i] = 0;
                } else {
                    buffer[valread] = '\0';  // Завершение строки нулевым символом.
                    if (strcmp(buffer, "quit\n") == 0 || strcmp(buffer, "quit") == 0) {  // Проверка команды выхода.
                        printf("Пользователь %d вышел из чата (quit)\n", client_id[i]);
                        close(sd);
                        client_socket[i] = 0;
                        client_id[i] = 0;
                        continue;
                    }
                    printf("Пользователь %d написал: %s", client_id[i], buffer);  // Логирование полученного сообщения.
                    
                    char out_buffer[BUFFER_SIZE + 50];  // Формирование сообщения для рассылки с идентификатором.
                    snprintf(out_buffer, sizeof(out_buffer), "Пользователь %d: %s", client_id[i], buffer);
                    
                    // Рассылка сообщения всем остальным подключенным клиентам.
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (client_socket[j] != 0 && client_socket[j] != sd) {
                            send(client_socket[j], out_buffer, strlen(out_buffer), 0);
                        }
                    }
                }
            }
        }
    }
    return 0;  // Теоретически, выполнение программы никогда не доходит до этой строки.
}
```

---

## Заключение

Проект *TCP Chat Room* демонстрирует базовый пример работы с сокетами в Unix-подобных операционных системах, позволяя пользователям обмениваться текстовыми сообщениями в реальном времени. В дополнение к подробному разбору кода, этот документ описывает процесс сборки, установки и запуска приложения, что делает его понятным для дальнейшей эксплуатации и расширения функциональности.
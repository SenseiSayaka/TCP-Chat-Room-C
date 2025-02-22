# Документация проекта TCP Chat Room на языке C

Данный файл содержит подробное описание работы проекта *TCP Chat Room*, состоящего из двух файлов: `client.c` и `server.c`. Ниже приведён поэтапный разбор каждой строки кода с пояснениями функциональности.

---

## Файл: client.c

Этот файл реализует клиентскую часть чата. Клиент подключается к серверу, отправляет и получает сообщения, используя потоки для асинхронного приема данных.

### Разбор кода

1. **Подключение необходимых заголовочных файлов:**
   ```c
   #include <stdio.h>      // Стандартная библиотека ввода/вывода.
   #include <stdlib.h>     // Функции для работы с памятью, системные утилиты.
   #include <string.h>     // Функции для работы со строками.
   #include <unistd.h>     // Функции POSIX, например, для работы с файловыми дескрипторами.
   #include <arpa/inet.h>  // Функции для работы с IP-адресами и преобразованием их представления.
   #include <sys/socket.h> // Функции для работы с сокетами.
   #include <pthread.h>    // Поддержка потоков (POSIX threads) для параллельного выполнения.
   ```
   Каждая библиотека обеспечивает различные функции, необходимые для работы с сетью, потоками и стандартным вводом/выводом.

2. **Определение констант:**
   ```c
   #define PORT 12345
   #define BUFFER_SIZE 1024
   ```
   - `PORT`: номер порта, на котором происходит соединение с сервером.
   - `BUFFER_SIZE`: размер буфера для отправки/приема сообщений.

3. **Глобальная переменная для сокета:**
   ```c
   int sock = 0;
   ```
   Переменная `sock` используется для хранения файлового дескриптора созданного сетевого сокета.

4. **Функция для приема сообщений (поток):**
   ```c
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
   ```
   - Функция предназначена для работы в отдельном потоке.
   - Постоянно читает данные из сокета.
   - При успешном чтении завершает строку нулевым символом и выводит сообщение пользователю.
   - Повторно запрашивает ввод сообщения, чтобы пользователь знал, что можно продолжать ввод.

5. **Функция `main`: точка входа клиента:**
   ```c
   int main() {
       struct sockaddr_in serv_addr;
   ```
   - Объявляется структура `serv_addr`, которая хранит адрес сервера для соединения.

6. **Создание сокета:**
   ```c
       if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
           perror("Ошибка создания сокета");
           return -1;
       }
   ```
   - Создается TCP сокет (`SOCK_STREAM`).
   - В случае ошибки выводится сообщение и программа завершается.

7. **Настройка адреса сервера:**
   ```c
       serv_addr.sin_family = AF_INET;
       serv_addr.sin_port = htons(PORT);
   ```
   - `sin_family` задается как `AF_INET` для работы с IPv4.
   - `sin_port` устанавливается в заданный порт, при этом используется `htons` для преобразования в сетевой порядок байт.

8. **Преобразование IP-адреса и его установка:**
   ```c
       if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
           perror("Неверный адрес / адрес не поддерживается");
           return -1;
       }
   ```
   - Функция `inet_pton` конвертирует строковый IP-адрес в числовое представление.
   - Проверяется корректность конвертации.

9. **Подключение к серверу:**
   ```c
       if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
           perror("Ошибка подключения");
           return -1;
       }
   ```
   - Функция `connect` устанавливает соединение с сервером по заданному адресу.
   - При ошибке соединения выводится сообщение и программа завершается.

10. **Уведомление о подключении:**
    ```c
       printf("Подключено к серверу 127.0.0.1:%d\n", PORT);
    ```
    - Выводится сообщение об успешном подключении к серверу.

11. **Запуск потока для получения сообщений:**
    ```c
       pthread_t thread_id;
       pthread_create(&thread_id, NULL, receive_messages, NULL);
    ```
    - Создается новый поток, который выполняет функцию `receive_messages`, позволяя одновременно принимать сообщения от сервера и вводить их.

12. **Цикл для отправки сообщений:**
    ```c
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
    ```
    - В цикле запрашивается ввод сообщения от пользователя.
    - Если ввод совпадает со строкой "quit" (с переводом строки или без), цикл завершается и клиент выходит.
    - Иначе, сообщение отправляется на сервер посредством сокета.

13. **Закрытие сокета и завершение программы:**
    ```c
       close(sock);
       return 0;
    }
    ```
    - После выхода из цикла происходит закрытие соединения.
    - Программа завершает работу с кодом возврата 0.

---

## Файл: server.c

Реализует серверную часть чата. Сервер принимает подключения нескольких клиентов, получает и пересылает сообщения между ними.

### Разбор кода

1. **Подключение необходимых заголовочных файлов:**
   ```c
   #include <stdio.h>       // Стандартная библиотека ввода/вывода.
   #include <stdlib.h>      // Функции для работы с памятью и утилиты.
   #include <string.h>      // Функции для работы со строками.
   #include <unistd.h>      // Функции POSIX, например, работа с файловыми дескрипторами.
   #include <errno.h>       // Отслеживание ошибок, определение errno.
   #include <arpa/inet.h>   // Функции для работы с IP-адресами.
   #include <netinet/in.h>  // Структуры для представления интернет-адресов.
   #include <sys/socket.h>  // Функции и структуры для работы с сокетами.
   #include <sys/types.h>   // Типы данных, используемые в системных вызовах.
   #include <sys/select.h>  // Функции для мониторинга множественных файловых дескрипторов.
   ```
   Каждый заголовочный файл отвечает за свои системные вызовы и функции, необходимые для организации сетевого взаимодействия.

2. **Определение констант:**
   ```c
   #define PORT 12345
   #define MAX_CLIENTS 10
   #define BUFFER_SIZE 1024
   ```
   - `PORT`: номер порта, на котором сервер прослушивает подключения.
   - `MAX_CLIENTS`: максимальное количество одновременно подключенных клиентов.
   - `BUFFER_SIZE`: размер буфера для обмена сообщениями.

3. **Главная функция `main`: точка входа сервера:**
   ```c
   int main() {
       int server_fd, new_socket, activity, i, valread, sd;
       int client_socket[MAX_CLIENTS];
       int client_id[MAX_CLIENTS];  // идентификаторы клиентов
       int next_id = 1;             // следующий идентификатор
       int max_sd;
       struct sockaddr_in address;
       socklen_t addrlen = sizeof(address);
       char buffer[BUFFER_SIZE];
   ```
   - Объявление переменных для работы с серверным сокетом, клиентскими сокетами, идентификаторами клиентов и массив для получения данных.
   - `client_id` используется для присвоения пользователям уникальных номеров.
   - `next_id` отслеживает следующий доступный идентификатор.

4. **Инициализация множества дескрипторов для работы с `select`:**
   ```c
       fd_set readfds;
   ```
   - `readfds` — набор файловых дескрипторов, которые будут отслеживаться на возможность чтения.

5. **Инициализация массивов для клиентов:**
   ```c
       // Инициализация массивов для клиентов
       for (i = 0; i < MAX_CLIENTS; i++) {
           client_socket[i] = 0;
           client_id[i] = 0;
       }
   ```
   - Заполнение массивов нулями, что означает отсутствие подключенных клиентов на момент старта сервера.

6. **Создание серверного сокета:**
   ```c
       // Создание серверного сокета
       if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
           perror("socket failed");
           exit(EXIT_FAILURE);
       }
   ```
   - Создается TCP сокет.
   - При неудачном создании выводится сообщение об ошибке, и программа завершается с ошибкой.

7. **Настройка опции переиспользования адреса:**
   ```c
       // Опция переиспользования адреса
       int opt = 1;
       if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
           perror("setsockopt");
           exit(EXIT_FAILURE);
       }
   ```
   - Параметр `SO_REUSEADDR` позволяет повторно использовать адрес, чтобы можно было перезапустить сервер без ожидания освобождения порта.

8. **Настройка структуры адреса:**
   ```c
       // Настройка адреса
       address.sin_family = AF_INET;
       address.sin_addr.s_addr = INADDR_ANY;  // слушаем на всех интерфейсах
       address.sin_port = htons(PORT);
   ```
   - `sin_family` задается как `AF_INET` для IPv4.
   - `sin_addr.s_addr = INADDR_ANY` позволяет серверу принимать соединения на любом локальном IP-адресе.
   - `sin_port` преобразуется в сетевой порядок байт.

9. **Привязка сокета к адресу:**
   ```c
       // Привязка сокета
       if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
           perror("bind failed");
           exit(EXIT_FAILURE);
       }
       printf("Сервер запущен. Слушаем порт %d...\n", PORT);
   ```
   - Функция `bind` связывает созданный сокет с заданным адресом и портом.
   - При успешном выполнении сервер выводит сообщение о запуске.

10. **Прослушивание входящих подключений:**
    ```c
       // Прослушивание входящих подключений
       if (listen(server_fd, 3) < 0) {
           perror("listen");
           exit(EXIT_FAILURE);
       }
    ```
    - Функция `listen` переводит сокет в режим прослушивания входящих соединений.
    - Аргумент `3` задает размер очереди ожидающих подключений.

11. **Главный цикл сервера для обработки активности:**
    ```c
       while(1) {
           // Очистка множества сокетов
           FD_ZERO(&readfds);
           FD_SET(server_fd, &readfds);
           max_sd = server_fd;
    ```
    - Начало бесконечного цикла для постоянного отслеживания активности.
    - Множество дескрипторов `readfds` очищается с помощью `FD_ZERO`.
    - Добавляется серверный сокет для отслеживания новых подключений.
    - В переменную `max_sd` записывается максимальный дескриптор для функции `select`.

12. **Добавление клиентских сокетов в множество:**
    ```c
           // Добавляем все клиентские сокеты
           for (i = 0; i < MAX_CLIENTS; i++) {
               sd = client_socket[i];
               if(sd > 0)
                   FD_SET(sd, &readfds);
               if(sd > max_sd)
                   max_sd = sd;
           }
    ```
    - Для каждого подключенного клиента, если сокет не равен 0, добавляем его в множество.
    - Производится обновление максимального значения дескриптора `max_sd`.

13. **Ожидание активности на одном из сокетов:**
    ```c
           // Ожидание активности
           activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
           if ((activity < 0) && (errno != EINTR)) {
               perror("select error");
           }
    ```
    - Функция `select` блокируется до появления активности на любом из дескрипторов.
    - Если функция возвращает отрицательное значение, выводится сообщение об ошибке (если не было прерывание сигнала).

14. **Обработка нового подключения:**
    ```c
           // Новое подключение
           if (FD_ISSET(server_fd, &readfds)) {
               if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                   perror("accept");
                   exit(EXIT_FAILURE);
               }
    ```
    - Проверяется, имеется ли активность на серверном сокете.
    - Если активность обнаружена, вызывается функция `accept` для установления нового соединения.
    - При успешном вызове возвращается дескриптор нового сокета.

15. **Назначение нового клиента и уведомление:**
    ```c
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
    ```
    - Перебором ищется свободное место для хранения дескриптора нового клиента.
    - Клиенту присваивается уникальный номер (`client_id`) и выводится информация о подключении.

16. **Обработка активности на клиентских сокетах:**
    ```c
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
    ```
    - Для каждого активного клиентского сокета вызывается `read` для получения сообщения.
    - Если `read` возвращает 0 или отрицательное значение, это означает, что клиент отключился или произошла ошибка — сервер выводит сообщение об отключении, закрывает сокет и удаляет его из списка клиентов.
    - Если сообщение получено:
      - Завершается строка нулевым символом.
      - Если сообщение содержит команду выхода (`quit`), выполняется аналогичная процедура отключения.
      - В противном случае, сервер логирует сообщение.
      - Формируется строка, содержащая идентификатор пользователя и текст сообщения.
      - Эта строка отправляется всем остальным подключенным клиентам.

---

## Общие замечания по работе проекта

- **Архитектура:**
  - Клиентская часть осуществляет асинхронное получение сообщений с помощью потоков, позволяя одновременно отправлять и получать данные.
  - Сервер реализует модель с использованием функции `select`, которая позволяет обрабатывать множественные сокеты в одном потоке, что упрощает масштабирование для ограниченного числа клиентов.

- **Протокол обмена:**
  - Простое текстовое общение, где сообщения отправляются в виде строк.
  - Спецкоманда "quit" используется для выхода из чата.

- **Управление соединениями:**
  - Сервер поддерживает до 10 подключений клиентов одновременно.
  - Клиенты идентифицируются уникальными номерами для удобства логирования и рассылки сообщений.

- **Параллелизм:**
  - Клиент запускает дополнительный поток для приема сообщений, что позволяет не блокировать основной цикл отправки сообщений.

Эта документация должна помочь понять принцип работы проекта и детально разобрать каждую строку кода, обеспечивая полное представление о внутренней логике работы чата.
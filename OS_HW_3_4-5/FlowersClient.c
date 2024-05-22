#include "DieWithError.c"
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <pthread.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <unistd.h>     /* for close() */

#define RCVBUFSIZE 32  /* Size of receive buffer */
#define FLOWERS_NUM 10 /* Size of receive buffer */

// Процессы жизни цветов
void flower_proc(int id, unsigned short servPort, char* servIP) {
    // Создададим 10 процессов жизни цветка
    if (id < FLOWERS_NUM) {
        int pid = fork();
        if (pid < 0) {
            printf("Can\'t fork\n");
            exit(-1);
        }
        else if (pid == 0) {
            // Child
            flower_proc(id + 1, servPort, servIP);
            return;
        }
    }

    int sock;                      /* Socket descriptor */
    struct sockaddr_in servAddres; /* Echo server address */

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&servAddres, 0, sizeof(servAddres));     /* Zero out structure */
    servAddres.sin_family = AF_INET;                /* Internet address family */
    servAddres.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
    servAddres.sin_port = htons(servPort);          /* Server port */

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr*)&servAddres, sizeof(servAddres)) < 0)
        DieWithError("connect() failed");

    /* SEND CLNT TYPE TO GET IDENTIFIED BY SERVER */
    if (send(sock, (int[]) { 1 }, sizeof(int), 0) != sizeof(int))
        DieWithError("send() sent a different number of bytes than expected");

    int buffer[FLOWERS_NUM]; // Буффер, который в дальнеём буду использовать для
    int recvMsgSize;
    // принятия
    // сообщений от сервера

    //......................................................................................

    srand(time(NULL) + id);
    int flag = 1;
    while (flag) {
        sleep(rand() % 10 + 1);
        // Cообщаю серверу, что готов к приёму
        if (send(sock, (int[]) { 1 }, sizeof(int), 0) != sizeof(int))
            DieWithError("send() sent a different number of bytes than expected");

        /* get flowers status */
        if ((recvMsgSize = recv(sock, buffer, sizeof(int) * FLOWERS_NUM, 0)) < 0)
            DieWithError("recv() failed");
        /*for (int i = 0; i < FLOWERS_NUM; ++i) {
            printf("%d ", buffer[i]);
        }
        printf("\n");*/

        if (buffer[id - 1] == 0) {
            ++buffer[id - 1];
            printf("Flower %d is thirsty\n", id);
        }
        else { // Если shar_list[id] < 0, то цветок полили несколько раз (такого
            // по идее быть не может) и он умирает. Если shar_list[id] > 0, то
            // цветок не полили вовремя и он тоже умирает. В обоих случаях
            // ставим ему -1 (индикатор смерти цветка для садовника)
            buffer[id - 1] = -1;
            printf("Flower %d has died\n", id);
            //// Cообщаю серверу, что цветок умер
            //if (send(sock, (int[]) { 0 }, sizeof(int), 0) != sizeof(int))
            //    DieWithError("send() sent a different number of bytes than expected");
            flag = 0;
        }

        /*for (int i = 0; i < FLOWERS_NUM; ++i) {
            printf("%d ", buffer[i]);
        }
        printf("\n");*/
        /* Отправляю инфу о цветках */
        if (send(sock, buffer, sizeof(int) * FLOWERS_NUM, 0) !=
            sizeof(int) * FLOWERS_NUM)
            DieWithError("send() sent a different number of bytes than expected");
    }
    close(sock);
    exit(0);
}

int main(int argc, char* argv[]) {
    unsigned short servPort; /* Echo server port */
    char* servIP;            /* Server IP address (dotted quad) */

    if (argc != 3) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n",
            argv[0]);
        exit(1);
    }

    servIP = argv[1];         /* First arg: server IP address (dotted quad) */
    servPort = atoi(argv[2]); /* Use given port */
    flower_proc(1, servPort, servIP);
    return 0;
}

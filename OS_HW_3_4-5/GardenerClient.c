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

int main(int argc, char* argv[]) {
    int sock;                      /* Socket descriptor */
    struct sockaddr_in servAddr;   /* Echo server address */
    unsigned short servPort;       /* Echo server port */
    char* servIP;                  /* Server IP address (dotted quad) */
    char echoBuffer[RCVBUFSIZE];   /* Buffer for echo string */
    int bytesRcvd, totalBytesRcvd; /* Bytes read in single recv()
                                      and total bytes read */

    if (argc != 3) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n",
            argv[0]);
        exit(1);
    }

    servIP = argv[1];         /* First arg: server IP address (dotted quad) */
    servPort = atoi(argv[2]); /* Use given port */

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&servAddr, 0, sizeof(servAddr));       /* Zero out structure */
    servAddr.sin_family = AF_INET;                /* Internet address family */
    servAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
    servAddr.sin_port = htons(servPort);          /* Server port */

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
        DieWithError("connect() failed");

    /* SEND CLNT TYPE TO GET IDENTIFIED BY SERVER */
    if (send(sock, (int[]) { 0 }, sizeof(int), 0) != sizeof(int))
        DieWithError("send() sent a different number of bytes than expected");

    /* get gardener id */
    int buffer[FLOWERS_NUM]; // Буффер, который в дальнеём буду использовать для
    // принятия
    // сообщений от сервера
    int recvMsgSize; /* Size of received message */
    if ((recvMsgSize = recv(sock, buffer, sizeof(int), 0)) < 0)
        DieWithError("recv() failed");

    //......................................................................................

    int gardener_id = buffer[0];
    for (;;) {
        int dead_flowers_num = 0;
        // Cообщаю серверу, что готов к приёму
        if (send(sock, (int[]) { 1 }, sizeof(int), 0) != sizeof(int))
            DieWithError("send() sent a different number of bytes than expected");

        /* get flowers status */
        if ((recvMsgSize = recv(sock, buffer, sizeof(int) * FLOWERS_NUM, 0)) < 0)
            DieWithError("recv() failed");
       /* for (int i = 0; i < FLOWERS_NUM; ++i) {
            printf("%d ", buffer[i]);
        }
        printf("\n");*/

        int poured = 0;
        // Польём 1 рандомный голодный цветок
        for (int i = 0, j = rand() % FLOWERS_NUM; i < FLOWERS_NUM;
            ++i, j = (j + 1) % FLOWERS_NUM) {
            int flower_id = j + 1;
            if (buffer[j] > 0) {
                --buffer[j];
                ++poured;
                printf("Gardener %d has poured flower %d\n", gardener_id, flower_id);
                break;
            }
            else if (buffer[j] < 0) {
                ++dead_flowers_num;
            }
        }
        if (dead_flowers_num == FLOWERS_NUM) {
            printf("Gardener %d is fired because all flowers have died\n",
                gardener_id);
            break;
        }
        else if (!poured) {
            printf("No work for Gardener %d\n", gardener_id);
        }

        /*for (int i = 0; i < FLOWERS_NUM; ++i) {
            printf("%d ", buffer[i]);
        }
        printf("\n");*/
        /* Отправляю инфу о цветках */
        if (send(sock, buffer, sizeof(int) * FLOWERS_NUM, 0) !=
            sizeof(int) * FLOWERS_NUM)
            DieWithError("send() sent a different number of bytes than expected");
        // Садовник идёт на отдых
        sleep(rand() % 2 + 1);
    }
    //// Cообщаю серверу, что меня уволили
    //if (send(sock, (int[]) { 0 }, sizeof(int), 0) != sizeof(int))
    //    DieWithError("send() sent a different number of bytes than expected");

    close(sock);
    exit(0);
}

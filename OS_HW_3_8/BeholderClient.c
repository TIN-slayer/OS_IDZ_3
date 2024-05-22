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
#define LOG_LEN 13     /* Size of log */

// Процессы жизни цветов
int main(int argc, char* argv[]) {
    int sock;                      /* Socket descriptor */
    struct sockaddr_in servAddres; /* Echo server address */
    unsigned short servPort;       /* Echo server port */
    char* servIP;                  /* Server IP address (dotted quad) */

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
    memset(&servAddres, 0, sizeof(servAddres));     /* Zero out structure */
    servAddres.sin_family = AF_INET;                /* Internet address family */
    servAddres.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
    servAddres.sin_port = htons(servPort);          /* Server port */

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr*)&servAddres, sizeof(servAddres)) < 0)
        DieWithError("connect() failed");

    /* SEND CLNT TYPE TO GET IDENTIFIED BY SERVER */
    if (send(sock, (int[]) { 2 }, sizeof(int), 0) != sizeof(int))
        DieWithError("send() sent a different number of bytes than expected");

    int buffer[LOG_LEN]; // Буффер, который в дальнеём буду использовать для
    int recvMsgSize = 1;
    // принятия
    // сообщений от сервера

    //......................................................................................

    while (recvMsgSize > 0) {
        // Cообщаю серверу, что мы живы
        if (send(sock, (int[]) { 1 }, sizeof(int), 0) != sizeof(int))
            DieWithError("send() sent a different number of bytes than expected");
        /* get log */
        if ((recvMsgSize = recv(sock, buffer, sizeof(int) * LOG_LEN, 0)) < 0) {
            DieWithError("recv() failed");
            //break;
        }
        if (buffer[1]) {
            printf("Server send following message to client ");
        }
        else {
            printf("Server received following message from client ");
        }
        printf("by socket №%d:\n", buffer[0]);
        for (int i = 0; i < buffer[2]; ++i) {
            printf("%d ", buffer[3 + i]);
        }
        printf("\n");
        //sleep(1);
    }
    close(sock);
    exit(0);
}

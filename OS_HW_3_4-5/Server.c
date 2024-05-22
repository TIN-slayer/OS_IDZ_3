#include "AcceptTCPConnection.c"
#include "CreateTCPServerSocket.c"
#include "DieWithError.c"
#include <pthread.h> /* for POSIX threads */
#include <stdio.h>   /* for printf() and fprintf() */
#include <stdlib.h>  /* for atoi() and exit() */
#include <unistd.h>  /* for close() */

#define RCVBUFSIZE 32 /* Size of receive buffer */
#define MAX_GARDENER_COUNT 2
#define FLOWERS_NUM 10

pthread_mutex_t mutex;
int servSock; /* Socket descriptor for server */
int gardenerCount = 0;
int flowers[FLOWERS_NUM] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* Structure of arguments to pass to client thread */
struct ThreadArgs {
    int clntSock; /* Socket descriptor for client */
    int clntType;
};

void* RequestsHandler(void* threadArgs); /* Main program of a thread */
void* ServerThread(void* threadArgs);
void HandleGardenerId(int clntSocket);
void HandleActiveClnt(int clntSocket);


int main(int argc, char* argv[]) {
    int clntSock;                  /* Socket descriptor for client */
    unsigned short echoServPort;   /* Server port */
    pthread_t threadID;            /* Thread ID from pthread_create() */
    struct ThreadArgs* threadArgs; /* Pointer to argument structure for thread */

    if (argc != 2) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <SERVER PORT>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]); /* First arg:  local port */

    servSock = CreateTCPServerSocket(echoServPort);

    pthread_mutex_init(&mutex, NULL);

    // pthread_create(&threadID, NULL, RequestsHandler, NULL);

    for (;;) /* run forever */
    {
        clntSock = AcceptTCPConnection(servSock);

        /* Create separate memory for client argument */
        if ((threadArgs = (struct ThreadArgs*)malloc(sizeof(struct ThreadArgs))) ==
            NULL)
            DieWithError("malloc() failed");
        threadArgs->clntSock = clntSock;

        /* Identify client type */
        int clntType[1]; /* Buffer for echo string */
        int recvMsgSize; /* Size of received message */
        if ((recvMsgSize = recv(clntSock, clntType, sizeof(int), 0)) < 0)
            DieWithError("recv() failed");
        threadArgs->clntType = *clntType;

        /* Create client thread */
        if (pthread_create(&threadID, NULL, ServerThread, (void*)threadArgs) != 0)
            DieWithError("pthread_create() failed");
        printf("with thread %ld\n", (long int)threadID);
    }
    /* NOT REACHED */
}

void* RequestsHandler(void* threadArgs) /* Main program of a thread */
{
    for (;;) /* run forever */
    {
    }
    return (NULL);
}

void* ServerThread(void* threadArgs) {
    int clntSock; /* Socket descriptor for client connection */
    int clntType;

    /* Guarantees that thread resources are deallocated upon return */
    pthread_detach(pthread_self());

    /* Extract socket file descriptor from argument */
    clntSock = ((struct ThreadArgs*)threadArgs)->clntSock;
    clntType = ((struct ThreadArgs*)threadArgs)->clntType;
    free(threadArgs); /* Deallocate memory for argument */

    switch (clntType) {
    case 0:
        HandleGardenerId(clntSock);
        HandleActiveClnt(clntSock);
        break;
    case 1:
        HandleActiveClnt(clntSock);
        break;
    }

    return (NULL);
}

void HandleGardenerId(int sock) {
    int buffer[1]; /* Buffer for echo string */
    int recvMsgSize = 1;          /* Size of received message */
    if (send(sock, (int[]) { ++gardenerCount }, sizeof(int), 0) != sizeof(int))
        DieWithError("send() failed");
}

void HandleActiveClnt(int sock) {
    int buffer[1]; /* Buffer for echo string */
    int recvMsgSize = 1;          /* Size of received message */

    while (recvMsgSize > 0) {
        /* Ждём, пока садовник не проснётся */
        if ((recvMsgSize = recv(sock, buffer, sizeof(int), 0)) < 0)
            DieWithError("recv() failed");
        /*if (buffer[0] == 0) {
            break;
        }*/
        pthread_mutex_lock(&mutex);
        // Отправляем состояние цветов
        if (send(sock, flowers, sizeof(int) * FLOWERS_NUM, 0) !=
            sizeof(int) * FLOWERS_NUM)
            DieWithError("send() sent a different number of bytes than expected");

        /* Полчаем состояние цветков */
        if ((recvMsgSize = recv(sock, flowers, sizeof(int) * FLOWERS_NUM, 0)) < 0)
            DieWithError("recv() failed");
        //for (int i = 0; i < FLOWERS_NUM; ++i) {
        //    printf("%d ", flowers[i]);
        //}
        //printf("\n");
        pthread_mutex_unlock(&mutex);
    }

    close(sock); /* Close client socket */
}
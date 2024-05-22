#include "AcceptTCPConnection.c"
#include "CreateTCPServerSocket.c"
#include "DieWithError.c"
#include "Queue.c"
#include <pthread.h> /* for POSIX threads */
#include <stdio.h>   /* for printf() and fprintf() */
#include <stdlib.h>  /* for atoi() and exit() */
#include <unistd.h>  /* for close() */

#define RCVBUFSIZE 32     /* Size of receive buffer */
#define MAX_BEHOLDERS 100 // Максимальное количество клиентов-смотрителей
#define FLOWERS_NUM 10

pthread_mutex_t mutex;
int servSock; /* Socket descriptor for server */
int gardenerCount = 0;
int flowers[FLOWERS_NUM] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
Queue queue;
int beholderCount = 0;
int beholderSockets[MAX_BEHOLDERS];
//int k = 0;

/* Structure of arguments to pass to client thread */
struct ThreadArgs {
    int clntSock; /* Socket descriptor for client */
    int clntType;
};

void* ServerThread(void* threadArgs);
void* BeholderClnt(void* threadArgs); /* Main program of a thread */
void HandleGardenerId(int sock);
void HandleActiveClnt(int sock);

int main(int argc, char* argv[]) {
    int clntSock;                  /* Socket descriptor for client */
    unsigned short echoServPort;   /* Server port */
    pthread_t threadID;            /* Thread ID from pthread_create() */
    struct ThreadArgs* threadArgs; /* Pointer to argument structure for thread */
    init(&queue);
    if (argc != 2) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <SERVER PORT>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]); /* First arg:  local port */

    servSock = CreateTCPServerSocket(echoServPort);

    pthread_mutex_init(&mutex, NULL);

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

        // Если тип смотрителя
        if (clntType[0] == 2) {
            beholderSockets[beholderCount++] = clntSock;
            if (beholderCount == 1) {
                if (pthread_create(&threadID, NULL, BeholderClnt, NULL) != 0)
                    DieWithError("pthread_create() failed");
                printf("with thread %ld\n", (long int)threadID);
            }
        }
        else{ 
            /* Create client thread */
            if (pthread_create(&threadID, NULL, ServerThread, (void*)threadArgs) != 0)
                DieWithError("pthread_create() failed");
            printf("with thread %ld\n", (long int)threadID);
        }
    }
    /* NOT REACHED */
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

void* BeholderClnt(void* threadArgs) {
    int buffer[1];       /* Buffer for echo string */
    int recvMsgSize = 1; /* Size of received message */

    for (;;) /* run forever */
    {
        pthread_mutex_lock(&mutex);
        while (!is_empty(&queue)) {
            int* msg = pop(&queue);
            for (int i = 0; i < beholderCount; ++i) {
                if (beholderSockets[i] == -1) {
                    continue;
                }
                if ((recvMsgSize = recv(beholderSockets[i], buffer, sizeof(int), 0)) <= 0) {
                    beholderSockets[i] = -1; // Смотритель умер
                    continue;
                }
                if (send(beholderSockets[i], msg, sizeof(int) * LOG_LEN, 0) !=
                    sizeof(int) * LOG_LEN)
                    DieWithError("send() failed");            
            }
            free(msg);
        }
        pthread_mutex_unlock(&mutex);
    }
}

void HandleGardenerId(int sock) {
    int buffer[1];       /* Buffer for echo string */
    int recvMsgSize = 1; /* Size of received message */
    if (send(sock, (int[]) { ++gardenerCount }, sizeof(int), 0) != sizeof(int))
        DieWithError("send() failed");
}

void HandleActiveClnt(int sock) {
    int buffer[1];       /* Buffer for echo string */
    int recvMsgSize = 1; /* Size of received message */

    while (recvMsgSize > 0) {
        /* Ждём, пока садовник не проснётся */
        if ((recvMsgSize = recv(sock, buffer, sizeof(int), 0)) < 0)
            break;
        pthread_mutex_lock(&mutex);
        // Логируем
        push(&queue, StoreRequest(sock, 0, 1, buffer));
        // Отправляем состояние цветов
        if (send(sock, flowers, sizeof(int) * FLOWERS_NUM, 0) !=
            sizeof(int) * FLOWERS_NUM)
            break;
        // Логируем
        push(&queue, StoreRequest(sock, 1, FLOWERS_NUM, flowers));
        /* Полчаем состояние цветков */
        if ((recvMsgSize = recv(sock, flowers, sizeof(int) * FLOWERS_NUM, 0)) < 0)
            break;
        // Логируем
        push(&queue, StoreRequest(sock, 0, FLOWERS_NUM, flowers));
        // for (int i = 0; i < FLOWERS_NUM; ++i) {
        //   printf("%d ", flowers[i]);
        // }
        // printf("\n");
        pthread_mutex_unlock(&mutex);
    }

    close(sock); /* Close client socket */
}

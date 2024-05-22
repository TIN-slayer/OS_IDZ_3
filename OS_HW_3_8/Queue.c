#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 100
#define LOG_LEN 13 /* Size of log */

typedef struct {
	int* data[MAX_SIZE];
	int bot;
	int top;
	int len;
} Queue;

int* StoreRequest(int sock, int msgType, int msgLength, int* message) {
	int* buffer = malloc(sizeof(int) * LOG_LEN);
	buffer[0] = sock;
	buffer[1] = msgType;
	buffer[2] = msgLength;
	for (int i = 3; i < msgLength + 3; i++) {
		buffer[i] = message[i - 3];
	}
	return buffer;
}

void init(Queue* queue) {
	queue->bot = 0;
	queue->top = 0;
	queue->len = 0;
}

int is_empty(Queue* queue) { return queue->len == 0; }

int is_full(Queue* queue) { return queue->len == MAX_SIZE; }

void push(Queue* queue, int* value) {
	if (is_full(queue)) {
		queue->len = 0;
	}
	++queue->len;
	queue->data[queue->top] = value;
	queue->top = (queue->top + 1) % MAX_SIZE;
}

int* pop(Queue* queue) {
	if (is_empty(queue)) {
		perror("Stack underflow\n");
		exit(1);
	}
	int* val = queue->data[queue->bot];
	--queue->len;
	queue->bot = (queue->bot + 1) % MAX_SIZE;
	return val;
}

// int main() {
//     Stack queue;
//     init(&queue);
//
//     push(&queue, (int*) { 69, 4, (int[]) { 1, 7, 3, 4 } });
//     push(&queue, (int*) { 56, 3, (int[]) { 5, 8, 9 } });
//     push(&queue, (int*) { 73, 2, (int[]) { 96, 8 } });
//
//     while (!is_empty(&queue)) {
//         int* el = pop(&queue);
//         printf("clntSock: %d, msgLength: %d, message: ", el.clntSock,
//         el.msgLength); for (int i = 0; i < el.msgLength; i++) {
//             printf("%d ", el.message[i]);
//         }
//         printf("\n");
//     }
//     return 0;
// }
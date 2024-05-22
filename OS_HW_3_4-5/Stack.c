#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 100

typedef struct {
    int clntSock;
    int msgLength;
    int* message;
} IntPair;

typedef struct {
    IntPair data[MAX_SIZE];
    int top;
} Stack;

void init(Stack* stack) { stack->top = -1; }

int is_empty(Stack* stack) { return stack->top == -1; }

int is_full(Stack* stack) { return stack->top == MAX_SIZE - 1; }

void push(Stack* stack, IntPair value) {
    if (is_full(stack)) {
        perror("Stack overflow\n");
        exit(1);
    }
    stack->data[++(stack->top)] = value;
}

IntPair pop(Stack* stack) {
    if (is_empty(stack)) {
        perror("Stack underflow\n");
        exit(1);
    }
    return stack->data[(stack->top)--];
}

//int main() {
//    Stack stack;
//    init(&stack);
//
//    push(&stack, (IntPair) { 69, 4, (int[]) { 1, 7, 3, 4 } });
//    push(&stack, (IntPair) { 56, 3, (int[]) { 5, 8, 9 } });
//    push(&stack, (IntPair) { 73, 2, (int[]) { 96, 8 } });
//
//    while (!is_empty(&stack)) {
//        IntPair el = pop(&stack);
//        printf("clntSock: %d, msgLength: %d, message: ", el.clntSock, el.msgLength);
//        for (int i = 0; i < el.msgLength; i++) {
//            printf("%d ", el.message[i]);
//        }
//        printf("\n");
//    }
//    return 0;
//}
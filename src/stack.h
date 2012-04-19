#ifndef STACK
#define STACK
#define STACK_SIZE 15
typedef struct STACK{
	char *stack[STACK_SIZE];
	int top;
}Stack;
#endif

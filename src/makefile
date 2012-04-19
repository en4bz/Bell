bell : bell.c builtins.o stack.h
	gcc -Wall -std=gnu99 -o bell bell.c builtins.o

builtins.o : builtins.c builtins.h stack.h
	gcc -Wall -std=gnu99 -c builtins.c

clean :
	rm *\.o

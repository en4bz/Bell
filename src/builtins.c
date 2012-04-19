#include "builtins.h"

int cd(char *dir){
	return chdir(dir);
}

int pwd(void){
	char *dir;
	printf("%s\n", (dir = getcwd(NULL,0)));
	free(dir);
	return EXIT_SUCCESS;
}

int pushd(char* dir, Stack *in){
	in->stack[++in->top] = getcwd(NULL,0);
	cd(dir);
	return EXIT_SUCCESS;
}

int popd(Stack *in){
	if(in->top == -1){
		return EXIT_FAILURE;
	}
	else{
		char *temp = in->stack[in->top--];
		cd(temp);
		free(temp);
		return EXIT_SUCCESS;
	}
}

int history(void){
	char homedir[200];
	sprintf(homedir, "%s/.shellHistory.txt", getenv("HOME"));
	FILE* openStream = fopen(homedir, "r");
	if(openStream == NULL){
		puts("Error");
		return EXIT_FAILURE;
	}
	else{
		char temp[MAX_CMD_LEN];
		while(fgets(temp, sizeof(temp), openStream) != NULL){
			printf("%s",temp);
		}
	}
	fclose(openStream);
	return EXIT_SUCCESS;
}

int env(void){
	char **temp = environ;
	for(;*temp != NULL; temp++){
		printf("%s\n",*temp);
	}
	return EXIT_SUCCESS;
}

int set(const char *name, const char *value){
	return setenv(name, value, 1);
}

int unset(const char *name){
	return unsetenv(name);
}

void clearstdinBuffer(void){
	char ch;
	while ((ch = getchar()) != '\n' && ch != EOF);
}

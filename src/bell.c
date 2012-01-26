#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Stack.h"

#define MAX_CMD_LEN 200
#define MAX_ARGS 10

extern char **environ;

int cd(char*);
int set(const char*, const char*);
int unset(const char*);
void pwd(void);
void pushd(char*, Stack*);
void popd(Stack*);
void history(void);
void env(void);

int main(){
	char *prompt = "#";
	char *argv[MAX_ARGS];
	char *user = getlogin();
	char host[20];
	gethostname(host,20);
	for(int i = 0; i < MAX_ARGS; i++){
		argv[i] = NULL;
	}
	Stack dStack;
	dStack.top = -1;

	while(1){
		char command[MAX_CMD_LEN];
		char *temp = getcwd(NULL,0);
		printf("%s@%s:[%s]%s", user, host, temp, prompt);
		free(temp);
		fgets(command, sizeof(command), stdin);
		/*
		FILE* openStream = fopen("~/.shellHistory","a");
		fputs(command, openStream);
		fclose(openStream);
		*/
		free(argv[0]);
		argv[0] = strdup(strtok(command, "\n "));
		for(int i = 1; (temp = strtok(NULL, "\n ")) != NULL && i < MAX_ARGS; i++){
			free(argv[i]);
			argv[i] = strdup(temp);
		} 
		if(!strcmp(argv[0], "exit")){
			exit(0);
		}
		else if(!strcmp(argv[0], "echo")){
			for(int i = 1; i < MAX_ARGS && argv[i] != NULL; i++){
				printf("%s ", argv[i]);
			}
			printf("\n");
		}
		else if(!strcmp(argv[0], "cd")){
			int rCode = cd(argv[1]);
		}
		else if(!strcmp(argv[0], "pwd")){
			pwd();
		}
		else if(!strcmp(argv[0], "pushd")){
			pushd(argv[1], &dStack);
		}
		else if(!strcmp(argv[0], "popd")){
			popd(&dStack);
		}
		else if(!strcmp(argv[0], "history")){
			history();
		}
		else if(!strcmp(argv[0], "env")){
			env();
		}
		else{
			int pid = fork();
			if(pid == 0){
				//Child
				exit(0);
			}
			else{
				//Parent
			}
		}
	}
}

int cd(char *dir){
	return chdir(dir);
}

void pwd(void){
	char *dir = getcwd(NULL,0);
	printf("%s\n", dir);
	free(dir);
	return;
}

void pushd(char* dir, Stack *in){
	in->stack[++in->top] = getcwd(NULL,0);
	cd(dir);
	return;
}

void popd(Stack *in){
	if(in->top == -1){
		return;
	}
	else{
		char *temp = in->stack[in->top--];
		cd(temp);
		free(temp);
	}
	return;
}

void history(void){
	FILE* openStream = fopen("~/.shellHistory", "r");
	if(openStream == NULL){
		//Error
	}
	else{
		char temp[MAX_CMD_LEN];
		while(fgets(temp, sizeof(temp), openStream) != NULL){
			fputs(temp,stdin);
		}
	}
	return;
}

void env(void){
	char **temp = environ; 
	while(temp++ != NULL){
		printf("%s\n",*temp);
	}
	return;
}

int set(const char *name, const char *value){
	return setenv(name, value, 1);
} 

int unset(const char *name){
	return unsetenv(name); 
}

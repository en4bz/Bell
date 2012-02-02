#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
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
void clearstdinBuffer(void);


int main(void){
	char *argv[MAX_ARGS];
	char *user = getlogin();
	char host[20];
	char *temp, tempArray[250];
	gethostname(host,20);

	char homedir[200];
	sprintf(homedir, "%s/.shellHistory.txt", getenv("HOME"));

	for(short int i = 0; i < MAX_ARGS; i++){
		argv[i] = NULL;
	}

	Stack dStack;
	dStack.top = -1;

	short int stdinBackup = dup(0);
	short int stdoutBackup = dup(1);

	while(1){
		close(0);
		dup(stdinBackup);
		close(1);
		dup(stdoutBackup);
		for(short int i = 0; i < MAX_ARGS; i++){
			free(argv[i]);
			argv[i] = NULL;
		}
		char command[MAX_CMD_LEN];
		temp = getcwd(NULL,0);
		printf("%s@%s:[%s]$ ", user, host, temp);
		free(temp);
		fgets(command, sizeof(command), stdin);
		if(command[0] == '\n'){
			continue;
		}
		FILE* openStream = fopen(homedir,"a");
		fputs(command, openStream);
		fclose(openStream);

		short int in = 0;
		short int out = 0;
		short int isPipe = 0;
		free(argv[0]);
		argv[0] = strdup(strtok(command, "\n "));
		for(short int i = 1; (temp = strtok(NULL, "\n ")) != NULL && i < MAX_ARGS; i++){
			argv[i] = strdup(temp);
			if(*(argv[i]) == '<'){
				in = i;
			}
			if(*(argv[i]) == '>'){
				out = i;
			}
			if(*(argv[i]) == '|'){
				isPipe = i;
			}
		}
		if(out != 0){
			close(1);
			if(*(argv[in+1]) == '/'){
				//Abs Path
				open(argv[out+1], O_WRONLY | O_APPEND |  O_CREAT, 0666);
			}
			else{
				//Rel Path
				temp = getcwd(NULL,0);
				sprintf(tempArray, "%s/%s", temp, argv[out+1]);
				free(temp);
				open(tempArray, O_WRONLY | O_APPEND | O_CREAT, 0666);
			}
			for(int i = out; i < MAX_ARGS && in == 0; i++){
				free(argv[i]);
				argv[i] = NULL;
			}
		}
		if(in != 0){
			close(0);
			if(*(argv[in+1]) == '/'){
				//Abs Path
				open(argv[in+1], O_RDONLY);
			}
			else{
				//Rel Path
				temp = getcwd(NULL,0);
				sprintf(tempArray, "%s/%s", temp, argv[in+1]);
				free(temp);
				open(tempArray, O_RDONLY);
			}
			for(int i = in; i < MAX_ARGS; i++){
				free(argv[i]);
				argv[i] = NULL;
			}
		}
		if(isPipe != 0){
			int fileDes[2];
			pipe(fileDes);
			int pid = fork();
			if(pid == 0){
				//Child Peforms first action and 
				//prints to stdin buffer
				for(int i = isPipe; i < MAX_ARGS; i++){
					free(argv[i]);
					argv[i] = NULL;
				}
				close(1);
				dup(fileDes[1]);
				close(fileDes[0]);
				close(fileDes[1]);
				execvp(argv[0], argv);
				exit(EXIT_FAILURE);
			}
			else{	
				//Parent waits for child then reads 
				//from stdin
				int rCode;
				wait(&rCode);
				close(0);
				dup(fileDes[0]);
				close(fileDes[0]);
				close(fileDes[1]);
				free(argv[0]);
				argv[0] = argv[isPipe+1];
				for(int i = 1; i < MAX_ARGS; i++){
					if(i != isPipe+1){
						free(argv[i]);
					}
					argv[i] = NULL;
				}
			}
		}
		if(!strcmp(argv[0], "exit")){
			exit(0);
		}
		else if(!strcmp(argv[0], "echo")){
			for(short int i = 1; i < MAX_ARGS && argv[i] != NULL; i++){
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
		else if(!strcmp(argv[0], "set")){
			char *name = strdup(strtok(argv[1], "="));
			char *value = strdup(strtok(NULL,"\n "));
			set(name, value);
			free(name);
			free(value);
		}
		else if(!strcmp(argv[0], "unset")){
			unset(argv[1]);
		}
		else if(!strcmp(argv[0], "getenv")){
			if((temp = getenv(argv[1])) != NULL){
				printf("%s\n", temp);
			}
			else{
				printf("(null)\n");
			}
		}
		else{
			int pid = fork();
			if(pid == 0){	
				char cmd[MAX_CMD_LEN];
				if(*argv[0] == '.'  && *(argv[0]+1) == '/'){
					//Look in cwd if "./"
					sprintf(cmd, "%s%s", getenv("PWD"), (argv[0]+1));
					execvp(cmd, argv);
					
				}
				else{
					execvp(argv[0], argv);
				}
				exit(EXIT_FAILURE);
			}
			else{
				int rCode;
				wait(&rCode);
			}
		}
	}
}

int cd(char *dir){
	set("PWD", dir);
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
	char homedir[200];
	sprintf(homedir, "%s/.shellHistory.txt", getenv("HOME"));
	FILE* openStream = fopen(homedir, "r");
	//rewind(openStream);
	if(openStream == NULL){
		//Error
		puts("Error");
	}
	else{
		char temp[MAX_CMD_LEN];
		while(fgets(temp, sizeof(temp), openStream) != NULL){
			printf("%s",temp);
		}
	}
	fclose(openStream);
	return;
}

void env(void){
	char **temp = environ;
	for(;*temp != NULL; temp++){
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

void clearstdinBuffer(void){
	char ch;	
	while ((ch = getchar()) != '\n' && ch != EOF);
}

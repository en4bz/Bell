#include "bell.h"

int main(void){
	static char *argv[MAX_ARGS];
	char *user = getenv("USER");
	char *workingDir = getcwd(NULL, 0);
	char host[20];
	char *temp, tempArray[250];
	gethostname(host, sizeof(host));

	char homedir[200];
	sprintf(homedir, "%s/.shellHistory.txt", getenv("HOME"));
	FILE* openStream = fopen(homedir,"a");

	Stack dStack;
	dStack.top = -1;

	int stdinBackup = dup(0);
	int stdoutBackup = dup(1);

	int rCode;

	while(1){
		char command[MAX_CMD_LEN];
		//Print Prompt
		printf("%s@%s:[%s]$ ", user, host, workingDir);
		fgets(command, sizeof command, stdin);
		if(command[0] == '\n' || command == NULL){
			continue;
		}
		fputs(command, openStream);
		int in = 0;
		int out = 0;
		int isPipe = 0;
		free(argv[0]);
		argv[0] = strdup(strtok(command, "\n "));
		for(int i = 1; (temp = strtok(NULL, "\n ")) != NULL && i < MAX_ARGS; i++){
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
				sprintf(tempArray, "%s/%s", workingDir, argv[out+1]);
				open(tempArray, O_WRONLY | O_APPEND | O_CREAT, 0666);
			}
			for(short int i = out; i < MAX_ARGS && in == 0; i++){
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
				sprintf(tempArray, "%s/%s", workingDir, argv[in+1]);
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
				//Child Peforms first action and prints to pipe
				for(int i = isPipe; i < MAX_ARGS; i++){
					free(argv[i]);
					argv[i] = NULL;
				}
				close(1);
				dup(fileDes[1]);
				close(fileDes[0]);
				close(fileDes[1]);
				rCode = execvp(argv[0], argv);
				exit(EXIT_FAILURE);
			}
			else{
				//Parent waits for child then reads from pipe
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
			break;
		}
		else if(!strcmp(argv[0], "echo")){
			if(*(argv[1]) == '$' && *(argv[1]+1) == '?'){
				printf("%d", rCode);
			}
			else{
				for(int i = 1; i < MAX_ARGS && argv[i] != NULL; i++){
					printf("%s ", argv[i]);
					rCode = 0;
				}
			}
			printf("\n");
		}
		else if(!strcmp(argv[0], "cd")){
			if(argv[1] == NULL){
				rCode = cd(homedir);
			}
			else{
				rCode = cd(argv[1]);
			}
			free(workingDir);
			set("PWD", (workingDir = getcwd(NULL,0)));
		}
		else if(!strcmp(argv[0], "pwd")){
			rCode = pwd();
		}
		else if(!strcmp(argv[0], "pushd")){
			rCode = pushd(argv[1], &dStack);
			free(workingDir);
			set("PWD", (workingDir = getcwd(NULL,0)));
		}
		else if(!strcmp(argv[0], "popd")){
			rCode = popd(&dStack);
			free(workingDir);
			set("PWD", (workingDir = getcwd(NULL,0)));
		}
		else if(!strcmp(argv[0], "history")){
			rCode = history();
		}
		else if(!strcmp(argv[0], "env")){
			rCode = env();
		}
		else if(!strcmp(argv[0], "set")){
			char *name = strdup(strtok(argv[1], "="));
			char *value = strdup(strtok(NULL,"\n "));
			rCode = set(name, value);
			free(name);
			free(value);
		}
		else if(!strcmp(argv[0], "unset")){
			rCode = unset(argv[1]);
		}
		else{
			int pid = fork();
			if(pid == 0){
				if(*argv[0] == '.'  && *(argv[0]+1) == '/'){
					//Look in cwd if "./"
					sprintf(tempArray, "%s%s", getenv("PWD"), (argv[0]+1));
					execvp(tempArray, argv);
				}
				else{
					execvp(argv[0], argv);
				}
				exit(EXIT_FAILURE);
			}
			else{
				wait(&rCode);
			}
		}
		close(0);
		dup(stdinBackup);
		close(1);
		dup(stdoutBackup);
		for(int i = 0; i < MAX_ARGS; i++){
			free(argv[i]);
			argv[i] = NULL;
		}
	}
	free(workingDir);
	fclose(openStream);
	return rCode;
}

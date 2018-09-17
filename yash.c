#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <unistd.h>
#include <sys/wait.h>

int num_spaces(char*);
void parse_spaces(char*, char**);
void handle_cd(char*);

int main(int argc, char const *argv[])
{

	while (true)
	{
		char *user_input = readline("# ");
		char *save_pointer_pipe;
		char *current_pipe, *current_input;
		int argument_index = 0;
		pid_t PID = 0;
		char array[10];
		int wstatus;
		char *has_pipe = strchr(user_input, '|');


		if (user_input[0] == 'c' && user_input[1] == 'd'){
			handle_cd(user_input);
			continue;
		}

		current_pipe = strtok_r(user_input, "|", &save_pointer_pipe); 
		
		while (current_pipe != NULL)
		{
			//printf("%s\n", current_pipe);
			PID = fork();
			if(PID == 0){ //child goes here

				if (!has_pipe){
					char *args[num_spaces(user_input)+2];
					args[num_spaces(user_input)+1] = NULL;
					parse_spaces(user_input, args);
					if(execvp(args[0], args) < 0){
						printf("ERROR\n");
					}



				} else {
					PID = 0;
					PID = fork();
					int pipefd[2];
					pipe(pipefd);
					char buf;


					if (PID == 0) {//child code
						close(pipefd[1]);
						read(pipefd[0], &buf, 10000);
						current_pipe = strtok_r(NULL, "|", &save_pointer_pipe); 
						//printf("child %s\n", current_pipe);
						char *args[num_spaces(current_pipe)+2];
						args[num_spaces(current_pipe)+1] = NULL;
						parse_spaces(current_pipe, args);
						if(execvp(args[0], args) < 0){
							printf("ERROR\n");
						}
						


					} else {// parent code
						close(pipefd[0]);

						//changes fdt output from stdout to pipe write 
						dup2(STDOUT_FILENO, STDIN_FILENO);//pipefd[1]);
						//printf("parent %s\n", current_pipe);
						close(pipefd[1]);
						char *args[num_spaces(current_pipe)+2];
						args[num_spaces(current_pipe)+1] = NULL;
						parse_spaces(current_pipe, args);
						if(execvp(args[0], args) < 0){
							printf("ERROR\n");
						}
					}
					
				}

				current_pipe = strtok_r(NULL, "|", &save_pointer_pipe); 


			}else{ //parent goes here
				//printf("Parent PID %d\n",PID);
				waitpid(PID, &wstatus, 0);
				break;


			}


			argument_index++;
		}
	}
	exit(0);
}

int num_spaces(char* str){
	int count = 0;
	int spaces = 0;
	while(true){
		if (str[count] == ' ') {
			spaces++;
		}
		if (str[count] == '\0'){
			break;
		}
		count ++;
	}
	return spaces;
}

void parse_spaces(char* str, char **parsed_string){

	char *save_pointer_input;

	int spaces = num_spaces(str) + 1;
	char *input = strtok_r(str, " ", &save_pointer_input);

	for(int i = 0; i < spaces; i++){
		parsed_string[i] = input;
		input = strtok_r(NULL, " ", &save_pointer_input);
	}
}

void handle_cd(char *input){
	if (strlen(input) > 3){
		chdir(input + 3);
	}
}
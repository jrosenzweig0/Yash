#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <unistd.h>
#include <sys/wait.h>

int num_spaces(char*);
void parse_spaces(char*, char**);

int main(int argc, char const *argv[])
{
	char *x[9];
	char y[] = "this is a test";
	parse_spaces(y, x);
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


		// if (current_input == "|" || current_input == ">" || current_input == "<"){
		// 	printf("yash: unexpected token near %s",current_input);
		// 	continue;
		// }
		current_pipe = strtok_r(user_input, "|", &save_pointer_pipe); 
		//current_input = strtok_r(current_pipe, " ", &save_pointer_input);
		
		while (current_pipe != NULL)
		{
			//printf("%s\n", current_pipe);
			PID = fork();
			if(PID == 0){ //child goes here
				printf("Child PID %d\n",PID);

				if (!has_pipe){

					execlp("wal", "wal", "-i", "/back");

					execlp(user_input,user_input, NULL);



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
						execlp(current_pipe,current_pipe, &buf, NULL);
						


					} else {// parent code
						close(pipefd[0]);

						//changes fdt output from stdout to pipe write 
						dup2(STDOUT_FILENO, STDIN_FILENO);//pipefd[1]);
						//printf("parent %s\n", current_pipe);
						close(pipefd[1]);
						execlp(current_pipe,current_pipe, NULL);

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
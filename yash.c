#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/***************************************/
//             Structs                 //
/***************************************/

struct Job{
	pid_t PID;
	int job_num;
	char status[15];
	char name[30];
	char symbol;
};

/***************************************/
//            functions                //
/***************************************/

int parse_tokens(char *str, char **strarray, bool has_pipe){
	int count = 0;
	for(int i = 0; i < strlen(str); i++){
		if(str[i] == '|'){
			strarray[count] = "|";
			has_pipe = true;
			count++;
		}
		if(str[i] == '<'){
			strarray[count] = "<";
			count++;
		}
		if(str[i] == '>'){
			if(str[i-1] == '2'){
				str[i-1] = ' ';
				strarray[count] = "2>";
				count++;
			}
			else {
				strarray[count] = ">";
				count++;
			}
		}
		if(str[i] == '&'){
			strarray[count] = "&";
			count++;
		}
	}
	return count;
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

void make_job(struct Job *jobs, pid_t PID, char *name, int pointer, char *status, char symbol){
	jobs[pointer].PID = PID;
	jobs[pointer].job_num = pointer + 1;
	strcpy(jobs[pointer].name, name);
	strcpy(jobs[pointer].status, status);
	jobs[pointer].symbol = symbol;
}

/***************************************/
//          Signal Handelers           //
/***************************************/

void SIG_HANDLE(int sig){ // ctrl c
	switch(sig){
		case SIGINT:
			printf("Caught SIGINT\n");
			exit(0);
			break;
		case SIGTSTP:
			printf("Caught SIGTSTP\n");
			break;
		case SIGCONT:
			printf("Caught SIGCONT\n");
			break;
		case SIGCHLD:
			printf("Caught SIGCHLD\n");
			break;
	}

}

/***************************************/
//            Main Code                //
/***************************************/

int main(int argc, char const *argv[])
{
	struct Job job_list[10];
	int job_pointer = 0;

	signal(SIGINT, SIG_HANDLE);
	signal(SIGTSTP, SIG_HANDLE);
	signal(SIGCONT, SIG_HANDLE);
	signal(SIGCHLD, SIG_HANDLE);


	while (true){
		char *tokens[100]; //array with each of the operators in the input ("|", "<", ">", ...)
		char commands[30][67]; //this is an array of the commands between the operators from the input
		int num_tokens; //number of operators
		char *save_pointer;
		bool has_pipe = false;
		int wstatus;
		char user_input_copy[2000];


		char *user_input = readline("# ");
		if (user_input[0] == 'c' && user_input[1] == 'd'){
			handle_cd(user_input);
			continue;
		}
		strcpy(user_input_copy, user_input);
		num_tokens = parse_tokens(user_input, tokens, has_pipe);
		/***************************************/
		//            Parse Input              //
		/***************************************/
		char *command = strtok_r(user_input, "|<>&", &save_pointer);
		strcpy(commands[0], command);
		int index = 1;

		while (command != NULL){
			if((command = strtok_r(NULL,"|<>&", &save_pointer)) == NULL){
				break;
			}
			strcpy(commands[index], command);
			index++;
		}

		/***************************************/
		//       processes start here          //
		/***************************************/
		pid_t PID = 0;
		PID = fork();
		if (PID == 0){//child code goes here
			make_job(job_list, getpid(), user_input_copy, job_pointer, "Runnung", '+');
			job_pointer++;

			/***************************************/
			//             No Pipe                 //
			/***************************************/
			if(!has_pipe){

				char *args[num_spaces(commands[0])+2];
				args[num_spaces(commands[0])+1] = NULL;
				parse_spaces(commands[0], args);

				for(int i = 0; i < num_tokens; i++){
					if (tokens[i] == ">"){
						int fd = open(commands[i+1], O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
						dup2(fd, STDOUT_FILENO);
					}
					else if (tokens[i] == "2>"){
						int fd = open(commands[i+1], O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
						dup2(fd, STDERR_FILENO);
					}
					else if (tokens[i] == "<"){
						int fd = open(commands[i+1], O_CREAT|O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
						dup2(fd, STDIN_FILENO);
					}
					else if (tokens[i] == "&"){
						printf("&&&& ahhhhh\n");
					}
					else{
						printf("ERROR UNEXPECTED TOKEN\n");
					}
				}

				if(execvp(args[0], args) < 0){
					printf("ERROR\n");
				}
			}
			/***************************************/
			//                Pipe                 //
			/***************************************/
			else if(has_pipe){
				int pipefd[2];
				if(pipe(pipefd) < 0){
					printf("PIPE ERROR\n");
				}
				PID = 0;
				PID = fork();

				if(PID == 0){//child command to right of pipe
					close(pipefd[1]);
					int pipe_position;
					for(int i = 0; i < num_tokens; i++){
						if(tokens[i] == "|"){
							pipe_position = i;
							break;
						}
					}

					char *args[num_spaces(commands[pipe_position+1])+2];
					args[num_spaces(commands[pipe_position+1])+1] = NULL;
					parse_spaces(commands[pipe_position+1], args);
					dup2(pipefd[0], STDIN_FILENO);

					for(int i = pipe_position; i < num_tokens; i++){
						if (i == pipe_position){
							continue;
						}
						if (tokens[i] == ">"){
							int fd = open(commands[i+1], O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, STDOUT_FILENO);
						}
						else if (tokens[i] == "2>"){
							int fd = open(commands[i+1], O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, STDERR_FILENO);
						}
						else if (tokens[i] == "<"){
							int fd = open(commands[i+1], O_CREAT|O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, STDIN_FILENO);
						}
						else if (tokens[i] == "&"){
							printf("&&&& ahhhhh\n");
						}
						else if (tokens[i] == "|"){
							break;
						}
						else{
							printf("ERROR UNEXPECTED TOKEN\n");
						}

					}

					if(execvp(args[0], args) < 0){
						printf("ERROR\n");
					}
					printf("ERROR\n");


				}
				else{//parent command to the left of pipe
					close(pipefd[0]);
					char *args[num_spaces(commands[0])+2];
					args[num_spaces(commands[0])+1] = NULL;
					parse_spaces(commands[0], args);
					dup2(pipefd[1], STDOUT_FILENO);

					for(int i = 0; i< num_tokens; i++){
						if (tokens[i] == ">"){
							int fd = open(commands[i+1], O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, STDOUT_FILENO);
						}
						else if (tokens[i] == "2>"){
							int fd = open(commands[i+1], O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, STDERR_FILENO);
						}
						else if (tokens[i] == "<"){
							int fd = open(commands[i+1], O_CREAT|O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, STDIN_FILENO);
						}
						else if (tokens[i] == "&"){
							printf("&&&& ahhhhh\n");
						}
						else if (tokens[i] == "|"){
							break;
						}
						else{
							printf("ERROR UNEXPECTED TOKEN\n");
						}
					}

					if(execvp(args[0], args) < 0){
						printf("ERROR\n");
					}
					printf("ERROR\n");

				}

			}
			else{
				printf("ERROR CODE 1\n");
			}
		}
		else{//parent code goes here
			waitpid(PID, &wstatus, 0);
			//waitPID depending on jobs
		}



	}
}
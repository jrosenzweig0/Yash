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
char* rm_s(char *str){
	if(str && str[0] == ' '){
		str = str + 1;
	}
	return str;
}

bool has_p(char *str){
	bool p = false;
	for(int i = 0; i < strlen(str); i++){
		if(str[i] == '|'){
			p = true;
		}
	}
	return p;
}

bool has_a(char *str){
	bool a = false;
	for(int i = 0; i < strlen(str); i++){
		if(str[i] == '&'){
			a = true;
		}
	}
	return a;
}

int parse_tokens(char *str, char **strarray){
	int count = 0;
	for(int i = 0; i < strlen(str); i++){
		if(str[i] == '|'){
			strarray[count] = "|";
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
	for(int i = 0; i < pointer; i++){
		jobs[i].symbol = '-';
	}
}

void print_jobs(struct Job *jobs, int job_pointer){
	for(int i = 0; i < job_pointer; i++){
		printf("[%d] %c %-11s%s\n",jobs[i].job_num, jobs[i].symbol, jobs[i].status, jobs[i].name);
	}
}

void update_jobs(struct Job *jobs, int job_pointer){

}

pid_t fg_process(struct Job *jobs, int job_pointer){
	for(int i = 0; i < job_pointer; i++){
		if(jobs[i].symbol = '+'){
			return jobs[i].PID;
		}
	}
	return -1;
}

void remove_job(struct Job *jobs, int job_pointer, pid_t PID){

}

/***************************************/
//          Signal Handelers           //
/***************************************/
struct Job job_list[20];
int job_pointer = 0;
void SIG_HANDLE(int sig){ // ctrl c
	switch(sig){
		case SIGINT:
			//printf("Caught SIGINT\n");
			exit(0);
			break;
		case SIGTSTP:
			//printf("Caught SIGTSTP\n");

			if (fg_process(job_list, job_pointer) > 0){
				kill(fg_process(job_list, job_pointer), SIGTSTP);
			}

			break;
		case SIGCONT:
			//printf("Caught SIGCONT\n");
			break;
		case SIGCHLD:
			//printf("Caught SIGCHLD\n");
			break;
	}

}

/***************************************/
//            Main Code                //
/***************************************/



int main(int argc, char const *argv[])
{

	signal(SIGINT, SIG_HANDLE);
	signal(SIGTSTP, SIG_HANDLE);
	signal(SIGCONT, SIG_HANDLE);
	signal(SIGCHLD, SIG_HANDLE);


	while (true){
		char *tokens[100]; //array with each of the operators in the input ("|", "<", ">", ...)
		char commands[30][67]; //this is an array of the commands between the operators from the input
		int num_tokens; //number of operators
		char *save_pointer;
		int wstatus;
		char user_input_copy[2000];


		char *user_input = readline("# ");
		if (user_input[0] == 'c' && user_input[1] == 'd'){
			handle_cd(user_input);
			continue;
		}
		if (user_input[0] == 'j' && user_input[1] == 'o' && user_input[2] == 'b' && user_input[3] == 's'){
			print_jobs(job_list, job_pointer);
		}
		bool has_pipe = has_p(user_input);
		bool background = has_a(user_input);

		strcpy(user_input_copy, user_input);
		if (background){
			user_input[strlen(user_input)-1] = ' ';
		}
		num_tokens = parse_tokens(user_input, tokens);
		/***************************************/
		//            Parse Input              //
		/***************************************/
		char *command = strtok_r(user_input, "|<>", &save_pointer);
		if (command != NULL){
			strcpy(commands[0], command);
		}
		int index = 1;

		while (command != NULL){
			if((command = strtok_r(NULL,"|<>", &save_pointer)) == NULL){
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


			/***************************************/
			//             No Pipe                 //
			/***************************************/
			if(!has_pipe){

				char *args[num_spaces(commands[0])+2];
				args[num_spaces(commands[0])+1] = NULL;
				parse_spaces(commands[0], args);

				for(int i = 0; i < num_tokens; i++){

					if (tokens[i] == ">"){
						int fd = open(rm_s(commands[i+1]), O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
						dup2(fd, STDOUT_FILENO);
					}
					else if (tokens[i] == "2>"){
						int fd = open(rm_s(commands[i+1]), O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
						dup2(fd, STDERR_FILENO);
					}
					else if (tokens[i] == "<"){
						int fd = open(rm_s(commands[i+1]), O_CREAT|O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
						dup2(fd, STDIN_FILENO);
					}
					else{
						printf("ERROR UNEXPECTED TOKEN\n");
					}
				}

				if(execvp(args[0], args) < 0){
					//printf("ERROR\n");
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
				pid_t PID1 = 0;
				PID1 = fork();

				if(PID1 == 0){//child command to right of pipe
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
							int fd = open(rm_s(commands[i+1]), O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, STDOUT_FILENO);
						}
						else if (tokens[i] == "2>"){
							int fd = open(rm_s(commands[i+1]), O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, STDERR_FILENO);
						}
						else if (tokens[i] == "<"){
							int fd = open(rm_s(commands[i+1]), O_CREAT|O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, STDIN_FILENO);
						}
						else if (tokens[i] == "|"){
							break;
						}
						else{
							printf("ERROR UNEXPECTED TOKEN\n");
						}

					}

					if(execvp(args[0], args) < 0){
						//printf("ERROR\n");
					}
					//printf("ERROR\n");


				}
				else{//parent command to the left of pipe
					close(pipefd[0]);
					char *args[num_spaces(commands[0])+2];
					args[num_spaces(commands[0])+1] = NULL;
					parse_spaces(commands[0], args);
					dup2(pipefd[1], STDOUT_FILENO);

					for(int i = 0; i< num_tokens; i++){

						if (tokens[i] == ">"){
							int fd = open(rm_s(commands[i+1]), O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, STDOUT_FILENO);
						}
						else if (tokens[i] == "2>"){
							int fd = open(rm_s(commands[i+1]), O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, STDERR_FILENO);
						}
						else if (tokens[i] == "<"){
							int fd = open(rm_s(commands[i+1]), O_CREAT|O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, STDIN_FILENO);
						}
						else if (tokens[i] == "|"){
							break;
						}
						else{
							printf("ERROR UNEXPECTED TOKEN\n");
						}
					}

					if(execvp(args[0], args) < 0){
						//printf("ERROR\n");
					}
					//printf("ERROR\n");

				}

			}
			else{
				printf("ERROR CODE 1\n");
			}
		}
		else{//parent code goes here
			make_job(job_list, PID, user_input_copy, job_pointer, "Runnung", '+');
			job_pointer++;
			if(!background){
				waitpid(PID, &wstatus, 0);
			}
			//waitPID depending on jobs
		}



	}
}
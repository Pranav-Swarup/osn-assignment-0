#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

enum CommandCodes{
	CMD_UNKNOWN = -1,
	CMD_INPUT = 1,
	CMD_PRINT,
	CMD_FIRST_N,
	CMD_LAST_N,
	CMD_LOG_N,
	CMD_STOP,
};

struct CodeValue{
	enum CommandCodes code;
	int n;
};

char contentpath[256], logpath[256];
int original_stdout, original_stdin;

struct CodeValue get_command_info(char *str) {

	struct CodeValue result = {CMD_UNKNOWN, -1};
	char input[128];
	strncpy(input, str, sizeof(input));
	input[sizeof(input) - 1] = '\0';

	int len = strlen(input);
	
	// Removing trailing spaces.

	while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r' || input[len - 1] == ' ')){
		input[--len] = '\0';
	}

	if(strcmp(input, "INPUT") == 0){
		result.code = CMD_INPUT;
	}
	else if(strcmp(input, "PRINT") == 0){
		result.code = CMD_PRINT;
	}
	else if(strcmp(input, "STOP") == 0){
		result.code = CMD_STOP;
	}
	else if(strncmp(input, "FIRST ", 6) == 0){
		if(sscanf(input + 6, "%d", &result.n) == 1)
			result.code = CMD_FIRST_N;
	}
	else if(strncmp(input, "LAST ", 5) == 0){
		if(sscanf(input + 5, "%d", &result.n) == 1)
			result.code = CMD_LAST_N;
	}
	else if(strncmp(input, "LOG ", 4) == 0){
		if(sscanf(input + 4, "%d", &result.n) == 1)
			result.code = CMD_LOG_N;
	}

	return result;
}

void handle_input(){

	char input[1024];
	int content_fd, log_fd;
	int saved_stdout = dup(STDOUT_FILENO);

	// Get input from user
	dup2(original_stdout, STDOUT_FILENO);
	printf("INPUT -> ");
	fflush(stdout);
	dup2(saved_stdout, STDOUT_FILENO);
	
	scanf("%[^\n]%*c", input);

	// Write to content file
	content_fd = open(contentpath, O_WRONLY | O_APPEND);
	if(content_fd < 0){
		printf("File open failed");
		close(saved_stdout);
		return;
	}
	
	dup2(content_fd, STDOUT_FILENO);
	printf("%s\n", input);
	fflush(stdout);
	close(content_fd);

	// Write to log file
	log_fd = open(logpath, O_WRONLY | O_APPEND);
	if(log_fd < 0){
		dup2(saved_stdout, STDOUT_FILENO);
		printf("File open failed");
		close(saved_stdout);
		return;
	}
	
	dup2(log_fd, STDOUT_FILENO);
	printf("INPUT\n");
	fflush(stdout);
	close(log_fd);
	
	dup2(saved_stdout, STDOUT_FILENO);
	close(saved_stdout);
}

void handle_print(){

	int content_fd, log_fd;
	int saved_stdin = dup(STDIN_FILENO);
	int saved_stdout = dup(STDOUT_FILENO);

	// Read and print content file
	content_fd = open(contentpath, O_RDONLY);
	if(content_fd < 0){
		printf("File open failed");
		close(saved_stdin);
		close(saved_stdout);
		return;
	}

	dup2(content_fd, STDIN_FILENO);
	dup2(original_stdout, STDOUT_FILENO);
	
	char ch;
	while(scanf("%c", &ch) == 1) {
		printf("%c", ch);
	}
	printf("\n");
	fflush(stdout);
	close(content_fd);

	// Write to log file
	log_fd = open(logpath, O_WRONLY | O_APPEND);
	if(log_fd < 0){
		dup2(saved_stdin, STDIN_FILENO);
		dup2(saved_stdout, STDOUT_FILENO);
		printf("File open failed");
		close(saved_stdin);
		close(saved_stdout);
		return;
	}
	
	dup2(log_fd, STDOUT_FILENO);
	printf("PRINT\n");
	fflush(stdout);
	close(log_fd);
	
	dup2(saved_stdin, STDIN_FILENO);
	dup2(saved_stdout, STDOUT_FILENO);
	close(saved_stdin);
	close(saved_stdout);
}

void handle_first(int n, char *msg){

	int content_fd, log_fd;
	int saved_stdin = dup(STDIN_FILENO);
	int saved_stdout = dup(STDOUT_FILENO);

	content_fd = open(contentpath, O_RDONLY);
	if(content_fd < 0){
		printf("File open failed");
		close(saved_stdin);
		close(saved_stdout);
		return;
	}

	dup2(content_fd, STDIN_FILENO);
	dup2(original_stdout, STDOUT_FILENO);

    char ch;
    int line_count = 0;

    while (scanf("%c", &ch) == 1 && line_count < n) {
        printf("%c", ch);
        if (ch == '\n') {
            line_count++;
        }
    }
	fflush(stdout);
	close(content_fd);

	// Write to log file
	log_fd = open(logpath, O_WRONLY | O_APPEND);
	if(log_fd < 0){
		dup2(saved_stdin, STDIN_FILENO);
		dup2(saved_stdout, STDOUT_FILENO);
		printf("File open failed");
		close(saved_stdin);
		close(saved_stdout);
		return;
	}
	
	dup2(log_fd, STDOUT_FILENO);
	printf("%s\n", msg);
	fflush(stdout);
	close(log_fd);
	
	dup2(saved_stdin, STDIN_FILENO);
	dup2(saved_stdout, STDOUT_FILENO);
	close(saved_stdin);
	close(saved_stdout);
}

void handle_last(int n, char *msg){

	int content_fd, log_fd;
	int saved_stdin = dup(STDIN_FILENO);
	int saved_stdout = dup(STDOUT_FILENO);

	// Count total lines by reading log file
	log_fd = open(logpath, O_RDONLY);
	if(log_fd < 0){
		printf("File open failed");
		close(saved_stdin);
		close(saved_stdout);
		return;
	}

	dup2(log_fd, STDIN_FILENO);
	char ch;
	int line_count = 0;

	while(scanf("%c", &ch) == 1){
		if(ch == 'I'){
			line_count++;
			// Skip rest of line
			while(scanf("%c", &ch) == 1 && ch != '\n');
		}
		else{
			// Skip rest of line
			while(scanf("%c", &ch) == 1 && ch != '\n');
		}
	}
	close(log_fd);

	int skip = line_count - n;
	if(skip < 0)
		skip = 0;

	// Read content file
	content_fd = open(contentpath, O_RDONLY);
	if(content_fd < 0){
		dup2(saved_stdin, STDIN_FILENO);
		dup2(saved_stdout, STDOUT_FILENO);
		printf("File open failed");
		close(saved_stdin);
		close(saved_stdout);
		return;
	}

	dup2(content_fd, STDIN_FILENO);
	dup2(original_stdout, STDOUT_FILENO);
	int currentline = 0;

	while(scanf("%c", &ch) == 1){
		if(currentline >= skip)
			printf("%c", ch);
		if(ch == '\n')
			currentline++;
	}
	fflush(stdout);
	close(content_fd);

	// Write to log file
	log_fd = open(logpath, O_WRONLY | O_APPEND);
	if(log_fd < 0){
		dup2(saved_stdin, STDIN_FILENO);
		dup2(saved_stdout, STDOUT_FILENO);
		printf("File open failed");
		close(saved_stdin);
		close(saved_stdout);
		return;
	}
	
	dup2(log_fd, STDOUT_FILENO);
	printf("%s\n", msg);
	fflush(stdout);
	close(log_fd);
	
	dup2(saved_stdin, STDIN_FILENO);
	dup2(saved_stdout, STDOUT_FILENO);
	close(saved_stdin);
	close(saved_stdout);
}

void handle_log(int n){

	int log_fd;
	int saved_stdin = dup(STDIN_FILENO);
	int saved_stdout = dup(STDOUT_FILENO);

	log_fd = open(logpath, O_RDONLY);
	if(log_fd < 0){
		printf("Log.txt open failed");
		close(saved_stdin);
		close(saved_stdout);
		return;
	}

	// Count total lines first
	dup2(log_fd, STDIN_FILENO);
	char ch;
	int line_count = 0;

	while(scanf("%c", &ch) == 1) {
		if(ch == '\n')
			line_count++;
	}
	close(log_fd);

	int skip = line_count - n;
	if(skip < 0)
		skip = 0;

	// Reopen and read from skip point
	log_fd = open(logpath, O_RDONLY);
	if(log_fd < 0){
		dup2(saved_stdin, STDIN_FILENO);
		dup2(saved_stdout, STDOUT_FILENO);
		printf("Log.txt open failed");
		close(saved_stdin);
		close(saved_stdout);
		return;
	}

	dup2(log_fd, STDIN_FILENO);
	dup2(original_stdout, STDOUT_FILENO);
	int currentline = 0;

	while(scanf("%c", &ch) == 1){
		if(currentline >= skip)
			printf("%c", ch);
		if(ch == '\n')
			currentline++;
	}
	fflush(stdout);
	close(log_fd);
	
	dup2(saved_stdin, STDIN_FILENO);
	dup2(saved_stdout, STDOUT_FILENO);
	close(saved_stdin);
	close(saved_stdout);
}


int main(){

	setvbuf(stdout, NULL, _IONBF,0);
	setvbuf(stdin, NULL, _IONBF,0);

	/* I am running this on WSL (Sublime Editor)
	This runtime environemnt caused my output buffer to not be flushed before a fork. 
	So I found a fix - disabling buffering entirely.
	Now the child processes don't inherit their parent's output buffer. */

	// Save original stdin/stdout for user interaction
	original_stdout = dup(STDOUT_FILENO);
	original_stdin = dup(STDIN_FILENO);

	pid_t pid = getpid();

	// FOLDER CREATION

	char foldername[128];
	snprintf(foldername, sizeof(foldername), "folder_%d", (int)pid);

	if(mkdir(foldername, 0755)!=0){
		// 0755 is the permissions for rwx for owner, group and others.
		printf("mkdir failed");
		return 1;
	}

	// FILE CREATION

	snprintf(contentpath, sizeof(contentpath), "%s/content.txt", foldername);
	snprintf(logpath, sizeof(logpath), "%s/logs.txt", foldername);

	int content_fd = open(contentpath, O_CREAT | O_WRONLY | O_APPEND, 0644);
	int log_fd = open(logpath, O_CREAT | O_WRONLY | O_APPEND, 0644);
	
	if(content_fd < 0 || log_fd < 0){
		printf("File creation failed");
		return 1;
	}
	
	close(content_fd);
	close(log_fd);

	// INPUT LOOP

	char input[1024];

	while(1){

		printf("Enter command:");
		scanf("%[^\n]%*c", input);
		printf("%s\n", input);
		struct CodeValue temp = get_command_info(input);

		//printf("CODE: %d N VALUE: %d\n", temp.code, temp.n);

		if(temp.code==6){
			close(original_stdout);
			close(original_stdin);
			return 0;
		}
		else if(temp.code==-1){
			printf("Unknown command.\n");
		}
		else if(temp.code==1){
			handle_input();
		}
		else if(temp.code==2){
			handle_print();
		}
		else if(temp.code==3){
			handle_first(temp.n, input);
		}
		else if(temp.code==4){
			handle_last(temp.n, input);
		}
		else if(temp.code==5){
			handle_log(temp.n);
		}
	}

	return 0;
}
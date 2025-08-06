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
int stdoutcopy, stdincopy;

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
	int contentfiledesc, logfiledesc;
	int stdinsaved = dup(STDIN_FILENO);
	int stdoutsaved = dup(STDOUT_FILENO);

	// redirecting to user terminal for input
	dup2(stdincopy, STDIN_FILENO);
	dup2(stdoutcopy, STDOUT_FILENO);
	printf("INPUT -> ");
	fflush(stdout);
	scanf("%[^\n]%*c", input);

	// Writing user input to content file
	contentfiledesc = open(contentpath, O_WRONLY | O_APPEND);
	if(contentfiledesc < 0){
		printf("File open failed");
		close(stdinsaved);
		close(stdoutsaved);
		return;
	}
	
	dup2(contentfiledesc, STDOUT_FILENO);
	printf("%s\n", input);
	fflush(stdout);
	close(contentfiledesc);

	logfiledesc = open(logpath, O_WRONLY | O_APPEND);
	if(logfiledesc < 0){
		dup2(stdinsaved, STDIN_FILENO);
		dup2(stdoutsaved, STDOUT_FILENO);
		printf("File open failed");
		close(stdinsaved);
		close(stdoutsaved);
		return;
	}
	
	dup2(logfiledesc, STDOUT_FILENO);
	printf("INPUT\n");
	fflush(stdout);
	close(logfiledesc);
	
	dup2(stdinsaved, STDIN_FILENO);
	dup2(stdoutsaved, STDOUT_FILENO);
	close(stdinsaved);
	close(stdoutsaved);
}

void handle_print(){

	int contentfiledesc, logfiledesc;
	int stdinsaved = dup(STDIN_FILENO);
	int stdoutsaved = dup(STDOUT_FILENO);

	contentfiledesc = open(contentpath, O_RDONLY);


	if(contentfiledesc < 0){
		printf("File open failed");
		close(stdinsaved);
		close(stdoutsaved);
		return;
	}

	dup2(contentfiledesc, STDIN_FILENO);
	dup2(stdoutcopy, STDOUT_FILENO);
	
	char ch;
	int result;
	while((result= scanf("%c", &ch))== 1) {
		printf("%c", ch);
	}
	
	/* I was facing an infinite loop after reading from file. So I use clearerr
	to reset errors when doing read-and-write operations with the file.*/

	if(result != 1) {
		clearerr(stdin);
	}
	
	printf("\n");
	fflush(stdout);
	close(contentfiledesc);

	logfiledesc = open(logpath, O_WRONLY | O_APPEND);
	if(logfiledesc < 0){
		dup2(stdinsaved, STDIN_FILENO);
		dup2(stdoutsaved, STDOUT_FILENO);
		printf("File open failed");
		close(stdinsaved);
		close(stdoutsaved);
		return;
	}
	
	dup2(logfiledesc, STDOUT_FILENO);
	printf("PRINT\n");
	fflush(stdout);
	close(logfiledesc);
	
	dup2(stdinsaved, STDIN_FILENO);
	dup2(stdoutsaved, STDOUT_FILENO);
	close(stdinsaved);
	close(stdoutsaved);
}

void handle_first(int n, char *msg){

	int contentfiledesc, logfiledesc;
	int stdinsaved = dup(STDIN_FILENO);
	int stdoutsaved = dup(STDOUT_FILENO);

	contentfiledesc = open(contentpath, O_RDONLY);
	if(contentfiledesc < 0){
		printf("File open failed");
		close(stdinsaved);
		close(stdoutsaved);
		return;
	}

	dup2(contentfiledesc, STDIN_FILENO);
	dup2(stdoutcopy, STDOUT_FILENO);

	char ch;
	int line_count = 0;
	int result;

	while ((result = scanf("%c", &ch)) == 1 && line_count < n) {
		printf("%c", ch);
		if (ch == '\n') {
			line_count++;
		}
	}
	
	if(result != 1) {
		clearerr(stdin);
	}
	
	fflush(stdout);
	close(contentfiledesc);

	logfiledesc = open(logpath, O_WRONLY | O_APPEND);
	if(logfiledesc < 0){
		dup2(stdinsaved, STDIN_FILENO);
		dup2(stdoutsaved, STDOUT_FILENO);
		printf("File open failed");
		close(stdinsaved);
		close(stdoutsaved);
		return;
	}
	
	dup2(logfiledesc, STDOUT_FILENO);
	printf("%s\n", msg);
	fflush(stdout);
	close(logfiledesc);
	
	dup2(stdinsaved, STDIN_FILENO);
	dup2(stdoutsaved, STDOUT_FILENO);
	close(stdinsaved);
	close(stdoutsaved);
}

void handle_last(int n, char *msg){

	int contentfiledesc, logfiledesc;
	int stdinsaved = dup(STDIN_FILENO);
	int stdoutsaved = dup(STDOUT_FILENO);

	// READING NUMBER OF LINES IN CONTENT.TXT BY CHECKING LOGS FOR 'INPUT'

	logfiledesc = open(logpath, O_RDONLY);
	if(logfiledesc < 0){
		printf("File open failed");
		close(stdinsaved);
		close(stdoutsaved);
		return;
	}

	dup2(logfiledesc, STDIN_FILENO);
	char ch;
	int line_count = 0;
	int result;

	while((result = scanf("%c", &ch)) == 1){
		if(ch == 'I'){
			line_count++;
			// skip rest of this line
			while((result = scanf("%c", &ch)) == 1 && ch != '\n');
			if(result != 1) break;
		}
		else{
			while((result = scanf("%c", &ch)) == 1 && ch != '\n');
			if(result != 1) break;
		}
	}
	
	if(result != 1) {
		clearerr(stdin);
	}
	
	close(logfiledesc);

	int skip = line_count - n;
	if(skip < 0)
		skip = 0;

	contentfiledesc = open(contentpath, O_RDONLY);
	if(contentfiledesc < 0){
		dup2(stdinsaved, STDIN_FILENO);
		dup2(stdoutsaved, STDOUT_FILENO);
		printf("File open failed");
		close(stdinsaved);
		close(stdoutsaved);
		return;
	}

	dup2(contentfiledesc, STDIN_FILENO);
	dup2(stdoutcopy, STDOUT_FILENO);
	int currentline = 0;

	while((result = scanf("%c", &ch)) == 1){
		if(currentline >= skip)
			printf("%c", ch);
		if(ch == '\n')
			currentline++;
	}
	
	if(result != 1) {
		clearerr(stdin);
	}
	
	fflush(stdout);
	close(contentfiledesc);

	// WRITE 'LAST N' INTO LOG.TXT

	logfiledesc = open(logpath, O_WRONLY | O_APPEND);
	if(logfiledesc < 0){
		dup2(stdinsaved, STDIN_FILENO);
		dup2(stdoutsaved, STDOUT_FILENO);
		printf("File open failed");
		close(stdinsaved);
		close(stdoutsaved);
		return;
	}
	
	dup2(logfiledesc, STDOUT_FILENO);
	printf("%s\n", msg);
	fflush(stdout);
	close(logfiledesc);
	
	dup2(stdinsaved, STDIN_FILENO);
	dup2(stdoutsaved, STDOUT_FILENO);
	close(stdinsaved);
	close(stdoutsaved);
}

void handle_log(int n){

	int logfiledesc;
	int stdinsaved = dup(STDIN_FILENO);
	int stdoutsaved = dup(STDOUT_FILENO);

	logfiledesc = open(logpath, O_RDONLY);
	if(logfiledesc < 0){
		printf("Log.txt open failed");
		close(stdinsaved);
		close(stdoutsaved);
		return;
	}

	dup2(logfiledesc, STDIN_FILENO);
	char ch;
	int line_count = 0;
	int result;

	while((result = scanf("%c", &ch)) == 1) {
		if(ch == '\n')
			line_count++;
	}
	
	if(result != 1) {
		clearerr(stdin);
	}
	
	close(logfiledesc);

	int skip = line_count - n;
	if(skip < 0)
		skip = 0;

	// reopen file for second pass
	logfiledesc = open(logpath, O_RDONLY);
	if(logfiledesc < 0){
		dup2(stdinsaved, STDIN_FILENO);
		dup2(stdoutsaved, STDOUT_FILENO);
		printf("Log.txt open failed");
		close(stdinsaved);
		close(stdoutsaved);
		return;
	}

	dup2(logfiledesc, STDIN_FILENO);
	dup2(stdoutcopy, STDOUT_FILENO);
	int currentline = 0;

	while((result = scanf("%c", &ch)) == 1){
		if(currentline >= skip)
			printf("%c", ch);
		if(ch == '\n')
			currentline++;
	}
	
	if(result != 1) {
		clearerr(stdin);
	}
	
	fflush(stdout);
	close(logfiledesc);
	
	dup2(stdinsaved, STDIN_FILENO);
	dup2(stdoutsaved, STDOUT_FILENO);
	close(stdinsaved);
	close(stdoutsaved);
}


int main(){

	setvbuf(stdout, NULL, _IONBF,0);
	setvbuf(stdin, NULL, _IONBF,0);

	/* I am running this on WSL (Sublime Editor)
	This runtime environemnt caused my output and input buffer 
	to not be flushed before a fork. So I found a fix - disabling buffering entirely.
	Now the child processes don't inherit their parent's output/input buffer. */

	stdoutcopy = dup(STDOUT_FILENO);
	stdincopy = dup(STDIN_FILENO);

	pid_t pid = getpid();

	// FOLDER CREATION

	char foldername[128];
	snprintf(foldername, sizeof(foldername), "folder_%d", (int)pid);

	if(mkdir(foldername, 0755)!=0){
		
		printf("mkdir failed");
		return 1;
	}

	// FILE CREATION

	snprintf(contentpath, sizeof(contentpath), "%s/content.txt", foldername);
	snprintf(logpath, sizeof(logpath), "%s/logs.txt", foldername);

	int contentfiledesc = open(contentpath, O_CREAT | O_WRONLY | O_APPEND, 0644);
	int logfiledesc = open(logpath, O_CREAT | O_WRONLY | O_APPEND, 0644);
	
	if(contentfiledesc < 0 || logfiledesc < 0){
		printf("File creation failed");
		return 1;
	}
	
	close(contentfiledesc);
	close(logfiledesc);

	// INPUT LOOP

	char input[1024];

	while(1){

		printf("Enter command:");
		fflush(stdout);
		scanf("%[^\n]%*c", input);
		printf("%s\n", input);
		fflush(stdout);
		struct CodeValue temp = get_command_info(input);

		//printf("CODE: %d N VALUE: %d\n", temp.code, temp.n);

		if(temp.code==6){
			close(stdoutcopy);
			close(stdincopy);
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
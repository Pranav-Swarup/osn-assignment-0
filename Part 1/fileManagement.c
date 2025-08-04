#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

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

struct CodeValue get_command_info(char *str) {

	struct CodeValue result = {CMD_UNKNOWN, -1};
	char input[128];
	strncpy(input, str, sizeof(input));
	input[sizeof(input) - 1] = '\0';

	int len = strlen(input);
	
	// Scrubbing trailing spaces.

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

void handle_input(FILE *contentptr, FILE *logptr){

	char input[1024];

	if(logptr == NULL || contentptr == NULL){
		printf("File open failed");
		return;
	}

	printf("INPUT -> ");
	scanf("%[^\n]%*c", input);

	for(int i = 0; input[i] != '\0'; i++) {
		fputc(input[i], contentptr);
	}
	fputc('\n', contentptr);
	

	const char *msg = "INPUT\n";
	for(int i = 0; msg[i] != '\0'; i++) {
		fputc(msg[i], logptr);
	}
}

void handle_print(FILE *contentptr, FILE *logptr){

	if(logptr == NULL || contentptr == NULL){
		printf("File open failed");
		return;
	}

	rewind(contentptr);

	int ch;
	while((ch = fgetc(contentptr)) != EOF) {
		printf("%c", ch);
	}
	printf("\n");

	const char *msg = "PRINT\n";
	for(int i = 0; msg[i] != '\0'; i++) {
		fputc(msg[i], logptr);
	}
}

void handle_first(int n, FILE *contentptr, FILE *logptr, char *msg){

	if(contentptr == NULL || logptr == NULL){
		printf("File open failed");
		return;
	}

	rewind(contentptr);

	int ch;
	int line_count = 0;

	while(!feof(contentptr) && line_count < n){
		printf("%c", ch);

		if(ch == '\n'){
			line_count++;
        }
    }

	for(int i = 0; msg[i] != '\0'; i++) {
		fputc(msg[i], logptr);
	}
	fputc('\n', logptr);
}

void handle_last(int n, FILE *contentptr, FILE *logptr, char *msg){

	if(contentptr == NULL || logptr == NULL){
		printf("File open failed");
		return;
	}

	rewind(contentptr);
	rewind(logptr);

	// READING NUMBER OF LINES IN CONTENT.TXT BY CHECKING LOGS FOR 'INPUT'

	int ch = fgetc(logptr);
	int line_count = 0;

	while(ch != EOF){
		if(ch == 'I'){
			line_count++;
		}
		else{
			while(ch != '\n' && ch != EOF)
			ch = fgetc(logptr);
		}
		ch = fgetc(logptr);
	}

	int skip = line_count - n;
	if(skip < 0)
		skip = 0;

	rewind(contentptr);
	int currentline = 0;

	while((ch = fgetc(contentptr)) != EOF){
		if(currentline >= skip)
			printf("%c", ch);
		if(ch == '\n')
			currentline++;
	}

	// WRITE 'LAST N' INTO LOG.TXT

	for(int i = 0; msg[i] != '\0'; i++){
		fputc(msg[i], logptr);
	}
	fputc('\n', logptr);
}

void handle_log(int n, FILE *logptr){

	if(logptr == NULL){
		printf("Log.txt open failed");
		return;
	}

	rewind(logptr);

	int ch;
	int line_count = 0;

	while((ch = fgetc(logptr)) != EOF) {
		if(ch == '\n')
			line_count++;
	}

	int skip = line_count - n;
	if(skip < 0)
		skip = 0;

	rewind(logptr);
	int currentline = 0;

	while((ch = fgetc(logptr)) != EOF){
		if(currentline >= skip)
			printf("%c", ch);
		if(ch == '\n')
			currentline++;
	}
}


int main(){

	setvbuf(stdout, NULL, _IONBF,0);

	/* I am running this on WSL (Sublime Editor)
	This runtime environemnt caused my output buffer to not be flushed before a fork. 
	So I found a fix - disabling buffering entirely.
	Now the child processes don't inherit their parent's output buffer. */

	pid_t pid = getpid();

	// FOLDER CREATION

	char foldername[128];
	//printf("Creating folder 'folder_%d'\n", (int)pid);
	snprintf(foldername, sizeof(foldername), "folder_%d", (int)pid);

	if(mkdir(foldername, 0755)!=0){

		// 0755 is the permissions for rwx for owner, group and others.
		printf("mkdir failed");
		return 1;
	}

	// FILE CREATION

	snprintf(contentpath, sizeof(contentpath), "%s/content.txt", foldername);
	snprintf(logpath, sizeof(logpath), "%s/logs.txt", foldername);

	FILE *contentptr = fopen(contentpath, "a+");
	FILE *logptr = fopen(logpath, "a+");
	
	if(!contentptr || !logptr){
		printf("File creation failed");
		return 1;
	}

	//printf("Files created. Shell started.\n");

	// INPUT LOOP

	char input[1024];

	while(1){

		printf("Enter command:");
		scanf("%[^\n]%*c", input);
		printf("%s\n", input);
		struct CodeValue temp = get_command_info(input);

		//printf("CODE: %d N VALUE: %d\n", temp.code, temp.n);

		if(temp.code==6){
			fclose(logptr);
			fclose(contentptr);
			// Exiting the shell here
			return 0;
		}
		else if(temp.code==-1){
			printf("Unknown command.\n");
		}
		else if(temp.code==1){
			handle_input(contentptr, logptr);
		}
		else if(temp.code==2){
			handle_print(contentptr, logptr);
		}
		else if(temp.code==3){
			handle_first(temp.n, contentptr, logptr, input);
		}
		else if(temp.code==4){
			handle_last(temp.n, contentptr, logptr, input);
		}
		else if(temp.code==5){
			handle_log(temp.n, logptr);
		}
	}

	return 0;
}

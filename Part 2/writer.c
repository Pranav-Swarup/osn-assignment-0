#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


int main(){

    FILE *fileptr = fopen("newfile.txt", "a+");

    if(fileptr == NULL){
        perror("Could not open file");
        return 1;
    }

    fprintf(fileptr, "Parent PID: %d\n", getppid());
    fclose(fileptr);

    return 0;
}

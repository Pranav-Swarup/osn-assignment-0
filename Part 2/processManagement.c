#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int task1(){


    int x = 25;
    pid_t pid = fork();

    if(pid < 0){
        perror("fork failed");
        return 1;
    }

    else if(pid == 0){

        printf("______TASK 1 (Child)_______\n");
        x += 10;
        printf("Child's PID = %d, Updated x value = %d\n", getpid(), x);
        exit(EXIT_SUCCESS);

    }

    else{

        printf("______TASK 1 (Parent)_______\n");
        x -= 5;
        printf("Parent's PID = %d, Updated x value = %d\n", getpid(), x);
        int status;
        
        if(waitpid(pid, &status, 0) == -1){
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

int task2(){

    pid_t pid = fork();

    if(pid < 0){
        perror("fork failed");
        return 1;
    }

    else if(pid == 0){

        printf("______TASK 2 (Child)_______\nWriting to newfile.txt\n");
        fflush(stdout);
        execlp("./writer", "writer", NULL);
        perror("exec failed");
        exit(EXIT_FAILURE);
    }

    else{

        printf("______TASK 2 (Parent)_______\nWaiting for child to finish\n");
        int status;
        if(waitpid(pid, &status, 0) == -1){
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

int task3(){


    pid_t pid = fork();

    if(pid < 0){
        perror("fork failed");
        return 1;
    }

    if(pid > 0){
        printf("______TASK 3 (Parent)_______\n");
        printf("Parent with PID %d, is exiting.\n", getpid());
        
    }
    else{
        sleep(2);
        printf("______TASK 3 (Child)_______\n");
        printf("Child PID %d, Updated Parent PID: %d\n", getpid(), getppid());
        exit(EXIT_SUCCESS);
    }

    return 0;
}

int main(){

    if(task1() != 0)
        return 1;
    sleep(1);
    
    if(task2() != 0)
        return 1;
    sleep(1);
    
    if(task3() != 0)
        sleep(3);
        return 1;
    sleep(3);

    return 0;

}
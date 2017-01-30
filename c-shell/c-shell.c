#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

//Function prototypes
void parseLine(char *line, char **args);
void executeLine(char **args);
void pipeArgs(char **args1, char **args2);

//Constants
#define MAX_ARG_LENGTH 256
#define MAX_ARGS 16

void main() {

    char line[MAX_ARG_LENGTH]; //Stores a line of chars read from stdin
    char line2[MAX_ARG_LENGTH]; //Additional line storage for piping
    char *args[MAX_ARGS]; //Stores parsed command/arguments    
    char *args2[MAX_ARGS]; //Additional command/arguments storage for piping    

    //Variables used when reading chars in from stdin
    char c;     
    int i;
    int j;    
    int chars_read;
    int pipe_read; 

    //Display info/instructions for shell use
    printf("CS 3113 HW2 - ande6432\n");
    printf("To exit, type \"exit\" or CTRL + C\n");

    //Each iteration of loop handles one command with arguments
    //Exit on "exit", CTRL + C
    while (1) {

        //Reset piping boolean
        pipe_read = 0;

        //Reset counter variable
        i = 0;
        j = 0;

        //Shell line formatting
        printf("\n");
        printf(">");
        fflush(stdout);

        //Each iteration reads in one char from stdin to 'line'
        //Exit when end of file or newline character is found
        while ((chars_read = read(0, &c, 1)) > 0 && (c != '\n')) {
            
            //Update piping boolean if "|" is found
            if (c == '|') {

                pipe_read = 1;
            }
            else if (!pipe_read) {

                line[i] = c;
                i++;
            }
            else {

                line2[j] = c;
                j++;
            }
        }

        //Add final terminating characters
        if (!pipe_read) {

            line[i] = '\0';
        }         
        else {
    
            line[i - 1] = '\0';
            line2[j] = '\0';
        }
    
        //Exit conditions
        if (strcmp(line, "exit") == 0 || chars_read == 0) {

            exit(0);
        }
    
        if (!pipe_read) { //No piping case

            parseLine(line, args); //Parse 'line' and store result in 'args'
        
            printf("\n"); //Shell formatting

            executeLine(args); //Execute command/arguments stored in 'args'
        }
        else { //Piping case

            parseLine(line, args);    
            parseLine(line2, args2);

            printf("\n"); //Shell formatting

            pipeArgs(args, args2);
        }
    }
}

//Takes an array of chars 'line' and parses into words, ignoring whitespace
//Words stored in 'args'
void parseLine(char *line, char **args) {
    
    //Each iteration replaces leading whitespace with terminating chars
    //and stores next word of 'line' in 'args'
    while (*line != '\0') {
        
        //Replace leading whitespace with terminating chars
        while (*line == ' ' || *line == '\t' || *line == '\n') {
            
            *line = '\0';
            *line++;
        } 
        
        //Store next word in 'args'
        *args = line;
        *args++;
    
        //Advance pointer to end of word for next loop iteration
        while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n') {
            
            line++;
        }
        //Add terminating char to end of 'args'
        *args = '\0';
    }
}

//Creates a child process to execute command/arguments stored in 'args'
//Parent process waits until child process completion
void executeLine(char **args) {
    
    int status; //Used with wait()
    pid_t pid; //Used with fork()

    //Create child process
    pid = fork();
    
    switch(pid) {
        //Error case
        case -1 : 
            printf("Fork failed\n");
            exit(-1);

        //Child case
        case 0 : 
            //Execute command/arguments stored in 'args'
            //Notify user of invalid commands
            if (execvp(*args, args) == -1) {

                printf("Invalid Command\n");   
            }
            exit(0);
        
        //Parent case
        default : 
            while (wait(&status) != pid) {

                //wait                      
            }
            break;
    }
}

//Pipe the output of the command/arguments in 'args1' to the input of
//the commands/arguments in 'args2'
void pipeArgs(char **args1, char **args2) {

    //Create pipe
    int fd[2];
    pipe(fd);
    
    pid_t pid1;
    pid_t pid2;

    //Create first child process for piping
    pid1 = fork(); 

    //Child process for left side of pipe
    switch (pid1) {
        //Error case
        case -1 : 
            printf("Fork failed\n");
            exit(-1); 
            
        //Child case
        case 0 : 
            dup2(fd[1], 1); //Replace stdout with pipe write

            close(fd[0]); //This process doesn't need to read from the pipe

            execvp(*args1, args1); //Execute command left of pipe

            break;
    }
    
    //Create second child process for piping
    pid2 = fork();

    switch (pid2) {
        //Error case
        case -1 : 
            printf("Fork failed\n");
            exit(-1); 

        //Child case
        case 0 : 
            dup2(fd[0], 0); //Replace stdin with pipe read

            close(fd[1]); //This process doesn't need to write to the pipe

            execvp(*args2, args2); // Execute command right of pipe

            break;
    }

    //Close pipe
    close(fd[0]);
    close(fd[1]);
    
    //Wait for child processes to end
    wait(0);
    wait(0); 
}

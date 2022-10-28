#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


struct Pipes{
    int fd[2];
};
typedef struct Pipes Pipes;

const int READ = 0;
const int WRITE = 1;

size_t Split(const char *command, const char *delimeter, char **commands){
    size_t command_count = 0;

    char *command_copy = strdup(command);

    char *token = strtok(command_copy, delimeter); //primer

    while (token != NULL){
        commands[command_count] = malloc(sizeof(char) * strlen(token));
        strcpy(commands[command_count++], token);

        token = strtok(NULL, delimeter);

    }
    commands[command_count] = NULL;
    return command_count;
}

void eggXecute(char **command, Pipes *pipes, const size_t current_pipe_index, const int command_count){
    // ["ls",-l", "-a", NULL]
    pid_t pid = fork();

    if(pid == 0){ //child
        // 1. first command
        if(current_pipe_index == 0){
            close(pipes[current_pipe_index].fd[READ]);
            //set up pipe
            if(dup2(pipes[current_pipe_index].fd[WRITE], STDOUT_FILENO) < 0){
                char msg[256];
                strcat("Failed to redirect for Command: ", command[0]);
                perror(msg);
                exit(EXIT_FAILURE);
            }
        }
        // 2. last command
        else if(current_pipe_index == command_count -1){
            close(pipes[current_pipe_index - 1].fd[WRITE]);
            //set up pipe
            if(dup2(pipes[current_pipe_index - 1].fd[READ], STDIN_FILENO) < 0){
                char msg[256];
                strcat("Failed to redirect for Command: ", command[0]);
                perror(msg);
                exit(EXIT_FAILURE);
            }

        }
        // 3. middle command
        else{
            
            //set up pipe
            close(pipes[current_pipe_index - 1].fd[WRITE]);
            if(dup2(pipes[current_pipe_index - 1].fd[READ], STDIN_FILENO) < 0){
                char msg[256];
                strcat("Failed to redirect for Command: ", command[0]);
                perror(msg);
                exit(EXIT_FAILURE);
            }
            close(pipes[current_pipe_index].fd[READ]);
            if(dup2(pipes[current_pipe_index].fd[WRITE], STDOUT_FILENO) < 0){
                char msg[256];
                strcat("Failed to redirect for Command: ", command[0]);
                perror(msg);
                exit(EXIT_FAILURE);
            }
        }
        execvp(command[0], command);
    }
    else{//parent
        if( current_pipe_index != 0 )
            close(pipes[current_pipe_index - 1].fd[WRITE]);

        wait(NULL);
    }


}
void printPar(size_t cmdCount, char **commands){
    for(size_t i = 0; i < cmdCount; ++i){
        printf("%s\n", commands[i]);
    }

}
int main(){
    char *commands[256];
    
    char simple_command[256];
    const char *PIPE = "|";
    const char *SPACE = " ";
    const char *redirect = ">";
    
    printf("Evgeny's shell: ");
    scanf("%[^\n]", simple_command);
    size_t command_count = Split(simple_command, PIPE, commands);
    
    if(command_count == 1){
        char *single_command[256];
        size_t single_command_count = Split(commands[0], SPACE, single_command);
        execvp(single_command[0], single_command);
    }
    else{
        Pipes *pipes = malloc(sizeof(Pipes) * command_count - 1);
        for(size_t i = 0; i < command_count - 1; ++i){
            pipe(pipes[i].fd);
        }

        for(size_t i = 0; i < command_count; ++i){
            char *single_command[256];
            size_t single_command_count = Split(commands[i], SPACE, single_command);
            single_command[single_command_count] = NULL;
            eggXecute(single_command, pipes, i, command_count);
        }
    }

    

    return 0;
}
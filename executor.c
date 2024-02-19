#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <syslog.h>

#include "executor.h"

void execute(char *task, int mode, char *output_file) {
    char **commands;
    int num_commands;
    int i;
    pid_t pid;

    pid_t pids[10];
    int fds[10][2];

    int outfd;

    if(mode ==0) outfd = STDOUT_FILENO;
    else if(mode ==1) outfd = STDERR_FILENO;
    else outfd = open(output_file, O_CREAT | O_WRONLY | O_APPEND, 0644);
    write(outfd, task, strlen(task));
    write(outfd, "\n", 1);

    parse_command(task, &commands, &num_commands);

    for (int i = 0; i < num_commands; i++) {

        if(i != num_commands - 1)
            if ( pipe(fds[i]) == -1) {
                syslog(LOG_DEBUG, "Pipe create failed");
                exit(EXIT_FAILURE);
            }

        pid = fork();
        if (pid == (pid_t) 0) {
        // Child process
            if (i == num_commands - 1) {
                dup2(outfd, STDOUT_FILENO);
            }
            else if (i < num_commands - 1) {
                dup2(fds[i][1], STDOUT_FILENO);
                close(fds[i][1]);
            }

            if (i == 0) {
                dup2(STDIN_FILENO, STDIN_FILENO);
            }
            else {
                dup2(fds[i-1][0], STDIN_FILENO);
                close(fds[i-1][0]);
            }

            // isolate parameters from program
            int j=0;
            char *args[10];

            char *arg = strtok(commands[i], " ");
            args[j++] = arg;      

            while ((arg = strtok(NULL, " ")) != NULL && j < 9) {
                args[j++] = arg;
            }
            args[j] = NULL;
            // execute
            if (j == 1) execlp(args[0], args[0], NULL);
            else  execvp(args[0], args);

            syslog(LOG_DEBUG, "execlp failed");
            exit(EXIT_FAILURE);
        }
        else {
        // Parent process
            pids[i] = pid;
            close(fds[i][1]);
            if(i>0){
                close(fds[i-1][0]);
            }
            waitpid(pids[i], NULL, 0);
        }
    }

     // Wait for all child processes to finish
    for (int i = 0; i < num_commands; i++) {
        waitpid(pids[i], NULL, 0);
        close(fds[i][0]);
        close(fds[i][1]);
    }

    write(outfd, "\n", 1);
    close(outfd);
    free(commands);
    syslog(LOG_DEBUG, "Execute done");
}

void parse_command(char *input, char ***commands, int *num_commands) {
    int i,x = 0,last =0;

    *num_commands = 1;
    for (i = 0; input[i] != '\0'; i++) {
        if (input[i] == '|') {
            (*num_commands)++;
        }
    }
    *commands = malloc((*num_commands) * sizeof(char *));
    
    for(i = 0 ; input[i] != '\0'; i++){
        if (input[i] == '|') {
            int size = i - last +1;
            (*commands)[x] = malloc(sizeof(char)*size);
            memcpy((*commands)[x], &input[last], size-1);
            (*commands)[x][size-1] = '\0';
            x++;
            last = i+1;
        }
    }

    int size = i - last + 1;
    (*commands)[x] = malloc(size * sizeof(char));

    memcpy((*commands)[x], &input[last], size-1);

    (*commands)[x][size-1] = '\0';
}
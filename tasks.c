#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <syslog.h>

#include "tasks.h"

void sortTasks(TASKARRAY *tasks) {
    int i, j;
    TASK temp;

    for (i = 0; i < tasks->size - 1; i++) {
        for (j = i + 1; j < tasks->size; j++) {
            if (tasks->tab[i].hour > tasks->tab[j].hour ||
                (tasks->tab[i].hour == tasks->tab[j].hour && tasks->tab[i].minute > tasks->tab[j].minute)) {
                // Swap tasks
                temp = tasks->tab[i];
                tasks->tab[i] = tasks->tab[j];
                tasks->tab[j] = temp;
            }
        }
    }
}

void loadTasks(TASKARRAY *tasks, char *filename) {
    int fileDes;
    ssize_t bytes_read;
    char buffer[1024];
    int lines = 0;

    if(tasks->size>0){
        tasks->size = 0;
        free(tasks->tab);
    }

    fileDes = open(filename, O_RDONLY);
    if (fileDes == -1) {
        syslog(LOG_DEBUG, "File open error");
        exit(EXIT_FAILURE);
    }
    // Count lines in file
    while ((bytes_read = read(fileDes, buffer, 1024)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n')
                lines++;
        }
    }

    // Allocate memory for tasks
    tasks->tab = malloc(sizeof(*tasks->tab) * lines);
    if (tasks->tab == NULL) {
        syslog(LOG_DEBUG, "Memory allocate error");
        exit(EXIT_FAILURE);
    }

    tasks->size = lines;
    lseek(fileDes, 0, SEEK_SET);

    // Read tasks
    int total = 0, i = 0;
    while ((bytes_read = read(fileDes, buffer + total, 1)) > 0) {
        if (buffer[total] == '\n') {
            buffer[total] = '\0';
            total = 0;
            sscanf(buffer, "%d:%d:%[^:]:%d", &(tasks->tab[i].hour), &(tasks->tab[i].minute), tasks->tab[i].command, &(tasks->tab[i].mode));
            i++;
        } 
        else {
            total++;
        }

    }
    close(fileDes);

    if (i != lines) {
        syslog(LOG_DEBUG, "File format error");
        exit(EXIT_FAILURE);
    }


}
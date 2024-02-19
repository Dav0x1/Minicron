#ifndef TASKS_H
#define TASKS_H

typedef struct {
    int hour;
    int minute;
    char command[100];
    int mode;
}TASK;

typedef struct {
    TASK *tab;
    int size;
}TASKARRAY;

void sortTasks(TASKARRAY *tasks);
void loadTasks(TASKARRAY *tasks, char *filename);

#endif
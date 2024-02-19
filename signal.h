#ifndef SIGNAL_H
#define SIGNAL_H

extern TASKARRAY tasks;
extern char taskfile[50];
extern int currentTask;

void SIGINTHandler(int signum);
void SIGUSR1Handler(int signum);
void SIGUSR2Handler(int signum);

#endif
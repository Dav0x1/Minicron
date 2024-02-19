#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include "tasks.h"
#include "executor.h"
#include "signal.h"

/* Global variables */
TASKARRAY tasks;
pid_t pid;
pid_t sid;
int currentTask = 0;
char taskfile[50];

int main(int argc, char* argv[]){
/* Open Log*/
	openlog("minicron", LOG_PID | LOG_CONS, LOG_USER);
/* Signal handling*/
	signal(SIGINT,SIGINTHandler);
	signal(SIGUSR1,SIGUSR1Handler);
	signal(SIGUSR2,SIGUSR2Handler);
/* Check amount of parameters */
	if(argc!=3){
		syslog(LOG_DEBUG, "Program require parameters <taskfile> <outfile>");
		exit(EXIT_FAILURE);
	}
	strcpy(taskfile,argv[1]);
/* Load and sort tasks*/
	loadTasks(&tasks,argv[1]);
	sortTasks(&tasks);
/* Create daemon */
	pid = fork();
	/* if failed */
	if(pid<0){
		syslog(LOG_DEBUG, "Fork failed");
		exit(EXIT_FAILURE);
	}
	/* Child process created successfully, so close parent process*/
	if(pid>0) exit(EXIT_SUCCESS);
	/* Change the file mod mask */
	umask(0);
	/* Create a new SID for the child process */
	sid= setsid();
	if(sid<0){
		syslog(LOG_DEBUG, "Setsid failed");
		exit(EXIT_FAILURE);
	}
	/* Change the current working directory */
	//if((chdir("/"))<0) exit(EXIT_FAILURE);
	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
/* Daemon loop */
	while(1){
		time_t t = time(NULL);
		struct tm *local_time = localtime(&t);
		/* time in seconds*/
		int waitTime = (((tasks.tab[currentTask].hour*60)+tasks.tab[currentTask].minute)*60)
			      -((((local_time->tm_hour*60)+local_time->tm_min)*60)+local_time->tm_sec);
		while(waitTime>0){
			sleep(waitTime);
			t = time(NULL);
			local_time = localtime(&t);
			/* time in seconds*/
			waitTime = (((tasks.tab[currentTask].hour*60)+tasks.tab[currentTask].minute)*60)
			      -((((local_time->tm_hour*60)+local_time->tm_min)*60)+local_time->tm_sec);
		};
		syslog(LOG_DEBUG, "Executing task: %s",tasks.tab[currentTask].command);
		execute(tasks.tab[currentTask].command,tasks.tab[currentTask].mode,argv[2]);
		currentTask++;
		if(currentTask>=tasks.size) break;
	}
	/* Done */
	closelog();
	exit(EXIT_SUCCESS);
}
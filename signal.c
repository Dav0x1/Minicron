#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include "tasks.h"

#include "signal.h"

void SIGINTHandler(int signum){
	syslog(LOG_DEBUG, "Program has stopped by SIGINT signal");
	closelog();
	exit(EXIT_SUCCESS);
}
void SIGUSR1Handler(int signum){
	loadTasks(&tasks,taskfile);
	sortTasks(&tasks);
	syslog(LOG_DEBUG, "Tasks have been reload SIGUSR1");
}
void SIGUSR2Handler(int signum){
	int i;
	syslog(LOG_DEBUG, "Tasks to execute");
	for(i = currentTask;i<tasks.size;i++){
		syslog(LOG_DEBUG, "%s",tasks.tab[i].command);
	}
}
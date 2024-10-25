#ifndef _PR_H
#define _PR_H

int jobs(int *bgPID, char **bgCmd, int numArgs, char *args[]);
int allJobs(int *bgPID, char **bgCmd);
int runJobs(int *bgPID, char **bgCmd);
int stopJobs(int *bgPID, char **bgCmd);
void sigHelp();
int sig(int pid, int sigNum);
int fg(int pid);

#endif
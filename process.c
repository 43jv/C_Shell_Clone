#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "process.h"
#define SIZE 1024

int jobs(int *bgPID, char **bgCmd, int numArgs, char *args[])
{
    bool sFlag = false, rFlag = false;
    char type = '\0';
    int error = 0;
    int opt;

    // parse through cmd line arguments
    while ((opt = getopt(numArgs, args, "-:sr")) != -1)
    {
        switch (opt)
        {
        // case for types
        case 's':
            type = 's';
            sFlag = true;
            break;
        case 'r':
            type = 'r';
            rFlag = true;
            break;
        case ':':
            // option is not been set so make sure to send error to user.
            fprintf(stderr, "missing argument for %c\n", optopt);
            errno = EINVAL;
            perror("");
            return 22;
            break;
        // check for all non optional arguments.
        case 1:
            // too many arguments are being used within discover.
            if (optind > 5)
            {
                errno = EINVAL;
                perror("Too many arguments declared in discover");
                return 22;
            }
            break;
        // check all other options
        case '?':
            errno = EINVAL;
            fprintf(stderr, "option -%c does not exist ", optopt);
            perror("");
            return 22;
            break;
        }
    }

    // type has not been set so all types will be searched
    if (type == '\0' || (sFlag == true && rFlag == true))
        allJobs(bgPID, bgCmd);
    // type has been set so type will be assessed.
    if (type == 's' && sFlag == true)
        stopJobs(bgPID, bgCmd);
    if (type == 'r' && rFlag == true)
        runJobs(bgPID, bgCmd); 
    return 0;
}
int allJobs(int *bgPID, char **bgCmd)
{
    char fileName[SIZE], st;
    FILE *fp;
    for (int i = 1; i < 100; i++)
    {
        int b = bgPID[i];
        if (!b)
            continue;

        sprintf(fileName, "/proc/%d/stat", b);
        // Open file in read mode
        fp = fopen(fileName, "r");
        // Error Handling
        if (fp == NULL)
        {
            perror("Error: ");
            return -1;
        }
        else
        {
            // Scan variables from file
            fscanf(fp, "%*d %*s %c", &st);
            // Print appropriate msg
            if (st == 'T')
                printf("[%d] Stopped %s [%d]\n", i, bgCmd[i], bgPID[i]);
            else
                printf("[%d] Running %s [%d]\n", i, bgCmd[i], bgPID[i]);
        }
        fclose(fp);
    }
    return 0;
}
int stopJobs(int *bgPID, char **bgCmd)
{
    char fileName[SIZE], st;
    FILE *fp;
    for (int i = 1; i < 100; i++)
    {
        int b = bgPID[i];
        if (!b)
            continue;

        sprintf(fileName, "/proc/%d/stat", b);
        // Open file in read mode
        fp = fopen(fileName, "r");
        // Error Handling
        if (fp == NULL)
        {
            perror("Error: ");
            return -1;
        }
        else
        {
            // Scan variables from file
            fscanf(fp, "%*d %*s %c", &st);
            // Print appropriate msg
            if (st == 'T')
                printf("[%d] Stopped %s [%d]\n", i, bgCmd[i], bgPID[i]);
        }
        fclose(fp);
    }
    return 0;
}

int runJobs(int *bgPID, char **bgCmd)
{
    char fileName[SIZE], st;
    FILE *fp;
    for (int i = 1; i < 100; i++)
    {
        int b = bgPID[i];
        if (!b)
            continue;

        sprintf(fileName, "/proc/%d/stat", b);
        // Open file in read mode
        fp = fopen(fileName, "r");
        // Error Handling
        if (fp == NULL)
        {
            perror("Error: ");
            return -1;
        }
        else
        {
            // Scan variables from file
            fscanf(fp, "%*d %*s %c", &st);
            // Print appropriate msg
            if (st != 'T')
                printf("[%d] Running %s [%d]\n", i, bgCmd[i], bgPID[i]);
        }

        fclose(fp);
    }

    return 0;
}
// Displays list of supported signals
void sigHelp()
{
    printf("LIST OF SUPPORTED SIGNALS\n");
    printf("Signal\t\tNumber\n────────────────────────\n");
    printf("SIGHUP\t\t 1\nSIGINT\t\t 2\nSIGQUIT\t\t 3\nSIGILL\t\t 4\nSIGTRAP\t\t 5\nSIGABRT\t\t 6\nSIGIOT\t\t 6\nSIGBUS\t\t 7\nSIGFPE\t\t 8\nSIGKILL\t\t 9\nSIGUSR1\t\t 10\nSIGSEGV\t\t 11\nSIGUSR2\t\t 12\nSIGPIPE\t\t 13\nSIGALRM\t\t 14\nSIGTERM\t\t 15\nSIGSTKFLT\t 16\nSIGCHLD\t\t 17\nSIGCONT\t\t 18\nSIGSTOP\t\t 19\nSIGTSTP\t\t 20\nSIGTTIN\t\t 21\nSIGTTOU\t\t 22\nSIGURG\t\t 23\nSIGXCPU\t\t 24\nSIGXFSZ\t\t 25\nSIGVTALRM\t 26\nSIGPROF\t\t 27\nSIGWINCH\t 28\nSIGIO\t\t 29\nSIGPWR\t\t 30\nSIGSYS\t\t 31\n");
}
// Executes sig function, bg function
int sig(int pid, int sigNum)
{
    if (sigNum < 1 || sigNum > 31)
    {
        printf("Error: Not a Valid Signal\n");
        sigHelp();
        return -1;
    }
    int k = kill(pid, sigNum);
    if (k >= 0)
        return 0;
    else if (k < 0)
    {
        perror("Error: ");
        return -1;
    }
}
// Executes fg function
int fg(int pid)
{
    int st, o = 0, k;
    pid_t p = getpgid(0);
    // Set the process group ID of the process matching PID to PGID.
    // If PID is zero, the current process's process group ID is set.
    // If PGID is zero, the process ID of the process is used.
    setpgid(pid, p);
    bool f1, f2;
    // Ignore bkg read and write to terminal
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    // Set the foreground process group ID to PID.
    tcsetpgrp(o, pid);
    k = kill(pid, SIGCONT);
    if (k >= 0)
        ;
    else
        perror("Error: ");
    // Wait for child process to die and Return status for stopped children
    waitpid(pid, &st, WUNTRACED);
    pid_t g = getpgid(0);
    tcsetpgrp(o, g);
    // Take default actions for bkg read/write to terminal
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    if (!WIFSTOPPED(st))
        ;
    else
        return 0;
    if (!WIFEXITED(st))
        return (-pid);
    else
        return pid;
}
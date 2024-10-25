#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "command.h"

void getCmd(char *cmd, char ***acCmds)
{
  long unsigned size = 1024;
  char *str;
  // Read input and support autocomplete
  FILE* rl_instream = stdin;  
  str = readline(">");
  char *tmpCmd[20];
  tmpCmd[0] = NULL;
  if (strlen(str) != 0)
  {
    strcpy(cmd, str);
    add_history(cmd);
  }
  char *token;

  // Divide into tokens based on ;
  token = strtok(str, ";");
  long long int i1 = 0;
  while (token != NULL)
  {
    tmpCmd[i1] = token;
    tmpCmd[i1 + 1] = NULL;
    token = strtok(NULL, ";");
    i1++;
  }
  acCmds[0] = NULL;
  int i = 0;
  while (tmpCmd[i] != NULL)
  {
    acCmds[i + 1] = NULL;
    unsigned long sz = 20 * sizeof(char *);
    acCmds[i] = malloc(sz);
    acCmds[i][0] = NULL;
    // Handle piping commands
    char *token = strtok(tmpCmd[i], "|");
    int j = 0;
    while (token != NULL)
    {
      acCmds[i][j + 1] = NULL;
      acCmds[i][j] = token;
      token = strtok(NULL, "|");
      j++;
    }
    i++;
  }
}
int runCmd(int bkg, char **args)
{
  int pid = fork();
  if (pid == -1)
  {
    perror("Error: ");
    return -1;
  }
  // If fork returns valid process ID
  if (pid)
  {
    if (!bkg)
    {
      // Ignoring signals of bkg read and write to control terminal
      // Give child process access to terminal
      signal(SIGTTIN, SIG_IGN);
      signal(SIGTTOU, SIG_IGN);
      int st;
      tcsetpgrp(0, pid);
      waitpid(pid, &st, WUNTRACED);
      tcsetpgrp(0, getpgid(0));
      int w, y;
      // informs the kernel that there is no user signal handler for the given signal,
      // and that the kernel should take default action for it
      signal(SIGTTIN, SIG_DFL);
      signal(SIGTTOU, SIG_DFL);
      w = WEXITSTATUS(st);
      // If exit status of child process returns error
      if (w == 1)
        return -1;
      // Find out if child process has stopped
      y = WIFSTOPPED(st);
      // Return pid if stopped
      if (y)
        return pid;
    }
    if (bkg)
    {
      fprintf(stdout, "%s [%d]\n", args[0], pid);
      return pid;
    }
  }
  if (!pid)
  {
    setpgid(0, 0);
    // Execute command taken from command line arg
    int x = execvp(args[0], args);
    // Error Handling
    if (x < 0)
    {
      perror("Error");
      exit(0);
    }
  }
  return 0;
}

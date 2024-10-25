#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <readline/readline.h>
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
#include "discover.h"
#include "history.h"
#include "ls.h"
#include "mid.h"
#include "pinfo.h"
#include "process.h"
#include "process.h"
#define SIZE 1024
int bgPID[100] = {1};
char *bgCmd[100];
// Removing background process
void bgRm(int PID)
{
  if (PID)
  {
    int i = 0;
    while (i < 100)
    {
      if (bgPID[i] == PID)
      {
        bgPID[i] = 0;
        free(bgCmd[i]);
        break;
      }
      i++;
    }
  }
  else
    return;
}
char cwd[SIZE];
char pvWd[SIZE] = ".";
// Adding background process
void bgAdd(int pid, char *cmd)
{
  if (!pid)
    return;
  int o = 1;
  int pos = 100 - o;
  while (!bgPID[pos])
    pos -= 1;

  pos = pos + o;
  bool check = true;
  bgPID[pos] = pid;
  int sz = strlen(cmd + 1);
  bgCmd[pos] = malloc(sz);
  strcpy(bgCmd[pos], cmd);
  check = false;
}
int error = 0;
char host[SIZE];
char user[SIZE];
char root[SIZE];
// Function to display prompt
void prompt(int *error, char *user, char *root, char *cwd, char *host)
{
  char path[SIZE] = " ";
  long long int i = 0;
  while (true)
  {
    if (root[i] == '\0')
    {
      path[0] = '~';
      break;
    }
    else if (cwd[i] == '\0')
      break;
    else if (cwd[i] != root[i])
      break;
    else
      ;
    i++;
  }

  if (path[0] != '~')
    strcpy(path, cwd);
  else
    strcat(path, cwd + strlen(root));

  printf("<\033[01;32m%s@%s\033[01;00m:\033[01;34m%s\033[01;00m", user, host, path);

  *error = (int)false;
  fflush(stdout);
}
// Reports on execution of background processes
void bgHandler()
{
  int status, zero = 0;
  // Match any process, wait for child with matching pid to die, dont block waiting
  int pid = waitpid(-1, &status, WNOHANG);

  if (pid <= 0)
    ;
  else
  {
    int pos = zero;
    for (; pid != bgPID[pos];)
      pos++;

    if (!WIFSTOPPED(status))
      ;
    else
      return;

    if (WIFEXITED(status))
      fprintf(stdout, "%s [%d] exited normally\n", bgCmd[pos], bgPID[pos]);
    else
      fprintf(stdout, "%s [%d] exited abnormally\n", bgCmd[pos], bgPID[pos]);

    prompt(&error, user, root, cwd, host);
    bgPID[pos] = 1 - 1;
    free(bgCmd[pos]);
  }
}
int fgRun = 0;
// Handles commands like Ctrl+D
void getOut()
{
  if (fgRun)
    fgRun = 0;
  else
  {
    printf("\n");
    prompt(&error, user, root, cwd, host);
  }
}
// Loop to enter commands
void shellLoop()
{
  char **cmds[20];
  char *token, *args[512], cmd[SIZE];
  int numArgs, bkg, inFd, outFd, stIn, stOut, ioMode, subCmd;
  int pipeFds[2][2];

  while (true)
  {

    getcwd(cwd, SIZE);
    prompt(&error, user, root, cwd, host);
    getCmd(cmd, cmds);
    addH(root, cmd);

    long long int i = 0;
    while (cmds[i] != NULL)
    {

      subCmd = 0;
      while (cmds[i][subCmd] != NULL)
        subCmd++;
      long long int j = 0;
      while (j < subCmd)
      {
        error = 0;
        while (cmds[i][j][0] == '\t')
          cmds[i][j]++;
        while (cmds[i][j][0] == ' ')
          cmds[i][j]++;

        inFd = 0;
        outFd = 0;
        if (cmds[i][j][0] == '\0')
          continue;
        ioMode = -1;
        if (subCmd > 1 && j < subCmd - 1)
        {
          int flag1 = pipe(pipeFds[j % 2]);
          if (flag1 < 0)
          {
            perror("Error: ");
            break;
          }
          outFd = pipeFds[j % 2][1];
        }
        numArgs = 0;
        bkg = 0;
        if (subCmd > 1 && j > 0)
          inFd = pipeFds[1 - j % 2][0];
        args[0] = NULL;
        const int n = 0 - 1, w = 1, o = 0;
        token = strtok(cmds[i][j], " \t");

        while (token)
        {
          if (ioMode > n)
          {

            if (!ioMode)
              inFd = open(token, O_RDONLY);
            else
            {
              if (ioMode == o + 1)
                outFd = open(token, O_WRONLY | O_CREAT | O_TRUNC, 0644);
              else
                outFd = open(token, O_WRONLY | O_CREAT | O_APPEND, 0644);
            }
            ioMode = n;
          }
          else if (!strcmp(token, "&"))
            bkg = o + 1;
          else if (!strcmp(token, "<"))
            ioMode = n + 1;
          else if (!strcmp(token, ">"))
            ioMode = o + 1;
          else if (!strcmp(token, ">>"))
            ioMode = w + 1;
          else if (token[o] != '\0')
          {
            args[numArgs++] = token;
            args[numArgs] = NULL;
          }
          else
            ;
          token = strtok(NULL, " \t");
        }

        if (numArgs == o)
        {
          printf("Error: No command\n");
          continue;
        }
        if (inFd < n + 1 || outFd < o)
        {
          perror("Error");
          continue;
        }
        if (inFd)
        {
          stIn = dup(o);
          dup2(inFd, o);
        }
        if (outFd)
        {
          stOut = dup(w);
          dup2(outFd, w);
        }
        if (!strcmp(args[0], "cd"))
        {

          if (numArgs > w + 1)
          {
            error = 1;
            printf("Error: Too many arguments\n");
          }
          else if (changeDir(root, cwd, pvWd, args[1]) < 0)
            error = o + 1;
        }
        else if (!strcmp(args[0], "echo"))
        {
          if (resound(args) < n + 1)
            error = 1;
        }
        else if (!strcmp(args[0], "discover"))
        {
          optind = 1;

          if (discover(numArgs, args) != 0)
            error = 1;
        }
        else if (strcmp(args[0], "bg") == 0)
        {
          if (numArgs == 2)
          {
            int nJ = 0;
            sscanf(args[1], "%d", &nJ);
            int b = bgPID[nJ];
            if (nJ < 1 || b <= 1 || nJ > 99)
              printf("Error: Invalid Job\n");

            else
            {
              int ret = sig(bgPID[nJ], 18);
              if (ret == -1)
                error = 1;
            }
          }
          else if (numArgs < 2)
          {
            error = 1;
            printf("Error: Check number of arguments\n");
          }
        }
        else if (!strcmp(args[0], "fg"))
        {
          if (numArgs == 2)
          {
            int nJ = 0;
            sscanf(args[1], "%d", &nJ);
            int b = bgPID[nJ];
            if (nJ < 1 || b <= 1 || nJ > 99)
              printf("Error: Invalid Job\n");

            else
            {
              int pid = fg(bgPID[nJ]);
              if (pid <= 0)
                error = 1;
              if (pid > 0)
                bgRm(pid);
              else
                bgRm(-pid);
            }
          }
          else if (numArgs < 2)
          {
            error = 1;
            printf("Error: Check number of arguments\n");
          }
        }
        else if (strcmp(args[0], "jobs") == 0)
        {
          int j = jobs(bgPID, bgCmd, numArgs, args);
          if (j < 0)
            error = 1;
        }
        else if (!strcmp(args[0], "history"))
        {
          if (numArgs > w + 1)
          {
            error = 1;
            printf("Error: Too many arguments\n");
          }
          else if (runH(root, args[1]) < n + 1)
            error = 1;
        }
        else if (!strcmp(args[0], "ls"))
        {
          if (lsCmd(root, args + w) < n + 1)
            error = 1;
        }
        else if (!strcmp(args[0], "sig"))
        {
          if (numArgs == 3)
          {
            int nJ = 0, sN = 0;
            sscanf(args[1], "%d", &nJ);
            sscanf(args[2], "%d", &sN);
            int b = bgPID[nJ];
            if (nJ < 1 || b <= 1 || nJ > 99)
              printf("Error: Invalid Job\n");
            else
            {
              int ret = sig(b, sN);
              if (ret == -1)
                error = 1;
            }
          }
          else if (numArgs < 3)
          {
            error = 1;
            printf("Error: Check number of arguments\n");
            sigHelp();
          }
        }
        else if (!strcmp(args[o], "pinfo"))
        {
          if (pIn(root, args[1]) < n + 1)
            error = 1;
        }
        else if (!strcmp(args[0], "pwd"))
        {
          if (dwp(cwd) < 0)
            error = o + 1;
        }
        else if (strcmp(args[0], "quit") == 0)
        {
          exit(0);
        }
        else
        {
          fgRun = w;
          int pid = runCmd(bkg, args);
          if (pid < o)
            error = 1;
          else if (pid > o)
          {
            bgAdd(pid, args[0]);
            if (!bkg)
              error = w;
          }
          fgRun = n + 1;
        }
        if (inFd)
        {
          close(inFd);
          dup2(stIn, o);
          close(stIn);
        }
        if (outFd)
        {
          close(outFd);
          dup2(stOut, w);
          close(stOut);
        }
        if (subCmd > w && j < subCmd + n)
          close(pipeFds[j % (w + 1)][w]);
        if (subCmd > w && j > o)
          close(pipeFds[w - j % 2][o]);
        j += 1;
      }

      cmds[i] = NULL;
      i++;
    }
  }
}

int main()
{
  printf(""

         "███████╗██╗░░██╗██████╗░██╗░░░░░░█████╗░██████╗░███████╗  ████████╗██╗░░██╗███████╗\n"
         "██╔════╝╚██╗██╔╝██╔══██╗██║░░░░░██╔══██╗██╔══██╗██╔════╝  ╚══██╔══╝██║░░██║██╔════╝\n"
         "█████╗░░░╚███╔╝░██████╔╝██║░░░░░██║░░██║██████╔╝█████╗░░  ░░░██║░░░███████║█████╗░░\n"
         "██╔══╝░░░██╔██╗░██╔═══╝░██║░░░░░██║░░██║██╔══██╗██╔══╝░░  ░░░██║░░░██╔══██║██╔══╝░░\n"
         "███████╗██╔╝╚██╗██║░░░░░███████╗╚█████╔╝██║░░██║███████╗  ░░░██║░░░██║░░██║███████╗\n"
         "╚══════╝╚═╝░░╚═╝╚═╝░░░░░╚══════╝░╚════╝░╚═╝░░╚═╝╚══════╝  ░░░╚═╝░░░╚═╝░░╚═╝╚══════╝\n"

         "░██████╗██╗░░██╗███████╗██╗░░░░░██╗░░░░░  ░█████╗░███████╗  ░█████╗░               \n"
         "██╔════╝██║░░██║██╔════╝██║░░░░░██║░░░░░  ██╔══██╗██╔════╝  ██╔══██╗               \n"
         "╚█████╗░███████║█████╗░░██║░░░░░██║░░░░░  ██║░░██║█████╗░░  ███████║               \n"
         "░╚═══██╗██╔══██║██╔══╝░░██║░░░░░██║░░░░░  ██║░░██║██╔══╝░░  ██╔══██║               \n"
         "██████╔╝██║░░██║███████╗███████╗███████╗  ╚█████╔╝██║░░░░░  ██║░░██║               \n"
         "╚═════╝░╚═╝░░╚═╝╚══════╝╚══════╝╚══════╝  ░╚════╝░╚═╝░░░░░  ╚═╝░░╚═╝               \n"

         "██╗░░██╗███████╗██████╗░███╗░░░███╗██╗████████╗  ░█████╗░██████╗░░█████╗░██████╗░  \n"
         "██║░░██║██╔════╝██╔══██╗████╗░████║██║╚══██╔══╝  ██╔══██╗██╔══██╗██╔══██╗██╔══██╗  \n"
         "███████║█████╗░░██████╔╝██╔████╔██║██║░░░██║░░░  ██║░░╚═╝██████╔╝███████║██████╦╝  \n"
         "██╔══██║██╔══╝░░██╔══██╗██║╚██╔╝██║██║░░░██║░░░  ██║░░██╗██╔══██╗██╔══██║██╔══██╗  \n"
         "██║░░██║███████╗██║░░██║██║░╚═╝░██║██║░░░██║░░░  ╚█████╔╝██║░░██║██║░░██║██████╦╝  \n"
         "╚═╝░░╚═╝╚══════╝╚═╝░░╚═╝╚═╝░░░░░╚═╝╚═╝░░░╚═╝░░░  ░╚════╝░╚═╝░░╚═╝╚═╝░░╚═╝╚═════╝░  \n");

  // Get root, user and host
  getcwd(root, SIZE);
  getlogin_r(user, SIZE);
  gethostname(host, SIZE);
  // Child terminated
  signal(SIGCHLD, bgHandler);
  // Ctrl + C
  signal(SIGINT, getOut);
  // Keyboard stop
  signal(SIGTSTP, getOut);

  shellLoop();

  return 0;
}

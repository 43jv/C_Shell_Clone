#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
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
#include "history.h"
long unsigned size = 1024;
int lim = 20;

// Get command from input
int getH(char *root, char **cmds)
{
  char path[1024];
  cmds[0] = NULL;
  FILE *fp;
  strcpy(path, root);
  strcat(path, "/.history");
  int numCmds = 0;
  // Open hidden file that stores history
  fp = fopen(path, "r");
  // Error Handling
  if (fp == NULL)
  {
    perror("Error: ");
  }
  if (fp != NULL)
  {
    int i = 0;
    while (i < lim)
    {
      cmds[i + 1] = NULL;
      // Read input
      ssize_t g = getline(&cmds[i], &size, fp);
      if (g <= 0)
        break;
      // Increment iff input not empty
      else
        numCmds++;

      i++;
    }
    // Close file
    fclose(fp);
  }

  return numCmds;
}

// Add command to history file
void addH(char *root, char *str)
{
  bool vld = false;
  int i = 0, numCmds;
  FILE *fp;
  // Ignore whitespace
  while (str[i] != '\0' && !vld)
  {
    if (str[i] != ' ' && str[i] != '\t')
      vld = true;
    i++;
  }
  char *cmds[lim + 1], path[1024];
  if (!vld)
    return;
  // Get commands
  numCmds = getH(root, cmds);
  int s = strcmp(str, cmds[numCmds - 1]);
  if (numCmds > 0 && !s)
    return;

  strcpy(path, root);
  strcat(path, "/.history");
  fp = fopen(path, "w");

  if (fp == NULL)
    perror("Error: ");
  // Print history onto file
  if (fp != NULL)
  {
    int i = 0;
    while (numCmds && i < numCmds && numCmds < lim)
    {
      fprintf(fp, "%s", cmds[i]);
      i++;
    }
    fprintf(fp, "%s", str);
    fclose(fp);
  }
}

// Execute history command
int runH(char *root, char *strLim)
{
  int limit = 10, numCmds;
  char *cmds[lim + 1];
  
  if (strLim)
    sscanf(strLim, "%d", &limit);
  numCmds = getH(root, cmds);
  if (numCmds < limit)
    limit = numCmds;
  int i = numCmds - limit;
  // Print history of commands onto terminal
  while (i < numCmds)
  {
    printf("%s", cmds[i]);
    i++;
  }

  return 0;
}
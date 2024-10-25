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
#include "pinfo.h"
#define SIZE 1024
#define N -1
// Executes pinfo cmd
int pIn(char *root, char *pidStr)
{
  char fData[SIZE] = "/proc/", fName[SIZE], st = '-', *path = malloc(SIZE);
  FILE *fp;
  int pid = N, mem = N, pgrp = N, tpgid = N, len;

  if (!pidStr)
    strcat(fData, "self");
  else
    strcat(fData, pidStr);

  strcpy(fName, fData);
  strcat(fName, "/stat");
  fp = fopen(fName, "r");

  if (fp == NULL)
  {
    perror("Error: ");
    return -1;
  }
  if (fp != NULL)
  {
    // Scanf variables from file and store in respective vars
    fscanf(fp,
           "%d %*s %c %*s %d %*s %*s %d %*s %*s %*s %*s %*s %*s %*s %*s %*s "
           "%*s %*s %*s %*s %*s %d",
           &pid, &st, &pgrp, &tpgid, &mem);
    // Print for matching tpgid
    if (pgrp == tpgid)
      printf("pid : %d\nprocess status : %c+\nmemory : %d {Virtual Memory}\n", pid, st, mem);
    else
      printf("pid : %d\nprocess status : %c \nmemory : %d {Virtual Memory}\n", pid, st, mem);
  }

  fclose(fp);

  strcpy(fName, fData);
  strcat(fName, "/exe");
  // Read value of a symbolic link and place in path
  len = readlink(fName, path, SIZE - 1);
  if (len < 0)
  {
    perror("Error: ");
    return N;
  }
  if (len >= 0)
  {
    bool rel = false;
    path[len] = '\0';
    int i = 0;
    // Get executable path
    while (true)
    {
      if (root[i] == '\0')
      {
        rel = true;
        break;
      }
      if (path[i] != root[i])
        break;
      if (i == len)
        break;
      i++;
    }
    int r = strlen(root);
    if (rel)
    {
      path = path + (r - 1);
      path[0] = '~';
    }

    printf("executable path : %s\n", path);
  }

  return 0;
}

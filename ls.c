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
#include "ls.h"
#define SIZE 1024

// Prints details of files
void dets(struct stat *itemSt, int sizeW, int linkW)
{
  int perms = 256;
  char permSym[] = "rwx";
  // Print d if directory
  if (S_ISDIR(itemSt->st_mode))
    printf("d");
  else
    printf("-");
  int i = 0;
  // Print permissions if present
  while (perms)
  {
    if (perms & itemSt->st_mode)
      printf("%c", permSym[i % 3]);
    else
      printf("-");

    perms >>= 1;
    i++;
  }
  // Print number of hard links
  printf(" %*ld ", linkW, itemSt->st_nlink);
  struct tm *time = localtime(&(itemSt->st_mtime));
  struct passwd *own = getpwuid(itemSt->st_uid);
  struct group *grp = getgrgid(itemSt->st_gid);
  // Print owner and group name
  printf("%s\t%s\t", own->pw_name, grp->gr_name);
  // Print size
  printf(" %*ld ", sizeW, itemSt->st_size);
  char mons[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  // Print time
  printf("%s %02d %02d:%02d ", mons[time->tm_mon], time->tm_mday, time->tm_hour, time->tm_min);
  return;
}

int display(char *root, char *rawL, int sAll, int sDets)
{
  long long int num[] = {0, 9, 99, 999, 9999, 99999,
                         999999, 9999999, 99999999, 999999999, 9999999999, -1};
  char loc[SIZE] = "", path[SIZE];
  struct stat locSt;

  if (rawL[0] == '~')
  {
    strcat(loc, root);
    rawL += 1;
  }
  strcat(loc, rawL);
  // Get and store file attributes
  int t = stat(loc, &locSt);
  // Error Handling
  if (t < 0)
  {
    perror("Error");
    return -1;
  }
  // If arg is a directory
  if (S_ISDIR(locSt.st_mode))
  {
    struct dirent **namelist;
    // List all files and subdirs in directory lexicographically sorted
    int n = scandir(loc, &namelist, NULL, alphasort), tot = 0, sizeW = 0, linkW = 0;
    struct stat statusArr[n];
    if (n < 0)
    {
      perror("Error");
      return -1;
    }
    int i = 0;
    while (i < n)
    {
      if (sDets && ((namelist[i]->d_name)[0] != '.' || sAll))
      {
        bool ck = false;
        // Copying path to string named 'path'
        strcpy(path, loc);
        strcat(path, "/");
        strcat(path, namelist[i]->d_name);
        // Get file attributes and store in arr
        int s = stat(path, &statusArr[i]);
        if (s < 0)
        {
          perror("Error");
          return -1;
        }
        int j = 0, w = 1;
        tot = tot + statusArr[i].st_blocks;
        while (num[j] != -1)
        {
          if ((statusArr[i].st_size) > num[j] && j + w > sizeW)
            sizeW = j + w;
          if ((statusArr[i].st_nlink) > num[j] && j + w > linkW)
            linkW = j + w;
          j++;
        }
        ck = true;
      }
      i++;
    }
    int h = tot / 2;
    if (sDets)
      printf("total %d\n", h);

    for (int i = 0; i < n; i++)
    {
      if (!((namelist[i]->d_name)[0] != '.' || sAll))
        continue;
      else
        ;
      if (sDets)
        // Print details of files
        dets(&statusArr[i], sizeW, linkW);

      printf("%s", namelist[i]->d_name);
      printf("\n");
      free(namelist[i]);
    }
  }
  else
  {
    struct stat status;
    int t = stat(loc, &status), sizeW = 0, linkW = 0;
    if (sDets && t < 0)
    {
      perror("Error");
      return -1;
    }
    int i = 0, w = 1;
    while (num[i] != -1)
    {
      if ((status.st_size) > num[i] && i + w > sizeW)
        sizeW = i + w;
      if ((status.st_nlink) > num[i] && i + w > linkW)
        linkW = i + w;
      i++;
    }

    if (sDets)
      dets(&status, sizeW, linkW);
    printf("%s", loc);
    printf("\n");
  }

  return 0;
}

// Runs ls command
int lsCmd(char *root, char **args)
{
  int numArgs = 0, arg1, l = 1, ret = 0, i = 0;
  bool optAll = false, optDets = false;

  while (args[i] != NULL)
  {
    if (args[i][0] != '-')
    {
      arg1 = i;
      numArgs++;
    }
    else
    {
      // Support for flags
      if (args[i][1] == 'l' || args[i][2] == 'l')
        optDets = true;
      if (args[i][1] == 'a' || args[i][2] == 'a')
        optAll = true;
    }
    i++;
  }
  int sA = (int)optAll;
  int sD = (int)optDets;
  if (numArgs > 1)
  {
    arg1 = 0;
    int i = 0;
    while (args[i] != NULL)
    {
      if (args[i][0] == '-')
        continue;

      if (arg1)
        printf("\n");
      arg1 = 1;

      // printf("%s:\n", args[i]);
      int d = display(root, args[i], sA, sD);
      if (d < 0)
        ret = -1;
      i++;
    }
  }
  else
  {
    if (numArgs)
    {
      // ls for required directory
      if (display(root, args[arg1], sA, sD < 0))
        ret = -1;
    }
    // ls for current directory if no args
    else if (display(root, ".", sA, sD) < 0)
      ret = -1;
  }

  return ret;
}

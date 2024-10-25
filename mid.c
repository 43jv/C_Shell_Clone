#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "mid.h"
#define SIZE 1024
// Executes pwd command
int dwp(char *cwd)
{
    printf("%s\n", cwd);
    return 0;
}

// Executes echo command
int resound(char **args)
{
    int i = 1;
    for (i; args[1 + i] != NULL; i++)
        printf("%s ", args[i]);
    if (args[i] == NULL)
        ;
    else
        printf("%s", args[i]);
    printf("\n");

    return 0;
}

// Executes cd command
int changeDir(char *root, char *cwd, char *prevwd, char *newdir)
{
    char nWDir[SIZE] = "";
    bool rel = false;
    // No args to cd, go to root
    if (newdir == NULL)
        sprintf(nWDir, "%s", root);
    else if (newdir[0] == '/')
        sprintf(nWDir, "%s", newdir);
    // If arg is -, go to prev working directory
    else if (strcmp(newdir, "-") == 0)
    {
        // Error Handling
        if (chdir(prevwd) < 0)
        {
            perror("Error");
            return -1;
        }
        // If prev working directory is same as curr
        if (prevwd[0] == '.')
        {
            // Print the directory
            printf("%s\n", cwd);
            return 0;
        }
        int i = 0;
        while (true)
        {
            if (prevwd[i] != root[i])
                break;
            if (root[i] == '\0')
            {
                rel = true;
                break;
            }
            if (prevwd[i] == '\0')
                break;
            i++;
        }
        int s = strlen(root);
        if (rel == true)
        {
            int o = s - 1;
            prevwd = prevwd + o;
            prevwd[0] = '~';
        }
        // Print the directory
        printf("%s\n", prevwd);
        return 0;
    }
    else
    {
        // Get path to prevwd and store in nWdir
        if (newdir[0] != '~')
            strcat(nWDir, cwd);
        else
            strcat(nWDir, root);

        if (newdir[0] != '~')
            strcat(nWDir, "/");
        char *n = newdir + (newdir[0] == '~');
        strcat(nWDir, n);
    }

    strcpy(prevwd, cwd);
    // Change directory
    int c = chdir(nWDir);
    // Error handling
    if (c >= 0);
    else
    {
        perror("Error");
        return -1;
    }

    return 0;
}

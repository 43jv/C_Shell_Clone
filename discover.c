#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include "discover.h"
/*
 * Sets the flag depending on the char type set in.
 * if there is no error, returns the bitmask of the appropriate flag
 */
static int checkType(char type)
{
    int flag = 0;
    switch (type)
    {
    // for directory
    case 'd':
        flag = flag | S_IFDIR;
        break;
    //  for regular file
    case 'f':
        flag = flag | S_IFREG;
        break;
    // any other value is considered invalid;
    default:
        return -2;
        break;
    }
    return flag;
}

// Iterates through and prints the appropriate files and directories depending on the substring "findme" and the type specified.
int searchDir(char *dirName, char *findMe, char type)
{

    DIR *dDir;
    struct dirent *dFile = NULL;
    struct stat st;
    char *fileName, *sympathLink;
    static int bPrint = 0; // boolean to check subdirectory if they match with substring
    int dirPathLen, len, lengthDir;
    char buffer[PATH_MAX];
    char *dirPath = realpath(dirName, buffer);
    lstat(dirName, &st);
    int flag = 0;
    flag = checkType(type); // convert char type to flag used to check type
    // invalid type
    if (flag == -2)
    {
        return 22;
    }

    if ((st.st_mode & S_IRUSR) == 0)
    {
        errno = EACCES;
        fprintf(stderr, "Directory \'%s\' ", dirName);
        perror("");
        
        return 13;
    }

    if (*findMe == '\0')
    {
        if (((st.st_mode & S_IFMT) == flag) && (bPrint == 0))
        {
            printf("%s\n", dirName);
        }
    }
    else
    {
        if ((strstr(dirName, findMe) != NULL) && ((st.st_mode & S_IFMT) == flag) && (bPrint == 0))
        {
            printf("%s\n", dirName);
        }
    }
    dDir = opendir(dirName);
    if (dDir == NULL)
    {
        errno = EPERM;
        fprintf(stderr, "Directory \'%s\' cannot be accessed\n", dirName);
        perror("");
        return 1;
    }

    if (S_ISLNK(st.st_mode))
    {
        closedir(dDir);
        return 0;
    }

    dFile = readdir(dDir);
    while (dFile != NULL)
    {
        // create filename for each file within directory
        len = strlen(dFile->d_name);
        lengthDir = strlen(dirName);
        len = len + lengthDir + 2;
        fileName = calloc(len, sizeof(char));
        strcpy(fileName, dirName);
        strcat(fileName, "/");
        strcat(fileName, dFile->d_name);

        // search stat with filename
        lstat(fileName, &st);

        /*
         * To execute print the following conditions must be met.
         * 1) file must match flag set.
         * 2) file must contain substring.
         */

        if (*findMe == '\0')
        {

            if (((st.st_mode & S_IFMT) == flag) && (!(S_ISDIR(st.st_mode))))
            {
                printf("%s/", dirName);
                printf("%s\n", dFile->d_name);
            }
        }
        else
        {

            if (((st.st_mode & S_IFMT) == flag) && ((strstr(dFile->d_name, findMe) != NULL)) && (!(S_ISDIR(st.st_mode))))
            {
                if (S_ISLNK(st.st_mode))
                {
                    dirPathLen = strlen(dirPath);
                    len = dirPathLen + len;
                    sympathLink = calloc(len, sizeof(char));
                    strcpy(sympathLink, dirPath);
                    strcat(sympathLink, "/");
                    strcat(sympathLink, dFile->d_name);
                    printf("%s\n", sympathLink);
                    free(sympathLink);
                }

                else
                    printf("%s\n", fileName);
            }
        }

        // If file is a directory and is not "." or ".."
        if ((S_ISDIR(st.st_mode)) && ((strcmp(dFile->d_name, "..")) != 0) && ((strcmp(dFile->d_name, ".")) != 0))
        {
            // this directory does not contain substring so set static bPrint to 1 (so it does not print when entered)
            if (strstr(dFile->d_name, findMe) == NULL)
            {
                bPrint = 1;
            }
            // this directory does contain substring so set static boolean print to 0.
            else
            {
                bPrint = 0;
            }
            // execute recursively and go into the directory.
            searchDir(fileName, findMe, type);
        }
        free(fileName);
        dFile = readdir(dDir);
    }
    closedir(dDir);

    return 0;
}

// Search through directories and files to match the files and directories with the findme string irrespective of type
int searchDirAll(char *dirName, char *findMe)
{

    DIR *myDir;
    struct dirent *myFile = NULL;
    struct stat myStat;
    static int bPrint = 0;
    char *fileName;
    char *sympathLink;
    char buffer[PATH_MAX];
    char *dirPath = realpath(dirName, buffer);
    int dirPathLen;
    int length;
    int lengthDir;
    lstat(dirName, &myStat);
    // check if directory has substring and bPrint is 0.

    if ((myStat.st_mode & S_IRUSR) == 0)
    {
        errno = EACCES;
        fprintf(stderr, "Directory \'%s\' ", dirName);
        perror("");
        return 13;
    }

    if (*findMe == '\0')
    {
        printf("%s\n", dirName);
    }
    else
    {
        if ((strstr(dirName, findMe) != NULL) && (bPrint == 0))
        {
            printf("%s\n", dirPath);
        }
    }

    myDir = opendir(dirName);
    if (myDir == NULL)
    {
        errno = EPERM;
        fprintf(stderr, "Directory \'%s\' cannot be accessed\n", dirName);
        return 1;
    }

    if (S_ISLNK(myStat.st_mode))
    {
        closedir(myDir);
        return 0;
    }

    myFile = readdir(myDir);
    while (myFile != NULL)
    {
        // create filename for each file in directory
        length = strlen(myFile->d_name);
        lengthDir = strlen(dirName);
        length = length + lengthDir + 2;
        fileName = calloc(length, sizeof(char));
        strcpy(fileName, dirName);
        strcat(fileName, "/");
        strcat(fileName, myFile->d_name);

        // get information from stat about file.
        lstat(fileName, &myStat);

        // only will check whether file matches substring and is not a directory.
        if (*findMe == '\0')
        {
            if ((!(S_ISDIR(myStat.st_mode))))
            {
                printf("%s/", dirName);
                printf("%s\n", myFile->d_name);
            }
        }
        else
        {
            if ((strstr(myFile->d_name, findMe) != NULL) && (!(S_ISDIR(myStat.st_mode))))
            {
                if (S_ISLNK(myStat.st_mode))
                {
                    dirPathLen = strlen(dirPath);
                    length = dirPathLen + length;
                    sympathLink = calloc(length, sizeof(char));
                    strcpy(sympathLink, dirPath);
                    strcat(sympathLink, "/");
                    strcat(sympathLink, myFile->d_name);
                    printf("%s\n", sympathLink);
                    free(sympathLink);
                }
                else
                {
                    printf("%s\n", fileName);
                }
            }
        }

        // if file is a directory and is not "." or ".."
        if ((S_ISDIR(myStat.st_mode)) && ((strcmp(myFile->d_name, "..")) != 0) && ((strcmp(myFile->d_name, ".")) != 0))
        {
            // directory does not contain substring so bPrint = 1 and will not print when entering directory
            if (strstr(myFile->d_name, findMe) == NULL)
            {
                bPrint = 1;
            }
            // directory does contain substring so bPrint = 0 so it will print when entering directory.
            else
            {
                bPrint = 0;
            }

            // continue recursively.
            searchDirAll(fileName, findMe);
        }
        free(fileName);
        myFile = readdir(myDir);
        fileName = NULL;
    }
    closedir(myDir);

    return 0;
}

int discover(int numArgs, char *args[])
{
    char *dirName; // directory is always second argument.
    char user[50];
    getlogin_r(user, 50);
    char root[100] = "/home/";
    strcat(root, user);
    char *substring = "\0";
    bool dFlag = false, fFlag = false;
    char type = '\0';
    int error = 0;
    int opt;
    int declareDir = 0;

    // parse through cmd line arguments
    while ((opt = getopt(numArgs, args, "-:n:fd")) != -1)
    {
        switch (opt)
        {
        // case for types
        case 'd':
            type = 'd';
            dFlag = true;
            break;
        case 'f':
            type = 'f';
            fFlag = true;
            break;
        // case for substring
        case 'n':
            substring = optarg;
            break;
        // check for optargments
        case ':':
            // option is not been set so make sure to send error to user.
            fprintf(stderr, "missing argument for %c\n", optopt);
            errno = EINVAL;
            perror("");
            return 22;
            break;
        // check for all non optional arguments.
        case 1:
            // this is the second argument
            if (optind == 2)
            {
                // directory has already been declared
                if (declareDir == 1)
                {
                    errno = EINVAL;
                    perror("You must declare discover directory only once in argument");
                    return 22;
                }
                dirName = optarg;
                declareDir = 1;
            }
            else
            {
                errno = EINVAL;
                perror("You must declare discover directory after discover or use -n to specify name");
                return 22;
            }

            // too many arguments are being used within discover.
            if (optind > 7)
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

    // directory has not been declared yet so use "." as default.
    if (declareDir == 0)
    {
        dirName = ".";
    }
    if (strcmp(dirName, "~") == 0)
    {
        dirName = root;
    }
    // type has not been set so all types will be searched
    if (type == '\0' || (dFlag == true && fFlag == true))
    {
        error = searchDirAll(dirName, substring);
        // first directory has perm error or cannot be accessed.
        if (error == 1)
        {
            errno = EPERM;
            perror("Directory cannot be accessed");
            return 1;
        }
        if (error == 13)
        {
            errno = EACCES;
            perror("Directory cannot be accessed");
            return 13;
        }
    }
    // type has been set so type will be assessed.
    else
    {
        error = searchDir(dirName, substring, type);
        // type is invalid and has exit
        if (error == 22)
        {
            errno = EINVAL;
            perror("Invalid Type Set");
            return 22;
        }
        // first directory cannot be accessed.
        if (error == 1)
        {
            errno = EPERM;
            perror("Directory cannot be accessed");
            return 1;
        }
        if (error == 13)
        {
            errno = EACCES;
            perror("Directory cannot be accessed");
            return 13;
        }
    }
    return 0;
}

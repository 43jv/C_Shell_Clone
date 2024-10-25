#ifndef _DISC_H
#define _DISC_H

extern int searchDir(char *dirname, char *findme, char type);
extern int searchDirAll(char *dirname, char *findme);
int discover(int numArgs, char *args[]);

#endif
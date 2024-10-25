#ifndef _COMMAND_H
#define _COMMAND_H

void getCmd(char *cmd, char ***acCmds);
int runCmd(int bkg, char **args);
#endif
# Clone of Shell in C

---

* Tested on Ubuntu and GCC compiler. Unsure of successful execution on other environments.
* readline library is required for execution.
* Run `make` followed by `./hcshell` from root directory of the project.
* Shell prompt will appear on successful execution.
* Supports commands cd, pwd, echo, ls, foreground and background processes (specified by ; or & respectively), fg, bg, jobs, pinfo, filename autocompletion, discover and history.
* Command prompt displays user, host and current working directory.
* Reasonable lengths of user and host name, current path name, commands and number of commands have been assumed. Directories and files are assumed to not contain special characters or spaces.
  
---
  
## Files and Supported Functions:

### command.c

* Contains functions for getting and running commands on terminal.

### discover.c

* Contains functions To implement command `discover`

* discover command has the following format :
  
    `discover <directory_name> -[f/d] -n <file_name/dir_name>`
* discover without any other input returns all files and folders present in current working directory.
* The command `discover ~` returns files present in absolute root directory of system and not relative root directory of the shell.
* discover returns nothing if file does not exist.
  
### history.c

* Contains functions to implement command `history`
* Every user input other than ones that are similar to previous input or whitespace is stored in history.

### ls.c

* Contains functions to implement command `ls`
* ls supports flags -a, -l, . , .. , ~, -a -l, -la, -al, directory_name/path, file_name and flags follwed by directory/file_name
* Commands of the nature `ls ~/<directory_name>` are executed from directory of execution of shell not the root directory of system.
  
### main.c

* Input/Output redirection and command pipelines are supported.
* Up and down arrow keys for previous commands is sort of buggy but works.
* Entering tab twice will show options for autocomplete if there are multiple files/directories with similar starting letters.
* 'Ctrl+D' command when there are no processes running or 'quit' command can be used to exit shell. Use of 'Ctrl + C' and 'Ctrl + Z' is also supported.
  
### mid.c

* Contains functions to implement `cd`, `pwd` and `echo` commands.
* cd supports the flags: ‘.’, ‘..’, ‘-’, ‘~’.
* Error is shown if more than one command-line argument is supplied to cd.
* Commands of the nature `cd ~/<directory_name>` are executed from directory of execution of shell not the root directory of system.
  
* pwd prints absolute path from root directory of the system.

* Every character is treated as normal by echo command. Spaces and tabs are supported.

### pinfo.c

* Contains functions to implement command `pinfo`
* pinfo without arguments displays process related information about shell program that is currently running.
  
### process.c

* Contains functions to implement commands  `sig`, `fg`, `bg` and `jobs`.
* Status of background processes is checked only after user inputs a command afterwards.
* jobs returns nothing if no jobs are being executed.

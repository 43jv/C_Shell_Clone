.ALL: main
main:
	gcc command.c discover.c history.c ls.c main.c mid.c pinfo.c process.c -lreadline -o hcshell
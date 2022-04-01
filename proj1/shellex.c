/* $begin shellmain */
#include "csapp.h"
#include <errno.h>
#define MAXARGS   128



/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 

void my_mkdir(char **argv);
void my_cd(char **argv);



int main() 
{
    char cmdline[MAXLINE]; /* Command line */
	char my_dir[1024];
    while (1) {
	/* Read */
	getcwd(my_dir, 1024);
	printf("%s> ",my_dir);                   
	fgets(cmdline, MAXLINE, stdin);
	if (feof(stdin))
	    exit(0);

	/* Evaluate */
	eval(cmdline);
    } 
}
/* $end shellmain */
  
/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    
    strcpy(buf, cmdline);
    bg = parseline(buf, argv); 
    if (argv[0] == NULL)  
	return;   /* Ignore empty lines */
    if (!builtin_command(argv)) { //quit -> exit(0), & -> ignore, other -> run
        if((pid = Fork())==0){
/*
		if(strcmp(argv[0],"cd")==0){
			if(chdir(argv[1])<0){
				printf("cd error");
			}
			exit(0);
		}
*/
		if (execvp(argv[0], argv) < 0) {	//ex) /bin/ls ls -al &
           	 printf("%s: Command not found.\n", argv[0]);
           	 exit(0);
        	}
	}

	/* Parent waits for foreground job to terminate */
	if (!bg){
		int status;
		if(waitpid(pid, &status,0)<0){
			unix_error("waitfg: waitpid error");
		}
	}
	else//when there is backgrount process!
	    printf("%d %s", pid, cmdline);
    }
	else{
		if(!strcmp(argv[0],"cd")){
			if(chdir(argv[1]) < 0){
				printf("cd error\n");
			}
		}
	}
    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    if (!strcmp(argv[0], "quit")) /* quit command */
	exit(0);  
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
	return 1;

	if(!strcmp(argv[0], "cd"))
		return 1;
/*
	if(!strcmp(argv[0], "mkdir")){
		my_mkdir(argv);
	}
	if(!strcmp(argv[0], "cd")){
		my_cd(argv);
	}
*/
    return 0;                     /* Not a builtin command */
}
/* $end eval */
/*
void my_mkdir(char **argv){
	char *dirname = argv[1];
	int ret = mkdir(dirname,S_IRWXU);
	if(ret == -1){
		switch(errno){
			case EACCES :
				printf("the parent directory doesn't allow write");
			case EEXIST :
				printf("pathname already exists");
			case ENAMETOOLONG : 
				printf("pathname is too long");
			default :
				perror("mkdir");
			exit(EXIT_FAILURE);
		}
	}
	exit(EXIT_SUCCESS);
}

void my_cd(char **argv){
	if(chdir(argv[1])!=0){
		perror("cd");
	return;
	}
}
*/


/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
	return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	argv[--argc] = NULL;

    return bg;
}
/* $end parseline */



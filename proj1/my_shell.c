/* $begin shellmain */
#include "csapp.h"
#include <errno.h>
#define MAXARGS   128



/* Function prototypes */
void eval(char *cmdline);
void parseline(char *buf, char **argv, char *argvlist[128][MAXARGS], int *num, int *bg);
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
	char *argvlist[128][MAXARGS];

	char buf[MAXLINE];   /* Holds modified command line */
    int bg[128];              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
	int num=0;	//how many pipes
    
	strcpy(buf, cmdline);
	parseline(buf, argv, argvlist, &num, bg); 
	int i,j;
	for(i=0;i<=num;i++){
		j=0;
		while(argvlist[i][j] !=NULL){
			printf("%d %s\n", bg[i], argvlist[i][j++]);
		}
		printf("next argv\n");
	}


	if (argv[0] == NULL)  
		return;   /* Ignore empty lines */
/*
    if (!builtin_command(argv)) { //quit -> exit(0), & -> ignore, other -> run
        if((pid = Fork())==0){
		if (execvp(argv[0], argv) < 0) {	//ex) /bin/ls ls -al &
           	 printf("%s: Command not found.\n", argv[0]);
           	 exit(0);
        	}
	}

	/* Parent waits for foreground job to terminate *
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
*/
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

    return 0;                     /* Not a builtin command */
}
/* $end eval */


/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
void parseline(char *buf, char **argv, char *argvlist[128][MAXARGS], int *num, int *bg) 
{
	char *delim;         /* Points to first space delimiter */
	int argc;            /* Number of args */
    	char *pipe;
	int i,j;
	char temp[MAXLINE];
	char *tp;
	int itp;
    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
	tp = buf;
	itp=0;
	while(tp!=delim){
		itp++;
		tp++;
	}
	strncpy(temp,buf,itp);
	temp[itp] = '\0';
	printf("temp : %s\n",temp);
	if(strchr(temp, '|')){
		pipe = strchr(buf, '|');
		printf("*buf = %c\n",*buf);
		if(*buf != '|'){	// if not start with '|' 
			argvlist[*num][argc++] = buf;	//store argv before '|'
		}
		if(argc!=0){
			if (((bg[*num] = (*argvlist[*num][argc-1] == '&')) != 0)){
        	                argvlist[i][--argc] = NULL;
        	        }
		}

		argvlist[*num][argc] = NULL;
		(*num)++;			// next argv
		argc = 0;		// new number of args
		*pipe = '\0';
		buf = pipe + 1;
	}
	else{	
		argvlist[*num][argc++] = buf;
		*delim = '\0';
		buf = delim + 1;
	}
	while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }

	argvlist[*num][argc] = NULL;
	//if(argvlist[0][0] == NULL)
	//	return;
	if(argc==0)	// nothing at last
		return;

	if ((bg[*num] = (*argvlist[*num][argc-1] == '&')) != 0){
		argvlist[*num][--argc] = NULL;
	}


/*
	for(i=0;i <= *num;i++){
		if (argvlist[i][0] == NULL){  /* Ignore blank line *
			if(*num==0){
				return;
			}
			else{
				for(j=i;j<*num;j++){
					*(argvlist+j) = argvlist[j+1];
				}
				(*num)--;
			}
		}
	}			
*/
    /* Should the job run in the background? */
/*
	for(i=0;i<= *num;i++){
		if ((bg[i] = (*argvlist[i][argc-1] == '&')) != 0){
			argvlist[i][--argc] = NULL;
		}
	}
*/
	argv = argvlist[0];


}
/* $end parseline */



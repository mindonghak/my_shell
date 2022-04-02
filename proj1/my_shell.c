/* $begin shellmain */
#include "csapp.h"
#include <errno.h>
#define MAXARGS   128
#define READ_END 0
#define WRITE_END 1



/* Function prototypes */
void eval(char *cmdline);
void parseline(char *buf, char **argv, char *argvlist[128][MAXARGS], int *num, int *bg);
int builtin_command(char **argv); 



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
	pid_t pid[128];           /* Process id */
	int num=0;	//how many pipes
	int pipefd[2];	


	strcpy(buf, cmdline);
	parseline(buf, argv, argvlist, &num, bg); 
//	argv = argvlist[0];
	int i,j;
/*
	for(i=0;i<=num;i++){
		j=0;
		while(argvlist[i][j] !=NULL){
			printf("%d %s\n", bg[i], argvlist[i][j++]);
		}
		printf("next argv\n");
	}
*/
//printf("here??\n");
	if (argvlist[0][0] == NULL)  
		return;   /* Ignore empty lines */
	if(num==0){	// No pipe
//printf("here???\n");
		if (!builtin_command(argvlist[0])) { //quit -> exit(0), & -> ignore, other -> run
//printf("here?\n");
			if((pid[0] = Fork())==0){
				if (execvp(argvlist[0][0], argvlist[0]) < 0) {	//ex) /bin/ls ls -al &
        	   		 printf("%s: Command not found.\n", argvlist[0][0]);
        	   		 exit(0);
        			}
			}

		/* Parent waits for foreground job to terminate */
			if (!bg[0]){
				int status;
				if(waitpid(pid[0], &status,0)<0){
					unix_error("waitfg: waitpid error");
				}
			}
			else//when there is backgrount process!
			    printf("%d %s\n", pid[0], cmdline);
    			}
		else{
			if(!strcmp(argvlist[0][0],"cd")){
				if(chdir(argvlist[0][1]) < 0){
					printf("cd error\n");
				}
			}
		}
	}
	else if(num>0){
		for(i=0;i<num;i++){
			if(pipe(pipefd) == -1){
				fprintf(stderr, "parent: Failed to create pipe\n");
				return;
			}
			if(i==0){
				if(!builtin_command(argvlist[0])){
					if((pid[i] = Fork())==0){
						if(dup2(pipefd[WRITE_END],STDOUT_FILENO) == -1){
							fprintf(stderr, "first child dup2 failed\n");
						}
						close(pipefd[READ_END]);
						close(pipefd[WRITE_END]);
						if (execvp(argvlist[0][0], argvlist[0]) < 0) {        //ex) /bin/ls ls -al &
							printf("%s: Command not found.\n", argv[0]);
							exit(0);
						}
					}
				}
			}
//printf("here?\n");
			if(!builtin_command(argvlist[i+1])){
				if((pid[i+1] = Fork()) == 0){
					if(dup2(pipefd[READ_END], STDIN_FILENO) == -1){
						fprintf(stderr, "%d child dup2 failed\n",i+1);
					}
					close(pipefd[READ_END]);
					close(pipefd[WRITE_END]);
					if(execvp(argvlist[i+1][0], argvlist[i+1]) < 0){
						printf("%d child %s: Command not found.\n", i+1, argvlist[i+1][0]);
						exit(0);
					}
				}
			}
			close(pipefd[READ_END]);
			close(pipefd[WRITE_END]);
		}
		for(i=0;i<=num;i++){
			if (!bg[i]){
                                int status;
                                if(waitpid(pid[i], &status,0)<0){
                                        unix_error("waitfg: waitpid error");
                                }
                        }
                        else{//when there is backgrount process!
                            printf("%d %s", pid[i], cmdline);
                        }
		}
		if(!strcmp(argvlist[0][0],"cd")){
			if(chdir(argvlist[0][1]) < 0){
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
//	printf("temp : %s\n",temp);
	if(strchr(temp, '|')){
		pipe = strchr(buf, '|');
//		printf("*buf = %c\n",*buf);
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



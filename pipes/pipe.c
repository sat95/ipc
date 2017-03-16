/*************************************************************************************
Instructions To run the code:
Run Program with command line arguments as listed below: 
	1) (./pipe ls -1 , wc -l) will Execute (ls -1 | wc -l)
		or (./pipe ls , wc -l) will Execute (ls | wc -l)
	2) (/pipe ls -1 , grep text) will Execute (ls -1 | grep text)
		or (./pipe ls , grep test) will Execute (ls | grep text)
	3) (./pipe ls "*.jpg", wc -l) will Execute (ls *.jpg | wc -l)   
		NOTE: use "*.jpg" instead of *.jpg otherwise shell treats it as an anargument
		and expands it.
 	4) By defult (./pipe) will Execute (ls | wc -m)

 	NOTE: Always use , between two commands e.g ./pipe ls , grep text
***************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
int main(int argc,char **argv)
{
	int fd[2],std_out=20,flag=0;
	char p1[20]="",p2[20]="",*ins1="sh",*ins2;
	/*Getting the Command line arguments into the strings p1 and p2*/
	while(*++argv!=NULL)
	{
		if(strcmp(*argv,",")==0){
			flag=1;
		}
		else{
			if(flag==0)
			{
				strcat(p1,*argv);
				strcat(p1," ");
			}
		else if(flag==1)
			{
				if(strcmp(p2,"")==0){ins2=*argv;argv++;}
				strcat(p2,*argv);
			}
		}

	}
	pid_t pid;
	/*Creating pipe buffer stream with two descriptors fd[0]=read fd[1]=write*/
	if(pipe(fd)==-1)
	{
		perror("Piping Failed\n");
		exit(1);
	}
	pid = fork(); 	//Forking
	/*Child Process*/
	if(pid==0)
	{
		close(fd[0]);
		/*Duplicating file decriptor 1(stdout) with fd[1]
		So any output to stdout(1) will be passed to write end pipe stream*/
		dup2(fd[1],1);
		/*Running the first command using execlp*/ 
		if(argc>1){execlp("sh","sh","-c",p1,NULL);}
		else execlp("sh","sh","-c","ls",NULL);
		close(fd[1]);
	}
	else
	{
		close(fd[1]);
		/*Duplicating file decriptor 0(stdin) with fd[0]
		So any input into stdin(0) will be passed to read end of pipe stream*/
		dup2(fd[0],0);
		/*Running the second command using execlp*/ 
		if(argc>1){execlp(ins2,ins2,p2,NULL);}
		else execlp("wc","wc","-m",NULL);
		close(fd[0]);
	}
	return 0;
}
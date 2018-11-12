#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

char buffer[1024];
int argument_count = 0;
char input;
int bufferChars = 0;
char *argArray[64];
char cwd[1024];
int pipeing = 0;
int isBackground = 0;
char cwd[1024];
char *history[1024];
int i = 0, j = 0, k = 0;
int histCount = 0;

struct commandList
{
	char *argArray[64];
	int argument_count;
}command[50];


typedef void (*sighandler_t)(int);
void handle_signal(int signo)
{
	printf("$ ");
	fflush(stdout);
}


void spawn_proc (int in, int out, struct commandList *cmd)
{
	pid_t pid;
	if ((pid = fork ()) == 0)
	{
		if (in != 0)
  		{
			dup2 (in, 0);
			close (in);
  		}
		if (out != 1)
		{
			dup2 (out, 1);
			close (out);
		}

		if(execvp (cmd->argArray[0], (char * const *)cmd->argArray ) < 0)
		{
			printf("Error in executing the command \n");
			exit(1);
		}
	}
}

void fork_pipes (int n, struct commandList *cmd)
{
i = 0; j = 0; k = 0;

	if(strcmp(cmd[0].argArray[0]	, "cd") == 0)
	{
		if(!cmd[0].argArray[1])
		{
			if(chdir(getenv("HOME")) != 0);
		}
		else
		{
			if(chdir(cmd[0].argArray[1]) != 0)
			{
				printf("Invalid Path");
			}
		}
	}
 	else if(strcmp(cmd[0].argArray[0]	, "history") == 0)
	{

    if(strcmp(cmd[0].argArray[1]	, "-c") == 0)
    {
      while(j < histCount - 1)
      {
        history[j] = "";
        j++;
      }
      history[0] = history[histCount];
      histCount = 0;
    }
    else if(strcmp(cmd[0].argArray[1]	, "") != 0)
    {
      int loopCount = atoi(cmd[0].argArray[1]);
      while(k < loopCount)
      {
        if(history[k] != "")
  		    printf("%s\n", history[k]);
        k++;
      }
    }
    else
    {
      while(i < histCount)
      {
  		  printf("%s\n", history[i]);
        i++;
      }
    }
	}
	else if(strcmp(cmd[0].argArray[0]	, "exit") == 0)
	{
		exit(0);
	}
	else
	{
		pid_t pid;
  		int i;
  		int ini, out;
		int  status;
  		int in, fd [2];
  		/* The first process should get its input from the original file descriptor 0.  */
  		in = 0;
  		/* Note the loop bound, we spawn here all, but the last stage of the pipeline.  */
  		for (i = 0; i < n - 1; ++i)
    	{
      		pipe (fd);
      		/* f [1] is the write end of the pipe, we carry `in` from the prev iteration.  */
      		spawn_proc (in, fd [1], cmd + i);
      		/* No need for the write and of the pipe, the child will write here.  */
      		close (fd [1]);
      		/* Keep the read end of the pipe, the next child will read from there.  */
      		in = fd [0];
    	}
  		/* Last stage of the pipeline - set stdin be the read end of the previous pipe
     	and output to the original file descriptor 1. */

  		if((pid = fork()) == 0)
  		{
  			if (in != 0){
    			dup2 (in, 0);
    			close(in);
			}

  			/* Execute the last stage with the current process. */
   			if(execvp (cmd [i].argArray[0], (char * const *)cmd [i].argArray )<0)
   			{
   				printf("Error in executing the command \n");
        		exit(1);
   			}
		}
		else
		{
			if(isBackground==0)
				while (wait(&status) != pid);
		}
	}
}

void parseCommand(char *cmd)
{
	char *token;
	token = strtok(cmd, " ");
  
	while( token != NULL )
   	{
   	   if(strcmp(token, "|") != 0 && strcmp(token, "<") != 0 && strcmp(token, "&") != 0)
   	   {
			command[pipeing].argArray[argument_count] = token;
			argument_count++;
			command[pipeing].argArray[argument_count] = NULL;
   	   }
   	   token = strtok(NULL, " ");

    if(token)
    {

			if(strcmp(token, "|") == 0)
			{
				command[pipeing].argArray[argument_count] = NULL;
				command[pipeing].argument_count = argument_count;
				argument_count = 0;
				pipeing++;
			}
		}

   	}
   	fork_pipes(pipeing+1, command);
}

void init()
{
	input = '\0';
	int i=0;
	int j=0;
	for(i=0;i<=pipeing;i++)
	{
		for(j=0;j<=command[i].argument_count;j++)
		{
			command[i].argArray[j] = NULL;
		}
	}
	pipeing = 0;
	isBackground = 0;

	argument_count = 0;
	while(bufferChars >= 0)
	{
		buffer[bufferChars] = '\0';
		bufferChars--;
	}
	bufferChars = 0;
}

void getCommand()
{
	init();
	while(input!='\n'){

		input = getchar();
		if(input=='\n') 
      break;

		buffer[bufferChars] = input;
		bufferChars++;
	}
  history[histCount++] = strdup(buffer);
	if(buffer[0]!='\0')
        parseCommand(buffer);
    else return;
}

int main(int argc, char **argv, char **envp)
{
	// Signal Handler for Ctrl + C 
	signal(SIGINT, SIG_IGN);
	signal(SIGINT, handle_signal);


	while(1){
		printf("$ ");
		getCommand();
	}
	return 0;
}
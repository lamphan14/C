#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
char buffer[1024];
int arg_Count = 0;
char input;
int bufferChars = 0;
char *argument_array[64];
char cwd[1024];
int Input_Redirect = 0;
int Output_Redirect = 0;
char *infile;
char *outfile;
int pipeing = 0;
int Background = 0;
char *history[1024];
int i = 0;
int histCount = 0;
struct Command_List
{
	char *argument_array[64];
	int arg_Count;
}
command[50];
typedef void (*sighandler_t)(int);
void handle_signal(int signo)
{
	printf("$ ");
	fflush(stdout);
}
void prox (int in, int out, struct Command_List *cmd)
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

		if(execvp (cmd->argument_array[0], (char * const *)cmd->argument_array ) < 0)
		{
			printf("Error in executing the command \n");
			exit(1);
		}
	}
}

void Fork (int n, struct Command_List *cmd)
{
	if(strcmp(cmd[0].argument_array[0]	, "cd") == 0)
	{
		if(!cmd[0].argument_array[1])
		{
			if(chdir(getenv("HOME")) != 0)
			;
		}
		else
		{
			if(chdir(cmd[0].argument_array[1]) != 0)
			{
				printf("Invalid Path");
			}
		}
	}
 	else if(strcmp(cmd[0].argument_array[0]	, "history") == 0)
	{
    for (int i=0; i < histCount; i++)
    {
		  printf("%s\n", history[i]);
         }
	}
	else if(strcmp(cmd[0].argument_array[0]	, "exit") == 0)
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
  		in = 0;
  		for (i = 0; i < n - 1; ++i)
    	{
      		pipe (fd);
      		prox (in, fd [1], cmd + i);
      		close (fd [1]);
      		in = fd [0];
    	}
  		if((pid = fork()) == 0)
  		{
  			if (in != 0){
    			dup2 (in, 0);
    			close(in);
			}
			if(Output_Redirect == 2)
			{
        		out = open(outfile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        		dup2(out, 1);
        		close(out);
      		}
			if(Output_Redirect == 4)
			{
				out = open(outfile, O_RDWR | O_APPEND | O_CREAT, 0777);
				dup2(out, 1);
				close(out);
			}
    		if(Input_Redirect == 2)
    		{
        		ini = open(infile, O_RDONLY);
        		dup2(ini, 0);
        		close(ini);
    		}
   			if(execvp (cmd [i].argument_array[0], (char * const *)cmd [i].argument_array )<0)
   			{
   				printf("Error in executing the command \n");
        		exit(1);
   			}
		}
		else
		{
			if(Background==0)
				while (wait(&status) != pid);
		}
	}
}
void TAN()
{
	input = '\0';
	int i=0;
	int j=0;
	for(i=0;i<=pipeing;i++)
	{
		for(j=0; j<=command[i].arg_Count; j++)
		{
			command[i].argument_array[j] = NULL;
		}
	}
	pipeing = 0;
	infile = NULL;
	outfile = NULL;
	Input_Redirect = 0;
	Background = 0;
	Output_Redirect = 0;
	arg_Count = 0;
	while(bufferChars >= 0)
	{
		buffer[bufferChars] = '\0';
		bufferChars--;
	}
	bufferChars = 0;
}
void Parse(char *cmd)
{
	char *token;
	token = strtok(cmd, " ");
  
	while( token != NULL )
   	{
   	   if(Output_Redirect == 0 && strcmp(token, "|") != 0 && strcmp(token, "<") != 0 && strcmp(token, "&") != 0)
   	   {
			command[pipeing].argument_array[arg_Count] = token;
			arg_Count++;
			command[pipeing].argument_array[arg_Count] = NULL;
   	   }
   	   token = strtok(NULL, " ");
   	   if(Output_Redirect==1)
   	   {
   	   		outfile = token;
   	   		Output_Redirect = 2;
   	   }
   	   if(Output_Redirect==3)
   	   {
			outfile = token;
			Output_Redirect = 4;
		}
   	   if(Input_Redirect==1)
   	   {
   	   		infile = token;
   	   		Input_Redirect = 2;
   	   }
   	   if(token)
   	   {
			if(strcmp(token, ">") == 0)
			{
				Output_Redirect = 1;
			}
			if(strcmp(token, ">>") == 0)
			{
				Output_Redirect = 3;
			}
			if(strcmp(token, "<") == 0 && Input_Redirect == 0)
			{
				Input_Redirect = 1;
			}
			if(strcmp(token, "&") == 0)
			{
				Background = 1;
				command[pipeing].argument_array[arg_Count] = NULL;
				command[pipeing].arg_Count = arg_Count;
				arg_Count = 0;
				pipeing++;
			}
			if(strcmp(token, "|") == 0)
			{
				command[pipeing].argument_array[arg_Count] = NULL;
				command[pipeing].arg_Count = arg_Count;
				arg_Count = 0;
				pipeing++;
			}
		}

   	}
   	Fork(pipeing+1, command);
}
void Get_Command()
{
	TAN();
	while(input!='\n'){

		input = getchar();
		if(input=='\n') 
      break;

		buffer[bufferChars] = input;
		bufferChars++;
	}
  history[histCount++] = strdup(buffer);
	if(buffer[0]!='\0')
        Parse(buffer);
    else return;
}
int main(int argc, char **argv, char **envp)
{
	printf("mysh ----- type 'exit' to exit this myshell -----\n");
	signal(SIGINT, SIG_IGN); 
	signal(SIGINT, handle_signal);
	while(1){
		printf("$");
		Get_Command();
	}
	return 0;
}
# csh
a shell on linux accomplished by c language

##Input Preprocess
###Data Structure

- JOB

JOB is a structure after a command being separated by '|' .

	struct JOB
	{
	    char *command;
	    char **argv;
	    char *inputFile;
	    char *outputFile;
	    int argNum;
	    pid_t jobid;
	};
	
-  COMMAND_EXE

COMMAND_EXE is a structure storing the whole command.

	struct COMMAND_EXE
	{
	    int mode;                   //BACKGROUND or FOREGROUND
	    struct JOB *task[MAX_PIPELINE];
	    int taskNum ;
	    pid_t cmdid;
	};
	
-  SHELLINFO

 SHELLINFO is a structure current working directory, all the command and number of jobs.

	struct SHELLINFO
	{
		char wd[PATH_SIZE];
		int  jobs;                  //exclude command "wait"
		struct COMMAND_EXE* tsk[MAX_COMMAND];
	}shell;


###Function

- ReadLine

Integrate  memory allocation,get a line from document,block read into a function which can be reused. Variable state indicataes whether it reads successfully.

	char *ReadLine(FILE* f,int * state)
	{
	       char *line = (char*)malloc(sizeof(char)*MAXLEN);
	       fgets(line,MAXLEN,f);
	       int bufLine = strlen(line)+1;
	       if(feof(f))
	       {
		   free(line);
		   *state = 0;
		   return line;
	       }

	       while(bufLine==MAXLEN && (*(line+strlen(line)-1)) != '\n' && !feof(f))
	       {
		    line = (char*)realloc(line ,sizeof(char)*MAXLEN+sizeof(char)*strlen(line));
		    char*buf = (char*)malloc(sizeof(char)*MAXLEN);
		    fgets(buf,MAXLEN,f);
		    strcpy(line+strlen(line),buf);
		    bufLine = strlen(buf)+1;
		    free(buf);
	       }
	       if(line[strlen(line)-1] != '\n')
	       {
		    line = (char*)realloc(line ,sizeof(char)*3+sizeof(char)*strlen(line));
		    line[strlen(line)] = '\n';
		    line[strlen(line)] = '\0';
	       }
	       return line;
	}

	
- Print Error

In order to quit the process elegantly, just use ***PrintErr()***, and ***Exit(1)***

	void PrintErr()
	{
	    char error_message[30] = "An error has occurred\n";
	    write(STDERR_FILENO, error_message, strlen(error_message));
	}


- Split_Line_To_Segment

Divide every command into JOB separated by **|**.

	struct COMMAND_EXE* Split_Line_To_Segment(char*line ,int *isLegal,FILE*f)
	{
	    struct COMMAND_EXE* cmd = (struct COMMAND_EXE*)malloc(sizeof(struct COMMAND_EXE));
	    int iter=0;
	    memset(cmd->task,0,sizeof(cmd->task));
	    cmd->mode= FOREGROUND;
	    cmd->taskNum = 0;
	    struct JOB *temp = NULL;
	    struct JOB *lastJob= NULL;
	    int i=0;
	    int length = strlen(line);
	    int pre = 0;
	    char *end = line;
	    int cmdLen = 0;
	    while(i<length)
	    {
		char ch =*(line+i);
		switch(*(line+i))
		{
		    case '\'':
		    case '\"':
		         end = strchr(line+i+1,ch);
		         int status = 1;
		         if(end==NULL)
		         {
		             while(status&&end==NULL)
		             {
		                putchar('>');
		                char *temp = ReadLine(f,&status);
		                line = (char*)realloc(line,sizeof(char)*(strlen(line)+strlen(temp)+1));
		                strcat(line,temp);
		                end = strchr(line+i+1,ch);
		             }
		         }

		         length = strlen(line);
		         if(end == NULL&&status)
		         {
		             PrintErr();
		             *isLegal = 0; 
		             return cmd;
		         }
		         i = end - line + 1;
		        break;
		    case '|':
		        if(cmd->mode == BACKGROUND)
		        {
		            *isLegal = 0; 
		            return cmd;
		        }
		    case '&':
		         if((*(line+i))=='&')
		            cmd->mode = BACKGROUND; 
		    case '\n':
		         cmdLen = i - pre ;
		         temp = (struct JOB*)malloc(sizeof(struct JOB));  
		         temp->inputFile = NULL;
		         temp->outputFile= NULL;
		         temp->command = (char*) malloc(sizeof(char)*(cmdLen+1));
		         memset(temp->command,0,cmdLen+1);
		         strncpy(temp->command,line+pre,cmdLen);
		         cmd->task[cmd->taskNum] = temp; cmd->taskNum++;
		         lastJob = temp;
		         if((*(line+i))=='&')
		         {
		             i++;
		             while(i<length&&isspace(*(line+i)))
		                 i++;
		         }
		         else 
		            i++;
		         pre = i;
		        break;
		    default:
		        i++;
		}
	    }
	    return cmd;
	}

- Split_Line 

It may be accomplished by calling function ***strtok*** directly, but I build it on the finite state machine, to handle some more complex situations.

	int  Split_Line(struct COMMAND_EXE* cmd,int*isLegal)
	{
	    int iter= 0 ;
	    for(iter = 0;iter<cmd->taskNum;iter++)
	    {
		struct JOB* temp= cmd->task[iter];
		int length = strlen(temp->command);
		char *line = temp->command;
		char ** args=NULL;
		//maintain variable
		int pre = 0;
		int iter = 0 ;
		int status = 0;
		int i=0;

		int cmdLen = 0;
		int fd;
		char *end;
		char *fileName = NULL;
		while(i<length)
		{
		    char ch = *(line+i);
		    switch(decide(ch))
		    {
			case SINGLEQUOTE:
			    if(decide(ch)==SINGLEQUOTE)
			        end = strchr(line+i+1,'\'');
			case DOUBLEQUOTE:
			    if(decide(ch)==DOUBLEQUOTE)
			        end = strchr(line+i+1,'\"');
			    pre = i + 1;
			    i = end - line + 1;
			    cmdLen = i - pre - 1 ;
			    args = (char**)realloc(args,sizeof(char*)*(iter+1));
			    *(args+iter) = (char*)malloc(sizeof(char)*(cmdLen + 1));
			    memset(*(args+iter),0,cmdLen+1);
			    strncpy(*(args+iter),line+pre,cmdLen);
			    iter++;
			    break;
			case SPACE:
			    if(status)
			    {
			        cmdLen = i - pre;
			        args = (char**)realloc(args,sizeof(char*)*(iter+1));
			        *(args+iter) = (char*)malloc(sizeof(char)*(cmdLen + 1));
			        memset(*(args+iter),0,cmdLen+1);
			        strncpy(*(args+iter),line+pre,cmdLen);
			        iter++;
			        status = 0;
			    }
			    i++;
			    pre = i;
			    break;
			case INDIRECTION:
			case OUTDIRECTION:
			    if(status)
			    {
			        cmdLen = i - pre;
			        args = (char**)realloc(args,sizeof(char*)*(iter+1));
			        *(args+iter) = (char*)malloc(sizeof(char)*(cmdLen + 1));
			        memset(*(args+iter),0,cmdLen+1);
			        strncpy(*(args+iter),line+pre,cmdLen);
			        iter++;
			        status = 0;
			    }
			    i++;
			    pre = i;

			    if(ch == '>')
			    {
			        int dcd = decide(*(line+i));
			        while(i<length&&decide(*(line+i))!=VALIDCHAR)     
			        {
			            i++;
			            pre = i;
			        }
			        while(i<length&&decide(*(line+i))==VALIDCHAR)     
			        {
			            i++;
			        }
			        cmdLen = i-pre;
			        if(!cmdLen || temp->outputFile != NULL)
			        {
			            PrintErr();
			            temp->argv = args;
			            *isLegal = 0;
			            return 0;
			        }
			        fileName = (char*)malloc(sizeof(char)*(1+cmdLen));
			        memset(fileName,0,cmdLen+1);
			        strncpy(fileName,(line+pre),cmdLen);
			        temp->outputFile= fileName;
			    }
			    if(ch == '<')
			    {
			        while(i<length&&decide(*(line+i))!=VALIDCHAR)     
			        {
			            i++;
			            pre = i;
			        }
			        while(i<length&&decide(*(line+i))==VALIDCHAR)     
			        {
			            i++;
			        }
			        cmdLen = i-pre;
			        if(!cmdLen || temp->inputFile!= NULL)
			        {
			            PrintErr();
			            temp->argv = args;
			            *isLegal = 0;
			            return 0;
			        }
			        fileName = (char*)malloc(sizeof(char)*(1+cmdLen));
			        memset(fileName,0,cmdLen+1);
			        strncpy(fileName,(line+pre),cmdLen);
			        temp->inputFile= fileName;
			    }
			    pre = i;
			    break;
			case VALIDCHAR:
			    i++; 
			    status = 1;
			    break;
			    
		    }
		}
		if(status)
		{
		    cmdLen = i - pre ;
		    args = (char**)realloc(args,sizeof(char*)*(iter+1));
		    *(args+iter) = (char*)malloc(sizeof(char)*(cmdLen + 1));
		    memset(*(args+iter),0,cmdLen+1);
		    strncpy(*(args+iter),line+pre,cmdLen);
		    iter++;
		    status = 0;
		}
		args = (char**)realloc(args,sizeof(char*)*(iter+1));
		*(args+iter) = NULL;
		temp->argNum = iter;
		temp->argv = args;
		args=NULL;
	    }
	    return 1;
	}

-  Csh_Pipeline

The pipeline's mechanism reminds of recursive form, so I just easily accomplished by recursion(in OS, it's not a good decision)


	void Csh_Exe_Command_Recursive(int i, struct COMMAND_EXE* cmd,int outfd[2])
	{
	    int infd[2] ; 
	    int io[2];
	    io[0] = STDIN_FILENO;
	    io[1] = STDOUT_FILENO;
	    if(i!=0)
		pipe(infd);
	    if(i!=0)
	    {
		pid_t child_id = fork();
		if(child_id == 0)
		{
		    Csh_Exe_Command_Recursive(i-1,cmd,infd);
		}
		else 
		{
		    wait(0);
		}
	    }
	    if(i==0)
	    {
		close(outfd[0]);
		dup2(outfd[1],STDOUT_FILENO);
		io[1] = outfd[1];
	    }
	    else if(i==(cmd->taskNum-1))
	    {
		close(infd[1]);
		dup2(infd[0],STDIN_FILENO);
		io[0] = infd[0];
	    }
	    else 
	    {
		close(infd[1]);
		close(outfd[0]);
		dup2(outfd[1],STDOUT_FILENO);
		dup2(infd[0],STDIN_FILENO);
		io[0] = infd[0];
		io[1] = outfd[1];
	    }
	    struct JOB* job = cmd->task[i];
	    int fd;
	    if(job->inputFile!=NULL)
	    {
		close(io[0]);
		fd=open(job->inputFile,O_RDONLY);
	    }
	    if(job->outputFile!=NULL)
	    {
		close(io[1]);
		fd=open(job->outputFile,O_WRONLY|O_CREAT,0666);
	    }
	    if(fd==-1)
	    {
		PrintErr();
		exit(0);
	    }
	    if(execvp(job->argv[0],job->argv)<0)
	    {
		PrintErr();
	    }
	    exit(0);
	}

	void Csh_Pipeline(struct COMMAND_EXE* cmd)
	{
	    int fd[2];
	    if(fork()==0)
		Csh_Exe_Command_Recursive(cmd->taskNum-1,cmd,fd);
	    else if(cmd->mode != BACKGROUND) wait(0);
	}

- Csh_Main_Loop

Wating for command lines and exucute them

	int Csh_Main_Loop(FILE* f)
	{
	    char *line =NULL;
	    char **arg;
	    int i=0;
	    int argNum=0;
	    do{
	       argNum = 0;
	       if(f==stdin)printf("mysh>");
	       int state = 1;
	       line = ReadLine(f,&state);
	       if(!state)break;
	       if(f!=stdin)
	       {
		   printf("mysh>");
		   printf("%s",line);
		   if(line[strlen(line)-1]!='\n')putchar('\n');
	       }

	       int isLegal = 1;
	       struct COMMAND_EXE *cmd = Split_Line_To_Segment(line,&isLegal,f);
	       if(isLegal)argNum = Split_Line(cmd,&isLegal);
	       //print(cmd);
	       if(isLegal)
	       {
		   if(cmd->taskNum == 1)Csh_Execute(cmd->task[0],cmd->mode,STDIN_FILENO,STDOUT_FILENO);
		   else Csh_Pipeline(cmd);
	       }
	       if(line!=NULL)free(line) ;
	       //free memory
	       if(cmd!=NULL)
	       {
		   int ii = 0;
		   for(ii=0;ii<cmd->taskNum;ii++)
		   {
		        struct JOB* j = cmd->task[ii];

		        if(j->command!=NULL)free(j->command);
		        if(j->inputFile!=NULL)free(j->inputFile);
		        if(j->outputFile!=NULL)free(j->outputFile);
		        for(i=0;i<j->argNum;i++)
		        {
		            if((j->argv[i]!=NULL))free(j->argv[i]);
		        }
		        if(j->argv!=NULL)free(j->argv);
		        free(j);
		   }
		   free(cmd);
	       }
	    }while(1);


	    return SUCCESS_EXIT;
	}

More details about ***pipeline*** , ***redirection*** ,***fork*** introduction,will be included.

Sunday, 14. October 2018 09:12PM 


























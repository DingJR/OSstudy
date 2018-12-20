#include <sys/wait.h>
#include <pthread.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <termios.h>
#include <ctype.h>
#define FOR(i,a) for(int i=0;i<a;i++)
#define DFOR(i,a) for(int i=a;i>=0;i--)
#define BUFFER_SIZE 1024
#define PATH_SIZE 1024
#define MAX_COMMAND 30
#define MAXLEN 1024
#define MAX_PIPELINE 1024

#define SUCCESS_EXIT 0
#define FAIL_EXIT -1

#define FOREGROUND 1
#define BACKGROUND 0
#define PIPELINE   -1

#define COMMAND_EXIT    1
#define COMMAND_CD_DIR  2
#define COMMAND_PWD     3
#define COMMAND_WAIT    4

#define delimiter " \t\n\r"
#define VALIDCHAR 0
#define SINGLEQUOTE  1
#define DOUBLEQUOTE  2
#define INDIRECTION  3
#define OUTDIRECTION 4
#define SPACE        5
#define NULLIFY      6

#define SHAMEMPID    1234
#define MAXPROCESSES 100


struct JOB
{
    char *command;
    char **argv;
    char *inputFile;
    char *outputFile;
    int argNum;
    pid_t jobid;
};

struct COMMAND_EXE
{
    int mode;                   //BACKGROUND or FOREGROUND
    struct JOB *task[MAX_PIPELINE];
    int taskNum ;
    pid_t cmdid;
};
    

struct SHELLINFO
{
    char wd[PATH_SIZE];
    int  jobs;                  //exclude command "wait"
    struct COMMAND_EXE* tsk[MAX_COMMAND];
}shell;

void PrintErr()
{
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}


pid_t mytask[MAXPROCESSES];
void Release_Zombie()
{
    int i=0;
    pid_t w = 0;
    for(i=0;i<MAXPROCESSES;i++)
    {
        if(mytask[i]!=0)
        {
            w = waitpid(mytask[i],(int*) 0,WNOHANG);    
            if(w != 0)mytask[i] = 0;
        }
    }
}

void AddProcess(pid_t w)
{
    int i=0;
    for(i=0;i<MAXPROCESSES;i++)
    {
        if(mytask[i]==0)
        {
            mytask[i] = w;  
            return ;
        }
    }
}

/*
struct shared_pid
{
    pthread_mutex_t smCtrl;
    pthread_mutex_t subCtrl;
    int gblPid ;
    int NumOfPid ;
    pid_t prcsStack[100] ;
};
struct shared_pid *smpid;
void Share_Memory()
{
    int snm;
    snm = shmget((key_t)SHAMEMPID, sizeof(struct shared_pid), 0666|IPC_CREAT);
    smpid = (struct shared_pid *)shmat(snm, 0, 0);
}
void Delete_Shared_Memory()
{
    shmdt((void*)smpid);
}
void ZombieClr(int signum)
{
    int ii = 0;
    int status;
    if(smpid->gblPid == 0)
    {
        pthread_mutex_unlock(&smpid->smCtrl);
        return ;
    }
    for(ii=0;ii<smpid->NumOfPid;ii++)
        if(smpid->prcsStack[ii] == smpid->gblPid)
        {
            pid_t w = waitpid(smpid->prcsStack[smpid->gblPid],&status,WUNTRACED | WCONTINUED);
            smpid->NumOfPid--;
            if(ii!=smpid->NumOfPid)
                smpid->prcsStack[ii] = smpid->prcsStack[smpid->NumOfPid];
            else smpid->prcsStack[ii] = 0;
            smpid->gblPid = 0;
            pthread_mutex_unlock(&smpid->smCtrl);
            return ;
        }
    pthread_mutex_unlock(&smpid->smCtrl);
}
*/

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

int decide(char a)
{
    if(a == '\'')return SINGLEQUOTE;
    if(a == '\"')return DOUBLEQUOTE;
    if(isspace(a)) return SPACE;
    if(a == '>') return OUTDIRECTION;
    if(a == '<') return INDIRECTION;
    if(a == '\0') return NULLIFY;
    return VALIDCHAR;
}


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
                case NULLIFY:
                    i++;
                    break;
                case SINGLEQUOTE:
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
                        if((*(line+pre)) == '$')
                        {
                            *(line+pre+cmdLen) = '\0';
                            if(getenv(line+pre+1)!=NULL)
                            {
                                free(*(args+iter));
                                *(args+iter) = (char*)malloc(sizeof(char)*PATH_SIZE);
                                strcpy(*(args+iter),getenv(line+pre+1));
                            }
                        }
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
                        if((*(line+pre)) == '$')
                        {
                            free(*(args+iter));
                            *(args+iter) = (char*)malloc(sizeof(char)*(PATH_SIZE));
                            strcpy(*(args+iter),getenv(line+pre+1));
                        }
                        iter++;
                        status = 0;
                    }
                    i++;
                    pre = i;
                    int dcd = decide(*(line+i));
                    if( i == length || (dcd != SPACE && dcd != VALIDCHAR))
                    {
                            PrintErr();
                            temp->argv = args;
                            *isLegal = 0;
                            return 0;
                    }

                    if(ch == '>')
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
                    while(i<length&&decide(*(line+i))!=VALIDCHAR)     
                    {
                        i++;
                        pre = i;
                    }
                    if(i<length)
                        if(ch == (*(line+i)) || decide(*(line+i)) == VALIDCHAR)
                        {
                            PrintErr();
                            temp->argv = args;
                            *isLegal = 0;
                            return 0;
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
            if((*(line+pre)) == '$')
            {
                *(line+pre+cmdLen) = '\0';
                if(getenv(line+pre+1)!=NULL)
                {
                    free(*(args+iter));
                    *(args+iter) = (char*)malloc(sizeof(char)*(PATH_SIZE));
                    strcpy(*(args+iter),getenv(line+pre+1));
                }
            }
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

int BuildIn(char *line)
{
    if(!strcmp(line,"exit"))return COMMAND_EXIT;
    if(!strcmp(line,"cd"))return COMMAND_CD_DIR;
    if(!strcmp(line,"pwd"))return COMMAND_PWD;
    if(!strcmp(line,"wait"))return COMMAND_WAIT;
    return 0;
}

void Csh_Execute(struct JOB* job,int mode,int infl,int otfl)
{
            int argc = job->argNum; char**argv = job->argv;
            int fd = 0;
            pid_t child_id ;
            int status = BuildIn(argv[0]);             //build in or bash command
        /* 
         * Build in Command
         */
            if(status)
            {
                switch(status)
                {
                    case COMMAND_EXIT:
                        exit(0);
                        break;
    
                    case COMMAND_CD_DIR:
                        if(argc == 1)
                        {
                            chdir(getenv("HOME"));
                            getcwd(shell.wd,PATH_SIZE);
                        }
                        else 
                        {
                            if(chdir(argv[1])<0)
                            {
                                PrintErr();
                            }
                            getcwd(shell.wd,PATH_SIZE);
                        }
                        break;
    
                    case COMMAND_PWD:
                        getcwd(shell.wd,PATH_SIZE);
                        if(job->outputFile!=NULL)
                        {
                            child_id = fork();
                            if(child_id==0)
                            {
                                fd=open(job->outputFile,O_WRONLY|O_CREAT,0666);
                                dup2(fd,otfl);
                                write(STDOUT_FILENO,shell.wd, strlen(shell.wd));
                                write(STDOUT_FILENO, "\n" , strlen("\n"));
                                exit(0);
                            }
                            else 
                            {
                                pid_t w = waitpid(child_id,&status,WUNTRACED | WCONTINUED);
                            }
                        }
                        else 
                        {
                                write(STDOUT_FILENO,shell.wd, strlen(shell.wd));
                                write(STDOUT_FILENO, "\n" , strlen("\n"));
                        }
                        break;
    
                    case COMMAND_WAIT:
                        {
                            signal(SIGCHLD, SIG_IGN);
                            wait(NULL);
                            signal(SIGCHLD,SIG_DFL);
                            break;
                        }
                }
            }

        /*
        * bash command
        */
            else 
            {
                    child_id = fork();
                    if(child_id<0)
                    {
                        PrintErr();
                    }
                    else if(child_id == 0)
                    {
                        /*
                        if(fork() == 0)
                        {
                        */
                            if(job->inputFile!=NULL)
                            {
                                fd=open(job->inputFile,O_RDONLY);
                                dup2(fd,infl);
                            }
                            if(fd==-1)
                            {
                                PrintErr();
                                exit(1);
                            }
                            if(job->outputFile!=NULL)
                            {
                                fd=open(job->outputFile,O_WRONLY|O_CREAT,0666);
                                dup2(fd,otfl);
                            }
                            if(fd==-1)
                            {
                                PrintErr();
                                exit(1);
                            }
                            if(execvp(argv[0],argv)<0)PrintErr();
                            exit(FAIL_EXIT);
                        /*
                        }
                        else 
                        {
                            int ii=0;
                            //printf("lala :%d\n",smpid->NumOfPid);
                            //printf("lala: %d\n",smpid->gblPid);

                            waitpid(0,&status,WUNTRACED | WCONTINUED);
                            pthread_mutex_lock(&smpid->subCtrl);
                            pthread_mutex_lock(&smpid->subCtrl);
                            pthread_mutex_lock(&smpid->smCtrl);
                            smpid->gblPid = 0;
                            for(ii=0;ii<smpid->NumOfPid;ii++)
                                if(smpid->prcsStack[ii] == getpid())
                                {
                                    smpid->gblPid = getpid();
                                    break;
                                }
                            pthread_mutex_unlock(&smpid->subCtrl);
                            Share_Memory();
                            exit(0);
                        }
                        */
                    }
                    else 
                    {
                            //signal(SIGCHLD, SIG_DFL);
                            pid_t w;
                            if(mode == FOREGROUND)
                                w = waitpid(child_id,&status,WUNTRACED | WCONTINUED);
                            if(mode == BACKGROUND)
                            {
                                AddProcess(child_id);
                                /*
                                pthread_mutex_lock(&smpid->smCtrl);
                                pthread_mutex_unlock(&smpid->subCtrl);
                                smpid->NumOfPid++;
                                int ii = 0;
                                for(ii=0;ii<smpid->NumOfPid;ii++)
                                {
                                    if(smpid->prcsStack[ii] == 0)
                                    {
                                        smpid->prcsStack[ii] = child_id;
                                        break;
                                    }
                                }
                                pthread_mutex_unlock(&smpid->smCtrl);
                                signal(SIGCHLD,ZombieClr); 
                                */
                            }
                            /*
                            shell.tsk[shell.job] = (struct COMMAND_EXE*)malloc(sizeof(struct COMMAND_EXE));
                            struct COMMAND_EXE* temp =shell.tsk[shell.job];
                            temp->status = 1;
                            shell.jobs=(shell.jobs+1)%MAX_COMMAND;
                            */
                    }
            }
}

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
    int mode = cmd->mode;
    int child_id = fork();
    pid_t w;
    int status;
    if(child_id==0)
    {
        Csh_Exe_Command_Recursive(cmd->taskNum-1,cmd,fd);
    }
    else 
    {
        if(mode == FOREGROUND)
            w = waitpid(child_id,&status,WUNTRACED | WCONTINUED);
        if(mode == BACKGROUND)
        {
            AddProcess(child_id);
        }
    }
}

void print(struct COMMAND_EXE *in)
{
    int i;
    puts("In Print");
    int iter = 0;
    printf("%d\n",in->taskNum);
    for(iter = 0;iter<in->taskNum;iter++)
    {
        struct JOB* temp = in->task[iter];
        printf("%s\n",temp->command);
        printf("input: %s\n",temp->inputFile);
        printf("output: %s\n",temp->outputFile);
        for(i=0;i<temp->argNum;i++)
        {
            printf("%d:%s ##\n",i,temp->argv[i]);
        }
        putchar('\n');
    }
    exit(0);
}


int Csh_Main_Loop(FILE* f)
{
    char *line =NULL;
    char **arg;
    int i=0;
    int argNum=0;
    char mysh[10] = "mysh> ";

    do{
       argNum = 0;
       if(f==stdin)
           write(STDOUT_FILENO, mysh, strlen(mysh));
       int state = 1;
       line = ReadLine(f,&state);
       if(!state)break;
       if(f!=stdin)
       {
           write(STDOUT_FILENO, mysh, strlen(mysh));
           write(STDOUT_FILENO, line, strlen(line));
           if(line[strlen(line)-1]!='\n')putchar('\n');
       }

       int isLegal = 1;
       struct COMMAND_EXE *cmd = Split_Line_To_Segment(line,&isLegal,f);
       if(isLegal)argNum = Split_Line(cmd,&isLegal);
       Release_Zombie();
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

int main(int argc,char**argv)
{
    FILE* f=stdin;
    /*
    Share_Memory();
    smpid->NumOfPid = 0;
    smpid->gblPid   = 0;
    int ii=0;
    for(ii=0;ii<100;ii++)
        smpid->prcsStack[ii] = 0;
    pthread_mutex_init(&smpid->smCtrl,NULL);
    pthread_mutex_init(&smpid->subCtrl,NULL);
    */
    if(argc == 2)
    {
        f = fopen(argv[1],"r");
        if(f==NULL)
        {
            PrintErr();
            exit(-1);
        }
        if(Csh_Main_Loop(f)==FAIL_EXIT)return -1;
    }
    else
    {
        if(Csh_Main_Loop(f)==FAIL_EXIT)return -1;
    }
    //pthread_mutex_destroy(&smpid->smCtrl);
    //Delete_Shared_Memory();
    return 0;
}

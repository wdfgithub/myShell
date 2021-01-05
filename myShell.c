#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <unistd.h>
#include <sys/wait.h>

char **get_input(char *);
char *get_front_input(char *);
char *get_tail_input(char *);
void exe_command(char **);
int isPipeComm(char *);
int main() {
    pid_t child_pid;
    int stat_loc;
    while (1) {
        char *input;
        //input为输入的内容
        input = readline("myshell>>");
        //输入exit退出
        if (strcmp(input,"exit")==0){
            free(input);
            break;
        }
        //输入help有提示
        if (strcmp(input,"help")==0){
            free(input);
            printf("you can use exit/ls/cp file...\n");
            continue;
        }
        if(isPipeComm(input)==0){//如果不是管道命令，正常执行即可
            //get_input将输入内容分解为命令和参数，存放在command中
            char **command;
            command = get_input(input);
            //子进程执行命令
            child_pid = fork();
            if (child_pid == 0) {
                exe_command(command);
                free(input);
                free(command);
                break;
            } else {
                waitpid(child_pid, &stat_loc, WUNTRACED);
                free(input);
                free(command);
            }
        }else{//如果是一重管道命令，将命令分为两部分执行
            //get_input将输入内容分解为命令和参数，存放在command中
            char *front_command,*tail_command;
            char **command1,**command2;
            front_command = get_front_input(input);
            tail_command = get_tail_input(input);
            int    fd[2];
            pid_t pid1,pid2;
            //子进程执行管道命令
            child_pid = fork();
            if (child_pid == 0) {
                pipe(fd); 
                pid1 = fork(); 
                
                if(pid1 == 0) {
                    close(fd[0]);
                    dup2(fd[1],1);
                    command1 = get_input(front_command);
                    exe_command(command1);
                }   
                else if(pid1 > 0) {
                    waitpid(pid1, &stat_loc, WUNTRACED); 
                    pid2 = fork(); 
                    if(pid2 == 0) {
                        close(fd[1]);
                        dup2(fd[0],0); 
                        command2 = get_input(tail_command);
                        exe_command(command2);
                    }
                    else {
                        close(fd[0]); 
                        close(fd[1]); 
                        waitpid(pid2, &stat_loc, WUNTRACED); 
                    }
                }
                
                free(input);
                free(command1);
                free(command2);
                free(front_command);
                free(tail_command);
                break;
            } else {
                waitpid(child_pid, &stat_loc, WUNTRACED);
                free(input);
                free(front_command);
                free(tail_command);
            }
        }
    }

    return 0;
}
char **get_input(char *input) {
    char **command = malloc(8 * sizeof(char *));
    char *separator = " ";
    char *parsed;
    int index = 0;
    parsed = strtok(input, separator);
    while (parsed != NULL) {
        command[index] = parsed;
        index++;
        parsed = strtok(NULL, separator);
    }
    command[index] = NULL;
    return command;
}
char *get_front_input(char *input) {
    char *ans=malloc(32*sizeof(char));
    int i=0;
    while(input[i]!='|'){
        ans[i]=input[i];
        i++;
        
    }
    ans[i]='\0';
    return ans;
}
char *get_tail_input(char *input) {
    char *ans=malloc(32*sizeof(char));
    int i=0,j=0;
    while(input[i]!='|'){
        i++;
    }
    i++;
    while(input[i]!='\0'){
        ans[j]=input[i];
        i++;
        j++;
    }
    ans[j]='\0';
    return ans;
    
}
void exe_command(char **command){
    execvp(command[0], command);
    printf("command wrong!\n");
}
int isPipeComm(char *input){
    int i=0;
    while (input[i]!='\0') {
        if(input[i++]=='|'){
            return 1;
        }
    }
    return 0;
}

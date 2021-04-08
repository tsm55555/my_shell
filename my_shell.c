#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

void clrscr();
void my_exit(char command[]);
void read_command(char command[], char *parameters[]);
void execute(char command[], char *parameters[]);
int redirect_to(char *parameters[], char command[]);
int redirect_from(char *parameters[]);
int my_pipe(char *parameters[]);
void common_syntax_error(char command[]);

int main() {
    char user_name[1024];
    gethostname(user_name, sizeof(user_name));

    while (1){
        char cwd[1024];
        char *parameters[1024];
        char command[1000];

        getcwd(cwd, sizeof(cwd));
        printf("\033[0;92m");
        printf("%s :",user_name);
        printf("\033[0;44m");
        printf("%s$", cwd);
        printf("\033[0;37m");
        printf(" ");
        
        read_command(command, parameters);
        if(fork()!=0){
            wait(NULL);            
        }
        else{
            int redirect_to_index = redirect_to(parameters, command);
            int redirect_from_index;
            //printf("redirect_to_index %d\n", redirect_to_index);
            if(redirect_to_index != 2 && redirect_to_index != 1){
                redirect_from_index = redirect_from(parameters);
            }
            int my_pipe_index;
            if(redirect_to_index != 3 && redirect_to_index != 2 && redirect_to_index != 1){
                my_pipe_index = my_pipe(parameters);
            }
            if(redirect_to_index == 0 && redirect_from_index == 0 && my_pipe_index == 0){
                execute(command, parameters);
            }
            else{
                exit(0);
            }
        }
    }
    return 0;
}

void clrscr()
{
    printf("\033[2J\033[1;1H");
}

void my_exit(char command[]){
    if(strcmp("exit", command)== 0){
        //printf("Good Bye!!\n");
        _exit(0);
    }
    else if(strcmp("clear", command)== 0){
        clrscr();
    }
}

void read_command(char command[], char *parameters[]){
    char input[1024];
    fgets(input, 1024, stdin);
    input[strcspn(input, "\n")] = 0;
    
    char *pch, *array[100];
    pch = strtok(input, " ");
    int i = 0;
    while (pch != NULL)
    {
        array[i++] = strdup(pch);
        pch = strtok (NULL, " ");
    }
    strcpy(command, array[0]);
    my_exit(command);
    for (int j = 0; j < i; j++){
        parameters[j] = array[j];
    }
    parameters[i] = NULL;
}

void execute(char command[], char *parameters[]){
    if(execvp(command, parameters)<0){
        common_syntax_error(command);
        exit(0);
    }
}

int redirect_to(char *parameters[], char command[]){
    int i = 0;
    int done = 0;
    int my_pipe_index;
    int redirect_from_index;
    while(parameters[i] != NULL){   
        if(*parameters[i] == '>'){
            done = 1;
            parameters[i] = NULL;
            freopen(parameters[i+1],"w",stdout);
            i++;
            pid_t pid = fork();
            if(pid == 0){
                my_pipe_index = my_pipe(parameters);
                redirect_from_index = redirect_from(parameters);
                if(my_pipe_index == 0 && redirect_from_index == 0){
                    execvp(command, parameters);
                }
            if(pid > 0){
                wait(0);
                break;
            }
            }
        }
        else{
            i++;
        }
    }
    if(my_pipe_index == 1){
        return 3;
    }
    else if(redirect_from_index == 1){
        return 2;
    }
    else if(done == 1){
        return 1;
    }
    else{
        return 0;
    }
}

int redirect_from(char *parameters[]){
    int i = 0;
    int done = 0;
    while(parameters[i] != NULL){
        if(*parameters[i] == '<'){
            parameters[i] = NULL;
            freopen(parameters[i+1],"r", stdin);
            i++;
            done = 1;            
            pid_t pid = fork();
            if(pid == 0){
                execvp(*parameters, parameters);
            }
            if(pid > 0){
                wait(0);
                printf("\n");
            }
            
        }
        else{
            i++;
        } 
    }
    if(done == 1){
        return 1;
    }
    else{
        return 0;
    }  
}

int my_pipe(char *parameters[]){
    int i = 0;
    int done = 0;
    while(parameters[i] != NULL){
        if(*parameters[i] == '|'){
            parameters[i] = NULL;
            int fd[2];
            pipe(fd);

            pid_t pid = fork();
            //printf("pid %ld\n", (long)pid);
            if(pid > 0){
                wait(0);
                close(fd[1]);
                dup2(fd[0], STDIN_FILENO); 
                close(fd[0]);
                char command[100];
                char *par[100];
                strcat (command, parameters[i+1]);
                int next = i+1;
                while(parameters[next] != NULL){
                    if(*parameters[next] == '|' ){
                        break;
                    }
                    else{
                        next++;
                    }
                }
                int index = i + 1;
                for(int k=0; k < next-i-1; k++){
                    par[k] = parameters[index];
                    index++;
                }
                execvp(command, par);   
            }
            else if(pid == 0){
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);     
                execvp(parameters[0], parameters);
            }
            done = 1;
            i++;
        }
        else{
            i++;
        }
    }
    if(done == 1){
        return 1;
    }
    else{
        return 0;
    }
}

void common_syntax_error(char command[]){
    if(strcmp(";s", command) == 0){
        printf("Command ';s' not found, did you mean: 'ls'?\n\n");
    }
    else if(strcmp("pws", command)== 0){
        printf("Command 'pws' not found, did you mean: 'pwd'?\n\n");
    }
    else{
        printf("syntax error near unexpected token '%s'\n\n", command);
    }
}

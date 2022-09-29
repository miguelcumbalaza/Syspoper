#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_LINE 40 /* The maximum length command */

int n = 0;
char history[100][40];
int h = 0;

int main(void){
    pid_t pid;
    int pipe_child[2], readbytes;
    char *args[MAX_LINE]; /* command line arguments */
    char command[MAX_LINE];
    size_t argsSize = 40;
    int should_run = 1; /* flag to determine when to exit program */
    while (should_run){
        pipe(pipe_child);
        pipe(pipe_parent);
        char buffer1[MAX_LINE] = {};
        bool ampersan = false;
        bool openFd = false, openFd2 = false;
        char * openFile;
        int fdOut, fdIn;
        fflush(stdout);
        fputs("prompto>", stdout);
        fgets(command, argsSize, stdin);
        bool hist = true;
        if(strcmp(command, "!!\n") == 0){
            if(h > 0){
                strcpy(command, history[h - 1]);
                printf("%s", command);
            } else{
                hist = false;
                printf("no commands in history\n");
                continue;
            }
        } else{
            strcpy(history[h], command);
            h++;
        }
        char * token = strtok(command, " ");
        n = 0;
        args[n] = token;
        n++;
        while(token != NULL && n < MAX_LINE){
            token = strtok(NULL, " ");
            args[n] = token;
            n++;
        }
        if(strcmp(args[0], "exit\n") == 0){
            should_run = 0;
        }
        else if(strcmp(args[0], "\n") != 0){
            args[n - 2] = strtok(args[n - 2], "\n");
            if(strcmp(args[n - 2], "&") == 0){
                n--;
                ampersan = true;
            }
            if(n > 3 && (strcmp(args[n - 3], ">") == 0 || strcmp(args[n - 3], "<") == 0)){
                if(strcmp(args[n - 3], ">") == 0){
                    for(int j = 0; j < n - 3; j++){
                        strcat(buffer1, args[j]);
                        strcat(buffer1, " ");
                    }
                    openFile = args[n - 2];
                    openFd = true;
                } else if(strcmp(args[n - 3], "<") == 0){
                    for(int j = 0; j < n - 3; j++){
                        strcat(buffer1, args[j]);
                        strcat(buffer1, " ");
                    }
                    openFile = args[n - 2];
                    openFd2 = true;
                }
            } else{
                for(int j = 0; j < n - 1; j++){
                    strcat(buffer1, args[j]);
                    strcat(buffer1, " ");
                }
            }
            
            pid = fork();
            if(pid < 0){
                fprintf(stderr, "prompto> Fork Failed\n");
                return 1;
            } else if(pid == 0){
                char * argsR[MAX_LINE];
                char buffer2[MAX_LINE];
                int j = 0;
                close(pipe_child[1]);/*cierro escritura*/
                
                while( (readbytes = read( pipe_child[0], buffer2, MAX_LINE ) ) > 0)
                
                close( pipe_child[0] );/*cierro lectura*/
                if(openFd){
                    fdOut = open(openFile, O_CREAT | O_WRONLY | O_TRUNC);
                    fdOut = dup2(fdOut, STDOUT_FILENO);
                }
                char * token = strtok(buffer2, " ");
                n = 0;
                argsR[n] = token;
                n++;
                while(token != NULL && n < MAX_LINE){
                    token = strtok(NULL, " ");
                    argsR[n] = token;
                    n++;
                }

                if(openFd2){
                    fdIn = open(openFile, O_CREAT | O_RDONLY);
                    fdIn = dup2(fdIn, STDIN_FILENO);
                }

                if(strcmp(argsR[n - 3], "|") == 0){
                    char * argsP[MAX_LINE];
                    int pipe_child2[2], readbytes2;
                    int m = 0, newArgs;
                    pid_t pid2;
                    pipe(pipe_child2);

                    for(int i = 0; i < n - 3; i++){
                        argsP[i] = argsR[i];
                        m++;
                    }
                    argsP[m] = NULL;
                    m++;

                    pid2 = fork();
                    if(pid2 < 0){
                        fprintf(stderr, "prompto> Fork Failed\n");
                        return 1;
                    } else if(pid2 == 0){
                        close(pipe_child2[1]);
                        char buffer3[MAX_LINE];
                        char * argsT[MAX_LINE];
                        int k = 0;
                        
                        dup2(pipe_child2[0], STDIN_FILENO);

                        argsT[k] = argsR[n - 2];
                        k++;
                        argsT[k] = NULL;
                        k++;
                        execvp(argsT[0], argsT);
                    } else{
                        close(pipe_child2[0]);

                        dup2(pipe_child2[1], STDOUT_FILENO);
                        execvp(argsP[0], argsP);
                    }
                } else{
                    execvp(argsR[0], argsR);
                }
            } else{
                close(pipe_child[0]); /*cierro lectura hijo*/
		write(pipe_child[1], buffer1, MAX_LINE);
                close(pipe_child[1]);/*cierro lectura hijo*/
                if(ampersan){
                    wait(NULL);
                }
                
            }
        }
        waitpid( pid, NULL, 0 );
        
    }
    return 0;
}

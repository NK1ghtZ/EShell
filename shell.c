#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

void affiche_cmd(char *argv[]){
    int i = 0;
    while(argv[i]!=NULL){
        printf("%s",argv[i]);
        printf(" ");
        i++;
    }
    printf("NULL\n");
}

int parse_line(char *s, char **argv[]){ 
    if(s == NULL || *s == 0 || strcmp(s," ") == 0) return 1;
    if(s[0] == '#'){
        printf("this is a comment, we exit the shell\n"); //in fact i leave the shell because there will be calls of fork() and 100% CPU
        exit(1);
    } 
    s = strtok(s, "#");
    char* buffer = NULL;
    buffer = strtok(s, " "); //takes the character string as a parameter, when there is a space it will return the first character string and the store in buffer
    int i = 0;
    (*argv)[i] = malloc(8124);
    strcpy((*argv)[i],buffer); //copy the buffer in argv
    while(buffer != NULL){
        i++;
        buffer = strtok(NULL, " ");
        if(buffer == NULL){
            break;
        }
        (*argv)[i] = malloc(strlen(buffer)+1);
        strcpy((*argv)[i],buffer);
    }
    (*argv)[i+1]=NULL; 
    return i; 
}

void free_parse(char **argv[]){
    int i = 0;
    while((*argv)[i] != NULL){
        free((*argv)[i]);
        (*argv)[i] = NULL;
        i++;  
    }
    (*argv)[i+1]=NULL;
}

int simple_cmd(char *argv[]){
    if(argv[0] == NULL){
        return 1;
    }
    pid_t p;
    if(!strcmp(argv[0], "cd")){ 
        //change of directory without do call of fork()
        if(argv[1] == NULL){
            return 1;
        }
        int fd = chdir(argv[1]);
        if(fd == -1){ 
            //error if no folder is contained in fd
            write(STDOUT_FILENO,"Directory not found\n",strlen("Directory not found\n"));
            return 1;
        }
    }else if(!strcmp(argv[0],"exit")){
        //receive exit so the program is stopped
        exit(1);
    }else{
        p = fork(); //child process
        if(p == 0){
            execvp(argv[0],argv);
            return 0;
        }
        while(1){
            int attente = waitpid(p,NULL,WNOHANG); //allows to avoid 100% of CPU
            if(attente == p){
                break;
            }
        }
   return 1;
  }   
  return 0; 
}

void exec_script(int s){ //execute the first line contained in a file but not the rest
    char* buffer = malloc(8124);
    int nb;
    int i = 0;
    char* tmp = malloc(sizeof(char*)*100);
    char** argv = malloc(sizeof(char*)*100);
    while((nb = read(s,buffer,8124))>0){  
        buffer[nb] = '\0';
        if(nb == 0){
            return;
        }       
        while(buffer[i] != '\0'){
            //int j = 0;
            if(buffer[i] == '\n'){
                //lseek(s,j,SEEK_SET);
                //printf("I skip a line");
            }else{
                tmp[i]=buffer[i];
            }                
        i++;
        }
        argv[i] ='\0';
        parse_line(tmp,&argv);
        simple_cmd(argv);
    }
    free(tmp);
    free(argv);
    free(buffer);
}

int parse_line_redir(char *s, char **argv [], char **in, char **out){ //the parse works but the redirect does not work
    *in = NULL;
    *out = NULL;
    int i = 0;
    if(s[0] == '#'){
        printf("this is a comment, we exit the shell\n"); //same as in parse_line..
        exit(1);
    } 
    s = strtok(s, "#");
    char* buffer = strtok(s, " ");
    (*argv)[i] = buffer;
    while(buffer != NULL){
        buffer = strtok(NULL, " ");
        if(buffer != NULL){
            if(!strcmp(buffer,"<")){
                buffer = strtok(NULL, " ");
                if(buffer == NULL){
                    break;
                }
                *in = buffer;
            }else if(!strcmp(buffer,">")){
                buffer = strtok(NULL, " ");
                if(buffer == NULL){
                    break;
                }
                *out = buffer;
            }else{
                i++;
                (*argv)[i] = buffer;
            }
        }
    }
    (*argv)[i+1] = NULL;
    return 0;
}

int redir_cmd(char *argv[], char *in , char *out){ //probably doesn't work because of the redirection issue caused by parse_line_redir..
    int fdin;
    int fdout;
    int dupin;
    int dupout;
    if(in != NULL){
        fdin = open(in, O_RDONLY);
        dupin = dup(STDIN_FILENO);
        dup2(fdin, STDIN_FILENO);
    } 
    if(out != NULL){
        fdout = open(out, O_WRONLY|O_CREAT, 0644);
        dupout = dup(STDOUT_FILENO);
        dup2(fdout, STDOUT_FILENO);
    }
    simple_cmd(argv);
    if(in != NULL){ 
        close(fdin); 
        dup2(dupin, STDIN_FILENO); 
    } 
    if(out != NULL){ 
        close(fdout); 
        dup2(dupout, STDOUT_FILENO); 
    }
    return 0;
}

int main(int argc, char** argv){
    //affiche_cmd(argv);
    //char s [] = "hello world";
    //char s2[] = "ls -l";
    char** argv2 = malloc(sizeof(char*)*100);
    char *in;
    char *out;
    //parse_line(s2,&argv2); 
    char* curdir = malloc(sizeof(char)*1024);
    char cmd[1024];
    char** argvs = malloc(sizeof(char*)*100);
    //simple_cmd(argv2);
    int fd = open(argv[1],O_RDONLY);
    if(argc>1){
        if(fd == -1){
            write(STDOUT_FILENO,"No such file or directory\n",strlen("No such file or directory\n"));
            exit(EXIT_FAILURE);
        }
        exec_script(fd);
    }
    while(1){
        curdir = getcwd(curdir,200);
        write(STDOUT_FILENO, curdir, strlen(curdir));
        write(STDOUT_FILENO," $ ", strlen(" $ "));
        int nb = read(0,cmd,1024);
        if(nb==0){
            printf("\n");
            break;
        }
        cmd[nb-1]='\0';
        parse_line_redir(cmd,&argvs, &in, &out);
        simple_cmd(argvs);
    }
    close(fd);
    free_parse(&argv2);
    free(argvs);
    free(argv2);
    free(curdir);
    return 0;
}    

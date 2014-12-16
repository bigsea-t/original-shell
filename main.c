/*
 * Original shell writtein in C.
 *
 * Copyright (c) 2014 Taikai Takeda <297.1951@gmail.com>
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> /* fork, wait */
#include <sys/wait.h>  /* wait */
#include <string.h>

const int BUF_SIZE = 1024;
void exec(char** argv, int argc);
int pipe_idx(char** argv, int argc);
void exec_no_pipe(char ** argv, int argc);
int split_cmd(char* cmd, char** argv);


void print_args(char** argv, int argc){
    printf("argc: %d\n",argc);
    printf("argv: ");
    for (int i = 0; i <= argc; i++) {
      printf("%s ",argv[i]);
    }
    printf("\n");
}


/* int split_by_pipe(char** argv, int argc, char** argv_r){ */
/*   int p_idx = pipe_idx(argv, argc); */
/*   if(p_idx==-1){ */
/*     argv[argc] = NULL; */
/*     exec_no_pipe(argv, argc); */
/*   } else { */
/*     printf("pipe is in %d\n",p_idx); */
/*     char** argv_r = argv + p_idx + 1; */
/*     char** argv_l = argv; */
/*     int argc_l = p_idx; */
/*     int argc_r = argc - 1 - argc_l; */
/*     *(argv_l + argc_l) = NULL; */
/*     *(argv_r + argc_r) = NULL; */
/*     print_args(argv_l, argc_l); */
/*     print_args(argv_r, argc_r); */
/*   } */
/*   return  */

void exec(char** argv, int argc){
  printf("exec.....\n");
  int p_idx = pipe_idx(argv, argc);
  if(p_idx==-1){
    argv[argc] = NULL;
    exec_no_pipe(argv, argc);
  } else {
    printf("pipe is in %d\n",p_idx);
    char** argv_l = argv;
    char** argv_r = argv + p_idx + 1;
    int argc_l = p_idx;
    int argc_r = argc - 1 - p_idx;
    *(argv_l + argc_l) = NULL;
    *(argv_r + argc_r) = NULL;
    print_args(argv_l, argc_l);
    print_args(argv_r, argc_r);
    pid_t pid;
    int fd[2];
    if (pipe (fd) == -1){
      perror ("pipe");
      exit (1);
    }
    if((pid = fork())==0){
      printf("child process in exec\n");
      print_args(argv_r,argc_r);
      printf("child argv[0]: %s\n",argv_r[0]);
      dup2(fd[0], 0);//stdin
      close(fd[0]);
      close(fd[1]);
      exec(argv_r, argc_r);
    }else{
      printf("parent process in exec\n");
      dup2(fd[1], 1);//stdout
      close(fd[0]);
      close(fd[1]);
      exec_no_pipe(argv_l, argc_l);
    }
  }
}

void exec_no_pipe(char** argv, int argc){
  printf("exec no pipe\n");

  execvp(argv[0],argv);
}

/* search first pipe char '|' in argv
 *  | return index of char '|'
 *  | return -1 if not exist
 */
int pipe_idx(char** argv, int argc){
  printf("pipe_idx...\n");
  for (int i = 0; i < argc; i++) {
    if(strcmp(argv[i],"|")==0)
      return i;
  }
  return -1;
}

int split_cmd(char* cmd, char** argv){
  int argc = 0;
  char* token= strtok (cmd, " ");
  while (token != NULL){
    printf ("token: %s\n",token);
    argv[argc]= token;
    token = strtok (NULL, " ,.-");
    argc++;
  }
  return argc;
}

int main()
{
  pid_t cpid;
  int status;
  char* cmd = (char*)malloc(BUF_SIZE*sizeof(char));
  char** argv = (char**)malloc(BUF_SIZE*sizeof(char));
  int argc;


  for(;;){
    /* fork error */
    if((cpid = fork()) == -1)
      perror("fork");

    /* child process */
    else if(cpid == 0){
      printf("$ ");
      scanf("%[^\n]" , cmd);
      argc = split_cmd(cmd, argv);
      exec(argv,argc);
      exit(1);
    }

    /* parent process  */
    if(wait(&status) == (pid_t)-1){
      exit(1);
    }
  }
  return 0;
}

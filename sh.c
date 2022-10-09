#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <glob.h>
#include <math.h>
#include "sh.h"

int sh( int argc, char **argv, char **envp )
{
  extern char ** environ;
  char *command, *commandpath, *prompt, *p, *pwd, *owd;
  int pid, uid, status, argsct, go = 1;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;
  char *cwd;

  char *tmp; // dummy var to hold the freeable memory

  uid = getuid();
  password_entry = getpwuid(uid);               /* get passwd info */
  homedir = password_entry->pw_dir;		/* Home directory to start
						  out with*/
     
  if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
  {
    perror("getcwd");
    exit(2);
  }

  // signal handlers
  signal(SIGINT, sigHandler);
  signal(SIGTSTP, sigHandler);

  // initializing args
  char **args= malloc(( 1 + MAXARGS ) * sizeof(char*));
  for (int i = 0; i < MAXARGS + 1; i++) {
    args[i] = NULL;
  }

  // initialize prompt
  prompt = malloc(PROMPTMAX * sizeof(char));
  strcpy(prompt, "");

  owd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(owd, pwd, strlen(pwd));
  
  /* Put PATH into a linked list */
  pathlist = get_path();
  while ( go )
  {
    /* print your prompt */
    printf("%s [%s]> ", prompt ? prompt : "", owd);
    /* get command line and process */
    argsct = getargs(args);
    if (argsct == -1) {
      clearargs(args);
      free(prompt);
      free(args);
      free(pwd);
      free(owd);
      go = 0;
      printf("Exiting\n");
      exit(29);
    } else if (argsct == 0) {
      continue;
    }

    /* check for each built in command and implement */

    /* EXIT FUNCTION */
    if (strcmp(args[0], "exit") == 0) {
      printf("Exiting\n");
      free(pathlist->element);
      struct pathelement *tmppathelement;
      while (pathlist != NULL) {
        tmppathelement = pathlist;
        pathlist = pathlist->next;
        free(tmppathelement);
      }
      clearargs(args);
      free(prompt);
      free(args);
      free(pwd);
      free(owd);
      exit(1);
    }
    
    /* WHICH FUNCTION */
    else if (strcmp(args[0], "which") == 0) {
      printf("Executing 'which'\n");
      if (args[1] == NULL) {
        printf("Error: which requires at least 1 argument\n");
      } else {
        tmp = which(args[1], pathlist);
        if (tmp) {
          printf("Path found: %s\n", tmp);
          free(tmp);
        } else {
          printf("%s: not found\n", args[1]);
        }
      }
    }

    /* WHERE FUNCTION */
    else if (strcmp(args[0], "where") == 0) {
      printf("Executing 'where'\n");
      if (args[1] == NULL) {
        printf("Error: where requires at least 1 argument\n");
      } else {
        tmp = where(args[1], pathlist);
        if (tmp) {
          printf("Path found: %s\n", tmp);
          free(tmp);
        } else {
          printf("%s: not found\n", args[1]);
        }
      }
    }

    /* CD FUNCTION */
    else if (strcmp(args[0], "cd") == 0) {
      printf("Executing 'cd'\n");
      cd(owd, pwd, homedir, args, argsct);
    }

    /* PWD FUNCTION */
    else if (strcmp(args[0], "pwd") == 0) {
      printf("Executing 'pwd'\n");
      printwd();
    }

    /* LIST FUNCTION */
    else if (strcmp(args[0], "list") == 0) {
      printf("Executing 'list'\n");
      list(owd);
    }

    /* PID FUNCTION */
    else if (strcmp(args[0], "pid") == 0) {
        printf("Executing 'pid'\n");
      printf("PID: %d\n", getpid());
    }

    /* KILL FUNCTION */
    else if (strcmp(args[0], "kill") == 0) {
      printf("Executing 'kill'\n");
      int didkill = 1;
      
      if (argsct == 2) {
        didkill = kill(atoi(args[1]), SIGTERM);
      }
      else if (argsct == 3) {
        if (args[1][0] == '-') {
          int signal = atoi(args[1] + 1);
          didkill = kill(atoi(args[2]), signal);
        }
        else if (args[2][0] == '-') {
          int signal = atoi(args[2] + 1);
          didkill = kill(atoi(args[1]), signal);
        }
      }
      if (didkill) {
        perror("Error with kill");
      }
    }

    /* PROMPT FUNCTION */
    else if (strcmp(args[0], "prompt") == 0) {
      printf("Executing 'prompt'\n");
      setprompt(prompt, args, argsct);
    }

    /* PRINTENV FUNCTION*/
    else if (strcmp(args[0], "printenv") == 0) {
      printf("Executing 'printenv'\n");
      printenv(environ, args, argsct);
    }

    /* SETENV FUNCTION */
    else if (strcmp(args[0], "setenv") == 0) {
      printf("Executing 'setenv'\n");
      setenviron(environ, args, argsct);
    }

    /* TEST FUNCTION */
    else if (strcmp(args[0], "test") == 0) {
      printf("Executing 'test'\n");
      printf("testing\n");
    }

     /*  else  program to exec */
    else {
      /* find it */
      char *fnpath;

      // checking for relative paths
      if (args[0][0] == '/' || args[0][0] == '.') {
        if (!access(args[0], F_OK & X_OK)) {
          fnpath = malloc(strlen(args[0]) * sizeof(char));
          strcpy(fnpath, args[0]);
        } else {
          fnpath = NULL;
        }
      } else {
        fnpath = which(args[0], pathlist);
      }

      /* do fork(), execve() and waitpid() */
      if (fnpath) {
        printf("Executing '%s'\n", fnpath);
        free(args[0]);
        args[0] = fnpath;
        pid = fork();

        int status;
        
        if (pid == 0) {   // CHILD
          signal(SIGINT, cSigHandler);
          signal(SIGTSTP, cSigHandler);
          int ec = execve(args[0], args, environ);
          exit(ec);
        } else {          // PARENT
          waitpid(-1, &status, 0);
          if (WIFEXITED(status)) {
            printf("Process exited successfully: Exit code %d\n", WEXITSTATUS(status));
          } else {
            fprintf(stderr, "Process exited abnormally\n");
          }
        }
      } else {
        fprintf(stderr, "%s: Command not found.\n", args[0]);
      }
    }
  }
  return 0;
} /* sh() */

/**
 * @brief loops through all directories contained in PATH environment variable and returns first
 * instance of the desired executable, returns NULL if not founD
 * ALLOCATES RETURN STRING
 * 
 * @param command command to be searched
 * @param pathlist head of the linked list 
 * @return char* path of the first executable found
 */
char *which(char *command, struct pathelement *pathlist )
{
  struct pathelement *tmp;
  tmp = pathlist;
  char buffer[128];
  char *tmppath;

  while (tmp != NULL) {
    // builds buffer
    strcpy(buffer, tmp->element);
    strcat(buffer, "/");
    strcat(buffer, command);

    // if path found, return
    if (!access(buffer, F_OK & X_OK)) {
      tmppath = malloc( (1 + strlen(buffer)) * sizeof(char));
      strcpy(tmppath, buffer);
      return tmppath;
    }
    tmp = tmp->next;
  }
  return NULL;
} /* which() */

/**
 * @brief loops through all directories contained in PATH environment variable and finds all 
 * instances of a given command, returns NULL if not found
 * ALLOCATES RETURN STRING
 * 
 * @param command command to be searched
 * @param pathlist head of the linked list of PATH values
 * @return char* path of all executables found joined by :
 */
char *where(char *command, struct pathelement *pathlist )
{
  struct pathelement *tmp;
  tmp = pathlist;
  char buffer[128];
  char bigbuffer[256] = "\0";
  char *path;

  while (tmp != NULL) {
    // builds buffer
    strcpy(buffer, tmp->element);
    strcat(buffer, "/");
    strcat(buffer, command);

    // if path found, concatenate
    if (!access(buffer, F_OK & X_OK)) {
      strcat(bigbuffer, buffer);
      strcat(bigbuffer, ":");
    }
    tmp = tmp->next;
  }

  // if any instances of target func found, malloc and return
  if (bigbuffer[0]) {
    path = malloc( (1 + strlen(bigbuffer)) * sizeof(char));
    strcpy(path, bigbuffer);
    return path;
  }

  // else return
  return NULL;
} /* where() */

/**
 * @brief utilized by the list command, prints out all file and directory names in a given directory
 * structure given by dgookin on c-for-dummies.com
 * 
 * @param dir directory to be indexed
 */
void list ( char *dir ) 
{
  // handle for directory to be searched
  DIR *folder;
  struct dirent *entry;
  int files = 0;

  folder = opendir(dir);
  if(folder == NULL)
  {
    perror("Unable to read directory");
    return;
  }

  readdir(folder);
  readdir(folder);
  while(( entry = readdir(folder) )) {
    files++;
    printf("%s\n", entry->d_name);
  }

  closedir(folder);
} /* list() */


/**
 * @brief either polls for user input and overwrites prompt or copies input from args
 * 
 * @param p pointer to prompt
 * @param args args containing new prompt value or NULL
 */
void setprompt(char *p, char **args, int argsct) {
  if (!args[1]) {
    printf("Input prompt prefix: ");

    // read input into a buffer
    char buffer[128];
    fgets(buffer, 127, stdin);
    int len = strlen(buffer);
    if (strlen(buffer) >= PROMPTMAX) {
      buffer[PROMPTMAX - 1] = '\0';
    } else {
      buffer[len - 1] = '\0';
    }
    strcpy(p, buffer);
  } else {
    char buffer[128] = "\0";
    for (int i = 1; i < argsct; i++) {
      strcat(buffer, args[i]);
      strcat(buffer, " ");
    }
    strncpy(p, buffer, PROMPTMAX - 1);
    p[PROMPTMAX - 1] = '\0';
  }
} /* setprompt() */


/**
 * @brief accepts either 0 or 1 arguments and switches the current working directory based on 
 * the input. no input means home dir, - means previous directory, and anything else will try to find a
 * directory with that name and switch into it
 * 
 * @param owd open working directory
 * @param pwd previous working directory
 * @param homedir home directory
 * @param args command line args
 * @param argsct number of individual args
 */
void cd(char *owd, char *pwd, char *homedir, char **args, int argsct) {  
  char owdbuf[256];
  if (argsct == 2) {
    // go to previous directory
    if (strcmp(args[1], "-") == 0) {
      chdir(pwd);
      strcpy(pwd, owd);
      getcwd(owdbuf, sizeof(owdbuf));
      strcpy(owd, owdbuf);
    }

    // check if args[1] is a dir and go 
    else {
      struct stat statbuf;
      int direxists = stat(args[1], &statbuf);
      if ( direxists != -1 && S_ISDIR( statbuf.st_mode ) ) {
        strcpy(pwd, owd);
        chdir(args[1]);
        getcwd(owdbuf, sizeof(owdbuf));
        strcpy(owd, owdbuf);
      } else {
        printf("%s is not a directory\n", args[1]);
      }
    }    
  }
  
  // go to home directory
  else if (argsct == 1) {
    strcpy(pwd, owd);
    chdir(homedir);
    getcwd(owdbuf, sizeof(owdbuf));
    strcpy(owd, owdbuf);
  }

  // too many args
  else {
    printf("Too many arguments entered\n");
  }
} /* cd() */

/**
 * @brief prints current working directory
 */
void printwd() {
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("Current working dir: %s\n", cwd);
  } else {
    perror("getcwd() error");
  }
} /* printwd() */

/**
 * @brief prints off all environment variables onto their own line
 * 
 * @param environ pointer to the list of environment vars
 * @param args list of arguments
 * @param argsct number of arguments
 */
void printenv(char **environ, char **args, int argsct) {
  if (argsct == 1) {
    int i = 0;
    while (environ[i] != NULL) {
      printf("%s\n", environ[i]);
      i++;
    }
  } else if (argsct == 2) {
    char *env;
    env = getenv(args[1]);
    printf("%s\n", env);
  } else {
    fprintf(stderr, "Too many arguments for printenv\n");
  }
} /* printenv() */

/**
 * @brief sets the system environment variables based on input
 * 
 * @param environ pointer to environment variables
 * @param args list of arguments
 * @param argsct number of arguments
 */
void setenviron(char **environ, char **args, int argsct) {
  if (argsct == 1) {
    printenv(environ, args, argsct);
  } else if (argsct == 2) {
    setenv(args[1], "", 1);
  } else if (argsct == 3) {
    setenv(args[1], args[2], 1);
  } else {
    fprintf(stderr, "setenv can only take 2 arguments\n");
  }
} /* setenv() */
 
/**
 * @brief clears and replaces args with input from command line
 * 
 * @param args holds the address of args to be revised
 */
int getargs(char **args) {
  // clear args
  clearargs(args);
  
  // grab input
  char buffer[128];
  char *checkeof;
  fgets(buffer, 127, stdin);
  if (checkeof == NULL) {
    printf("\nEOF Detected\n");
    fflush(stdout);
    return -1;
  }
  buffer[strlen(buffer) - 1] = '\0';
  
  // chop strings and copy to args
  char *token;
  token = strtok(buffer, " ");
  int i = 0;
  while (token != NULL && i < MAXARGS) {
    args[i] = malloc( (1 + strlen(token) ) * sizeof(char));
    strcpy(args[i], token);
    token = strtok(NULL, " ");
    i++;
  }
  return i;
} /* getargs() */

/**
 * @brief frees all elements in args list and sets them to NULL
 * useful for freeing args and ensuring that old args don't stick around and cause problems on repeated 
 * uses of getargs
 * 
 * @param args pointer to list of args to be modified
 */
void clearargs(char **args) {
  for (int i = 0; i < MAXARGS; i++) {
    if (args[i] != NULL) {
      free( *(args+i) );
      args[i] = NULL;
    }
  }
}

/**
 * @brief simple function to print args, used for testing
 * 
 * @param args pointer to list of args to be printed
 */
void printargs(char **args) {
  for (int i = 0; i < MAXARGS; i++) {
    printf("args[%d]: %s\n", i, args[i]);
  }
}

void sigHandler(int sig) {
  switch (sig)
  {
  case SIGINT:
    signal(SIGINT, sigHandler);
    printf("\n Cannot be terminated using Ctrl+C\n");
    fflush(stdout);
    break;
  
  case SIGTSTP:
    signal(SIGTSTP, sigHandler);
    printf("\n Cannot be terminated using Ctrl+Z\n");
    fflush(stdout);
    break;
  }
}

void cSigHandler(int sig) {
  switch (sig)
  {
  case SIGINT:
    exit(128 + SIGINT);
    break;
  
  case SIGTSTP:
    exit(128 + SIGTSTP);
    break;
  }
}
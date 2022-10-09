#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "sh.h"

int sh( int argc, char **argv, char **envp )
{
  extern char ** environ;
  char *command, *commandpath, *p, *pwd, *owd;
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

  // initializing args
  char **args= malloc(( 1 + MAXARGS ) * sizeof(char*));
  for (int i = 0; i < MAXARGS + 1; i++) {
    args[i] = NULL;
  }

  owd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(owd, pwd, strlen(pwd));
  
  /* Put PATH into a linked list */
  pathlist = get_path();
  while ( go )
  {
    /* print your prompt */
    printf("%s: ", pwd);
    /* get command line and process */
    getargs(args);

    /* check for each built in command and implement */

    /* EXIT FUNCTION */
    if (strcmp(args[0], "exit") == 0) {
      free(pathlist->element);
      struct pathelement *tmppathelement;
      while (pathlist != NULL) {
        tmppathelement = pathlist;
        pathlist = pathlist->next;
        free(tmppathelement);
      }
      clearargs(args);
      free(args);
      free(pwd);
      free(owd);
      exit(1);
    }

    /* LIST FUNCTION */
    else if (strcmp(args[0], "list") == 0) {
      list(pwd);
    }
    
    /* WHICH FUNCTION */
    else if (strcmp(args[0], "which") == 0) {
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

    /* TEST FUNCTION */
    else if (strcmp(args[0], "test") == 0) {
      printf("testing\n");
    }

     /*  else  program to exec */
    else {
      /* find it */
      char *fnpath = which(args[0], pathlist);

      /* do fork(), execve() and waitpid() */
      if (fnpath) {
        free(args[0]);
        args[0] = fnpath;
        pid = fork();
        
        if (pid == 0) {   // CHILD
          execve(args[0], args, environ);
        } else {          // PARENT
          wait(NULL);
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
 * instance of the desired executable. returns NULL if not found
 * 
 * CURRENTLY ALLOCATES MEMORY
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
    strcpy(buffer, tmp->element);
    strcat(buffer, "/");
    strcat(buffer, command);
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
 * instances of a given command. returns Null if not found
 * 
 * @param command command to be searched
 * @param pathlist head of the linked list 
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
    strcpy(buffer, tmp->element);
    strcat(buffer, "/");
    strcat(buffer, command);
    if (!access(buffer, F_OK & X_OK)) {
      strcat(bigbuffer, buffer);
      strcat(bigbuffer, ":");
    }
    tmp = tmp->next;
  }
  if (bigbuffer[0]) {
    path = malloc( (1 + strlen(bigbuffer)) * sizeof(char));
    strcpy(path, bigbuffer);
    return path;
  }
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
 * @brief clears and replaces args with input from command line
 * 
 * @param args holds the address of args to be revised
 */
void getargs(char **args) {
  // clear args
  clearargs(args);
  
  // grab input
  char buffer[128];
  fgets(buffer, 127, stdin);
  buffer[strlen(buffer) - 1] = '\0';
  
  // chop strings and copy to args
  char *token;
  token = strtok(buffer, " ");
  for (int i = 0; token != NULL && i < MAXARGS; i++) {
    args[i] = malloc( (1 + strlen(token) ) * sizeof(char));
    strcpy(args[i], token);
    token = strtok(NULL, " ");
  }
}

void clearargs(char **args) {
  for (int i = 0; i < MAXARGS; i++) {
    if (args[i] != NULL) {
      free( *(args+i) );
      args[i] = NULL;
    }
  }
}

void printargs(char **args) {
  for (int i = 0; i < MAXARGS; i++) {
    printf("args[%d]: %s\n", i, args[i]);
  }
}
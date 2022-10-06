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
  char *command, *arg, *commandpath, *p, *pwd, *owd;
  int uid, i, status, argsct, go = 1;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;
  int pid;
  char **arglist;
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

  owd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(owd, pwd, strlen(pwd));
  
  /* Put PATH into a linked list */
  pathlist = get_path();
  while ( go )
  {
    /* print your prompt */
    printf("%s: ", pwd);
    /* get command line and process */
    arglist = getcmd();
    
    /* check for each built in command and implement */

    /* EXIT FUNCTION */
    if (strcmp(arglist[0], "exit") == 0) {
      free(pathlist->element);
      struct pathelement *tmppathelement;
      while (pathlist != NULL) {
        tmppathelement = pathlist;
        pathlist = pathlist->next;
        free(tmppathelement);
      }
      free(*arglist);
      free(arglist);
      free(pwd);
      free(owd);
      exit(1);
    }

    /* LIST FUNCTION */
    else if (strcmp(arglist[0], "list") == 0) {
      list(pwd);
    }
    
    /* WHICH FUNCTION */
    else if (strcmp(arglist[0], "which") == 0) {
      tmp = which(arglist[1], pathlist);
      free(tmp);
    }

    /* TEST FUNCTION */
    else if (strcmp(arglist[0], "test") == 0) {
      printf("Testing access funciton\n");
      tmp = malloc(strlen(pwd) * 2 * sizeof(char));
      strcpy(tmp, pwd);
      printf("%s\n", tmp);
      strcat(tmp, "/hello.sh");
      printf("%s\n", tmp);
      printf("access result: %d\n", access(tmp, F_OK & X_OK));
      free(tmp);
    }
     /*  else  program to exec */
    else {
      /* find it */
      char *freeMe;
      freeMe = arglist[0];

      char *path = where(arglist[0], pathlist);
      arglist[0] = path;

      /* do fork(), execve() and waitpid() */
      if (path != NULL) {
        if (fork() == 0) {
          execve(path, arglist, environ);
          free(freeMe);
          exit(1);
        }
        else {
          waitpid(0, 0, 0);
        }
      }
      else {
        free(freeMe);
      }

      /* else */

      /* fprintf(stderr, "%s: Command not found.\n", args[0]); */
    }
    free(arglist);
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
  char *tmppath;
  tmppath = malloc(50 * sizeof(char));

  while (tmp != NULL) {
    strcpy(tmppath, tmp->element);
    strcat(tmppath, "/");
    strcat(tmppath, command);
    if (!access(tmppath, F_OK & X_OK)) {
      printf("Found something\n");
      printf("%s\n", tmppath);
      return tmppath;
    }
    tmp = tmp->next;
  }
  free(tmppath);
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
  /* similarly loop through finding all locations of command */
  struct pathelement *tmp;
  tmp = pathlist;
  char *tmppath;

  while (tmp != NULL) {

    strcpy(tmppath, tmp->element);
    strcat(tmppath, command);
    if (!access(tmppath, F_OK & X_OK)) {
      return tmppath;
    }
    tmp = tmp->next;
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
  while( (entry=readdir(folder)) ){
    files++;
    printf("%s\n", entry->d_name);
  }

  closedir(folder);
} /* list() */

/**
 * @brief polls for user input and seperates into array of strings. allocates both a single input string
 *     and the list of pointers pointing to that string. must be freed using
 * 
 * @return char** a array of string pointers. cmdargs[0] holds the function to be called and cmdargs[1] and on holds the arguments
 */
char **getcmd() {
  char buffer[128];
  fgets(buffer, 127, stdin);
  if (strcmp(buffer, "") == 0) {
    return NULL;
  }
  int len = strlen(buffer);
  char *trimmed = malloc(sizeof(char) * len);
  strncpy(trimmed, buffer, len);
  trimmed[len-1] = '\0';
  
  char *token;
  token = strtok(trimmed, " ");
  
  char *ptrbuffer[MAXARGS];
  int i = 0;
  while (token != NULL) {
    ptrbuffer[i] = token;
    token = strtok(NULL, " ");
    i++;
  }
  
  char **cmdargs;
  cmdargs = malloc(i * sizeof(char*));
  for (int index = 0; index < i; index++) {
    cmdargs[index] = ptrbuffer[index];
  }

  return cmdargs;
}


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
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *owd;
  char **args = calloc(MAXARGS, sizeof(char*));
  int uid, i, status, argsct, go = 1;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;
  int pid;
  char **arglist;
  char *cwd;

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
  prompt[0] = ' '; prompt[1] = '\0';

  
  /* Put PATH into a linked list */
  pathlist = get_path();
  while ( go )
  {
    /* print your prompt */
    printf("%s: ", pwd);
    /* get command line and process */
    arglist = getcmd();
    
    /* check for each built in command and implement */
    if (strcmp(arglist[0], "exit") == 0) {
      free(arglist[0]);
      free(arglist);
      exit(1);
    }
    else if (strcmp(arglist[0], "list") == 0) {
      list(pwd);
    }
     /*  else  program to exec */
    else {
      /* find it */
      
      /* do fork(), execve() and waitpid() */
      if (fork() == 0) {
        execve(arglist[0], arglist+1, environ);
        exit(1);
      }
      else {
        waitpid(0, 0, 0);
      }

      /* else */

      /* fprintf(stderr, "%s: Command not found.\n", args[0]); */
    }
    free(arglist[0]);
    free(arglist);
  }
  return 0;
} /* sh() */

char *which(char *command, struct pathelement *pathlist )
{
   /* loop through pathlist until finding command and return it.  Return
   NULL when not found. */
  struct pathelement *tmp;
  tmp = pathlist;
  while (tmp != NULL) {

    tmp = tmp->next;
  }
  return NULL;
} /* which() */

char *where(char *command, struct pathelement *pathlist )
{
  /* similarly loop through finding all locations of command */
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
    return;
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


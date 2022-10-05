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
  char *input;
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
    input = getString();
    
    /* check for each built in command and implement */
    if (strcmp(input, "exit") == 0) {
      free(input);
      exit(1);
    }
    else if (strcmp(input, "ls") == 0) {
      list(pwd);
    }
     /*  else  program to exec */
    else {
      /* find it */
      
      /* do fork(), execve() and waitpid() */
      if (fork() == 0) { 
        execve(input, args, environ);
        exit(1);
      }
      else {

      }

      /* else */

      /* fprintf(stderr, "%s: Command not found.\n", args[0]); */
    }
    free(input);
  }
  return 0;
} /* sh() */

char *which(char *command, struct pathelement *pathlist )
{
   /* loop through pathlist until finding command and return it.  Return
   NULL when not found. */

} /* which() */

char *where(char *command, struct pathelement *pathlist )
{
  /* similarly loop through finding all locations of command */
} /* where() */

/**
 * @brief utilized by the ls command, prints out all file and directory names in a given directory
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
    printf("%-16s", entry->d_name);
    if ((files % 6) == 0) {
      printf("\n");
    }
  }
  if ((files % 6) != 0) {
    printf("\n");
  }

  closedir(folder);
} /* list() */

int callfn(char *string) {

  return 0;
}

char* getString() {
    char buffer[128];
    fgets(buffer, 127, stdin);
    int len = strlen(buffer);
    char *trimmed = malloc(sizeof(char) * len);
    strncpy(trimmed, buffer, len);
    trimmed[len-1] = '\0';
    return trimmed;
}


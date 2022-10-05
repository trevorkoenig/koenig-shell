
#include "get_path.h"

extern int pid;
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list ( char *dir );
void printenv(char **envp);
char **getcmd();

#define PROMPTMAX 32
#define MAXARGS 10

struct argnode
{
  char *element;			/* a dir in the path */
  struct argnode *next;		/* pointer to next node */
};

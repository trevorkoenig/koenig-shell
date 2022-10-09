
#include "get_path.h"

extern int pid;
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list ( char *dir );
void setprompt(char *p, char **args);
void printenv(char **envp);
void getargs(char **args);
void clearargs(char **args);
void printargs(char **args);

#define PROMPTMAX 32
#define MAXARGS 10
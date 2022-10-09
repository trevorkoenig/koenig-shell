
#include "get_path.h"

extern int pid;
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list ( char *dir );
void setprompt(char *p, char **args, int argsct);
void cd(char *owd, char *pwd, char *homedir, char **args, int argsct);
int printwd();
void printenv(char **environ, char **args, int argsct);
int getargs(char **args);
void clearargs(char **args);
void printargs(char **args);

#define PROMPTMAX 32
#define MAXARGS 10
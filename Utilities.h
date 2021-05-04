#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

void dieWithError(char *error); 

#define MAX_INPUT_SIZE 256

int available_variable_space;
int num_tokens;

typedef void (*sighandler_t)(int);

typedef struct variable{

    char *name;
    char *value;

} variable;

typedef enum tokentype{ HASH, BANG, EQUALS, CD, LV, QUIT, UNSET, INFROM, OUTTO, SUBSTITUTE, VARNAME, GENERAL } tokentype;

typedef struct token{

    tokentype type;
    char value[MAX_INPUT_SIZE];

} token;

token *inputScanner(char *input);
int inputParser(token scanned_input[]);
void execute( char *path, char *command, char *args[]);
sighandler_t setSignalHandler(int signum, sighandler_t handler);
int insertVariable(variable *dictionary, char *name, char *value);
char* searchVariable(variable *dictionary, char *name);
int unsetVariable( variable *dictionary, char *name);
void printVariables(variable *dictionary, char* PATH, char* CWD, char* PS);
char* cd(char *path);
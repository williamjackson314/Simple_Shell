#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>


void dieWithError(char *error); 

#define MAX_INPUT_SIZE 256

int available_variable_space = 10;
int num_tokens;

typedef struct {

    char *name;
    char *value;

} variable;

typedef enum { HASH, BANG, EQUALS, CD, LV, QUIT, UNSET, INFROM, OUTTO, SUBSTITUTE, VARNAME, GENERAL } tokentype;

typedef struct {

    tokentype type;
    char value[MAX_INPUT_SIZE];

} token;

token *inputScanner(char *input);
int inputParser(token scanned_input[]);
void execute( char *path, char *command, char *args[]);
int insertVariable(variable *dictionary, char *name, char *value);
char* searchVariable(variable *dictionary, char *name);
int unsetVariable( variable *dictionary, char *name);
void printVariables(variable *dictionary, char* PATH, char* CWD, char* PS);
char* cd(char *path);
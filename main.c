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
#define TOKEN_STRUCT_SIZE 260

int available_variable_space = 10;

typedef struct variable{

    char *name;
    char *value;

} variable;

typedef enum tokentype{ HASH, BANG, EQUALS, CD, LV, QUIT, UNSET, INFROM, OUTTO, VARNAME, GENERAL } tokentype;

typedef struct token{

    tokentype type;
    char value[MAX_INPUT_SIZE];

} token;

token *inputScanner(char *input){

    char input_cpy[MAX_INPUT_SIZE];
    strncpy(input_cpy, input, strlen(input)+1);
    
    static token tokenized_input[TOKEN_STRUCT_SIZE * MAX_INPUT_SIZE];

    tokentype type;
    tokentype prev_type;

    int cnt = 0;  

    const char *delims = " \t";
    char *token;
    char *prev_token = '\0';

    token = strtok(input_cpy, delims);

    while (token != NULL){

        if (strcmp(token, "#") == 0){
            type = HASH;
        }
        if (strcmp(token, "!") == 0){
            type = BANG;
        }        
        else if (strcmp(token, "=") == 0){
            type = EQUALS;
            prev_type = VARNAME;

            tokenized_input[cnt-1].type = prev_type; 
            strcpy(tokenized_input[cnt-1].value, prev_token);

        }
        else if (strcmp(token, "cd") == 0){
            type = CD;
        }
        else if (strcmp(token, "lv") == 0){
            type = LV;
        }
        else if (strcmp(token, "quit") == 0){
            type = QUIT;
        }
        else if (strcmp(token, "unset") == 0){
            type = UNSET;
        }
        else if (strcmp(token, "infrom:") == 0){
            type = INFROM;
        }
        else if (strcmp(token, "outto:") == 0){
            type = OUTTO;
        }
        else {
            prev_token = token;
            type = GENERAL;
        }

        tokenized_input[cnt].type = type;
        strcpy(tokenized_input[cnt].value, token);

        cnt += 1;
        
        token = strtok(NULL, delims);
    } 
 
    return tokenized_input;
}

void inputParser(token scanned_input){

}

/**
 * @brief For use after parent has forked, 
 *        Changes working directory to path and runs 
 *        command passes args/envp
 * 
 * @param path 
 * @param command 
 * @param args 
 * @param envp  
 */
void execute( char *path, char *command, char *args[], char envp*[]) {
    cd(path);
    int status = execve(command, args, envp);
    if (status == -1) dieWithError("Error: Could not execute program\n");
}

/**
 * @brief Wrapper for establising a handler;
 *        Function from lecture slides (3/8)
 * 
 * @param signum 
 * @param handler 
 * @return sighandler_t 
 */
sigset_t mask, prev;
typedef void (*sighandler_t)(int);
sighandler_t setSignalHandler(int signum, sighandler_t handler){
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigfillset(&action.sa_mask);
    action.sa_flags = 0;
    if(sigaction(signum, &action, &old_action) < 0)
        dieWithError("Signal error");
    return old_action.sa_handler;
}

/**
 * @brief If name already exists in dictionary, value is updated; otherwise, inserts new variable into dictionary
 * 
 * @param dictionary 
 * @param name 
 * @param value 
 * @return int Returns 0 on successful update/insert
 */
int insertVariable(variable *dictionary, char *name, char *value){
    // if name already exists, value is updated
    for (int i = 0; i < available_variable_space; i++){
        if (dictionary[i].name == name){
            dictionary[i].value = value;
            return 0;
        }
    }
    // insert new name/variable into dictionary
    for (int i = 0; i < available_variable_space; i++){
        if (dictionary[i].name == NULL){
            dictionary[i].name = name;
            dictionary[i].value = value;
            return 0;
        }
    }
    
    // use realloc to allocate more space if dictionary is full 
    available_variable_space += 1;
    dictionary = (variable*)realloc(dictionary, available_variable_space * sizeof(variable));
    dictionary[available_variable_space - 1].name = name;
    dictionary[available_variable_space - 1].value = value;
    return 0;
}

/**
 * @brief Searches for variable in dictionary 
 * 
 * @param dictionary 
 * @param name 
 * @return Returns the value of the stored variable name from dictionary, 
 *         returns NULL if not found
 */
char* searchVariable(variable *dictionary, char *name){
    for (int i; i < available_variable_space; i++){
        if (dictionary[i].name == name)
            return dictionary[i].value;
    }
    return NULL;
}

/**
 * @brief Sets dictionary variable to NULL
 * 
 * @param dictionary 
 * @param name 
 * @return returns 0 if successful, 
 *         returns -1 if name isn't in dictionary
 */
int unsetVariable( variable *dictionary, char *name){
    for (int i = 0; i < available_variable_space; i++){
        if (dictionary[i].name == name){
            dictionary[i].name = NULL;
            dictionary[i].value = NULL;
            return 0;
        }
    }
    return -1;
}

/**
 * @brief Prints predefined variables and dictionary variables 
 * 
 * @param dictionary 
 * @param PATH 
 * @param CWD 
 * @param PS 
 */
void printVariables(variable *dictionary, char* PATH, char* CWD, char* PS){
    // prints predefined variables 
    printf("Predefined Variables\n");
    printf("--------------------\n");
    printf("PATH: %s\n", PATH);
    printf("CWD: %s\n", CWD);
    printf("PS: %s\n", PS);

    // prints user defined variables 
    printf("User Defined Variables\n");
    printf("----------------------\n");
    for(int i = 0; i < available_variable_space; i++){
        if(dictionary[i].name != NULL) 
            printf("%s: %s\n", dictionary[i].name, dictionary[i].value);
    }
}

/**
 * @brief cd command changes current working directory (cwd) to path
 * 
 * @param path 
 * @return char * Updated CWD 
 */
char* cd(char *path){
    int status = chdir(path);
    if (status == -1) 
        dieWithError("Error: could not change directory\n");
    return getcwd(NULL, PATH_MAX);
}

void quit(){

    exit(0);
}

int main(){

    setSignalHandler(SIGINT, SIG_IGN); // catches ^C 
    DIR *directory;
    struct dirent *de;
    char *PATH = "/usr/bin"; // in testing, /bin:/usr/bin did not exist; using path "/usr/bin" executed programs properly
    char *CWD;
    char *PS = "> ";
    char usr_input[MAX_INPUT_SIZE];
    token *input_tokens;
    input_tokens = (token *)malloc(MAX_INPUT_SIZE * sizeof(token));

    if (getcwd(CWD, PATH_MAX) == NULL) dieWithError("getcwd error");

    variable *dictionary;
    dictionary = (variable *)calloc(available_variable_space, sizeof(variable));

    for(;;){
        fgets(usr_input, MAX_INPUT_SIZE, stdin);
        input_tokens = inputScanner(usr_input);

    }

    free(dictionary);
    return 0;
}
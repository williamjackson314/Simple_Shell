#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "Utilities.h"
#include "ScannerAndParser.c"

available_variable_space = 10;

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

int main(){

    setSignalHandler(SIGINT, SIG_IGN); // catches ^C 
    DIR *directory;
    struct dirent *de;
    char *PATH = "/usr/bin"; // in testing, /bin:/usr/bin did not exist; using path "/usr/bin" executed programs properly
    char *CWD = '\0';
    char *PS = "> ";
    char usr_input[MAX_INPUT_SIZE];
    int is_valid;
    token *input_tokens;
    input_tokens = (token *)malloc(MAX_INPUT_SIZE * sizeof(token));

    if (getcwd(CWD, PATH_MAX) == NULL) dieWithError("getcwd error");

    variable *dictionary;
    dictionary = (variable *)calloc(available_variable_space, sizeof(variable));

    for(;;){
        
        if (feof(stdin)){
	        printf("\n");
	        exit(0);
        }

        printf("%s", PS);
        fgets(usr_input, MAX_INPUT_SIZE, stdin);
        input_tokens = inputScanner(usr_input);
        
        if (num_tokens > 0){
            // is_valid = inputParser(input_tokens);
            // if (is_valid){
            //      //execute command
            // } else {
            //     exit(0);
            // }
        }
        else {
            printf("%s", PS);
        }
    }
    
    free(dictionary);

    return 0;
}
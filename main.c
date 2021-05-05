#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "ScannerAndParser.c"

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
            is_valid = inputParser(input_tokens);
            if (is_valid){
                 //execute command
            } 
        }
        else {
            printf("%s", PS);
        }
    }
    
    free(dictionary);

    return 0;
}
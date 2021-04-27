#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

void dieWithError(char *error); 

const size_t MAX_INPUT_SIZE = 256; //bytes

const int MAX_NUM_VARIABLES = 10;

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

    token *tokenized_input;
    tokenized_input = (token *)malloc(MAX_INPUT_SIZE * sizeof(token));

    tokentype type;
    int cnt = 0;  
    const char *delims = " \t";
    char *token;
    char *prev_token = '\0';
    tokentype prev_type;

    do {

        token = strtok(input, delims);

        /*TODO: add part for varname  */
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
            tokenized_input[cnt-1].value = prev_token;
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
            prev_token = *token; //may need to use memcpy here instead, be careful to make sure the token before previous is erased
            type = GENERAL;
        }

        tokenized_input[cnt].type = type;
        tokenized_input[cnt].value = token;

        cnt += 1;

    } while (token != NULL);

    free(tokenized_input);

    return ;
}

void inputParser(token scanned_input){

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

int insertVariable(variable *dictionary, char *name, char *value){
    // if name already exists, value is updated
    for (int i = 0; i < MAX_NUM_VARIABLES; i++){
        if (dictionary[i].name == name){
            dictionary[i].value = value;
            return 0;
        }
    }
    // insert new name/variable into dictionary
    for (int i = 0; i < MAX_NUM_VARIABLES; i++){
        if (dictionary[i].name == NULL){
            dictionary[i].name = name;
            dictionary[i].value = value;
            return 0;
        }
    }
    else // not enough space in dictionary
        return -1;
}

char* searchVariable(variable *dictionary, char *name){
    for (int i; i < MAX_NUM_VARIABLES; i++){
        if (dictionary[i].name == name)
            return dictionary[i].value;
    }
    return NULL;
}

int unsetVariable( variable *dictionary, char *name){
    for (int i = 0; i < MAX_NUM_VARIABLES; i++){
        if (dictionary[i].name == name){
            dictionary[i].name = NULL;
        }
    }
}

void printVariables(variable *dictionary, char* PATH, char* CWD, char* PS){
    // prints predefined variables 
    printf("PATH: %s\n", PATH);
    printf("CWD: %s\n", CWD);
    printf("PS: %s\n", PS);

    // prints user defined variables 
    printf("User Defined Variables\n");
    for(int i = 0; i < MAX_NUM_VARIABLES; i++){
        if(dictionary[i].name != NULL) 
            printf("%s: %s\n", dictionary[i].name, dictionary[i].value);
    }
}

/**
 * @brief cd command changes current working directory (cwd) to path
 * 
 * @param path 
 * @return char* updated cwd 
 */
char* cd(char *path){
    chdir(path);
    return getcwd(NULL, PATH_MAX);
}

int main(){
    
    DIR *directory;
    struct dirent *de;
    char *PATH = "/bin:/usr/bin";
    char *CWD = getcwd(NULL, PATH_MAX);
    char *PS = "> ";
    char usr_input[MAX_INPUT_SIZE];

    if (getwd(CWD) == NULL) dieWithError(errno);

    variable *dictionary;
    dictionary = (variable *)calloc(MAX_NUM_VARIABLES, sizeof(variable));

    for(;;){
        fgets(usr_input, MAX_INPUT_SIZE, stdin);


    }

    free(dictionary);
    return 0;
}
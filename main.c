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

typedef struct variable{

    char *name;
    char *value;

} variable;

typedef enum tokentype{ HASH, BANG, EQUALS, CD, LV, QUIT, UNSET, INFROM, OUTTO, SUBSTITUTE, VARNAME, GENERAL } tokentype;

typedef struct token{

    tokentype type;
    char value[MAX_INPUT_SIZE];

} token;

/**
 * @brief Takes users input, converts it to an array of token structs,
 *          with each token signifying its type and value
 * 
 * @param input
 * @return token *, returns the tokenized array
 */
token *inputScanner(char *input){

    char input_cpy[MAX_INPUT_SIZE];
    strncpy(input_cpy, input, strlen(input)+1); //strlen + 1 to encompass the null character strlen discards
    
    static token tokenized_input[MAX_INPUT_SIZE];
    memset(tokenized_input, 0, sizeof tokenized_input); //reset values

    tokentype type;
    tokentype prev_type;

    int index = 0;  
    num_tokens = 0;

    const char *delims;
    char *saveptr;
    char *token;
    char *prev_token = '\0';

    // if first character is space or tab, exit shell
    if ((input_cpy[0] == ' ') || (input_cpy[0] == '\t')) dieWithError("Space/Tab may not be first character");

    if (input_cpy[0] == '\"'){
        delims = "\"\n\r";
    }
    else {
        delims = " \t\n\r";
    }

    token = strtok_r(input_cpy, delims, &saveptr);

    while (token != NULL){

        if (strcmp(token, "#") == 0){
            type = HASH;
        }
        else if (strcmp(token, "!") == 0){
            type = BANG;
        }        
        else if (strcmp(token, "=") == 0){
            type = EQUALS;
            prev_type = VARNAME;
            
            if (index > 0){
                tokenized_input[index-1].type = prev_type; 
                strcpy(tokenized_input[index-1].value, prev_token);
            }
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
            if (token[0] == '$'){
                type = SUBSTITUTE;
            }
            else{
                prev_token = token;
                type = GENERAL;
            }
        }

        tokenized_input[index].type = type;
        strcpy(tokenized_input[index].value, token);

        index += 1;

        /* If start of next strtok_r call is a space/tab, then we check the character right after it
            to see if it is a double quote. If so, we set that as the point where the next strtok_r call
            will start. This is so multiple double quotes enclosed strings will both be tokenized correctly */
        if (*saveptr == ' '){
            if (*(saveptr+1) == '\"'){
                delims = "\"\n\r";
                saveptr++;
            }
            else {
                delims = " \t\n\r";
            }
        }
        else if(*saveptr == '\t'){
            if (*(saveptr+1) == '\"'){
                delims = "\"\n\r";
                saveptr++;
            }
            else {
                delims = " \t\n\r";
            }
        }
        else {
            if (*saveptr == '\"'){
                delims = "\"\n\r";
            }
            else {
                delims = " \t\n\r";
            }        
        }


        token = strtok_r(NULL, delims, &saveptr);
        
    } 
 
    for (int i=0;i<index;i++){
        printf("Type: %d\n", tokenized_input[i].type);
        printf("Value: %s\n", tokenized_input[i].value);
    }

    num_tokens += index; //index is incremented everytime a token is created

    return tokenized_input;
}

/**
 * @brief For use after the user input has been split
 *          into tokens, checks if they syntax of input
 *          is valid.
 * 
 * @param scanned_input
 * @return int, 0 for invalid, anything else for valid 
 */
int inputParser(token scanned_input[]){


    if (scanned_input[0].type == GENERAL){
        printf("Invalid syntax\n");
        return 0;
    }
    for (int i=0;i<num_tokens;i++){
        switch (scanned_input[i].type){
            case HASH :
                if (i == 0) { //if first token is # then the syntax of the other tokens doesn't matter
                    return 1;
                }
                else { // '#' token anywhere other than first position is syntax error
                    printf("Error: '#' must be first\n");
                    return 0;
                }
            case BANG :
                if (num_tokens == 1){
                    printf("Error: no command entered\n");
                    return 0;
                }
                if (i != 0) {  // '!' token anywhere other than first  position is syntax error
                    printf("Error: '!' must be first\n");
                    return 0;
                }
            case EQUALS : 
                if (i == 0){
                    return 0;
                }
                if (i == num_tokens-1){
                    printf("Please enter the value of the variable\n");
                    return 0;
                }
                else {
                    if (scanned_input[i+1].type != GENERAL){
                        printf("Invalid value\n");
                        return 0;
                    }
                }
            case CD :
                if (num_tokens == 1){
                    printf("Please enter a directory name\n");
                    return 0;
                }
                if (i != 0){
                    printf("Error: \"cd\" must be first");
                    return 0;
                }
            case LV :
                if (num_tokens != 1){
                    printf("Invalid syntax: \"lv\"\n");
                    return 0;
                }
            case QUIT :
                if (num_tokens != 1){
                    printf("Invalid syntax, could not quit\n");
                    return 0;
                }
            case UNSET :
                if ( strcmp(scanned_input[i].value, "PATH") || strcmp(scanned_input[i].value, "CWD") || strcmp(scanned_input[i].value, "PS") == 0){
                    printf("Cannot unset built-in variable\n");
                    return 0;
                }
            case INFROM :
                if (i == num_tokens-1){
                    printf("Please enter infrom file\n");
                    return 0;
                }
                else {
                    if (scanned_input[i+1].type != GENERAL){
                        printf("Invalid file name\n");
                        return 0;
                    }
                }
            case OUTTO :
                if (i == num_tokens-1){
                    printf("Please enter file to redirect to\n");
                    return 0;
                }
                else {
                    if (scanned_input[i+1].type != GENERAL){
                        printf("Invalid file name\n");
                        return 0;
                    }
                }
            case VARNAME :
                if ((scanned_input[i].value[0] < 'A') || (scanned_input[i].value[0] > 'z')){
                    printf("Invalid variable name\n");
                    return 0;
                }
            case SUBSTITUTE :

                // same as standard varname, but ignoring the first '$' character
                if ((scanned_input[i].value[1] < 'A') || (scanned_input[i].value[1] > 'z')){
                    printf("Invalid variable name\n");
                    return 0;
                }
        }

    }
    return 1;
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
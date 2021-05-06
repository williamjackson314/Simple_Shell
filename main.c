#include "Utilities.h"

int available_variable_space = 10;

/**
 * @brief For use after parent has forked, 
 *        Changes working directory to path and runs 
 *        command passes args/envp
 * 
 * @param path 
 * @param command 
 * @param args 
 */
void execute(char *path, char *command, char *args[], char *in, char *out) {
    cd(path);
    int status; 
    if (fork() == 0) { // child process
        if(in != NULL) inFrom(in);
        if(out != NULL) outTo(out);
        execve(command, args, NULL);
    } else{ // parent process 
        wait(&status);
        if (status < 0) dieWithError("Error: Could not execute program\n");
    }
}

/**
 * @brief IO redirection simulating "<"
 *        Redirects STDIN to read from file_name
 * 
 * @param file_name 
 */
void inFrom(char *file_name){
    int file = open(file_name, O_RDONLY, 0777);
    if (file < 0) dieWithError("Could not read infrom specified file\n");
    dup2(file, STDIN_FILENO);
    close(file);
}

/**
 * @brief IO redirection simulating ">"
 *        Redirects STDOUT to read from file_name
 * 
 * @param file_name 
 */
void outTo(char *file_name){
    int file = open(file_name, O_WRONLY | O_CREAT, 0777);
    if (file < 0) dieWithError("Could not write outto specified file\n");
    dup2(file, STDOUT_FILENO);
    close(file);
}

void parse(char *line, char **argv){
    while (*line != '\0'){
        while (*line == ' ' || *line == '\n')
            *line++ = '\0'; 
        *argv++ = line;
        while (*line != '\0' && *line != ' ' && *line != '\n')
            line++;
    }
    *argv = NULL;
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
        if (dictionary[i].name == name){
            char *ret;
            ret = malloc(MAX_INPUT_SIZE);
            ret = dictionary[i].value;
            return ret;
        }
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
            printf("Unset %s successfully\n", name);
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
    printf("\nPredefined Variables\n");
    printf("--------------------\n");
    printf("PATH: %s\n", PATH);
    printf("CWD:  %s\n", CWD);
    printf("PS:  %s\n", PS);
    printf("--------------------\n");


    // prints user defined variables 
    printf("User Defined Variables\n");
    printf("----------------------\n");
    for(int i = 0; i < available_variable_space; i++){
        if(dictionary[i].name != NULL) 
            printf("%s: %s\n", dictionary[i].name, dictionary[i].value);
    }
    printf("----------------------\n");
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
    char *PS = "\033[0;32m >\033[0m ";
    char usr_input[MAX_INPUT_SIZE];
    int is_valid;
    token *input_tokens;
    input_tokens = (token *)malloc(MAX_INPUT_SIZE * sizeof(token));

    if (getcwd(CWD, PATH_MAX) == NULL) dieWithError("getcwd error\n");
    else CWD = getcwd(NULL, PATH_MAX);
    variable *dictionary;
    dictionary = (variable *)calloc(available_variable_space, sizeof(variable));

    char *var_name, *var_val;
    for(;;){
        if (feof(stdin)){ // Detects ^D and exits
	        printf("\n");
	        exit(0);
        }

        printf("%s", PS);
        fgets(usr_input, MAX_INPUT_SIZE, stdin);
        input_tokens = inputScanner(usr_input);
        
        if (num_tokens > 0){
            is_valid = inputParser(input_tokens);
            if (is_valid){
                for (int i = 0; i < num_tokens; i++){ // substitute any $values 
                    if(input_tokens[i].type == SUBSTITUTE){
                        char *name = input_tokens[i].value + 1;
                        char *value = searchVariable(dictionary, name);
                        if(value != NULL)
                            strcpy(input_tokens[i].value, value);
                        else printf("Unable to substitute for %s\n", name);
                    }
                }
                if(input_tokens[0].type == QUIT){
                    printf("\n");
                    exit(0);

                } else if(input_tokens[0].type == CD){
                    CWD = cd(input_tokens[1].value);

                } else if(input_tokens[0].type == BANG){
                    char *in = NULL, *out = NULL;
                    int argCount = num_tokens; // subtract 2 for "!" and "command"
                    for(int i = 2; i < num_tokens; i++){ // check for inFrom/outTo parameters 
                        if (input_tokens[i].type == INFROM) {
                            in = input_tokens[i].value;
                            argCount -= 1;
                        }
                        if (input_tokens[i].type == OUTTO){ 
                            out = input_tokens[i].value;
                            argCount -= 1;
                        }
                    }
                    char raw[MAX_INPUT_SIZE];
                    memset(raw, 0, sizeof(raw));
                    char *argv[MAX_INPUT_SIZE];
                    if (argCount > 0){
                        for(int i = 1; i < num_tokens; i++){
                            strcat(raw, input_tokens[i].value);
                            strcat(raw, " ");
                        }                        
                        printf("final: %s\n", raw);
                        parse(raw, argv);
                    }
                    
                    execute(PATH, input_tokens[1].value, argv, in, out);

                } else if(input_tokens[0].type == UNSET){
                    unsetVariable(dictionary, input_tokens[1].value);
                
                } else if(input_tokens[0].type == VARNAME){
                    var_name = input_tokens[0].value;
                    var_val = input_tokens[2].value;
                    printf("VAR NAME: %s \n", var_name);
                    printf("VAR VAL:  %s \n", var_val);
                    if(!(strcmp("PATH", var_name))){
                        PATH = var_val;
                    } else if(!(strcmp(var_name, "PS"))){
                        PS = var_val;
                    } else {
                        insertVariable(dictionary, var_name, var_val);
                        printVariables(dictionary, PATH, CWD, PS);
                    }

                } else if(input_tokens[0].type == LV){
                    printVariables(dictionary, PATH, CWD, PS);

                }
            }
        }
    }
    
    free(dictionary);

    return 0;
}
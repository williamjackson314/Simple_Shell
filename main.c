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
 * @param envp  
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
                 //execute command
<<<<<<< HEAD
            } 
        }
        else {
            printf("%s", PS);
=======
            }
>>>>>>> 275a8324ef587596a44f67c6befbdbce1dac59ed
        }
        
    }
    
    free(dictionary);

    return 0;
}
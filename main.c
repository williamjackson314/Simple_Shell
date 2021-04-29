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

typedef struct {

    char *name;
    char *value;

} variable;

typedef enum { HASH, BANG, EQUALS, CD, LV, QUIT, UNSET, INFROM, OUTTO, VARNAME, GENERAL } tokentype;

typedef struct {

    tokentype type;
    char value[MAX_INPUT_SIZE];

} token;

token *inputScanner(char *input){

    char input_cpy[MAX_INPUT_SIZE];

    strncpy(input_cpy, input, strlen(input)+1);
    static token tokenized_input[sizeof(token) * MAX_INPUT_SIZE];

    tokentype type;
    int cnt = 0;  
    const char *delims = " \t";
    char *token;
    char *prev_token = '\0';
    tokentype prev_type;

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
            
            printf("Type: %d\n", tokenized_input[cnt-1].type);
            printf("Value: %s\n", tokenized_input[cnt-1].value);

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
            prev_token = token; //may need to use memcpy here instead, be careful to make sure the token before previous is erased
            type = GENERAL;
        }

        tokenized_input[cnt].type = type;
        strcpy(tokenized_input[cnt].value, token);
        printf("Type: %d\n", tokenized_input[cnt].type);
        printf("Value: %s\n", tokenized_input[cnt].value);
        cnt += 1;
        
        token = strtok(NULL, delims);
    } 
 
    return tokenized_input;
}

void inputParser(token scanned_input){

}

int main(){
    
    DIR *directory;
    struct dirent *de;
    char *PATH = "/bin:/usr/bin";
    char *CWD;
    char *PS = "> ";
    char usr_input[MAX_INPUT_SIZE];
    token *input_tokens;
    input_tokens = (token *)malloc(MAX_INPUT_SIZE * sizeof(token));

    if (getcwd(CWD, PATH_MAX) == NULL) dieWithError(errno);

    variable *dictionary;
    dictionary = (variable *)calloc(10, sizeof(variable));

    for(;;){
        fgets(usr_input, MAX_INPUT_SIZE, stdin);
        input_tokens = inputScanner(usr_input);

    }

    free(dictionary);
    return 0;
}
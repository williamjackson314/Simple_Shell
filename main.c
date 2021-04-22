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

int main(){
    
    DIR *directory;
    struct dirent *de;
    char *PATH = "/bin:/usr/bin";
    char *CWD;
    char *PS = "> ";
    char usr_input[MAX_INPUT_SIZE];

    if (getwd(CWD) == NULL) dieWithError(errno);

    variable *dictionary;
    dictionary = (variable *)calloc(10, sizeof(variable));

    for(;;){
        fgets(usr_input, MAX_INPUT_SIZE, stdin);


    }

    free(dictionary);
    return 0;
}
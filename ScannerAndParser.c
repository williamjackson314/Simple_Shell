#include "Utilities.h"

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
                if (tokenized_input[index-1].type != GENERAL){
                    return tokenized_input; //syntax error detected, do not read any more input
                }
                else {
                    tokenized_input[index-1].type = prev_type; 
                    strcpy(tokenized_input[index-1].value, prev_token);
                }
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
    printf("Num tokens = %d\n", num_tokens);
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
                if (i == 0){ //make sure = is not first token
                    printf("Variable name must come before '='\n");
                    return 0;
                }
                else {     
                    if (scanned_input[i-1].type != VARNAME){
                        printf("Invalid variable name\n");
                        return 0;
                    }
                }
                if (i == num_tokens-1){ //make sure = is not last token
                    printf("Must have value after '='\n");
                    return 0;
                }
                else {
                    if (scanned_input[i+1].type != GENERAL){
                        printf("Invalid value\n");
                        return 0;
                    }
                }
                if (num_tokens > 3){
                    printf("Too many arguments for setting variable\n");
                    return 0;
                }
            case CD :
                if (num_tokens == 1){
                    printf("Please enter a directory name\n");
                    return 0;
                }
                else if (num_tokens > 2){
                    printf("Too many arguments to \"cd\"");
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
                if (i != 0){
                    printf("\"unset\" must come first\n");
                    return 0;
                }
                if ((num_tokens == 1)||(num_tokens > 2)){
                    printf("Incorrect number of arguments to \"unset\"");
                    return 0;
                }
                if (scanned_input[i+1].type != VARNAME){
                    printf("Must enter valid variable to unset\n");
                    return 0;
                }
                if ( strcmp(scanned_input[i].value, "PATH") || strcmp(scanned_input[i].value, "CWD") || strcmp(scanned_input[i].value, "PS") == 0){
                    printf("Cannot unset built-in variable\n");
                    return 0;
                }
            case INFROM :
                if (i == num_tokens-1){
                    printf("Must enter infrom file\n");
                    return 0;
                }
                else {
                    if (scanned_input[i+1].type != GENERAL){
                        printf("Invalid file name\n");
                        return 0;
                    }
                }
                if ((i == 0) || (i < 2)){ //infrom: must not be first token and must be at least third token
                    printf("Invalid syntax: \"infrom:\" position\n");
                    return 0;
                }
                // If infrom: not the second to last token and OUTTO not next
                else if ( (i != (num_tokens-2)) && (scanned_input[i+1].type != OUTTO) ){
                    printf("Invalid syntax: \"infrom:\" position\n");
                    return 0;
                }
            case OUTTO :
                if (i == num_tokens-1){
                    printf("Must enter file to redirect to\n");
                    return 0;
                }
                else {
                    if (scanned_input[i+1].type != GENERAL){
                        printf("Invalid file name\n");
                        return 0;
                    }
                }
                if ((i == 0) || (i < 2)){ //outto: must not be first token and must be at least third token
                    printf("Invalid syntax for '!' command\n");
                    return 0;
                }
            case SUBSTITUTE :

                // same as standard varname, but ignoring the first '$' character
                if ((scanned_input[i].value[1] < 'A') || (scanned_input[i].value[1] > 'z')){
                    printf("Invalid variable name\n");
                    return 0;
                }
            case VARNAME :
                if ((scanned_input[i].value[0] < 'A') || (scanned_input[i].value[0] > 'z')){
                    printf("Invalid variable name\n");
                    return 0;
                }
                if (num_tokens > 3){
                    printf("Too many arguments for setting variable\n");
                    return 0;
                }
        }

    }
    return 1;
}
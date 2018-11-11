#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include <string.h>

#define MAX_LENGTH 50
#define INCREMENT 100
#define LEX_ERR -1

typedef enum {
    KEYWORD_DEF,
    KEYWORD_DO,
    KEYWORD_ELSE,
    KEYWORD_END,
    KEYWORD_IF,
    KEYWORD_NOT,
    KEYWORD_NIL,
    KEYWORD_THEN,
    KEYWORD_WHILE,

    ID,
    NUM_INT,
    NUM_FLOAT,
    NUM_EXP,
    STRING,
    LEX_EOL,
    COMMA,
    ROUNDL,
    ROUNDR,
    ASSIGN,
    PLUS,
    MINUS,
    MUL,
    DIV,
    LESS,
    MORE,
    LESS_EQUAL,
    MORE_EQUAL,
    EQUAL,
    NOT_EQUAL,

    START,
    LEX_EOF,
    NUM,
    IDENTIFIER,
    COMMENT,
    BLOCK_COMMENT,
    IDENTIF

} Types;

FILE *file;

unsigned int i = 0;
char *string;

int checkKeywords(char *tmp) {

    //char *keyWords[] = {"def", "do", "else", "end", "if", "not", "nil", "then", "while"};
    if (strcmp("def", tmp) == 0) return KEYWORD_DEF;
    else if (strcmp("do", tmp) == 0) return KEYWORD_DO;
    else if (strcmp("else", tmp) == 0) return KEYWORD_ELSE;
    else if (strcmp("end", tmp) == 0) return KEYWORD_END;
    else if (strcmp("if", tmp) == 0) return KEYWORD_IF;
    else if (strcmp("not", tmp) == 0) return KEYWORD_NOT;
    else if (strcmp("nil", tmp) == 0) return KEYWORD_NIL;
    else if (strcmp("then", tmp) == 0) return KEYWORD_THEN;
    else if (strcmp("while", tmp) == 0) return KEYWORD_WHILE;
    else return -1;

}

int addCharToArray(char c, char *str) {

    if (i >= 100) {
        string = realloc(string, INCREMENT);
        if (string == NULL) {
            printf("realloc err\n");
            return LEX_ERR;
        }
    }
    str[i] = c;
    i++;
    str[i] = '\0';
    printf("string: %s\n", str);
    return 0;
}

int getToken(char *value, int *line) {

    int s, state = START,tmp;
    string[0] = '\0';
    while (1) {

        //todo

        s = fgetc(stdin);
        switch (state) {

            case START:
                if (feof(stdin)) {
                    return LEX_EOF;
                }
                else if (s == '\n') {
                    line++;
                    return LEX_EOL;
                }

                else if (isspace(s));
                else if (islower(s) || s == '_' && !isdigit(s)) {
                    printf("identifier\n");
                    addCharToArray(s,string);
                    state = IDENTIF;
                    break;


                }
                else if (isdigit(s)) {
                    printf("num\n");
                    addCharToArray(s,string);
                    state = NUM;
                    break;
                }
                else {
                    switch (s) {
                        //operatory
                        case '+':
                            return PLUS;
                        case '-':
                            return MINUS;
                        case '*':
                            return MUL;
                        case '/':
                            return DIV;
                        case '<':
                            s = fgetc(stdin);
                            if (s == '=') {
                                return LESS_EQUAL;
                            } else {
                                ungetc(s, file);
                                return LESS;
                            }
                        case '>':
                            s = fgetc(stdin);
                            if (s == '=') {
                                return MORE_EQUAL;
                            } else {
                                ungetc(s, file);
                                return MORE;
                            }
                        case '(':
                            return ROUNDL;
                        case ')':
                            return ROUNDR;
                        case ',':
                            return COMMA;
                        case '"':
                            state = STRING;
                            break;

                        case '=':
                            s = fgetc(stdin);
                            if (s == '=') {
                                return EQUAL;
                            } else if (s == 'b' || s == 'e') {
                                while (!isspace(s)) {

                                    s = fgetc(stdin);
                                    addCharToArray(s, string);
                                }
                                if (strcmp(string, "begin") == 0) {
                                    state = BLOCK_COMMENT;
                                    break;
                                }
                                else {
                                    return LEX_ERR;
                                }
                            } else {
                                ungetc(s, stdin);
                                return ASSIGN;
                            }
                    }
                    default:
                        printf("LEX_ERR\n");
                }
            case STRING:
                tmp = 1;
                while(1){
                    s = fgetc(stdin);
                    if(s == '"'){
                        break;
                    }
                    addCharToArray(s,string);
                }
                strcpy(value, string);
                string[0] = '\0';
                i = 0;
                state = START;
                return STRING;

            case IDENTIF:
                if (isalnum(s) || s == '_' || s == '?' || s == '!') {
                    while (isalnum(s) || s == '_') {

                        //printf("pridavam do pola znak %c\n", s);
                        addCharToArray(s, string);
                        s = fgetc(stdin);
                    }

                    if (s == '?' || s == '!') {
                        addCharToArray(s, string);
                    } else {
                        ungetc(s, stdin);
                    }
                    int a = checkKeywords(string);
                    printf("a = %d\n", a);
                    if (a == -1) {
                        strcpy(value, string);
                        string[0] = '\0';
                        i = 0;
                        return IDENTIFIER;

                    } else {
                        strcpy(value, string);
                        string[0] = '\0';
                        i = 0;
                        return a;
                    }
                }
//            case BLOCK_COMMENT:
//                int i = 1;
//                while(i){
//                    s = fgetc(stdin);
//                    if()
//                }
            case NUM:

                while (isdigit(s)) {
                    //printf("pridavam do pola znak %c\n", s);
                    addCharToArray(s,string);
                    s = fgetc(stdin);
                    //printf("nacitam nove s:%c\n", s);


                }
                //printf("vyskocil som z whilu\n");
                if(s == '.'){
                    addCharToArray(s,string);
                    state = NUM_FLOAT;
                    break;
                }
                else if(s == 'e' || s == 'E'){
                    addCharToArray(s,string);
                    state = NUM_EXP;
                    break;
                }
                else{
                    strcpy(value, string);
                    string[0] = '\0';
                    i = 0;
                    ungetc(s,stdin);
                    state = START;
                    return NUM_INT;
                }
            case NUM_FLOAT:

                while (isdigit(s)) {
                    addCharToArray(s,string);
                    s = fgetc(stdin);
                }
                if(s == 'e' || s == 'E'){
                    addCharToArray(s,string);
                    state = NUM_EXP;
                    break;
                }
                else{
                    strcpy(value, string);
                    string[0] = '\0';
                    i = 0;
                    ungetc(s,stdin);
                    state = START;
                    return NUM_FLOAT;
                }
            case NUM_EXP:

                if(s == '+'||s=='-'){
                    addCharToArray(s,string);
                    s = fgetc(stdin);
                }
                if(!isdigit(s)){
                    return LEX_ERR;
                }
                while(isdigit(s)){
                    addCharToArray(s,string);
                    s = fgetc(stdin);
                }
                strcpy(value, string);
                string[0] = '\0';
                i = 0;
                ungetc(s,stdin);
                state = START;
                return NUM_EXP;

        }
    }
}


int main() {
    int a;
    char value[100];
    int line;
    string = malloc(sizeof(char) * MAX_LENGTH);
    if (string == NULL) {
        printf("malloc err\n");
        return LEX_ERR;
    }
    file = stdin;
    if (file == NULL) {
        printf("file cant open\n");
        return LEX_ERR;
    }

    while ((a = getToken(value, &line)) != LEX_EOF) {

        printf("Value: %s line: %d type: %d\n", value, line, a);
        value[0] = '\0';
        if (a == LEX_ERR) {
            break;
        }

    }
    return 0;
}

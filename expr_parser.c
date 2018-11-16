#include <zconf.h>
#include "expr_parser.h"
#include "error.h"


#define SIZE 7
#define SYNTAX_OK 101
#define MATH 0
#define BOOL 1

#define GET_TOKEN() do {if ((token = getToken(value, &line)) == ERR_LEXICAL)\
return ERR_LEXICAL;} while(0)

#define ACCEPT(type) do{ if(token != type) return ERR_SYNTAX;\
    GET_TOKEN();} while(0)

extern int token;


string *value;
int line;

const char prec_table[SIZE][SIZE] = {
//             0    1   2   3   4   5   6
//             + - * / rel  (   )  ID   $
/*  0 + - */ {GR, LE, GR, LE, GR, LE, GR},

/*  1 * / */
             {GR, GR, GR, LE, GR, LE, GR},

/*  2 rel */
             {LE, LE, NO, LE, GR, LE, GR},

/*  3 (   */
             {LE, LE, LE, LE, EQ, LE, NO},

/*  4 )   */
             {GR, GR, GR, NO, GR, NO, GR},

/*  5 id  */
             {GR, GR, GR, NO, GR, NO, GR},

/*  6 $   */
             {LE, LE, LE, LE, NO, LE, NO}
};

t_stack *stack;

int decode(int symbol) {
    switch (symbol) {
        case MINUS:
        case PLUS:
            return 0;
        case MUL:
        case DIV:
            return 1;
        case EQUAL:
        case NOT_EQUAL:
        case LESS:
        case LESS_EQUAL:
        case MORE:
        case MORE_EQUAL:
            return 2;
        case ROUNDL:
            return 3;
        case ROUNDR:
            return 4;
        case ID:
        case NUM_EXP:
        case NUM_FLOAT:
        case NUM_INT:
            return 5;
        case LEX_EOF:
        case LEX_EOL:
        case KEYWORD_DO:
        case KEYWORD_THEN:
            return 6;
        default:
            return -1;
    }
}

bool check_rule(t_stack *stack, int count, ...) {
    va_list vaList;
    va_start(vaList, count);

    int *params = malloc(count * sizeof(int));

    if (params == NULL) {
        va_end(vaList);
        return false;
    }

    for (int i = 0; i < count; ++i) { //give params to array in reverse order
        params[count - 1 - i] = va_arg(vaList, int);
    }

    t_stack_item *tmp = stack->top;

    for (int i = 0; i < count; ++i) {
        if (tmp->symbol != params[i]) {
            free(params);
            va_end(vaList);
            return false;
        }

        if (tmp->next == NULL) {
            free(params);
            va_end(vaList);
            return false;
        }

        tmp = tmp->next;
    }

    if (tmp->symbol != LE) {
        free(params);
        va_end(vaList);
        return false;
    }

    free(params);
    va_end(vaList);
    return true;
}

void pop_rule(t_stack *stack, int count, int symbol) {
    multi_pop(stack, count + 1); //pop rule and symbol '<'
    push(stack, symbol);
}


int rules(t_stack *stack) {
    if (check_rule(stack, 3, EXPR, PLUS, EXPR)) {
        pop_rule(stack, 3, EXPR);
        return SYNTAX_OK;
    } else if (check_rule(stack, 3, EXPR, MINUS, EXPR)) {
        pop_rule(stack, 3, EXPR);
        return SYNTAX_OK;
    } else if (check_rule(stack, 3, EXPR, MUL, EXPR)) {
        pop_rule(stack, 3, EXPR);
        return SYNTAX_OK;
    } else if (check_rule(stack, 3, EXPR, DIV, EXPR)) {
        pop_rule(stack, 3, EXPR);
        return SYNTAX_OK;
    } else if (check_rule(stack, 3, EXPR, LESS, EXPR)) {
        pop_rule(stack, 3, EXPR);
        return SYNTAX_OK;
    } else if (check_rule(stack, 3, EXPR, MORE, EXPR)) {
        pop_rule(stack, 3, EXPR);
        return SYNTAX_OK;
    } else if (check_rule(stack, 3, EXPR, LESS_EQUAL, EXPR)) {
        pop_rule(stack, 3, EXPR);
        return SYNTAX_OK;
    } else if (check_rule(stack, 3, EXPR, MORE_EQUAL, EXPR)) {
        pop_rule(stack, 3, EXPR);
        return SYNTAX_OK;
    } else if (check_rule(stack, 3, EXPR, NOT_EQUAL, EXPR)) {
        pop_rule(stack, 3, EXPR);
        return SYNTAX_OK;
    } else if (check_rule(stack, 3, EXPR, EQUAL, EXPR)) {
        pop_rule(stack, 3, EXPR);
        return SYNTAX_OK;
    } else if (check_rule(stack, 1, ID)) {
        pop_rule(stack, 1, EXPR);
        return SYNTAX_OK;
    } else if (check_rule(stack, 3, ROUNDL, EXPR, ROUNDR)) {
        pop_rule(stack, 3, EXPR);
        return SYNTAX_OK;
    } else if (check_rule(stack, 1, NUM_INT)) {
        pop_rule(stack, 1, EXPR);
        return SYNTAX_OK;
    } else if (check_rule(stack, 1, NUM_FLOAT)) {
        pop_rule(stack, 1, EXPR);
        return SYNTAX_OK;
    } else return ERR_SYNTAX;
}


int expresion(int type) {
    int retval;
    t_stack *stack = malloc(sizeof(t_stack));
    stack_init(stack);
    value = malloc(sizeof(string));
    if (value == NULL) return ERR_INTERNAL;
    if (strInit(value) == STR_ERROR) return ERR_INTERNAL;

    push(stack, LEX_EOF);

    // printf("%d\n",token);

    do {
        if (type == BOOL && (token == ROUNDR || token == KEYWORD_THEN || token == KEYWORD_DO) &&
            top_term(stack)->symbol == LEX_EOF) {
            break;
        }


        switch (prec_table[decode(top_term(stack)->symbol)][decode(token)]) {
            case EQ:
                push(stack, token);
                GET_TOKEN();
                break;
            case LE:
                insert_after_first_terminal(stack, LE);
                push(stack, token);
                GET_TOKEN();
                break;
            case GR:
                retval = rules(stack);
                if (retval != SYNTAX_OK) {
                    while (stack->top != NULL) pop(stack);
                    free(stack);
                    return retval;
                }
                break;
            case NO:
            default:
                while (stack->top != NULL) pop(stack);
                free(stack);
                return ERR_SYNTAX;
        }

    } while (token != LEX_EOL || top_term(stack)->symbol != LEX_EOF);

    while (stack->top != NULL) pop(stack);
    free(stack);
    return SYNTAX_OK;

}

int bool_expr() {
    return expresion(BOOL);
}

int math_expr() {
    return expresion(MATH);
}


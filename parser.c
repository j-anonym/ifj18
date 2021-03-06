/**
* School project to subject IFJ (Formal Languages and Compilers)
* Compiler implementation of imperative language IFJ18
*
* Module for recursive descent parser
*
* Author: Jan Zauska  Julius Marko  Jan Vavro
* Login:  xzausk00    xmarko17      xvavro05
*/

#include "lex.h"
#include "error.h"
#include "parser.h"
#include "semantic.h"
#include "expr_parser.h"
#include "code_gen.h"

int token;

//temp
string *value;
string *temp;
int err;
extern int prev_token;

int assign(char *fun_id) {
    // pravidlo <assign> -> "ID" = <value>
    char previous_token_value[value->length + 1];
    strcpy(previous_token_value, value->str);
    strCopyString(temp, value);
    switch (token) {
        case ROUNDL:
            if ((err = math_expr(fun_id)) != SYNTAX_OK) return err;
            GEN_INSTR("MOVE %s %s", "LF@$retval", "GF@expr_res");

            ACCEPT(LEX_EOL);
            return SYNTAX_OK;

        case IDF:
        case ID:
            strcpy(previous_token_value, value->str);

            GET_TOKEN();

            if (is_function(previous_token_value)) {
                if ((err = fun_call(previous_token_value, fun_id)) != SYNTAX_OK) return err;
                return SYNTAX_OK;
            }

            if (IS_VALID_PARAM || token == ROUNDL) {
                insert_fun_to_st(previous_token_value, 0, false, false);
                if ((err = fun_call(previous_token_value, fun_id)) != SYNTAX_OK) return err;
                return SYNTAX_OK;
            } else if (token == LEX_EOL) {
                if (semantic_check_var_defined(fun_id, previous_token_value) == ERR_SEMANTIC_DEFINITION) {
                    insert_fun_to_st(previous_token_value, 0, false, false);
                    if ((err = fun_call(previous_token_value, fun_id)) != SYNTAX_OK) return err;
                    return SYNTAX_OK;
                }
            } else {
                prev_token = token;
                token = ID;
                strcpy(value->str, previous_token_value);
                if ((err = math_expr(fun_id)) != SYNTAX_OK) return err;
                else GEN_INSTR("MOVE %s %s", "LF@$retval", "GF@expr_res");
            }

            ACCEPT(LEX_EOL);
            return SYNTAX_OK;

        case PRINT:
        case INPUTF:
        case INPUTI:
        case INPUTS:
        case SUBSTR:
        case LENGTH:
        case CHR:
        case ORD:
            GET_TOKEN();
            if ((err = fun_call(previous_token_value, fun_id)) != SYNTAX_OK) return err;
            // GEN_INSTR("MOVE %s %s ", "GF@expr_res", "TF@$retval");
            return SYNTAX_OK;

        case NUM_INT:
        case NUM_FLOAT:
        case NUM_EXP:
        case STRING:
        case KEYWORD_NIL:
            if ((err = math_expr(fun_id)) != SYNTAX_OK) return err;
            GEN_INSTR("MOVE %s %s", "LF@$retval", "GF@expr_res");

            ACCEPT(LEX_EOL);
            return SYNTAX_OK;

        case ERR_LEXICAL:
            return ERR_LEXICAL;
        default:
            return ERR_SYNTAX;

    }
}


int fun_params(char *fun_id) {
    static size_t params_count = 0;

    // <FUN_PARAMS> -> (<ITEM><ITEM_LIST)
    switch (token) {
        case ROUNDR:
            GET_TOKEN();
            ACCEPT(LEX_EOL);
            params_count = 0;

            return SYNTAX_OK;

        case ID:
            err = semantic_add_fun_param(fun_id, value->str);
            if (err != 0) return err;
            params_count++;
            GEN_INSTR("DEFVAR LF@%s", value->str);


            GEN_INSTR("MOVE LF@%s LF@%%%d", value->str, params_count);

            GET_TOKEN();

            if (token == ROUNDR) {
                return fun_params(fun_id);
            }
            if (token == COMMA) {
                GET_TOKEN();
                if (token == ID)
                    return fun_params(fun_id);
                return ERR_SYNTAX;
            }
            return ERR_SYNTAX;

        case ERR_LEXICAL:
            return ERR_LEXICAL;
        default:
            return ERR_SYNTAX;
    }
}


int fun_declr() {
    // <FUN_DECLR> -> DEF <ID> <FUN_PARAMS> EOL <STAT_LIST> END
    char previous_token_value[value->length + 1];
    strcpy(previous_token_value, value->str);
    strCopyString(temp, value);
    if (token != ID && token != IDF)
        return ERR_SYNTAX;
    GET_TOKEN();

    if (semantic_check_fun_defined(previous_token_value) == ERR_SEMANTIC_DEFINITION) return ERR_SEMANTIC_DEFINITION;

    if (gen_fun_header(previous_token_value) == ERR_INTERNAL) return ERR_INTERNAL;

    ACCEPT(ROUNDL);

    if ((err = fun_params(previous_token_value)) != SYNTAX_OK) return err;

    if ((err = stat_list(previous_token_value)) != SYNTAX_OK) return err;

    ACCEPT(KEYWORD_END);

    ACCEPT(LEX_EOL);

    if (gen_fun_footer(previous_token_value) == ERR_INTERNAL) return ERR_INTERNAL;

    return SYNTAX_OK;
}


int params(char *fun_id, char *called_from_fun, unsigned *par_count) {
    // pravidlo <CALL_PARAMS> -> <ITEM><ITEM_LIST
    char previous_token_value[value->length + 1];
    static size_t params_count = 0;
    switch (token) {
        case LEX_EOL:
        case ROUNDR:
            err = semantic_check_fun_call_params(fun_id, params_count);
            *par_count = (unsigned) params_count;
            params_count = 0;
            if (err != 0) return err;
            return SYNTAX_OK;
        case INPUTS:
        case INPUTI:
        case INPUTF:
        case LENGTH:
        case SUBSTR:
        case ORD:
        case CHR:
        case ID:
            strcpy(previous_token_value, value->str);
            if (is_function(previous_token_value)) return ERR_SEMANTIC_OTHER;
            if (semantic_check_var_defined(called_from_fun, previous_token_value) == ERR_SEMANTIC_DEFINITION)
                return ERR_SEMANTIC_DEFINITION;
        case NUM_INT:
        case NUM_FLOAT:
        case NUM_EXP:
        case STRING:
        case KEYWORD_NIL:
            params_count++;
            GEN_INSTR("DEFVAR TF@%%%d", params_count);

            if (token == NUM_INT) {
                GEN_INSTR("MOVE TF@%%%d int@%s", params_count, value->str);
            } else if (token == NUM_FLOAT || token == NUM_EXP) {
                char *endptr;
                double tmp = strtod(value->str, &endptr);
                GEN_INSTR("MOVE TF@%%%d float@%a", params_count, tmp);
            } else if (token == STRING) {
                GEN_INSTR("MOVE TF@%%%d string@%s", params_count, value->str);
            } else if (token == ID) {
                GEN_INSTR("MOVE TF@%%%d LF@%s", params_count, value->str);
            } else if (token == KEYWORD_NIL) {
                GEN_INSTR("MOVE TF@%%%d nil@nil", params_count);
            }

            // check if param is defined
            GET_TOKEN();

            if (token == COMMA) {
                GET_TOKEN();
                if (token != ID && token != NUM_INT && token != NUM_FLOAT && token != NUM_EXP && token != STRING)
                    return ERR_SYNTAX;
                return params(fun_id, called_from_fun, par_count);
            }
            if (token != LEX_EOL && token != ROUNDR) return ERR_SYNTAX;
            return params(fun_id, called_from_fun, par_count);
        case ERR_LEXICAL:
            return ERR_LEXICAL;
        default:
            return ERR_SYNTAX;
    }
}


int fun_call(char *fun_id, char *called_from_fun) {
    // pravidlo <FUN_CALL> ->
    if (!is_function(fun_id))
        return ERR_SEMANTIC_DEFINITION;
    if (!is_fun_defined(fun_id) && !is_fun_builtin(fun_id) && is_main_scope(called_from_fun))
        return ERR_SEMANTIC_DEFINITION;

    int brackets = 0;
    if (token == ROUNDL) {
        brackets = 1;
        GET_TOKEN();
    }

    GEN_INSTR("%s", "CREATEFRAME");
    unsigned par_count;
    int err = params(fun_id, called_from_fun, &par_count);
    if (err != SYNTAX_OK) return err;

    if (brackets) {
        ACCEPT(ROUNDR);
    }
    ACCEPT(LEX_EOL);

    if (is_fun_builtin(fun_id)) {
        if (gen_builtin_fun(fun_id, par_count) == ERR_INTERNAL) return ERR_INTERNAL;
    }

    if (!is_print_fun(fun_id)) GEN_INSTR("CALL *%s", fun_id);  // print is handled inside gen_print()
    GEN_INSTR("MOVE %s %s ", "LF@$retval", "TF@$retval");
    GEN_INSTR("MOVE %s %s ", "GF@expr_res", "TF@$retval");

    return SYNTAX_OK;
}

int stat_list(char *fun_id) {
    bool is_id = false;
    static int cnt = 0;
    int label_id = 0;
    char previous_token_value[value->length + 1];
    strcpy(previous_token_value, value->str);
    switch (token) {
        case KEYWORD_IF:
            cnt++;
            label_id = cnt;
            // pravidlo IF <EXPR> EOL <STAT_LIST> ELSE EOL <STAT_LIST> END
            GET_TOKEN();
            if ((err = bool_expr(fun_id)) != SYNTAX_OK) return err;
            GEN_INSTR("JUMPIFNEQ ELSE_LABEL_%d %s %s", cnt, "GF@expr_res", "bool@true");
            ACCEPT(KEYWORD_THEN);
            ACCEPT(LEX_EOL);

            if ((err = stat_list(fun_id)) != SYNTAX_OK) return err;

            GEN_INSTR("JUMP ELSE_END_%d", label_id);

            ACCEPT(KEYWORD_ELSE);
            GEN_INSTR("LABEL ELSE_LABEL_%d", label_id);
            ACCEPT(LEX_EOL);

            if ((err = stat_list(fun_id)) != SYNTAX_OK) return err;

            ACCEPT(KEYWORD_END);
            ACCEPT(LEX_EOL);
            GEN_INSTR("LABEL ELSE_END_%d", label_id);

            return stat_list(fun_id);

        case KEYWORD_WHILE:
            cnt++;
            label_id = cnt;

            GEN_INSTR("LABEL WHILE_START_%d", label_id);
            // pravidlo WHILE <EXPR> DO EOL <STAT_LIST> END
            GET_TOKEN();
            if ((err = bool_expr(fun_id)) != SYNTAX_OK) return err;

            GEN_INSTR("JUMPIFNEQ WHILE_END_%d %s %s", cnt, "GF@expr_res", "bool@true");

            ACCEPT(KEYWORD_DO);
            ACCEPT(LEX_EOL);


            if ((err = stat_list(fun_id)) != SYNTAX_OK) return err;

            ACCEPT(KEYWORD_END);
            ACCEPT(LEX_EOL);
            GEN_INSTR("JUMP WHILE_START_%d", label_id);
            GEN_INSTR("LABEL WHILE_END_%d", label_id);
            return stat_list(fun_id);
        case LEX_EOL:
            GET_TOKEN();
            return stat_list(fun_id);
        case KEYWORD_NIL:
            GET_TOKEN();
            ACCEPT(LEX_EOL);
            return stat_list(fun_id);
        case KEYWORD_DEF:
        case KEYWORD_END:
        case KEYWORD_ELSE:
            return SYNTAX_OK;

        case LEX_EOF:
            return SYNTAX_OK;

        case ID:
            strcpy(previous_token_value, value->str);

            GET_TOKEN();
            if (token == ASSIGN) {
                GET_TOKEN();
                if (!is_variable(previous_token_value, fun_id)) {
                    if (find_instr("LABEL *%s\n", fun_id) == ERR_INTERNAL) return ERR_INTERNAL;
                    if (insert_instr_after("MOVE TF@%s nil@nil\n", previous_token_value) == ERR_INTERNAL)
                        return ERR_INTERNAL;

                    if (insert_instr_after("DEFVAR TF@%s\n", previous_token_value) == ERR_INTERNAL) return ERR_INTERNAL;

                }

                if ((err = insert_var_to_st(previous_token_value, fun_id, true)) != 0) return err;

                if ((err = assign(fun_id)) != SYNTAX_OK) return err;
                GEN_INSTR("MOVE LF@%s GF@%s ", previous_token_value, "expr_res");

                return stat_list(fun_id);
            }
            is_id = true;
        case IDF:
            if (!is_id) {
                strcpy(previous_token_value, value->str);
                GET_TOKEN();
                is_id = false;
            }

            if (is_function(previous_token_value)) {
                if ((err = fun_call(previous_token_value, fun_id)) != SYNTAX_OK) return err;
                return stat_list(fun_id);
            }

            /* insert ID as function not defined yet */
            if (IS_VALID_PARAM || token == ROUNDL) {
                insert_fun_to_st(previous_token_value, 0, false, false);
                if ((err = fun_call(previous_token_value, fun_id)) != SYNTAX_OK) return err;
                return stat_list(fun_id);
            } else if (token == LEX_EOL) { /* insert ID as function not defined yet */
                if (semantic_check_var_defined(fun_id, previous_token_value) == ERR_SEMANTIC_DEFINITION) {
                    insert_fun_to_st(previous_token_value, 0, false, false);
                    if ((err = fun_call(previous_token_value, fun_id)) != SYNTAX_OK) return err;
                    return stat_list(fun_id);
                }
            } else {
                prev_token = token;
                token = ID;
                strcpy(value->str, previous_token_value);
                if ((err = math_expr(fun_id)) != SYNTAX_OK) return err;
                else GEN_INSTR("MOVE %s %s", "LF@$retval", "GF@expr_res");
            }

            ACCEPT(LEX_EOL);
            return stat_list(fun_id);

        case NUM_EXP:
        case NUM_FLOAT:
        case STRING:
        case NUM_INT:
        case ROUNDL:
            if ((err = math_expr(fun_id)) != SYNTAX_OK) return err;
            GEN_INSTR("MOVE %s %s", "LF@$retval", "GF@expr_res");
            ACCEPT(LEX_EOL);
            return stat_list(fun_id);
        case INPUTS:
        case INPUTF:
        case INPUTI:
        case PRINT:
        case LENGTH:
        case ORD:
        case CHR:
        case SUBSTR:
            strcpy(previous_token_value, value->str);
            GET_TOKEN();
            if ((err = fun_call(previous_token_value, fun_id)) != SYNTAX_OK) return err;
            return stat_list(fun_id);
        case ERR_LEXICAL:
            return ERR_LEXICAL;
        default:
            return ERR_SYNTAX;
    }
}

int program() {
    switch (token) {
        case KEYWORD_DEF:
            GET_TOKEN();
            if ((err = fun_declr()) != SYNTAX_OK) return err;
            return program();

        case LEX_EOF:
            return SYNTAX_OK;

        case ERR_LEXICAL:
            return ERR_LEXICAL;

        case KEYWORD_END:
        case KEYWORD_ELSE:
            GET_TOKEN();
            return program();

        default:
            if ((err = stat_list("MAIN")) != SYNTAX_OK) return err;
            return program();
    }
}


int parse() {
    int result;
    value = malloc(sizeof(string));
    temp = malloc(sizeof(string));

    if (value == NULL || temp == NULL) return ERR_INTERNAL;
    if (strInit(value) == STR_ERROR) return ERR_INTERNAL;
    if (strInit(temp) == STR_ERROR) return ERR_INTERNAL;

    if (semantic_prepare() == ERR_INTERNAL) return ERR_INTERNAL;
    if (code_gen_prepare() == ERR_INTERNAL) return ERR_INTERNAL;

    gen_header();
    GET_TOKEN();
    result = program();

    if (err == SYNTAX_OK && !all_ids_defined()) return ERR_SEMANTIC_DEFINITION;

    if (err == SYNTAX_OK) code_generate();

    if (semantic_clean() == ERR_INTERNAL) return ERR_INTERNAL;
    if (code_gen_clean() == ERR_INTERNAL) return ERR_INTERNAL;

    strFree(value);
    free(value);

    strFree(temp);
    free(temp);
    return result;
}

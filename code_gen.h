#ifndef IFJ_CODE_GEN_H
#define IFJ_CODE_GEN_H

#include <stdio.h>
#include <stdbool.h>
#include "list.h"

#define GEN_INSTR(format, ...) do {if (ERR_INTERNAL == gen_instr(format"\n",  __VA_ARGS__)) \
return ERR_INTERNAL;} while(0)

//does not add newline
#define ADD_INSTR(...) do {if (ERR_INTERNAL == gen_instr( __VA_ARGS__)) \
return ERR_INTERNAL;} while(0)


FILE *fp;

int code_gen_prepare();

int code_gen_clean();

int gen_header();

int gen_main();

int gen_instr(char *c, ...);

void code_generate();

void print_code(tList *L);

int insert_instr_after(char *string, ...);

int find_instr(char *string, ...);


int gen_fun_header(char *label);

int gen_fun_footer(char *label);

int gen_builtin_fun(char *fun_id, unsigned);

int gen_inputs();

int gen_inputi();

int gen_inputf();

int gen_print(char *fun_id, unsigned params_count);

int gen_length();

int gen_substr();

int gen_ord();

int gen_chr();

bool is_print_fun(char *fun_id);

int gen_semantic_type_check_header(char *fun_id);

int gen_semantic_type_check(char *fun_id, char *frame_var, char *desired_type);


#endif //IFJ_CODE_GEN_H

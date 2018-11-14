//
// Created by Július  Marko  on 13/11/2018.
//

#ifndef IFJ_SEMANTIC_H
#define IFJ_SEMANTIC_H

// TODO: move functions doing only symtable stuff to another header file

#include "symtable.h"

int semantic_prepare();
int add_builtin_funcs_to_st();
void init_fun_elem_data(elem_data *data, size_t params_count, bool defined, bool builtin); // x
int insert_fun_to_st(char *fun_id, size_t params_count, bool defined, bool builtin); // x
int semantic_add_fun_param(char *fun_id, char *param);

int semantic_check_fun_definition(char *fun_id);
int semantic_check_fun_call_params(char *fun_id, size_t params_count);

#endif //IFJ_SEMANTIC_H

#include "symtable.h"
#include "error.h"

int hash_code(t_key key) {
    int retval = 1;
    int idlen = strlen(key);
    for (int i = 0; i < idlen; i++)
        retval += key[i];
    return (retval % SYMTABLE_SIZE);
}

void st_init(st *st_ptr) {
    if (st_ptr) {
        for (int i = 0; i < SYMTABLE_SIZE; i++) {
            (*st_ptr)[i] = NULL;
        }
    }
}

st_elem *st_search(st *st_ptr, t_key key) {
    if (st_ptr == NULL || key == NULL)
        return NULL;
    int index = hash_code(key);
    st_elem *tmp = (*st_ptr)[index];
    while (tmp) {
        if (strcmp(tmp->key, key) == 0) {
            return tmp;
        }
        tmp = tmp->ptrnext;
    }
    return NULL;
}

int st_insert(st *st_ptr, t_key key, elem_data * data) {
    if (st_ptr == NULL || key == NULL) {
        return ERR_INTERNAL;
    }
    st_elem *tmp = st_search(st_ptr, key);
    if (tmp) {
        tmp->data = data;
    } else { //item with id doesnt exist

        tmp = malloc(sizeof(struct st_elem));
        if (tmp == NULL) {
            return ERR_INTERNAL;
        }
        tmp->key = malloc(strlen(key) + 1);
        if (tmp->key == NULL) {
            return ERR_INTERNAL;
        }
        strcpy(tmp->key, key);
        tmp->data = data;
        tmp->ptrnext = NULL;
        int index = hash_code(key);
        if ((*st_ptr)[index]) { //insert before first item
            tmp->ptrnext = (*st_ptr)[index];
        }
        (*st_ptr)[index] = tmp;
    }
    return 0;
}

elem_data *st_read(st *st_ptr, t_key key) {
    if (st_ptr == NULL || key == NULL) {
        return NULL;
    }
    st_elem *tmp = st_search(st_ptr, key);
    if (tmp) {
        return tmp->data;
    }
    return NULL;
}

void st_delete(st *st_ptr, t_key key) {
    if (st_ptr == NULL || key == NULL) {
        return;
    }
    int index = hash_code(key);
    st_elem *tmp = (*st_ptr)[index], *prev = NULL;

    while (tmp) {
        if (strcmp(tmp->key, key) == 0) {
            if (prev) { //item to be deleted is not first in list
                prev->ptrnext = tmp->ptrnext;
            } else { // -||- is first -||-
                (*st_ptr)[index] = tmp->ptrnext;
            }
            free(tmp->key);
            free(tmp);
            return;
        }
        prev = tmp;
        tmp = tmp->ptrnext;
    }
}

void st_clear_all(st *st_ptr) {
    if (st_ptr == NULL) {
        return;
    }
    st_elem *tmp, *next;
    for (int i = 0; i < SYMTABLE_SIZE; ++i) {
        next = (*st_ptr)[i];
        while (next) {
            tmp = next;
            next = tmp->ptrnext;
            free(tmp->key);
            free(tmp);
        }
        (*st_ptr)[i] = NULL;
    }
}
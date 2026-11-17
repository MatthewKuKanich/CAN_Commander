#pragma once

#include <stddef.h>
#include <string.h>

static inline size_t cc_strnlen(const char* str, size_t max_len) {
    size_t len = 0U;
    while(len < max_len && str[len] != '\0') {
        len++;
    }
    return len;
}

static inline char* cc_strtok_r(char* str, const char* delimiters, char** saveptr) {
    if(str == NULL) {
        str = *saveptr;
    }
    if(str == NULL) {
        return NULL;
    }

    while(*str != '\0' && strchr(delimiters, *str) != NULL) {
        str++;
    }
    if(*str == '\0') {
        *saveptr = NULL;
        return NULL;
    }

    char* token = str;
    while(*str != '\0' && strchr(delimiters, *str) == NULL) {
        str++;
    }
    if(*str != '\0') {
        *str = '\0';
        *saveptr = str + 1;
    } else {
        *saveptr = NULL;
    }

    return token;
}

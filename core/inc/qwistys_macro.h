#ifndef __QWISTYS_MACROS_H
#define __QWISTYS_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define __NOP (void) 0
#define QWISTYS_ZERO 0

#define QWISTYS_NRM "\x1B[0m"
#define QWISTYS_RED "\x1B[31m"
#define QWISTYS_GRN "\x1B[32m"
#define QWISTYS_YLW "\x1B[33m"
#define QWISTYS_BLU "\x1B[34m"
#define QWISTYS_MAG "\x1B[35m"
#define QWISTYS_CYN "\x1B[36m"
#define QWISTYS_WHT "\x1B[37m"

#define QWISTYS_MACRONAME "[QWISTYS MACRO]"
#define QWISTYS_TAG_TODO "\x1B[36mTODO: \x1B[33m"
#define VALENTINA_TAG_TODO "\x1B[36mVAENTINA_TODO: \x1B[33m"
#define QWISTYS_TAG_IMPLEMENTED "NOT IMPLEMENTED "
#define QWISTYS_TAG_HALT "\x1B[31mHALT: \x1B[34m"
#define QWISTYS_TAG_SE "SET EQUAL"
#define QWISTYS_TAG_NULL "NULL VALUE"

#define QWISTYS_MSG(tag, msg)                                                                                    \
    fprintf(stderr, "%s%s %s%s:%s:%d %s%s\n%s", QWISTYS_BLU, QWISTYS_MACRONAME, QWISTYS_MAG, __FILE__, __func__, \
            __LINE__, tag, msg, QWISTYS_NRM);

#define QWISTYS_HALT(tag)                   \
    {                                       \
        QWISTYS_MSG(QWISTYS_TAG_HALT, tag); \
        exit(0);                            \
    }

#define QWISTYS_ASSERT(x) \
    if (x == NULL)        \
    QWISTYS_HALT(QWISTYS_TAG_NULL)

#define QWISTYS_DEFER(x) \
    do {                 \
        result = (x);    \
        goto defer;      \
    } while (0)

#define QWISTYS_MALLOC_NEW(call, c) \
    call;                           \
    c++;
#define QWISTYS_MIN(a, b) (((a) < (b)) ? (a) : (b))

#define QWISTYS_TODO_ENABLE
#ifdef QWISTYS_TODO_ENABLE
#    ifdef QWISTYS_TODO_FORCE
#        define QWISTYS_TODO_MSG(msg) QWISTYS_HALT(msg)
#    else
#        define QWISTYS_TODO_MSG(msg) QWISTYS_MSG(QWISTYS_TAG_TODO, msg)
#    endif /* QWISTYS_TODO_FORCE */
#else
#    define QWISTYS_TODO_MSG(msg)
#endif /*QWISTYS_TODO*/

#define QWISTYS_UNIMPLEMENTED()                     \
    do {                                            \
        QWISTYS_MSG(QWISTYS_TAG_IMPLEMENTED, "YET") \
        QWISTYS_HALT(QWISTYS_TAG_IMPLEMENTED)       \
    } while (0)

#define QWISTYS_UNREACH_VIM()              \
    do {                                   \
        char cmd[500];                     \
        char line[5];                      \
        snprintf(line, 5, "%d", __LINE__); \
        strcpy(cmd, "vim +");              \
        strcat(cmd, line);                 \
        strcat(cmd, " ");                  \
        strcat(cmd, __FILE__);             \
        system(cmd);                       \
        exit(1);                           \
    } while (0)

#define QWISTYS_SET_EQ(a, b)             \
    do {                                 \
        a = b;                           \
        if (a == NULL)                   \
            QWISTYS_HALT(QWISTYS_TAG_SE) \
    } while (0)

#define QWISTYS_ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))
#define QWISTYS_ARRAY_ACCESS(arraym, index) (assert(index >= 0), assert(index < QWISTYS_ARRAY_LEN(array)), array[index])

#ifdef __cplusplus
}
#endif

#endif /* __QWISTYS_MACROS_H */
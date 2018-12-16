#ifndef STUTTER_H
#define STUTTER_H


#include <stdint.h>


/* embedded strings */
volatile char author[] = "Author: Kyle Kloberdanz";
volatile char license[] = "License: GNU GPLv3";


/* typdefs */
typedef int64_t number;
typedef double real;


/* enums */
typedef enum {
    NUMBER,
    REAL,
    BOOL
} StutterType;


typedef enum {
    CONDITIONAL,
    OPERATOR,
    LEAF
} ASTkind;


typedef enum {
    NOOP,
    ADD,
    SUB,
    MUL,
    DIV
} Operator;


/* structs */
typedef struct StutterObject {
    StutterType type;
    union {
        number number_value;
        real real_value;
        bool bool_value;
    } value;
} StutterObject;


typedef struct ASTNode {
    ASTkind kind;
    StutterObject *obj;
    Operator op;
    struct ASTNode *left;
    struct ASTNode *condition;
    struct ASTNode *right;
} ASTNode;


/* constructors */
StutterObject *make_number_obj(number);

ASTNode *make_ast_node(ASTkind, /* base constructor */
                       StutterObject *,
                       Operator,
                       ASTNode *,
                       ASTNode *,
                       ASTNode *);

ASTNode *make_leaf_node(StutterObject *); /* just holds stutter object */

ASTNode *make_operator_node(Operator,  /* holds operator and child items */
                            ASTNode *, /* to operate on */
                            ASTNode *);
                                           


/* destructors */
void destroy_obj(StutterObject *);
void destroy_ast_node(ASTNode *);


#endif /* STUTTER_H */
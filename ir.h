#ifndef IR_H
#define IR_H


typedef enum ir_kind {
    IR_OP,
    IR_NUMBER,
} ir_kind;


typedef struct Ir {
    ir_kind kind;
    char *repr;
} Ir;


#endif /* IR_H */

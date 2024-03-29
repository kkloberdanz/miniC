/*
 * Author: Kyle Kloberdanz
 * Project Start Date: 27 Nov 2018
 * License: GNU GPLv3 (see LICENSE.txt)
 *     This file is part of minic.
 *
 *     minic is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     minic is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with minic.  If not, see <https://www.gnu.org/licenses/>.
 * File: minic.c
 */


#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


#include "minic.h"
#include "linkedlist.h"
#include "ir.h"
#include "instructions.h"
#include "bst.h"
#include "util.h"


#define MAX(A, B) ((A) > (B) ? (A) : (B))


char token_string[MAX_TOKEN_SIZE+1];
int LARGEST_LABEL = 0;
int VAR_INDEX = 0;
struct BST *id_map = NULL;

/* constructors */
MinicObject *make_number_obj(char *n) {
    MinicObject *obj;
    int len_n;
    if ((obj = (MinicObject *)malloc(sizeof(MinicObject))) == NULL) {
        fprintf(stderr, "failed to allocate memory");
        return NULL;
    }
    obj->type = NUMBER_TYPE;

    len_n = strlen(n);
    obj->value.number_value = (char *)calloc(len_n + 1, sizeof(char));
    memcpy(obj->value.number_value, n, len_n);
    return obj;
}


MinicObject *make_string_obj(char *str) {
    MinicObject *obj;
    if ((obj = (MinicObject *)malloc(sizeof(MinicObject))) == NULL) {
        fprintf(stderr, "failed to allocate memory");
        return NULL;
    }
    obj->type = STRING_TYPE;
    obj->value.string_value = make_str(str);
    return obj;
}


MinicObject *make_id_obj(char *symb) {
    MinicObject *obj;
    if ((obj = (MinicObject *)malloc(sizeof(MinicObject))) == NULL) {
        fprintf(stderr, "failed to allocate memory");
        return NULL;
    }
    obj->type = VOID_TYPE;
    obj->value.symbol = make_str(symb);
    return obj;
}


/* TODO: track line number and column number */
ASTNode *make_ast_node(ASTkind kind,
                       MinicObject *obj,
                       Operator op,
                       ASTNode *left,
                       ASTNode *condition,
                       ASTNode *right) {

    ASTNode *node = minic_malloc(sizeof(ASTNode));

    node->kind = kind;
    node->sibling = NULL;
    node->obj = obj;
    node->op = op;
    node->left = left;
    node->condition = condition;
    node->right = right;
    return node;
}


ASTNode *make_leaf_node(MinicObject *obj) {
    ASTNode *node = make_ast_node(LEAF, obj, OP_NIL, NULL, NULL, NULL);
    return node;
}


ASTNode *make_operator_node(Operator op, ASTNode *left, ASTNode *right) {
    ASTNode *node = make_ast_node(OPERATOR, NULL, op, left, NULL, right);
    return node;
}


ASTNode *make_conditional_node(ASTNode *condition,
                               ASTNode *left,
                               ASTNode *right) {
    ASTNode *node = make_ast_node(CONDITIONAL,
                                  NULL,
                                  OP_NIL,
                                  left,
                                  condition,
                                  right);
    return node;
}


ASTNode *make_assign_node(ASTNode *leaf_obj, ASTNode *right) {
    MinicObject *obj = leaf_obj->obj;
    ASTNode *node = make_ast_node(ASSIGN_EXPR, obj, OP_NIL, NULL, NULL, right);
    return node;
}


ASTNode *make_declare_node(ASTNode *leaf_obj) {
    MinicObject *obj = leaf_obj->obj;
    ASTNode *node = make_ast_node(DECLARE_STMT, obj, OP_NIL, NULL, NULL, NULL);
    return node;
}


ASTNode *make_load_node(ASTNode *leaf_obj) {
    MinicObject *obj = leaf_obj->obj;
    ASTNode *node = make_ast_node(LOAD_STMT, obj, OP_NIL, NULL, NULL, NULL);
    return node;
}


ASTNode *make_function_node(ASTNode *leaf_obj, ASTNode *right) {
    MinicObject *obj = leaf_obj->obj;
    ASTNode *node = make_ast_node(FUNC_DEF, obj, OP_NIL, NULL, NULL, right);
    return node;
}


ASTNode *make_func_call_node(ASTNode *leaf_obj, ASTNode *args) {
    MinicObject *obj = leaf_obj->obj;
    ASTNode *node = make_ast_node(FUNC_CALL, obj, OP_NIL, NULL, NULL, args);
    return node;
}


/* destructors */
void destroy_obj(MinicObject *obj) {
    free(obj->value.number_value);
    free(obj);
}


void destroy_ast_node(ASTNode *node) {
    if (node) {
        if (node->obj) {
            destroy_obj(node->obj);
            node->obj = NULL;
        }

        /* recursive call */
        if (node->condition) {
            destroy_ast_node(node->condition);
            node->condition = NULL;
        }

        /* recursive call */
        if (node->left) {
            destroy_ast_node(node->left);
            node->left = NULL;
        }

        /* recursive call */
        if (node->right) {
            destroy_ast_node(node->right);
            node->right = NULL;
        }

        if (node->sibling) {
            destroy_ast_node(node->sibling);
            node->sibling = NULL;
        }

        free(node);
        node = NULL;
    }
}


static Ir *get_op_ir(Operator op) {
    Ir *ir = (Ir *)malloc(sizeof(Ir));
    if (ir == NULL) {
        fprintf(stderr, "%s\n", "failed to allocate Ir object");
        exit(EXIT_FAILURE);
    }
    ir->kind = IR_OP;
    switch (op) {
        case OP_NIL:
            ir->repr = "\tNOP";
            ir->value.op = NOP;
            break;

        case OP_PLUS:
            ir->repr = "\tADD";
            ir->value.op = ADD;
            break;

        case OP_MINUS:
            ir->repr = "\tSUB";
            ir->value.op = SUB;
            break;

        case OP_TIMES:
            ir->repr = "\tMUL";
            ir->value.op = MUL;
            break;

        case OP_DIVIDE:
            ir->repr = "\tDIV";
            ir->value.op = DIV;
            break;

        case OP_GE:
            ir->repr = "\tGE";
            ir->value.op = GE;
            break;

        case OP_GT:
            ir->repr = "\tGT";
            ir->value.op = GT;
            break;

        case OP_EQ:
            ir->repr = "\tEQ";
            ir->value.op = EQ;
            break;

        case OP_NE:
            ir->repr = "\tNE";
            ir->value.op = NE;
            break;

        case OP_LT:
            ir->repr = "\tLT";
            ir->value.op = LT;
            break;

        case OP_LE:
            ir->repr = "\tLE";
            ir->value.op = LE;
            break;

        case OP_NOT:
            ir->repr = "\tNOT";
            ir->value.op = NOT;
            break;
    }
    return ir;
}


char *get_op_val(char *str, MinicObject *obj) {
    switch (obj->type) {
        case NUMBER_TYPE:
            sprintf(str, "%s", obj->value.number_value);
            break;

        default:
            fprintf(stderr, "unhandled case: %d\n", obj->type);
            return NULL;
    }
    return str;
}


static Ir *get_ir_node(ASTNode *ast) {
    Ir *ir_node = NULL;
    switch (ast->kind) {
        case LEAF:
        {
            char *value = ast->obj->value.number_value;
            ir_node = (Ir *)malloc(sizeof(Ir));
            ir_node->kind = IR_NUMBER;
            ir_node->value.number = value;
            ir_node->repr = value;
            break;
        }

        default:
            fprintf(stderr, "incorrect ast kind: %d\n", ast->kind);
            exit(EXIT_FAILURE);
    }
    if (ir_node == NULL) {
        fprintf(stderr, "%s\n", "failed to initialize ir_node");
        exit(EXIT_FAILURE);
    }
    return ir_node;
}


/* code generation */
static linkedlist *rec_codegen_stack_machine(ASTNode *ast,
                                             int current_label) {
    linkedlist *program = NULL;
    linkedlist *cursor = NULL;
    if (ast == NULL) {
        return NULL;
    }
    switch (ast->kind) {
        case CONDITIONAL:
        {
            /*
             * evaluate condition
             * if condition == 1
             *     then do left sub-tree
             * else
             *     then do right sub-tree
             *
             * if (1 > 0) {
             *     putchar('y');
             * } else {
             *     putchar('n');
             * }
             *
             * PUSH 1         ; (1 > 0)
             * PUSH 0
             * GT             ; 1 if true, 0 if false
             *
             * JZ _else       ; jump if 0 (i.e. if false, goto else block)
             * _if:           ; if block
             *     PUSH 'y'
             *     PRINTC
             *     J _end_if  ; break out of if (skip over the else block)
             *
             * _else:
             *     PUSH 'n'
             *     PRINTC
             * _end_if:       ; continue with program
             * ...
             */
            char else_label[255];
            char target_else_label[255];
            char if_label[255];
            char end_if_label[255];
            char target_end_if[255];
            sprintf(else_label, "_else_%d", current_label);
            sprintf(target_else_label, "_else_%d:", current_label);
            sprintf(if_label, "_if_%d:", current_label);
            sprintf(end_if_label, "_end_if_%d", current_label);
            sprintf(target_end_if, "_end_if_%d:", current_label);

            /* eval condition */
            program = rec_codegen_stack_machine(ast->condition,
                                                current_label + 1);
            cursor = program;

            if (ast->right != NULL) {
                /* append jump to else if 0 */
                cursor = ll_append(cursor, ir_new_jump_inst(JZ, else_label));
            } else {
                /* if no else, then jump to endif if false */
                cursor = ll_append(cursor, ir_new_jump_inst(JZ, end_if_label));
            }

            /* append if label (not needed but helps for clarity in ASM) */
            cursor = ll_append(cursor, ir_new_label(if_label));

            /* concat eval left */
            ll_concat(cursor, rec_codegen_stack_machine(ast->left,
                                                        current_label + 1));

            /* if there is no else */
            if (ast->right != NULL) {
                /* append jump to end if */
                cursor = ll_append(cursor, ir_new_jump_inst(J, end_if_label));

                /* append else label */
                cursor = ll_append(cursor, ir_new_label(target_else_label));

                /* concat eval right */
                ll_concat(cursor,
                          rec_codegen_stack_machine(ast->right,
                                                    current_label + 1));
            }

            /* append end if label */
            cursor = ll_append(cursor, ir_new_label(target_end_if));

            current_label++;
            break;
        }

        case OPERATOR:
            program = rec_codegen_stack_machine(ast->right,
                                                current_label);
            ll_concat(program, rec_codegen_stack_machine(ast->left,
                                                         current_label));
            ll_append(program, get_op_ir(ast->op));
            break;

        case LEAF:
        {
            Ir *ir = (Ir *)malloc(sizeof(Ir));
            ir->repr = "\tPUSH";
            ir->kind = IR_OP;
            ir->value.op = PUSH;
            program = ll_new(ir);
            ll_append(program, get_ir_node(ast));
            break;
        }

        case DECLARE_STMT:
        {
            char *id = ast->obj->value.symbol;
            int location = VAR_INDEX++;

            id_map = bst_insert(id_map, id, location);
            program = rec_codegen_stack_machine(ast->right, current_label);
            break;
        }

        case ASSIGN_EXPR:
        {
            /*
             * lookup var's location in storage
             * execute ast->right
             * save to var's location
             */
            char *id = ast->obj->value.symbol;
            int location = -1;
            struct BST *location_node = bst_find(id_map, id);
            if (location_node == NULL) {
                fprintf(stderr, "identifier: '%s' has not been declared\n", id);
                exit(EXIT_FAILURE);
            }
            location = location_node->value;
            program = rec_codegen_stack_machine(ast->right,
                                                current_label);
            ll_append(program, ir_new_push_immediate(location));
            ll_append(program, ir_new_save());
            break;
        }

        case LOAD_STMT:
        {
            char *id = ast->obj->value.symbol;
            int location = -1;
            struct BST *location_node = bst_find(id_map, id);
            if (location_node == NULL) {
                fprintf(stderr, "identifier: '%s' has not been declared\n", id);
                exit(EXIT_FAILURE);
            }
            location = location_node->value;
            program = ll_new(ir_new_push_immediate(location));
            ll_append(program, ir_new_load());
            break;
        }

        case FUNC_DEF:
        {
            /*
             * put function label
             * put funciton body
             * put ret
             */
            char *id = ast->obj->value.symbol;
            int location = VAR_INDEX++;
            ASTNode *func_body = ast->right;
            char func_label[255];
            linkedlist *func_code = NULL;
            ASTNode *cursor;

            sprintf(func_label, "%s:", id);
            id_map = bst_insert(id_map, id, location);
            program = ll_new(ir_new_label(func_label));

            for (cursor = func_body; cursor != NULL; cursor = cursor->sibling) {
                func_code = rec_codegen_stack_machine(cursor, current_label);
                ll_concat(program, func_code);
            }
            ll_append(program, ir_new_ret());
            break;
        }

        case FUNC_CALL:
        {
            /*
             *
             */
            break;
        }
    }
    LARGEST_LABEL = MAX(LARGEST_LABEL, current_label);
    return program;
}


static linkedlist *codegen_stack_machine(ASTNode *ast) {
    linkedlist *program = NULL;
    linkedlist *cursor = NULL;
    for (;ast != NULL; ast = ast->sibling) {
        if (program == NULL) {
            program = rec_codegen_stack_machine(ast, LARGEST_LABEL);
            cursor = program;
        } else {
            cursor = rec_codegen_stack_machine(ast, LARGEST_LABEL);
            cursor = ll_concat(program, cursor);
        }
    }
    return program;
}


int emit(FILE *output, ASTNode *ast) {
    linkedlist *program = codegen_stack_machine(ast);
    program = ir_halt_program(program);
    ir_print_program(output, program);
    /*ir_free_list(program);*/
    return 0;
}

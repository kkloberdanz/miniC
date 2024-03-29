#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "ir.h"
#include "linkedlist.h"
#include "instructions.h"
#include "util.h"


void ir_print_program(FILE *output, const linkedlist *program) {
    while (program) {
        Ir *ir = (Ir *)program->value;
        if (ir->value.op == PUSH) {
            fprintf(output, "%s ", ir->repr);
        } else {
            fprintf(output, "%s\n", ir->repr);
        }
        program = program->next;
    }
}


struct Ir *ir_call_main() {
    Ir *ir = (Ir *)minic_malloc(sizeof(Ir));
    ir->kind = IR_CALL;
    ir->repr = make_str("\tCALL main");
    ir->value.op = CALL;
    return ir;
}


linkedlist *ir_halt_program(linkedlist* program) {
    Ir *ir = (Ir *)minic_malloc(sizeof(Ir));
    ir->kind = IR_END;
    ir->repr = make_str("\tHALT");
    ir->value.op = HALT;
    ll_append(program, ir);
    return program;
}


Ir *ir_new_label(const char *label) {
    Ir *ir = minic_malloc(sizeof(struct Ir));
    ir->kind = IR_LABEL;
    ir->repr = make_str(label);
    ir->value.number = NULL;
    return ir;
}


struct Ir *ir_new_jump_inst(inst_t instruction, const char *label) {
    char *tmp_str;
    const char *inst_name;
    struct Ir *ir = minic_malloc(sizeof(struct Ir));
    inst_name = inst_names[instruction];

    /* allocate enough memory for \0 and a space between the two strings */
    tmp_str = minic_malloc(strlen(inst_name) + strlen(label) + 2);

    sprintf(tmp_str, "\t%s %s", inst_name, label);
    ir->kind = IR_JMP;
    ir->repr = tmp_str;
    ir->value.number = NULL;
    return ir;
}


struct Ir *ir_new_save() {
    struct Ir *ir = minic_malloc(sizeof(struct Ir));
    ir->kind = IR_SAVE;
    ir->repr = make_str("\tSAVE");
    ir->value.number = NULL;
    return ir;
}


struct Ir *ir_new_load() {
    struct Ir *ir = minic_malloc(sizeof(struct Ir));
    ir->kind = IR_LOAD;
    ir->repr = make_str("\tLOAD");
    ir->value.number = NULL;
    return ir;
}


struct Ir *ir_new_push_immediate(int immediate) {
    struct Ir *ir = minic_malloc(sizeof(struct Ir));
    char *repr = calloc(255, sizeof(char));
    sprintf(repr, "\tPUSH %d", immediate);
    ir->kind = IR_PUSH;
    ir->repr = repr;
    ir->value.number = NULL;
    return ir;
}


struct Ir *ir_new_ret() {
    struct Ir *ir = minic_malloc(sizeof(struct Ir));
    ir->kind = IR_RET;
    ir->repr = make_str("\tRET");
    ir->value.number = NULL;
    return ir;
}


void ir_free_list(linkedlist *ll) {
    linkedlist *free_me = NULL;
    while (ll) {
        Ir *ir = ll->value;
        switch (ir->kind) {
            case IR_JMP:
            case IR_END:
            case IR_LABEL:
            case IR_SAVE:
            case IR_LOAD:
            case IR_PUSH:
            case IR_RET:
                free(ir->repr);
                ir->repr = NULL;
                break;
            default:
                break;
        }
        free_me = ll;
        ll = ll->next;
        free(free_me);
        free(ir);
    }
}
